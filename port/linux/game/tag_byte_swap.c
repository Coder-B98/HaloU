/*
TAG_BYTE_SWAP.C

A map's tags turned from the Xbox's byte order (little-endian) to a big-endian
machine's (the Wii U's).

The game reads a map's tag data straight into the tag cache and uses it as
its structures, so on a big-endian machine every 2-, 4- and 8-byte integer
and real in it must be turned around before anything reads it. The walk is
the validator's (tag_validate.c): each tag's root through its group's schema
(tag_schema.h), and every block's elements through theirs. Where the integers
are in each structure is the compiler's (tag_swap_codes.c, which
tools/tag_swap_codes.py writes from the game's own structures), so nothing
is listed by hand. A schema lists every block, reference and tag data of a
structure, which is how the walk finds the blocks' elements (and moves their
pointers, for a tag cache that is not where the Xbox had it).

Bytes are turned around once: each structure's are marked as they are (a bit
a byte), so that blocks that overlap or point at each other cannot make the
walk turn bytes back or loop. The validator refuses such a map afterwards.

What a tag data field holds is its own layout, not a structure the schemas
name: those whose layout is known are turned around here (the unicode string
lists' UTF-16 text); the others are counted and logged, still to be done.
*/

/* ---------- headers */

#include "cseries.h"
#include "tag_schema.h"
#include "tag_byte_swap.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ---------- constants */

enum
{
	TAG_HEADER_SIGNATURE = 'tags',
	STRUCTURE_BSP_GROUP_TAG = 'sbsp',
	STRUCTURE_BSP_HEADER_SIGNATURE = 'sbsp',

	/* the largest region turned around at once (a Custom Edition map's tag
	cache: tag_schema.h) */
	MAXIMUM_SWAP_REGION_SIZE = TAG_VALIDATE_MAXIMUM_TAG_CACHE_SIZE,

	/* a vertex or index buffer in a header: a D3DResource (Common, Data,
	Lock) */
	BUFFER_LONGS = 3,

	MAXIMUM_SWAP_DEPTH = 32,

	/* the type lookup's table (a power of two, more than the types) */
	SWAP_TYPE_TABLE_SIZE = 1024,

	/* the unicode string lists' text (tag_schema_render.c: its maximum is
	twice a string list's) */
	UNICODE_STRING_MAXIMUM = 2 * SHORT_MAX,
};

/* ---------- structures */

/* a region of memory being turned around: tag data, or a structure bsp */
struct swap_region
{
	byte *base;
	unsigned long size;
	long relocation;
	boolean failed;
};

/* the tags' header (cache_files.c's struct cache_file_tag_header) */
struct swap_tag_header
{
	struct swap_tag_instance *instances;
	long scenario_tag_index;
	unsigned long checksum;
	long tag_count;
	long vertex_buffer_count;
	byte *vertex_buffers;
	long index_buffer_count;
	byte *index_buffers;
	unsigned long signature;
};

/* a tag in the header's table (cache_files.c's struct
cache_file_tag_instance) */
struct swap_tag_instance
{
	unsigned long group_tag;
	unsigned long parent_group_tags[2];
	long tag_index;
	char *name;
	void *base_address;
	unsigned long unused[2];
};

/* a bsp's header (cache_files.c's struct cache_file_structure_bsp_header) */
struct swap_structure_bsp_header
{
	void *base_address;
	long vertex_buffer_count;
	byte *vertex_buffers;
	long index_buffer_count;
	byte *index_buffers;
	unsigned long signature;
};

typedef char verify_swap_tag_header_size[sizeof(struct swap_tag_header) == 0x24 ? 1 : -1];
typedef char verify_swap_tag_instance_size[sizeof(struct swap_tag_instance) == 0x20 ? 1 : -1];
typedef char verify_swap_structure_bsp_header_size[sizeof(struct swap_structure_bsp_header) == 0x18 ? 1 : -1];

/* ---------- globals */

static struct
{
	/* each schema definition's type (by its address) */
	struct
	{
		struct tag_schema_definition const *definition;
		struct tag_swap_type const *type;
	} types[SWAP_TYPE_TABLE_SIZE];
	long unswapped_tags;
	long unswapped_data;
	/* the groups and data fields already logged */
	unsigned long logged_groups[64];
	short logged_group_count;
	void const *logged_fields[64];
	short logged_field_count;
} tag_byte_swap_globals;

/* a bit for each byte of the region: whether it has been turned around */
static byte tag_byte_swap_marks[MAXIMUM_SWAP_REGION_SIZE / 8];

