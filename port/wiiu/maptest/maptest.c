/*
MAPTEST.C

The Wii U port's first program: each Xbox map in sd:/halou/maps read, its
tags turned around for the Wii U's byte order where the Wii U's memory has
them (port/linux/game/tag_byte_swap.c), then checked by the tag validator
(port/linux/game/tag_validate.c), then each of its structure bsps the same
way: what the game will do as a map loads, without the game. Each map's
result is shown on the screen and written to sd:/halou/maptest.txt.

It is tools/map_validate.c for the Wii U: the same steps, with the tag cache
wherever the Wii U's memory gives it (the Xbox had it at 0x803A6000, which a
Wii U program cannot have), so every pointer in the tags is moved too.
*/

#include <coreinit/time.h>
#include <coreinit/thread.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/proc.h>

#include <dirent.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib_prefixed.h"

/* ---------- constants */

#define MAPS_FOLDER "fs:/vol/external01/halou/maps"
#define LOG_PATH "fs:/vol/external01/halou/maptest.txt"

enum
{
	/* where the Xbox had the tag cache, which the map's pointers are
	relative to, and its size (physical_memory_map.h) */
	XBOX_TAG_CACHE_ADDRESS = 0x803A6000,
	TAG_CACHE_SIZE = 0x1600000,

	CACHE_FILE_HEADER_SIZE = 0x800,
	CACHE_FILE_HEADER_SIGNATURE = 'head',
	CACHE_FILE_FOOTER_SIGNATURE = 'foot',
	SCENARIO_GROUP_TAG = 'scnr',
	CACHE_FILE_SECTOR_SIZE = 512,
	/* the scenario's structure bsps block (scenario_definitions.h) */
	SCENARIO_STRUCTURE_BSPS_OFFSET = 0x5A4,
	MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO = 32,
};

/* ---------- structures */

struct cache_file_header
{
	unsigned long header_signature;
	long version;
	long file_length;
	unsigned long reserved0C;
	long tag_data_offset;
	long tag_data_size;
	unsigned char reserved18[8];
	char name[0x20];
	char build[0x20];
	unsigned char reserved60[0x79C];
	unsigned long footer_signature;
};

struct tag_instance
{
	unsigned long group_tag;
	unsigned long parent_group_tags[2];
	long tag_index;
	char *name;
	void *base_address;
	unsigned long unused[2];
};

struct tag_header
{
	struct tag_instance *instances;
	long scenario_tag_index;
	unsigned long checksum;
	long tag_count;
};

struct tag_block
{
	long count;
	void *address;
	void *definition;
};

struct scenario_structure_bsp_reference
{
	long file_offset;
	long file_size;
	void *base_address;
	unsigned char unused[4];
	unsigned long group_tag;
	char *name;
	long name_length;
	long tag_index;
};

typedef char verify_cache_file_header_size[sizeof(struct cache_file_header) == CACHE_FILE_HEADER_SIZE ? 1 : -1];

/* ---------- the validator's and the byte swapper's (port/linux/game) */

unsigned char tag_validate_tags(void *tag_header, long tag_data_size, long file_length, char const *map_name);
unsigned char tag_validate_structure_bsp(long tag_index, void *base, long size);
long tag_validate_corrections(void);
void tag_byte_swap_cache_file_header(void *header);
unsigned char tag_byte_swap_tags(void *tag_header, long tag_data_size, long relocation);
unsigned char tag_byte_swap_structure_bsp(void *base, long size, long relocation);
long tag_byte_swap_unswapped_tags(void);
long tag_byte_swap_unswapped_data(void);

/* ---------- globals */

static FILE *log_file;
/* the validator's messages for the map being checked: all go to the file,
the first few to the screen */
static long map_messages;

/* ---------- output */

static void say(char const *format, ...)
{
	char line[512];
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(line, sizeof(line), format, arguments);
	va_end(arguments);
	WHBLogPrint(line);
	WHBLogConsoleDraw();
	if (log_file)
	{
		fprintf(log_file, "%s\n", line);
		fflush(log_file);
	}
}

void tag_validate_report(char const *message)
{
	if (map_messages++ < 4)
	{
		say("  %s", message);
	}
	else if (log_file)
	{
		fprintf(log_file, "  %s\n", message);
	}
}

/* (the game's printf family and string functions, which the validator calls
by these names: port/linux/include/stdio.h, cseries.h) */
int halo_linux_vsnprintf(char *buffer, size_t count, char const *format, va_list arguments)
{
	return vsnprintf(buffer, count, format, arguments);
}

