/*
WIIU_MAIN.C

Where the Wii U starts the game. The Wii U's own start-up comes first (the
HOME button and the foreground, WHBProcInit), then the game finds its
files on the SD card, in sd:/halou (HALO_DATA_ROOT: the maps in
sd:/halou/maps, config.toml beside them), and writes its log, everything
platform_log says, to sd:/halou/halou.log. Then the Xbox game's own main
(source/shell/shell_xbox.c, renamed halo_xbox_main by tools/wiiu_build.py)
runs, as the Xbox ran it, on a thread with as much stack as the desktop
builds' main thread has (a Wii U program's own is small).

(No platform.h here: wut's headers and the XDK's both declare BOOL.)
*/

#include <coreinit/debug.h>
#include <coreinit/exception.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "halou_build_id.h"

#define DATA_ROOT "fs:/vol/external01/halou"
#define LOG_PATH DATA_ROOT "/halou.log"
#define LOG_KEEP 5
#define GAME_STACK_SIZE (8 * 1024 * 1024)

int halo_xbox_main(void);

/* the C++ start-up (libgcc's eabi.S): the shader compiler's global objects
built, through __init and the constructor list. gcc puts a call to it at
the top of every main for the PowerPC EABI, and wut counts on that; clang,
which builds this file, does not, so main calls it itself. Without it the
compiler starts but crashes in its first compile, on objects never built
(T-002, runs r05 to r07). */
void __eabi(void);

/* ---------- the time in the log

Each line of the log starts with the seconds since the program started
([   12.345]), so a log alone says how long things took and how many frames
a second the game drew (xbox_kernel.c's platform_log asks for it). */

static OSTime start_time;

unsigned long wiiu_log_milliseconds(void)
{
	return (unsigned long)OSTicksToMilliseconds(OSGetSystemTime() - start_time);
}

static void log_time(void)
{
	unsigned long milliseconds = wiiu_log_milliseconds();

	fprintf(stderr, "[%5lu.%03lu] ", milliseconds / 1000, milliseconds % 1000);
}

/* a line of the log, after the time */
static void log_line(const char *format, ...) __attribute__((format(printf, 1, 2)));
static void log_line(const char *format, ...)
{
	va_list arguments;

	log_time();
	va_start(arguments, format);
	vfprintf(stderr, format, arguments);
	va_end(arguments);
}

/* the console's SD card only records how long a file is when the file is
flushed or closed, so a console that hangs (and is switched off) leaves an
empty halou.log however much was written. This thread flushes the log every
two seconds, and every sixteen says the program is still alive, with where
the game's thread is: the frames shown, the bytes read from files, and the
game thread's code address with the return addresses of its callers (look
them up with powerpc-eabi-addr2line on this build's halou.elf, after taking
off the difference between "main at" and main's address there). */
int main(int argc, char **argv);
volatile unsigned int wiiu_swap_count;
volatile unsigned long long wiiu_bytes_read;
static OSThread *volatile game_os_thread;

static void log_where_the_game_is(void)
{
	OSThread *thread = game_os_thread;
	uint32_t pc = 0, lr = 0, callers[8] = { 0 };
	int count = 0, waited;

	if (thread)
	{
		/* stopped only while its registers are copied: nothing is logged
		while it is held, as it may hold the log's lock */
		OSSuspendThread(thread);
		for (waited = 0; waited < 50 && !OSIsThreadSuspended(thread); waited++)
			OSSleepTicks(OSMicrosecondsToTicks(200));
		if (OSIsThreadSuspended(thread))
		{
			uint32_t low = (uint32_t)thread->stackEnd, high = (uint32_t)thread->stackStart;
			uint32_t frame = thread->context.gpr[1];

			pc = thread->context.srr0;
			lr = thread->context.lr;
			if (low > high)
			{
				uint32_t swap = low;

				low = high;
				high = swap;
			}
			while (count < 8 && frame >= low && frame + 8 <= high && !(frame & 3))
			{
				uint32_t next = *(uint32_t *)frame;

				if (next <= frame || next + 8 > high)
					break;
				callers[count++] = *(uint32_t *)(next + 4);
				frame = next;
			}
		}
		OSResumeThread(thread);
	}
	log_line("Halo U: still running; %u frames shown, %llu MB read; game at %08x lr %08x;"
		" callers %08x %08x %08x %08x %08x %08x %08x %08x\n",
		wiiu_swap_count, wiiu_bytes_read >> 20, (unsigned)pc, (unsigned)lr,
		(unsigned)callers[0], (unsigned)callers[1], (unsigned)callers[2], (unsigned)callers[3],
		(unsigned)callers[4], (unsigned)callers[5], (unsigned)callers[6], (unsigned)callers[7]);
}

