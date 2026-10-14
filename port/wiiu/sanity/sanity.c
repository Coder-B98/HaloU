/*
SANITY.C

A Wii U test of the console itself, without the game: on the console the
game's GPU work is never done and its sound is never heard, while in Cemu
both work. This app does only what every Wii U homebrew app does, the
WHBGfx way: it fills both screens with a colour that changes every second
and plays a steady tone. If the console shows the colours and plays the
tone, the console and its plugins are fine and the fault is in the game's
own graphics and sound layer; if not, it is in the console's setup.

Once a second, sd:/halou/sanity.txt gets a line: frames drawn, frames
sent and shown, the GPU's submitted and done timestamps, and where the
sound hardware is playing in its buffer (it moves if the sound runs).
HOME, then Close, ends it.

Built with -DSANITY_ON_THREAD (halou_sanity_thread.wuhb), the graphics
and the tone start and run on a second thread made as the game makes its
own (8 MB of stack, kept on CPU 1), while the main thread only answers
the system's messages, as in the game: if this one stays dark and silent
where the plain one works, the thread is the fault.
*/

#include <coreinit/cache.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <gx2/event.h>
#include <gx2/swap.h>
#include <sndcore2/core.h>
#include <sndcore2/device.h>
#include <sndcore2/voice.h>
#include <whb/gfx.h>
#include <whb/proc.h>

#include <malloc.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef HALOU_BUILD_ID
#define HALOU_BUILD_ID "unknown"
#endif

#define LOG_PATH "fs:/vol/external01/halou/sanity.txt"
#define TONE_FRAMES 4800

static FILE *log_file;
static OSTime start_time;

static void log_line(const char *format, ...) __attribute__((format(printf, 1, 2)));
#include <stdarg.h>
static void log_line(const char *format, ...)
{
	va_list arguments;

	if (!log_file)
		return;
	fprintf(log_file, "[%8.3f] ", (double)OSTicksToMilliseconds(OSGetSystemTime() - start_time) / 1000.0);
	va_start(arguments, format);
	vfprintf(log_file, format, arguments);
	va_end(arguments);
	fputc('\n', log_file);
	fflush(log_file);
	fsync(fileno(log_file));
}

/* a 400 Hz tone, looping, on both screens' speakers */
static AXVoice *tone_start(int16_t *samples)
{
	AXInitParams parameters;
	AXVoiceOffsets offsets;
	AXVoiceVeData volume = { 0x4000, 0 };
	AXVoiceDeviceMixData mix[6];
	AXVoice *voice;
	int index;

	memset(&parameters, 0, sizeof(parameters));
	parameters.renderer = AX_INIT_RENDERER_48KHZ;
	parameters.pipeline = AX_INIT_PIPELINE_SINGLE;
	AXInitWithParams(&parameters);
	for (index = 0; index < TONE_FRAMES; index++)
		samples[index] = (int16_t)(12000.0 * sin(index * 2.0 * M_PI * 400.0 / 48000.0));
	DCFlushRange(samples, TONE_FRAMES * sizeof(int16_t));
	voice = AXAcquireVoice(31, NULL, NULL);
	if (!voice)
		return NULL;
	memset(&offsets, 0, sizeof(offsets));
	offsets.dataType = AX_VOICE_FORMAT_LPCM16;
	offsets.loopingEnabled = AX_VOICE_LOOP_ENABLED;
	offsets.endOffset = TONE_FRAMES - 1;
	offsets.data = samples;
	memset(mix, 0, sizeof(mix));
	mix[0].bus[0].volume = 0x8000;
	mix[1].bus[0].volume = 0x8000;
	AXVoiceBegin(voice);
	AXSetVoiceType(voice, 0);
	AXSetVoiceVe(voice, &volume);
	AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_TV, 0, mix);
	AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_DRC, 0, mix);
	AXSetVoiceOffsets(voice, &offsets);
	AXSetVoiceSrcType(voice, AX_VOICE_SRC_TYPE_NONE);
	AXSetVoiceSrcRatio(voice, 1.0f);
	AXSetVoiceState(voice, AX_VOICE_STATE_PLAYING);
	AXVoiceEnd(voice);
	return voice;
}

static volatile int running = 1;
static void *test_body(void *unused);

#ifdef SANITY_ON_THREAD
/* as the game's thread: kept on CPU 1 (wiiu_main.c) */
static void *test_thread(void *unused)
{
	OSSetThreadAffinity(OSGetCurrentThread(), OS_THREAD_ATTRIB_AFFINITY_CPU1);
	OSYieldThread();
	return test_body(unused);
}
#endif

static void *test_body(void *unused)
{
	static const float colours[4][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 1, 1, 1 } };
	int16_t *samples;
	AXVoice *voice;
	unsigned long frames = 0, second = 0;

	(void)unused;
	if (!WHBGfxInit())
	{
		log_line("WHBGfxInit failed");
		return NULL;
	}
	log_line("graphics on");
	samples = memalign(64, TONE_FRAMES * sizeof(int16_t));
	voice = samples ? tone_start(samples) : NULL;
	log_line(voice ? "tone on" : "no tone (no voice or memory)");
#ifdef SANITY_ON_THREAD
	while (running)
#else
	while (WHBProcIsRunning())
#endif
	{
		unsigned long now = (unsigned long)(OSTicksToMilliseconds(OSGetSystemTime() - start_time) / 1000);
		const float *colour = colours[now % 4];

		WHBGfxBeginRender();
		WHBGfxBeginRenderTV();
		WHBGfxClearColor(colour[0], colour[1], colour[2], 1.0f);
		WHBGfxFinishRenderTV();
		WHBGfxBeginRenderDRC();
		WHBGfxClearColor(colour[0], colour[1], colour[2], 1.0f);
		WHBGfxFinishRenderDRC();
		WHBGfxFinishRender();
		frames++;
		if (now != second)
		{
			uint32_t swaps, flips;
			OSTime last_flip, last_vsync;

			second = now;
			GX2GetSwapStatus(&swaps, &flips, &last_flip, &last_vsync);
			log_line("second %lu: %lu frames drawn, %u sent, %u shown; GPU submitted %llu, done %llu; sound at %lu",
				second, frames, swaps, flips, (unsigned long long)GX2GetLastSubmittedTimeStamp(),
				(unsigned long long)GX2GetRetiredTimeStamp(),
				voice ? (unsigned long)AXGetVoiceCurrentOffsetEx(voice, samples) : 0UL);
		}
	}
	if (voice)
		AXFreeVoice(voice);
	AXQuit();
	WHBGfxShutdown();
	return NULL;
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	start_time = OSGetSystemTime();
	WHBProcInit();
	log_file = fopen(LOG_PATH, "a");
#ifdef SANITY_ON_THREAD
	log_line("sanity test on a second thread, build %s: starting", HALOU_BUILD_ID);
	{
		pthread_attr_t attributes;
		pthread_t thread;

		pthread_attr_init(&attributes);
		pthread_attr_setstacksize(&attributes, 8 * 1024 * 1024);
		if (pthread_create(&thread, &attributes, test_thread, NULL) != 0)
			log_line("cannot start the test thread");
		else
		{
			while (WHBProcIsRunning())
				OSSleepTicks(OSMillisecondsToTicks(10));
			running = 0;
			pthread_join(thread, NULL);
		}
		pthread_attr_destroy(&attributes);
	}
#else
	log_line("sanity test, build %s: starting", HALOU_BUILD_ID);
	test_body(NULL);
#endif
	log_line("closing");
	if (log_file)
		fclose(log_file);
	WHBProcShutdown();
	return 0;
}