int halo_linux_snprintf(char *buffer, size_t count, char const *format, ...)
{
	va_list arguments;
	int result;

	va_start(arguments, format);
	result = vsnprintf(buffer, count, format, arguments);
	va_end(arguments);
	return result;
}

void *csmemset(void *buffer, long c, unsigned long size) { return memset(buffer, (int)c, size); }
void *csmemcpy(void *destination, void const *source, unsigned long size) { return memcpy(destination, source, size); }
unsigned long csstrlen(char const *s) { return strlen(s); }
char *csstrcat(char *s1, char const *s2) { return strcat(s1, s2); }
long csstrcmp(char const *s1, char const *s2) { return strcmp(s1, s2); }

/* ---------- maps */

/* the map's bytes as the game sees them (inflated after its header if it is
compressed), its header turned around */
static unsigned char *map_read(char const *path, struct cache_file_header *header)
{
	FILE *file = fopen(path, "rb");
	unsigned char *bytes = NULL;
	unsigned char *compressed = NULL;
	long size;

	if (!file)
		return NULL;
	if (fseek(file, 0, SEEK_END) || (size = ftell(file)) < CACHE_FILE_HEADER_SIZE || fseek(file, 0, SEEK_SET) ||
		fread(header, CACHE_FILE_HEADER_SIZE, 1, file) != 1)
	{
		fclose(file);
		return NULL;
	}
	tag_byte_swap_cache_file_header(header);
	if (header->header_signature != CACHE_FILE_HEADER_SIGNATURE ||
		header->footer_signature != CACHE_FILE_FOOTER_SIGNATURE ||
		header->file_length < CACHE_FILE_HEADER_SIZE)
	{
		fclose(file);
		return NULL;
	}
	bytes = malloc(header->file_length);
	compressed = malloc(size - CACHE_FILE_HEADER_SIZE);
	if (bytes && compressed && fread(compressed, size - CACHE_FILE_HEADER_SIZE, 1, file) == 1)
	{
		memcpy(bytes, header, CACHE_FILE_HEADER_SIZE);
		if (size == header->file_length)
		{
			memcpy(bytes + CACHE_FILE_HEADER_SIZE, compressed, size - CACHE_FILE_HEADER_SIZE);
		}
		else
		{
			uLongf inflated = header->file_length - CACHE_FILE_HEADER_SIZE;

			if (uncompress(bytes + CACHE_FILE_HEADER_SIZE, &inflated, compressed, size - CACHE_FILE_HEADER_SIZE) != Z_OK ||
				inflated != (uLongf)(header->file_length - CACHE_FILE_HEADER_SIZE))
			{
				free(bytes);
				bytes = NULL;
			}
		}
	}
	else
	{
		free(bytes);
		bytes = NULL;
	}
	free(compressed);
	fclose(file);

	return bytes;
}

/* the map's tags turned around and checked where the tag cache is, then
each of its bsps where it loads: TRUE if the map plays */
static int map_check(struct cache_file_header const *header, unsigned char const *bytes, unsigned char *tag_cache,
	char const *name, long *corrections)
{
	struct tag_header *tags = (struct tag_header *)tag_cache;
	long relocation = (long)((unsigned long)tag_cache - XBOX_TAG_CACHE_ADDRESS);
	struct tag_block *bsps;
	long bsp_count;
	long index;

	*corrections = 0;
	if (header->tag_data_offset < 0 || header->tag_data_size <= 0 || header->tag_data_size > TAG_CACHE_SIZE ||
		header->tag_data_offset > header->file_length - header->tag_data_size)
	{
		say("%s: its tag data is outside it", name);
		return 0;
	}
	memset(tag_cache, 0xCD, TAG_CACHE_SIZE);
	memcpy(tag_cache, bytes + header->tag_data_offset, header->tag_data_size);
	if (!tag_byte_swap_tags(tag_cache, header->tag_data_size, relocation))
	{
		say("%s: its tags cannot be turned around", name);
		return 0;
	}
	if (!tag_validate_tags(tag_cache, header->tag_data_size, header->file_length, name))
		return 0;

	{
		short scenario_index = (short)tags->scenario_tag_index;
		struct tag_instance *scenario;

		if (scenario_index < 0 || scenario_index >= tags->tag_count ||
			tags->instances[scenario_index].group_tag != SCENARIO_GROUP_TAG)
		{
			say("%s: its tag header names no scenario", name);
			return 0;
		}
		scenario = &tags->instances[scenario_index];
		bsps = (struct tag_block *)((unsigned char *)scenario->base_address + SCENARIO_STRUCTURE_BSPS_OFFSET);
		bsp_count = bsps->count < MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO ? bsps->count : MAXIMUM_STRUCTURE_BSPS_PER_SCENARIO;
	}
	for (index = 0; index < bsp_count; index++)
	{
		struct scenario_structure_bsp_reference *reference =
			(struct scenario_structure_bsp_reference *)bsps->address + index;
		unsigned long load = (unsigned long)reference->base_address - (unsigned long)tag_cache;
		unsigned long read_size = ((unsigned long)reference->file_size + CACHE_FILE_SECTOR_SIZE - 1) &
			~(unsigned long)(CACHE_FILE_SECTOR_SIZE - 1);

		if (reference->file_offset < 0 || reference->file_size <= 0 ||
			reference->file_offset > header->file_length - reference->file_size ||
			load < (unsigned long)header->tag_data_size || load > TAG_CACHE_SIZE ||
			read_size > TAG_CACHE_SIZE - load)
		{
			say("%s: its structure bsp %ld is outside it", name, index);
			return 0;
		}
		memset(tag_cache + header->tag_data_size, 0xCD, TAG_CACHE_SIZE - header->tag_data_size);
		memcpy(reference->base_address, bytes + reference->file_offset, reference->file_size);
		if (!tag_byte_swap_structure_bsp(reference->base_address, reference->file_size, relocation))
		{
			say("%s: its structure bsp %ld cannot be turned around", name, index);
			return 0;
		}
		if (!tag_validate_structure_bsp(reference->tag_index, reference->base_address, reference->file_size))
			return 0;
	}
	*corrections = tag_validate_corrections();

	return 1;
}

