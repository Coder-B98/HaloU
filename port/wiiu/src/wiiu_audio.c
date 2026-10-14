/*
WIIU_AUDIO.C

The sound output of the Wii U build: DirectSound's software mixer
(port/linux/src/dsound_sdl.c, which the Linux build plays through SDL)
played through AX, the Wii U's sound hardware, to the TV and the GamePad.

AX plays two voices, left and right, each looping over a ring of 16-bit
samples at 48 kHz. A thread on its own core (CPU 0, away from the game's)
keeps the rings filled a little ahead of where AX is playing: it asks the
mixer for a chunk, converts it, and flushes it from the cache so AX reads
what was written. The further ahead, the steadier the sound and the later
it is heard: AHEAD_FRAMES is about 43 ms.

The splash music: while the Wii U shows the launch splash (until the first
frame of the game reaches the screen), sd:/halou/splash.wav plays under
the game's own sound and fades out. The file is the player's own (it is
never part of the build); without it there is no music. A thread of its
own reads it from the card a little at a time, so the sound thread never
waits on the card.
*/

#include <coreinit/cache.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <sndcore2/core.h>
#include <sndcore2/device.h>
#include <sndcore2/voice.h>

#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define RATE 48000
#define RING_FRAMES 8192
#define CHUNK_FRAMES 256
#define AHEAD_FRAMES 2048

void platform_log(const char *format, ...) __attribute__((format(printf, 1, 2)));

typedef void (*wiiu_audio_mix_function)(float *output, unsigned long frames);

static wiiu_audio_mix_function mix;
static AXVoice *voices[2];
static int16_t *rings[2];

/* ---------- the splash music */

#define MUSIC_PATH "fs:/vol/external01/halou/splash.wav"
#define MUSIC_RING_FRAMES 65536
#define MUSIC_READ_FRAMES 8192
#define MUSIC_VOLUME 0.8f
/* (a second, at 48 kHz, to fade out) */
#define MUSIC_FADE_FRAMES 48000

/* frames shown on the screen so far (wiiu_gl.c): the splash is gone once
there is one */
extern volatile unsigned int wiiu_gpu_flips;

/* the file's frames, as left/right pairs in the CPU's byte order; the
reader adds at music_written, the sound thread takes from music_taken
(each only ever moves its own) */
static int16_t music_ring[MUSIC_RING_FRAMES * 2];
static volatile unsigned long music_written, music_taken;
static volatile int music_reading, music_on;
/* how far the file's frames advance for each 48 kHz frame */
static double music_step, music_position;
static int music_channels;
static long music_fade = -1;

static uint32_t little32(const unsigned char *bytes)
{
	return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | (uint32_t)bytes[3] << 24;
}

static void *music_reader(void *argument)
{
	FILE *file = argument;
	static unsigned char block[MUSIC_READ_FRAMES * 4];
	int channels = music_channels;
	size_t frame_bytes = (size_t)channels * 2;

	for (;;)
	{
		size_t got, frame;

		if (!music_on)
			break;
		if (music_written - music_taken > MUSIC_RING_FRAMES - MUSIC_READ_FRAMES)
		{
			OSSleepTicks(OSMillisecondsToTicks(10));
			continue;
		}
		got = fread(block, frame_bytes, MUSIC_READ_FRAMES, file);
		for (frame = 0; frame < got; frame++)
		{
			const unsigned char *bytes = block + frame * frame_bytes;
			int16_t left = (int16_t)(bytes[0] | bytes[1] << 8);
			int16_t right = channels > 1 ? (int16_t)(bytes[2] | bytes[3] << 8) : left;
			unsigned long slot = (music_written + frame) % MUSIC_RING_FRAMES;

			music_ring[slot * 2] = left;
			music_ring[slot * 2 + 1] = right;
		}
		__sync_synchronize();
		music_written += got;
		if (got < MUSIC_READ_FRAMES)
			break;
	}
	fclose(file);
	music_reading = 0;
	return NULL;
}

