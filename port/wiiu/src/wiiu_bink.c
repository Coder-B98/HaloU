/*
WIIU_BINK.C

The Bink video SDK entry points bink_playback.c uses, for the Wii U build
(port/linux/src/bink_null.c, which plays nothing, is the Linux build's).
The movies are decoded by FFmpeg (wiiu_bink_decode.c). Their frames go into
the game's movie texture, and their sound into a DirectSound stream of the
port's mixer (dsound_sdl.c), which the Wii U plays (wiiu_audio.c).

Time is the Wii U's clock: frame n of a movie is due n frame lengths after
its first, and BinkWait says to wait until it is.
*/

#include "platform.h"
#include "wiiu_bink_decode.h"

#include <coreinit/time.h>
#include <stdlib.h>
#include <string.h>

typedef void *(__stdcall *rad_memory_allocate_proc)(unsigned long size);
typedef void (__stdcall *rad_memory_free_proc)(void *memory);
typedef void *(__stdcall *bink_sound_system_open_proc)(unsigned long param);

/* bink_playback.c's view of a movie comes first */
struct BINK
{
	unsigned long Width;
	unsigned long Height;
	unsigned long Frames;
	unsigned long FrameNum;
	unsigned long LastFrameNum;
};

struct wiiu_bink
{
	struct BINK bink;
	struct wiiu_bink_movie *movie;
	struct wiiu_bink_information information;
	LPDIRECTSOUNDSTREAM stream;
	OSTime started;
	int started_clock;
};

typedef struct BINK *HBINK;

static LPDIRECTSOUND bink_direct_sound;

void __stdcall RADSetMemory(rad_memory_allocate_proc allocate, rad_memory_free_proc release)
{
	/* (FFmpeg allocates from the heap itself) */
	(void)allocate;
	(void)release;
}

void *__stdcall BinkOpenDirectSound(unsigned long param)
{
	return (void *)param;
}

long __stdcall BinkSetSoundSystem(bink_sound_system_open_proc open, unsigned long param)
{
	bink_direct_sound = (LPDIRECTSOUND)open(param);
	return bink_direct_sound != NULL;
}

void __stdcall BinkSetIOSize(unsigned long io_size)
{
	(void)io_size;
}

static void __stdcall stream_sound(void *context, const void *samples, unsigned long size)
{
	struct wiiu_bink *record = context;
	XMEDIAPACKET packet;

	if (!record->stream)
		return;
	memset(&packet, 0, sizeof(packet));
	/* (the mixer copies the samples as they are given) */
	packet.pvBuffer = (void *)samples;
	packet.dwMaxSize = size;
	IDirectSoundStream_Process(record->stream, &packet, NULL);
}

static void open_sound(struct wiiu_bink *record)
{
	WAVEFORMATEX format;
	DSSTREAMDESC description;

	if (!bink_direct_sound || record->information.channels <= 0 || record->information.channels > 2)
		return;
	memset(&format, 0, sizeof(format));
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = (WORD)record->information.channels;
	format.nSamplesPerSec = (DWORD)record->information.sample_rate;
	format.wBitsPerSample = 16;
	format.nBlockAlign = (WORD)(format.nChannels * 2);
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	memset(&description, 0, sizeof(description));
	description.dwMaxAttachedPackets = 64;
	description.lpwfxFormat = &format;
	if (IDirectSound_CreateSoundStream(bink_direct_sound, &description, &record->stream, NULL) < 0)
		record->stream = NULL;
	else
		wiiu_bink_set_sound(record->movie, (wiiu_bink_sound_proc)stream_sound, record);
}

HBINK __stdcall BinkOpen(const char *name, unsigned long flags)
{
	struct wiiu_bink *record;
	char path[1024];

	(void)flags;
	if (!name)
		return NULL;
	record = calloc(1, sizeof(*record));
	if (!record)
		return NULL;
	platform_translate_path(name, path, sizeof(path));
	record->movie = wiiu_bink_open(path, &record->information);
	if (!record->movie)
	{
		platform_log("Bink: cannot open \"%s\" (%s)", name, path);
		free(record);
		return NULL;
	}
	record->bink.Width = record->information.width;
	record->bink.Height = record->information.height;
	record->bink.Frames = record->information.frames ? record->information.frames : 0x7fffffff;
	record->bink.FrameNum = 1;
	open_sound(record);
	platform_log("Bink: playing \"%s\", %lux%lu, %lu frames at %.2f a second, sound %d channels at %d Hz",
		name, record->bink.Width, record->bink.Height, record->information.frames,
		record->information.frames_per_second, record->information.channels, record->information.sample_rate);
	return &record->bink;
}

void __stdcall BinkClose(HBINK bink)
{
	struct wiiu_bink *record = (struct wiiu_bink *)bink;

	if (!record)
		return;
	if (record->stream)
	{
		IDirectSoundStream_Flush(record->stream);
		IDirectSoundStream_Release(record->stream);
	}
	wiiu_bink_close(record->movie);
	free(record);
}

long __stdcall BinkDoFrame(HBINK bink)
{
	struct wiiu_bink *record = (struct wiiu_bink *)bink;

	if (!record->started_clock)
	{
		record->started = OSGetSystemTime();
		record->started_clock = 1;
	}
	if (!wiiu_bink_decode_frame(record->movie))
	{
		/* the movie ended before its header said: it stops once BinkNextFrame
		has moved on from this frame (bink_playback.c stops a movie when
		FrameNum is Frames-1) */
		record->bink.Frames = record->bink.FrameNum + 2;
	}
	return 0;
}

void __stdcall BinkNextFrame(HBINK bink)
{
	bink->LastFrameNum = bink->FrameNum;
	bink->FrameNum++;
}

long __stdcall BinkWait(HBINK bink)
{
	struct wiiu_bink *record = (struct wiiu_bink *)bink;
	double due;
	double now;

	if (!record->started_clock)
		return 0;
	due = (double)(bink->FrameNum - 1) / record->information.frames_per_second;
	now = (double)OSTicksToMicroseconds(OSGetSystemTime() - record->started) / 1000000.0;
	return now < due;
}

long __stdcall BinkCopyToBuffer(HBINK bink, void *destination, long destination_pitch,
	unsigned long destination_height, unsigned long destination_x, unsigned long destination_y,
	unsigned long flags)
{
	struct wiiu_bink *record = (struct wiiu_bink *)bink;

	(void)flags;
	if (!destination)
		return 0;
	wiiu_bink_copy_frame(record->movie,
		(unsigned char *)destination + destination_y * destination_pitch + destination_x * 4,
		destination_pitch, bink->Width, destination_height);
	return 0;
}

/* (the game's movie statistics, shown only by a debug display) */
void __stdcall BinkGetSummary(HBINK bink, void *summary) { (void)bink; (void)summary; }
void __stdcall BinkGetRealtime(HBINK bink, void *realtime, unsigned long frame_count) { (void)bink; (void)realtime; (void)frame_count; }