/* ---------- private code */

static unsigned short swap16(unsigned short value)
{
	return (unsigned short)((value >> 8) | (value << 8));
}

static unsigned long swap32(unsigned long value)
{
	return (value >> 24) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | (value << 24);
}

static void swap_run(byte *address, short size, short count)
{
	short index;

	for (index = 0; index < count; index++, address += size)
	{
		switch (size)
		{
		case 2:
		{
			unsigned short value;

			memcpy(&value, address, 2);
			value = swap16(value);
			memcpy(address, &value, 2);
			break;
		}
		case 4:
		{
			unsigned long value;

			memcpy(&value, address, 4);
			value = swap32(value);
			memcpy(address, &value, 4);
			break;
		}
		case 8:
		{
			unsigned long halves[2];

			memcpy(halves, address, 8);
			halves[0] = swap32(halves[0]);
			halves[1] = swap32(halves[1]);
			memcpy(address, &halves[1], 4);
			memcpy(address + 4, &halves[0], 4);
			break;
		}
		}
	}

	return;
}

static void swap_longs(void *address, long count)
{
	swap_run(address, 4, (short)count);

	return;
}

/* a message, where the validator's go (tag_validate_report) */
static void report(char const *format, ...)
{
	char message[256];
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(message, sizeof(message), format, arguments);
	va_end(arguments);
	tag_validate_report(message);

	return;
}

static void log_once_group(unsigned long group_tag)
{
	short index;

	tag_byte_swap_globals.unswapped_tags++;
	for (index = 0; index < tag_byte_swap_globals.logged_group_count; index++)
	{
		if (tag_byte_swap_globals.logged_groups[index] == group_tag)
			return;
	}
	if (tag_byte_swap_globals.logged_group_count < NUMBEROF(tag_byte_swap_globals.logged_groups))
		tag_byte_swap_globals.logged_groups[tag_byte_swap_globals.logged_group_count++] = group_tag;
	report("tag_byte_swap: tags of group '%c%c%c%c' have no schema: left as read",
		(char)(group_tag >> 24), (char)(group_tag >> 16), (char)(group_tag >> 8), (char)group_tag);

	return;
}

static void log_once_field(struct tag_schema_definition const *definition, struct tag_schema_field const *field)
{
	short index;

	tag_byte_swap_globals.unswapped_data++;
	for (index = 0; index < tag_byte_swap_globals.logged_field_count; index++)
	{
		if (tag_byte_swap_globals.logged_fields[index] == field)
			return;
	}
	if (tag_byte_swap_globals.logged_field_count < NUMBEROF(tag_byte_swap_globals.logged_fields))
		tag_byte_swap_globals.logged_fields[tag_byte_swap_globals.logged_field_count++] = field;
	report("tag_byte_swap: %s's %s is data of its own layout: left as read", definition->name,
		field->name);

	return;
}

/* the type of a schema definition's structure (NULL if tag_swap_codes.c has
none: it is out of date) */
static struct tag_swap_type const *definition_type(struct tag_schema_definition const *definition)
{
	unsigned long slot = ((unsigned long)definition >> 2) & (SWAP_TYPE_TABLE_SIZE - 1);
	long index;

	while (tag_byte_swap_globals.types[slot].definition)
	{
		if (tag_byte_swap_globals.types[slot].definition == definition)
			return tag_byte_swap_globals.types[slot].type;
		slot = (slot + 1) & (SWAP_TYPE_TABLE_SIZE - 1);
	}
	for (index = 0; index < tag_swap_type_count; index++)
	{
		if (definition->type_name && !strcmp(tag_swap_types[index].name, definition->type_name))
		{
			struct tag_swap_type const *type = &tag_swap_types[index];

			if (type->size != definition->size)
				type = NULL;
			tag_byte_swap_globals.types[slot].definition = definition;
			tag_byte_swap_globals.types[slot].type = type;
			return type;
		}
	}
	tag_byte_swap_globals.types[slot].definition = definition;
	tag_byte_swap_globals.types[slot].type = NULL;

	return NULL;
}

static boolean region_contains(struct swap_region const *region, void const *address, unsigned long size)
{
	unsigned long offset = (unsigned long)((byte const *)address - region->base);

	return (byte const *)address >= region->base && offset <= region->size && size <= region->size - offset;
}

