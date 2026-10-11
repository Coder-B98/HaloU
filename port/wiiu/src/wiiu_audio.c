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
*/

#include <coreinit/cache.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <sndcore2/core.h>
#include <sndcore2/device.h>
#include <sndcore2/voice.h>

#include <malloc.h>
#include <pthread.h>
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
	return 1;
}
