/*
WIIU_ALLOC.C

A line in the log for each block of memory the program asks for and does
not get (T-002). Running out of memory looks like any other crash on the
Wii U, and in Cemu like the emulator closing, so the log has to say it
first. The linker sends every call of malloc, calloc, realloc and memalign,
the game's and the shader compiler's alike, here first (-Wl,--wrap in
tools/wiiu_build.py); each goes on to newlib's own (__real_malloc...).
*/

#include <stddef.h>
#include <stdio.h>

void *__real_malloc(size_t size);
void *__real_calloc(size_t count, size_t size);
void *__real_realloc(void *block, size_t size);
void *__real_memalign(size_t alignment, size_t size);

unsigned long wiiu_log_milliseconds(void);

/* (only the first of many failures: after that, the log would be all this) */
#define MOST_LINES 32

static int quiet;
static int lines;

/* for wiiu_gl.c's count of the largest free block, which asks for too much
on purpose */
void wiiu_alloc_quiet(int on)
{
	quiet = on;
}

static void said(const char *what, size_t size, void *caller)
{
	unsigned long milliseconds;

	if (quiet || lines >= MOST_LINES)
		return;
	lines++;
	milliseconds = wiiu_log_milliseconds();
	fprintf(stderr, "[%5lu.%03lu] Halo U: out of memory: %s of %lu bytes, called from %p\n",
		milliseconds / 1000, milliseconds % 1000, what, (unsigned long)size, caller);
}

void *__wrap_malloc(size_t size)
{
	void *block = __real_malloc(size);

	if (!block)
		said("malloc", size, __builtin_return_address(0));
	return block;
}

void *__wrap_calloc(size_t count, size_t size)
{
	void *block = __real_calloc(count, size);

	if (!block && count && size)
		said("calloc", count * size, __builtin_return_address(0));
	return block;
}

void *__wrap_realloc(void *old, size_t size)
{
	void *block = __real_realloc(old, size);

	if (!block && size)
		said("realloc", size, __builtin_return_address(0));
	return block;
}

void *__wrap_memalign(size_t alignment, size_t size)
{
	void *block = __real_memalign(alignment, size);

	if (!block)
		said("memalign", size, __builtin_return_address(0));
	return block;
}