/* marks size bytes at address as turned around: FALSE if any already were */
static boolean mark(struct swap_region const *region, void const *address, unsigned long size)
{
	unsigned long first = (unsigned long)((byte const *)address - region->base);
	unsigned long offset;

	for (offset = first; offset < first + size; offset++)
	{
		if (tag_byte_swap_marks[offset >> 3] & (1 << (offset & 7)))
			return FALSE;
	}
	for (offset = first; offset < first + size; offset++)
		tag_byte_swap_marks[offset >> 3] |= (byte)(1 << (offset & 7));

	return TRUE;
}

static void relocate(struct swap_region const *region, void *pointer_address)
{
	unsigned long pointer;

	memcpy(&pointer, pointer_address, 4);
	if (pointer && region->relocation)
	{
		pointer += (unsigned long)region->relocation;
		memcpy(pointer_address, &pointer, 4);
	}

	return;
}

static void swap_children(struct swap_region *region, byte *base, struct tag_schema_definition const *definition,
	short depth);

/* the structure at base (its own integers and those of the structures in
it), then its blocks' elements */
static void swap_element(struct swap_region *region, byte *base, struct tag_schema_definition const *definition,
	short depth)
{
	struct tag_swap_type const *type = definition_type(definition);
	long index;

	if (!type)
	{
		report("tag_byte_swap: no swap codes for %s (%s): tag_swap_codes.c is out of date",
			definition->name, definition->type_name ? definition->type_name : "?");
		region->failed = TRUE;
		return;
	}
	if (!region_contains(region, base, (unsigned long)definition->size) ||
		!mark(region, base, (unsigned long)definition->size))
		return;
	for (index = 0; index < type->run_count; index++)
		swap_run(base + type->runs[index].offset, type->runs[index].size, type->runs[index].count);
	swap_children(region, base, definition, depth);

	return;
}

/* a tag data field's bytes, where their layout is known */
static void swap_data(struct swap_region *region, struct tag_data *data,
	struct tag_schema_definition const *definition, struct tag_schema_field const *field)
{
	if (data->size <= 0 || !data->address)
		return;
	if (field->type == _tag_schema_file_data)
	{
		/* (its bytes are in the map file, read as they are needed: a sound's
		samples) */
		log_once_field(definition, field);
		return;
	}
	if (!region_contains(region, data->address, (unsigned long)data->size) ||
		!mark(region, data->address, (unsigned long)data->size))
		return;
	if (field->maximum == UNICODE_STRING_MAXIMUM)
		swap_run(data->address, 2, (short)(data->size / 2));
	else
		log_once_field(definition, field);

	return;
}

/* the pointers of the structure at base, whose integers are turned around,
and its blocks' elements */
static void swap_children(struct swap_region *region, byte *base, struct tag_schema_definition const *definition,
	short depth)
{
	struct tag_schema_field const *field;

	if (depth >= MAXIMUM_SWAP_DEPTH)
		return;
	for (field = definition->fields; field->type != _tag_schema_terminator && !region->failed; field++)
	{
		short index;

		for (index = 0; index < field->count && !region->failed; index++)
		{
			byte *address = base + field->offset + index * field->size;

			switch (field->type)
			{
			case _tag_schema_struct:
				swap_children(region, address, field->definition, depth + 1);
				break;

			case _tag_schema_block:
			{
				struct tag_block *block = (struct tag_block *)address;
				struct tag_schema_definition const *element = field->definition;
				long element_index;

				relocate(region, &block->address);
				if (block->count <= 0 || !block->address ||
					(unsigned long)block->count > region->size / (unsigned long)element->size)
					break;
				for (element_index = 0; element_index < block->count && !region->failed; element_index++)
					swap_element(region, (byte *)block->address + element_index * element->size, element, depth + 1);
				break;
			}

			case _tag_schema_data:
			case _tag_schema_file_data:
				if (field->type == _tag_schema_data)
					relocate(region, &((struct tag_data *)address)->address);
				swap_data(region, (struct tag_data *)address, definition, field);
				break;

			case _tag_schema_reference:
				relocate(region, &((struct tag_reference *)address)->name);
				break;

			default:
				break;
			}
		}
	}

	return;
}

static struct tag_schema_group const *schema_group_get(unsigned long group_tag)
{
	struct tag_schema_group const *const *list;

	for (list = tag_schema_group_lists; *list; list++)
	{
		struct tag_schema_group const *group;

		for (group = *list; group->group_tag; group++)
		{
			if (group->group_tag == group_tag)
				return group;
		}
	}

	return NULL;
}

