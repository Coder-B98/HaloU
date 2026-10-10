/*
WIIU_MEMORY.C

Xbox contiguous memory (XPhysicalAlloc) and page protection for the Wii U
build (port/linux/src/xbox_memory.c is the Linux build's).

The Xbox sees physical memory at virtual 0x80000000 + P, and the game asks
for its game state and tag cache at fixed physical addresses that way
(physical_memory_map.c). A Wii U program has no address it can ask for, so
the window is one block of its heap, 128 MB aligned to 128 MB: a physical
address P is then the window's base + P, and the arithmetic of platform.h
holds (a base OR P, an address AND NOT the base). Blocks are handed out of
it a page at a time, at the physical address asked for or top-down as the
Xbox kernel does. The Wii U has no page protection a program can set: the
protections are only remembered.
*/

#include "platform.h"

#include <stdlib.h>
#include <string.h>

/* newlib's (its malloc.h, which the Linux layer's own malloc.h hides) */
void *memalign(size_t alignment, size_t size);

#define PAGE_SIZE_BYTES 0x1000UL
#define CONTIGUOUS_PAGE_COUNT (PLATFORM_CONTIGUOUS_SIZE / PAGE_SIZE_BYTES)

unsigned long platform_contiguous_base = 0;

/* per page: 0 free, otherwise the protection of the block (PAGE_*); the
first page of a block also records the block length */
static DWORD page_protection[CONTIGUOUS_PAGE_COUNT];
static unsigned long block_page_count[CONTIGUOUS_PAGE_COUNT];
static pthread_mutex_t arena_lock = PTHREAD_MUTEX_INITIALIZER;

/* the window, before the game's first allocation */
static BOOL arena_reserve(void)
{
	void *window = NULL;

	if (platform_contiguous_base)
		return TRUE;
	window = memalign(PLATFORM_CONTIGUOUS_SIZE, PLATFORM_CONTIGUOUS_SIZE);
	if (!window)
	{
		platform_log("cannot allocate the Xbox contiguous memory window (%lu MB)",
			PLATFORM_CONTIGUOUS_SIZE >> 20);
		return FALSE;
	}
	platform_contiguous_base = (unsigned long)window;
	platform_log("the Xbox contiguous memory window is at %08lx", platform_contiguous_base);
	return TRUE;
}

void *halo_custom_edition_tag_cache(void)
{
	/* (Halo Custom Edition maps are not played on the Wii U) */
	return NULL;
}

BOOL platform_is_contiguous(const void *address)
{
	unsigned long value = (unsigned long)address;

	return platform_contiguous_base && value >= platform_contiguous_base &&
		value - platform_contiguous_base < PLATFORM_CONTIGUOUS_SIZE;
}

static BOOL pages_free(unsigned long first, unsigned long count)
{
	unsigned long page;

	if (first + count > CONTIGUOUS_PAGE_COUNT)
		return FALSE;
	for (page = first; page < first + count; page++)
	{
		if (page_protection[page])
			return FALSE;
	}
	return TRUE;
}

void *platform_contiguous_alloc(unsigned long size, unsigned long alignment,
	unsigned long physical_address, DWORD protect)
{
	unsigned long count = (size + PAGE_SIZE_BYTES - 1) / PAGE_SIZE_BYTES;
	unsigned long alignment_pages = alignment > PAGE_SIZE_BYTES ? alignment / PAGE_SIZE_BYTES : 1;
	unsigned long first = CONTIGUOUS_PAGE_COUNT;
	unsigned long page;
	void *address;

	if (!count)
		count = 1;
	protect &= ~(PAGE_WRITECOMBINE | PAGE_NOCACHE);
	if (!protect)
		protect = PAGE_READWRITE;

	pthread_mutex_lock(&arena_lock);
	if (!arena_reserve())
	{
		pthread_mutex_unlock(&arena_lock);
		return NULL;
	}
	if (physical_address != PLATFORM_ANY_PHYSICAL_ADDRESS)
	{
		unsigned long wanted = PLATFORM_VIRTUAL_TO_PHYSICAL(physical_address) / PAGE_SIZE_BYTES;

		if (pages_free(wanted, count))
			first = wanted;
	}
	else if (count <= CONTIGUOUS_PAGE_COUNT)
	{
		/* top-down first fit, like the Xbox contiguous allocator */
		unsigned long candidate = CONTIGUOUS_PAGE_COUNT - count;

		for (;;)
		{
			candidate -= candidate % alignment_pages;
			if (pages_free(candidate, count))
			{
				first = candidate;
				break;
			}
			if (candidate == 0)
				break;
			candidate--;
		}
	}
	if (first == CONTIGUOUS_PAGE_COUNT)
	{
		pthread_mutex_unlock(&arena_lock);
		return NULL;
	}

	address = (void *)(platform_contiguous_base + first * PAGE_SIZE_BYTES);
	/* (fresh pages are zeroed, as the Xbox's are) */
	memset(address, 0, count * PAGE_SIZE_BYTES);
	memory_watch_forget(address, count * PAGE_SIZE_BYTES);
	for (page = first; page < first + count; page++)
		page_protection[page] = protect;
	block_page_count[first] = count;
	pthread_mutex_unlock(&arena_lock);
	return address;
}

