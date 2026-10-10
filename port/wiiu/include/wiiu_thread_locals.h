/*
WIIU_THREAD_LOCALS.H

A Wii U program has no thread-local storage (__thread variables: its
loader cannot place them), so the platform layer's per-thread variables
are kept in a block for each thread instead (wiiu_thread_locals.c), and
platform.h's users reach them through this.
*/

#ifndef __HALO_WIIU_THREAD_LOCALS_H
#define __HALO_WIIU_THREAD_LOCALS_H

struct platform_thread_locals
{
	/* xbox_kernel.c: GetLastError's, and the thread's queued APCs */
	unsigned long last_error;
	void *apc_head;
	void *apc_tail;
	/* msvc_wide.c: where wcstok is up to */
	void *wcstok_next;
};

/* the calling thread's (zeroed when first asked for) */
struct platform_thread_locals *platform_thread_locals(void);

#endif