/* a header's vertex and index buffers (their D3DResources: the bytes they
point at are their own layout, still to be done) */
static void swap_buffers(struct swap_region *region, byte *buffers, long count)
{
	long index;

	if (count <= 0 || !region_contains(region, buffers, (unsigned long)count * BUFFER_LONGS * 4) ||
		!mark(region, buffers, (unsigned long)count * BUFFER_LONGS * 4))
		return;
	for (index = 0; index < count; index++)
	{
		byte *buffer = buffers + index * BUFFER_LONGS * 4;

		swap_longs(buffer, BUFFER_LONGS);
		relocate(region, buffer + 4);
	}

	return;
}

static void region_begin(struct swap_region *region, void *base, long size, long relocation)
{
	region->base = base;
	region->size = size < 0 ? 0 : (unsigned long)size;
	region->relocation = relocation;
	region->failed = region->size > MAXIMUM_SWAP_REGION_SIZE;
	memset(tag_byte_swap_marks, 0, (region->size + 7) / 8 < sizeof(tag_byte_swap_marks) ?
		(region->size + 7) / 8 : sizeof(tag_byte_swap_marks));

	return;
}

/* ---------- public code */

void tag_byte_swap_cache_file_header(
	void *header)
{
	byte *bytes = header;

	/* (header_signature, version, file_length, reserved, tag_data_offset,
	tag_data_size; the name and build are text; footer_signature) */
	swap_longs(bytes, 6);
	swap_longs(bytes + 0x7FC, 1);

	return;
}

boolean tag_byte_swap_tags(
	void *tag_header,
	long tag_data_size,
	long relocation)
{
	struct swap_tag_header *header = tag_header;
	struct swap_region region;
	long index;

	region_begin(&region, tag_header, tag_data_size, relocation);
	if (region.failed || region.size < sizeof(*header))
		return FALSE;
	swap_longs(header, sizeof(*header) / 4);
	mark(&region, header, sizeof(*header));
	relocate(&region, &header->instances);
	relocate(&region, &header->vertex_buffers);
	relocate(&region, &header->index_buffers);
	if (header->signature != TAG_HEADER_SIGNATURE || header->tag_count <= 0 ||
		header->tag_count > UNSIGNED_SHORT_MAX ||
		!region_contains(&region, header->instances, (unsigned long)header->tag_count * sizeof(*header->instances)) ||
		!mark(&region, header->instances, (unsigned long)header->tag_count * sizeof(*header->instances)))
		return FALSE;
	swap_buffers(&region, header->vertex_buffers, header->vertex_buffer_count);
	swap_buffers(&region, header->index_buffers, header->index_buffer_count);

	/* the table, then each tag's root and blocks */
	for (index = 0; index < header->tag_count; index++)
	{
		struct swap_tag_instance *instance = &header->instances[index];

		swap_longs(instance, sizeof(*instance) / 4);
		relocate(&region, &instance->name);
		relocate(&region, &instance->base_address);
	}
	for (index = 0; index < header->tag_count && !region.failed; index++)
	{
		struct swap_tag_instance *instance = &header->instances[index];
		struct tag_schema_group const *group;

		/* (a structure bsp's root is where it loads) */
		if (instance->group_tag == STRUCTURE_BSP_GROUP_TAG || !instance->base_address)
			continue;
		group = schema_group_get(instance->group_tag);
		if (!group || !group->definition)
		{
			log_once_group(instance->group_tag);
			continue;
		}
		swap_element(&region, instance->base_address, group->definition, 0);
	}

	return !region.failed;
}

boolean tag_byte_swap_structure_bsp(
	void *base,
	long size,
	long relocation)
{
	struct swap_structure_bsp_header *header = base;
	struct tag_schema_group const *group = schema_group_get(STRUCTURE_BSP_GROUP_TAG);
	struct swap_region region;

	region_begin(&region, base, size, relocation);
	if (region.failed || region.size < sizeof(*header))
		return FALSE;
	swap_longs(header, sizeof(*header) / 4);
	mark(&region, header, sizeof(*header));
	relocate(&region, &header->base_address);
	relocate(&region, &header->vertex_buffers);
	relocate(&region, &header->index_buffers);
	if (header->signature != STRUCTURE_BSP_HEADER_SIGNATURE)
		return FALSE;
	swap_buffers(&region, header->vertex_buffers, header->vertex_buffer_count);
	swap_buffers(&region, header->index_buffers, header->index_buffer_count);
	if (group && group->definition)
		swap_element(&region, header->base_address, group->definition, 0);

	return !region.failed;
}

long tag_byte_swap_unswapped_tags(
	void)
{
	return tag_byte_swap_globals.unswapped_tags;
}

long tag_byte_swap_unswapped_data(
	void)
{
	return tag_byte_swap_globals.unswapped_data;
}
