/*
WIIU_THREAD_LOCALS.C

Each thread's block of the platform layer's per-thread variables
(wiiu_thread_locals.h), made the first time the thread asks for it and
freed when it ends.
*/

#include "wiiu_thread_locals.h"

#include <pthread.h>
#include <stdlib.h>

static pthread_key_t locals_key;
static pthread_once_t locals_once = PTHREAD_ONCE_INIT;

static void locals_key_create(void)
{
	pthread_key_create(&locals_key, free);
}

struct platform_thread_locals *platform_thread_locals(void)
{
	/* (for a thread whose block cannot be made) */
	static struct platform_thread_locals fallback;
	struct platform_thread_locals *locals;

	pthread_once(&locals_once, locals_key_create);
	locals = pthread_getspecific(locals_key);
	if (!locals)
	{
		locals = calloc(1, sizeof(*locals));
		if (!locals || pthread_setspecific(locals_key, locals) != 0)
		{
			free(locals);
			return &fallback;
		}
	}
	return locals;
}