static void *log_sync_thread(void *unused)
{
	unsigned int ticks = 0;

	(void)unused;
	for (;;)
	{
		OSSleepTicks(OSMillisecondsToTicks(2000));
		if (++ticks % 8 == 0)
			log_where_the_game_is();
		fsync(fileno(stderr));
	}
	return NULL;
}

/* ---------- when something goes wrong: a last line in the log

A crash on the Wii U (or in Cemu) leaves nothing behind but the log, so the
ways a program can die each write where it was first. */

/* abort() (a failed check, or a C++ exception nobody caught), in place of
newlib's */
void abort(void)
{
	log_line("Halo U: abort() called from %p\n", __builtin_return_address(0));
	OSFatal("Halo U: abort() called. See sd:/halou/halou.log");
	for (;;)
		;
}

static void exit_said(void)
{
	log_line("Halo U: exit() called\n");
}

static BOOL crashed(const char *what, OSContext *context)
{
	log_line("Halo U: %s exception at %08x (called from %08x), address %08x\n", what,
		(unsigned)context->srr0, (unsigned)context->lr, (unsigned)context->dar);
	OSFatal("Halo U crashed. See sd:/halou/halou.log");
	return FALSE;
}

static BOOL crashed_dsi(OSContext *context) { return crashed("memory (DSI)", context); }
static BOOL crashed_isi(OSContext *context) { return crashed("code (ISI)", context); }
static BOOL crashed_program(OSContext *context) { return crashed("program", context); }

static void *game_thread(void *result)
{
	/* GX2, the graphics, belongs to the core it was started on: the game's
	thread stays on the main core (CPU 1), where the system expects it */
	OSSetThreadAffinity(OSGetCurrentThread(), OS_THREAD_ATTRIB_AFFINITY_CPU1);
	OSYieldThread();
	game_os_thread = OSGetCurrentThread();
	*(int *)result = halo_xbox_main();
	return NULL;
}

int main(int argc, char **argv)
{
	int result = EXIT_FAILURE;
	pthread_attr_t attributes;
	pthread_t thread;

	(void)argc;
	(void)argv;
	__eabi();
	start_time = OSGetSystemTime();
	WHBProcInit();
	WHBLogCafeInit();
	WHBLogUdpInit();

	/* the last runs' logs kept beside it, newest first (halou-1.log is the
	run before this one), so a launch that goes wrong is still there to read
	after the next */
	{
		char older[96], newer[96];
		int index;

		snprintf(older, sizeof(older), DATA_ROOT "/halou-%d.log", LOG_KEEP);
		remove(older);
		for (index = LOG_KEEP - 1; index >= 1; index--)
		{
			snprintf(newer, sizeof(newer), DATA_ROOT "/halou-%d.log", index);
			snprintf(older, sizeof(older), DATA_ROOT "/halou-%d.log", index + 1);
			rename(newer, older);
		}
		rename(LOG_PATH, DATA_ROOT "/halou-1.log");
	}
	/* the log on the SD card, each line written as it is said, so a crash
	leaves it whole */
	if (freopen(LOG_PATH, "w", stderr))
		setvbuf(stderr, NULL, _IONBF, 0);
	else
		WHBLogPrint("Halo U: cannot write " LOG_PATH);
	/* first, which build this is (the commit: tools/wiiu_build.py), so a log
	always says what made it */
	log_line("Halo U: build " HALOU_BUILD_ID "\n");
	log_line("Halo U: starting, data in " DATA_ROOT "\n");
	log_line("Halo U: main at %p\n", (void *)main);
	WHBLogPrint("Halo U: build " HALOU_BUILD_ID ", data in " DATA_ROOT);

	atexit(exit_said);
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_DSI, crashed_dsi);
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_ISI, crashed_isi);
	OSSetExceptionCallbackEx(OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OS_EXCEPTION_TYPE_PROGRAM, crashed_program);

	setenv("HALO_DATA_ROOT", DATA_ROOT, 1);
	if (chdir(DATA_ROOT) != 0)
		log_line("Halo U: there is no " DATA_ROOT " folder on the SD card\n");

	{
		pthread_t sync_thread;

		if (pthread_create(&sync_thread, NULL, log_sync_thread, NULL) == 0)
			pthread_detach(sync_thread);
	}

	pthread_attr_init(&attributes);
	pthread_attr_setstacksize(&attributes, GAME_STACK_SIZE);
	if (pthread_create(&thread, &attributes, game_thread, &result) == 0)
		pthread_join(thread, NULL);
	else
		log_line("Halo U: cannot start the game's thread\n");
	pthread_attr_destroy(&attributes);

	log_line("Halo U: the game returned %d\n", result);
	WHBLogUdpDeinit();
	WHBLogCafeDeinit();
	WHBProcShutdown();
	return result;
}
