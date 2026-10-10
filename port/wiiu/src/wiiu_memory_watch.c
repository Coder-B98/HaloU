/*
WIIU_MEMORY_WATCH.C

Write tracking for the guest memory the renderer caches, for the Wii U
build (port/linux/src/memory_watch.c is the Linux build's).

On Linux the pages behind a cached texture are made read-only, and the
first write to one faults and marks it written. A Wii U program cannot
protect its own pages, so nothing faults here. Instead the writes that
replace a texture's texels are marked where they happen: the file layer's
reads into the contiguous window (xbox_files.c), and blocks of it being
allocated or freed (wiiu_memory.c). A texture the CPU draws into by itself
is not noticed, and keeps its first upload.

The renderer's vertex and index buffers do not use this on the Wii U: the
GPU reads them straight from the window (d3d8_gl.c, mirror_range).
*/

#include "platform.h"

#define WATCH_PAGE_SIZE 0x1000UL
#define WATCH_PAGE_COUNT (PLATFORM_CONTIGUOUS_SIZE / WATCH_PAGE_SIZE)

static unsigned long page_generation[WATCH_PAGE_COUNT];
static volatile unsigned long current_generation = 1;

/* the pages of a range of the window, FALSE if it is outside it */
static BOOL page_range(unsigned long address, unsigned long size, unsigned long *first, unsigned long *last)
{
	if (!size || address + size <= PLATFORM_CONTIGUOUS_BASE ||
		address >= PLATFORM_CONTIGUOUS_BASE + PLATFORM_CONTIGUOUS_SIZE)
	{
		return FALSE;
	}
	*first = address < PLATFORM_CONTIGUOUS_BASE ? 0 : (address - PLATFORM_CONTIGUOUS_BASE) / WATCH_PAGE_SIZE;
	*last = (address + size - 1 - PLATFORM_CONTIGUOUS_BASE) / WATCH_PAGE_SIZE;
	if (*last >= WATCH_PAGE_COUNT)
		*last = WATCH_PAGE_COUNT - 1;
	return TRUE;
}

static void mark_written(unsigned long address, unsigned long size)
{
	unsigned long first, last, page, generation;

	if (!page_range(address, size, &first, &last))
		return;
	generation = __sync_add_and_fetch(&current_generation, 1);
	for (page = first; page <= last; page++)
		page_generation[page] = generation;
}

void memory_watch_initialize(void)
{
}

/* (nothing to protect: writes are marked as they are made) */
void memory_watch_protect(unsigned long address, unsigned long size)
{
	(void)address;
	(void)size;
}

unsigned long memory_watch_generation(unsigned long address, unsigned long size)
{
	unsigned long first, last, page, newest = 0;

	if (!page_range(address, size, &first, &last))
		return 0;
	for (page = first; page <= last; page++)
	{
		if (page_generation[page] > newest)
			newest = page_generation[page];
	}
	return newest;
}

unsigned long memory_watch_serial(void)
{
	return current_generation;
}

void memory_watch_prepare_write(void *address, unsigned long size)
{
	mark_written((unsigned long)address, size);
}

void memory_watch_forget(void *address, unsigned long size)
{
	mark_written((unsigned long)address, size);
}