void platform_contiguous_free(void *address)
{
	unsigned long first, count, page;

	if (!platform_is_contiguous(address))
		return;
	first = ((unsigned long)address - platform_contiguous_base) / PAGE_SIZE_BYTES;
	pthread_mutex_lock(&arena_lock);
	count = block_page_count[first];
	if (count)
	{
		memory_watch_forget(address, count * PAGE_SIZE_BYTES);
		for (page = first; page < first + count; page++)
			page_protection[page] = 0;
		block_page_count[first] = 0;
	}
	pthread_mutex_unlock(&arena_lock);
}

/* ---------- XAPI */

LPVOID WINAPI XPhysicalAlloc(SIZE_T size, ULONG_PTR physical_address, ULONG_PTR alignment, DWORD protect)
{
	/* A highest-acceptable address inside the window places the block
	there, which is how the game gets its fixed game state and tag cache
	addresses; anything else may go anywhere. */
	void *result = platform_contiguous_alloc(size, alignment,
		physical_address < PLATFORM_CONTIGUOUS_SIZE ? physical_address : PLATFORM_ANY_PHYSICAL_ADDRESS,
		protect);

	if (!result)
	{
		platform_log("XPhysicalAlloc: cannot allocate %lu bytes (physical address 0x%08lx)",
			(unsigned long)size, (unsigned long)physical_address);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	}
	return result;
}

VOID WINAPI XPhysicalFree(LPVOID address)
{
	platform_contiguous_free(address);
}

BOOL WINAPI VirtualProtect(LPVOID address, SIZE_T size, DWORD new_protect, PDWORD old_protect)
{
	unsigned long start = (unsigned long)address & ~(PAGE_SIZE_BYTES - 1);
	unsigned long end = ((unsigned long)address + size + PAGE_SIZE_BYTES - 1) & ~(PAGE_SIZE_BYTES - 1);

	if (old_protect)
		*old_protect = platform_is_contiguous(address) ?
			page_protection[(start - platform_contiguous_base) / PAGE_SIZE_BYTES] : PAGE_READWRITE;
	if (platform_is_contiguous((void *)start))
	{
		unsigned long page;

		pthread_mutex_lock(&arena_lock);
		for (page = (start - platform_contiguous_base) / PAGE_SIZE_BYTES;
			page < (end - platform_contiguous_base) / PAGE_SIZE_BYTES && page < CONTIGUOUS_PAGE_COUNT;
			page++)
		{
			if (page_protection[page])
				page_protection[page] = new_protect & ~(PAGE_WRITECOMBINE | PAGE_NOCACHE);
		}
		pthread_mutex_unlock(&arena_lock);
	}
	return TRUE;
}

VOID WINAPI XPhysicalProtect(LPVOID address, SIZE_T size, DWORD new_protect)
{
	VirtualProtect(address, size, new_protect, NULL);
}

DWORD WINAPI XQueryMemoryProtect(LPVOID address)
{
	DWORD protect = PAGE_READWRITE;

	if (platform_is_contiguous(address))
	{
		pthread_mutex_lock(&arena_lock);
		protect = page_protection[((unsigned long)address - platform_contiguous_base) / PAGE_SIZE_BYTES];
		pthread_mutex_unlock(&arena_lock);
		if (!protect)
			protect = PAGE_NOACCESS;
	}
	return protect;
}