/* the music file opened and its reader started; 0 if there is none (or it
is not 16-bit PCM) */
static int music_start(void)
{
	FILE *file = fopen(MUSIC_PATH, "rb");
	unsigned char header[12], chunk[8], format[16];
	unsigned long rate = 0;
	int channels = 0, bits = 0, kind = 0;
	pthread_t thread;
	pthread_attr_t attributes;

	if (!file)
		return 0;
	if (fread(header, 1, 12, file) != 12 || memcmp(header, "RIFF", 4) || memcmp(header + 8, "WAVE", 4))
		goto unusable;
	for (;;)
	{
		uint32_t size;

		if (fread(chunk, 1, 8, file) != 8)
			goto unusable;
		size = little32(chunk + 4);
		if (!memcmp(chunk, "fmt ", 4) && size >= 16)
		{
			if (fread(format, 1, 16, file) != 16)
				goto unusable;
			kind = format[0] | format[1] << 8;
			channels = format[2] | format[3] << 8;
			rate = little32(format + 4);
			bits = format[14] | format[15] << 8;
			if (size > 16 && fseek(file, (long)(size - 16 + (size & 1)), SEEK_CUR))
				goto unusable;
		}
		else if (!memcmp(chunk, "data", 4))
			break;
		else if (fseek(file, (long)(size + (size & 1)), SEEK_CUR))
			goto unusable;
	}
	if (kind != 1 || bits != 16 || channels < 1 || channels > 2 || rate < 8000 || rate > 96000)
		goto unusable;
	music_channels = channels;
	music_step = (double)rate / RATE;
	music_on = music_reading = 1;
	pthread_attr_init(&attributes);
	pthread_attr_setstacksize(&attributes, 64 * 1024);
	if (pthread_create(&thread, &attributes, music_reader, file) != 0)
	{
		pthread_attr_destroy(&attributes);
		music_on = music_reading = 0;
		goto unusable;
	}
	pthread_attr_destroy(&attributes);
	pthread_detach(thread);
	platform_log("AX: splash music from %s: %d channel(s), %lu Hz", MUSIC_PATH, channels, rate);
	return 1;

unusable:
	platform_log("AX: %s is not 16-bit PCM sound in a WAV file: no splash music", MUSIC_PATH);
	fclose(file);
	return 0;
}

/* the music added to a chunk of the mixer's output, until it ends or has
faded out after the splash */
static void music_mix(float *output, unsigned long frames)
{
	unsigned long frame;

	if (!music_on)
		return;
	if (music_fade < 0 && wiiu_gpu_flips > 0)
	{
		music_fade = MUSIC_FADE_FRAMES;
		platform_log("AX: the first frame is on the screen: the splash music fades out");
	}
	__sync_synchronize();
	for (frame = 0; frame < frames; frame++)
	{
		unsigned long whole = (unsigned long)music_position, slot, next;
		float part = (float)(music_position - (double)whole), volume = MUSIC_VOLUME;

		/* (two of the file's frames are needed, to go between them) */
		if (music_taken + whole + 1 >= music_written)
		{
			if (!music_reading)
			{
				music_on = 0;
				platform_log("AX: the splash music has ended");
			}
			return;
		}
		if (music_fade >= 0)
		{
			if (music_fade == 0)
			{
				/* (the reader sees this and closes the file) */
				music_on = 0;
				return;
			}
			volume *= (float)music_fade / MUSIC_FADE_FRAMES;
			music_fade--;
		}
		slot = (music_taken + whole) % MUSIC_RING_FRAMES;
		next = (slot + 1) % MUSIC_RING_FRAMES;
		output[frame * 2] += volume / 32768.0f *
			(music_ring[slot * 2] + part * (music_ring[next * 2] - music_ring[slot * 2]));
		output[frame * 2 + 1] += volume / 32768.0f *
			(music_ring[slot * 2 + 1] + part * (music_ring[next * 2 + 1] - music_ring[slot * 2 + 1]));
		music_position += music_step;
		whole = (unsigned long)music_position;
		music_taken += whole;
		music_position -= (double)whole;
	}
}

static int16_t sample(float value)
{
	value *= 32767.0f;
	if (value > 32767.0f) value = 32767.0f;
	if (value < -32768.0f) value = -32768.0f;
	return (int16_t)value;
}

