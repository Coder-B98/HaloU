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
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>
#include <whb/proc.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DATA_ROOT "fs:/vol/external01/halou"
#define LOG_PATH DATA_ROOT "/halou.log"
#define GAME_STACK_SIZE (8 * 1024 * 1024)

int halo_xbox_main(void);

static void *game_thread(void *result)
{
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
	WHBProcInit();
	WHBLogCafeInit();
	WHBLogUdpInit();

	/* the log on the SD card, each line written as it is said, so a crash
	leaves it whole */
	if (freopen(LOG_PATH, "w", stderr))
		setvbuf(stderr, NULL, _IONBF, 0);
	else
		WHBLogPrint("Halo U: cannot write " LOG_PATH);
	fprintf(stderr, "Halo U: starting, data in " DATA_ROOT "\n");
	WHBLogPrint("Halo U: starting, data in " DATA_ROOT);

	setenv("HALO_DATA_ROOT", DATA_ROOT, 1);
	if (chdir(DATA_ROOT) != 0)
		fprintf(stderr, "Halo U: there is no " DATA_ROOT " folder on the SD card\n");

	pthread_attr_init(&attributes);
	pthread_attr_setstacksize(&attributes, GAME_STACK_SIZE);
	if (pthread_create(&thread, &attributes, game_thread, &result) == 0)
		pthread_join(thread, NULL);
	else
		fprintf(stderr, "Halo U: cannot start the game's thread\n");
	pthread_attr_destroy(&attributes);

	fprintf(stderr, "Halo U: the game returned %d\n", result);
	WHBLogUdpDeinit();
	WHBLogCafeDeinit();
	WHBProcShutdown();
	return result;
}