static int compare_names(void const *a, void const *b)
{
	return strcmp(*(char *const *)a, *(char *const *)b);
}

static void check_maps(void)
{
	DIR *folder = opendir(MAPS_FOLDER);
	unsigned char *tag_cache;
	char *names[64];
	int name_count = 0;
	int passed = 0;
	int index;
	struct dirent *entry;

	if (!folder)
	{
		say("No maps found. Copy your maps folder to the SD card as sd:/halou/maps");
		return;
	}
	while ((entry = readdir(folder)) && name_count < 64)
	{
		size_t length = strlen(entry->d_name);

		if (length > 4 && !strcmp(entry->d_name + length - 4, ".map"))
			names[name_count++] = strdup(entry->d_name);
	}
	closedir(folder);
	qsort(names, name_count, sizeof(*names), compare_names);

	tag_cache = memalign(0x1000, TAG_CACHE_SIZE);
	if (!tag_cache)
	{
		say("No memory for the tag cache");
		return;
	}
	say("Tag cache at %08lx (the Xbox's was at %08x)", (unsigned long)tag_cache, XBOX_TAG_CACHE_ADDRESS);
	for (index = 0; index < name_count && WHBProcIsRunning(); index++)
	{
		char path[256];
		struct cache_file_header header;
		unsigned char *bytes;
		long corrections = 0;
		OSTime start = OSGetTime();

		snprintf(path, sizeof(path), "%s/%s", MAPS_FOLDER, names[index]);
		say("%s: reading...", names[index]);
		map_messages = 0;
		bytes = map_read(path, &header);
		if (!bytes)
		{
			say("%s: cannot be read", names[index]);
			continue;
		}
		if (map_check(&header, bytes, tag_cache, names[index], &corrections))
		{
			passed++;
			say("%s: passed, %ld corrections (%ld ms)", names[index], corrections,
				(long)OSTicksToMilliseconds(OSGetTime() - start));
		}
		else
		{
			say("%s: refused", names[index]);
		}
		free(bytes);
	}
	say("");
	say("%d of %d maps passed. Tags of groups without a schema: %ld; tag data left as read: %ld", passed, name_count,
		tag_byte_swap_unswapped_tags(), tag_byte_swap_unswapped_data());
	free(tag_cache);
	for (index = 0; index < name_count; index++)
		free(names[index]);
}

int main(int argc, char **argv)
{
	WHBProcInit();
	WHBLogConsoleInit();
	WHBLogConsoleSetColor(0x00284000);

	log_file = fopen(LOG_PATH, "w");
	say("Halo U map test: the Xbox maps in the Wii U's byte order");
	say("");
	check_maps();
	say("");
	say("Press HOME to quit.");
	if (log_file)
		fclose(log_file);
	log_file = NULL;

	while (WHBProcIsRunning())
	{
		WHBLogConsoleDraw();
		OSSleepTicks(OSMillisecondsToTicks(100));
	}

	WHBLogConsoleFree();
	WHBProcShutdown();

	return 0;
}