static void voice_start(int channel)
{
	AXVoiceOffsets offsets;
	AXVoiceVeData volume = { 0x8000, 0 };
	AXVoiceDeviceMixData mix_data[6];
	AXVoice *voice = voices[channel];

	memset(&offsets, 0, sizeof(offsets));
	offsets.dataType = AX_VOICE_FORMAT_LPCM16;
	offsets.loopingEnabled = AX_VOICE_LOOP_ENABLED;
	offsets.loopOffset = 0;
	offsets.endOffset = RING_FRAMES - 1;
	offsets.currentOffset = 0;
	offsets.data = rings[channel];

	AXVoiceBegin(voice);
	AXSetVoiceType(voice, 0);
	AXSetVoiceVe(voice, &volume);
	/* the TV's and the GamePad's main bus: channel 0 left, 1 right */
	memset(mix_data, 0, sizeof(mix_data));
	mix_data[channel].bus[0].volume = 0x8000;
	AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_TV, 0, mix_data);
	AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_DRC, 0, mix_data);
	AXSetVoiceOffsets(voice, &offsets);
	AXSetVoiceSrcType(voice, AX_VOICE_SRC_TYPE_NONE);
	AXSetVoiceSrcRatio(voice, 1.0f);
	AXVoiceEnd(voice);
}

static void *feeder(void *unused)
{
	float chunk[CHUNK_FRAMES * 2];
	unsigned long written = 0;

	(void)unused;
	OSSetThreadAffinity(OSGetCurrentThread(), OS_THREAD_ATTRIB_AFFINITY_CPU0);
	OSYieldThread();
	for (;;)
	{
		unsigned long playing = AXGetVoiceCurrentOffsetEx(voices[0], rings[0]) % RING_FRAMES;
		unsigned long ahead = (written + RING_FRAMES - playing) % RING_FRAMES;

		if (ahead < AHEAD_FRAMES)
		{
			unsigned long frame;

			mix(chunk, CHUNK_FRAMES);
			music_mix(chunk, CHUNK_FRAMES);
			for (frame = 0; frame < CHUNK_FRAMES; frame++)
			{
				rings[0][(written + frame) % RING_FRAMES] = sample(chunk[frame * 2]);
				rings[1][(written + frame) % RING_FRAMES] = sample(chunk[frame * 2 + 1]);
			}
			/* (RING_FRAMES is a multiple of CHUNK_FRAMES: a chunk never wraps) */
			DCFlushRange(rings[0] + written, CHUNK_FRAMES * sizeof(int16_t));
			DCFlushRange(rings[1] + written, CHUNK_FRAMES * sizeof(int16_t));
			written = (written + CHUNK_FRAMES) % RING_FRAMES;
		}
		else
			OSSleepTicks(OSMillisecondsToTicks(2));
	}
	return NULL;
}

/* sound from the mixer, through AX; 0 if it cannot start (the caller then
drains its voices some other way) */
int wiiu_audio_start(wiiu_audio_mix_function mixer)
{
	AXInitParams parameters;
	pthread_t thread;
	pthread_attr_t attributes;
	int channel;

	memset(&parameters, 0, sizeof(parameters));
	parameters.renderer = AX_INIT_RENDERER_48KHZ;
	parameters.pipeline = AX_INIT_PIPELINE_SINGLE;
	AXInitWithParams(&parameters);
	mix = mixer;
	for (channel = 0; channel < 2; channel++)
	{
		rings[channel] = memalign(64, RING_FRAMES * sizeof(int16_t));
		voices[channel] = AXAcquireVoice(31, NULL, NULL);
		if (!rings[channel] || !voices[channel])
		{
			platform_log("AX: cannot start the sound (no %s)", rings[channel] ? "voice" : "memory");
			return 0;
		}
		memset(rings[channel], 0, RING_FRAMES * sizeof(int16_t));
		DCFlushRange(rings[channel], RING_FRAMES * sizeof(int16_t));
		voice_start(channel);
	}

	pthread_attr_init(&attributes);
	pthread_attr_setstacksize(&attributes, 256 * 1024);
	if (pthread_create(&thread, &attributes, feeder, NULL) != 0)
	{
		pthread_attr_destroy(&attributes);
		platform_log("AX: cannot start the sound thread");
		return 0;
	}
	pthread_attr_destroy(&attributes);
	pthread_detach(thread);

	/* both voices at once, so left and right start on the same sample */
	AXVoiceBegin(voices[0]);
	AXVoiceBegin(voices[1]);
	AXSetVoiceState(voices[0], AX_VOICE_STATE_PLAYING);
	AXSetVoiceState(voices[1], AX_VOICE_STATE_PLAYING);
	AXVoiceEnd(voices[1]);
	AXVoiceEnd(voices[0]);
	platform_log("AX: sound on, %d Hz, %d ms ahead", RATE, AHEAD_FRAMES * 1000 / RATE);
	music_start();
	return 1;
}
