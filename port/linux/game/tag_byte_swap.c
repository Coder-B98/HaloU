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

The geometry the game draws is also bytes of its own layout: a model part's
vertices and triangle indices (in the buffers the tags' header lists) and a
structure bsp material's vertices (in its compressed vertex data, and the
buffers the bsp's header lists). Each is turned around as its part or
material is, by its vertex type's layout (swap_geometry), a vertex or index
at a time, so that buffers parts share are turned around once.

An animation's frames are bytes of their own layout too (swap_animation):
each frame, node by node, a rotation (four shorts), a translation (three
reals) and a scale (a real) for each node its flags say moves; its default
data the same for the nodes that don't; a compressed animation's header,
node headers, frame indices, defaults and keyframes; its frame info, reals.
*/

/* ---------- headers */

#include "cseries.h"
#include "tag_schema.h"
#include "tag_byte_swap.h"
#include "rasterizer/rasterizer_geometry.h"
#include "models/model_animation_definitions.h"
#include "structures/structure_bsp_definitions.h"

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

/* a model's part (tag_schema_models.c's struct model_geometry_part): its
buffers, after its own fields and its copies' blocks */
struct swap_model_geometry_part
{
	byte reserved[0x44];
	struct triangle_buffer triangle_buffer;
	struct vertex_buffer vertex_buffer;
};

typedef char verify_swap_model_geometry_part_size[sizeof(struct swap_model_geometry_part) == 0x68 ? 1 : -1];
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

			/* (a definition may be the start of its structure: a control
			tag ends at its denied effect) */
			if (type->size < definition->size)
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

/* whether the byte at offset in a definition's structure is in one of its
structure fields (_tag_schema_struct), which are turned around by their own
definitions: a schema may see a structure's bytes as another structure than
the game's declaration does (a contrail's shader, declared with reserved bytes,
is a shader_effect_definition), and the schema's is the more exact */
static boolean in_structure_field(struct tag_schema_definition const *definition, long offset)
{
	struct tag_schema_field const *field;

	for (field = definition->fields; field->type != _tag_schema_terminator; field++)
	{
		if (field->type == _tag_schema_struct && offset >= field->offset &&
			offset < field->offset + (long)field->size * field->count)
		{
			long within = (offset - field->offset) % field->size;

			return within < ((struct tag_schema_definition const *)field->definition)->size;
		}
	}

	return FALSE;
}

/* the integers of the structure at base, by its definition's type and those
of its structure fields; FALSE if a type is missing */
static boolean swap_structure(struct swap_region *region, byte *base, struct tag_schema_definition const *definition,
	short depth)
{
	struct tag_swap_type const *type = definition_type(definition);
	struct tag_schema_field const *field;
	long index;

	if (!type)
	{
		report("tag_byte_swap: no swap codes for %s (%s): tag_swap_codes.c is out of date",
			definition->name, definition->type_name ? definition->type_name : "?");
		region->failed = TRUE;
		return FALSE;
	}
	if (depth >= MAXIMUM_SWAP_DEPTH)
		return TRUE;
	for (index = 0; index < type->run_count; index++)
	{
		struct tag_swap_run const *run = &type->runs[index];
		short item;

		short size = run->size == TAG_SWAP_POINTER ? 4 : run->size;

		for (item = 0; item < run->count; item++)
		{
			long offset = run->offset + item * size;

			/* (only what the definition covers of its structure) */
			if (offset + size > definition->size)
				break;
			if (in_structure_field(definition, offset))
				continue;
			swap_run(base + offset, size, 1);
			if (run->size == TAG_SWAP_POINTER)
				relocate(region, base + offset);
		}
	}
	for (field = definition->fields; field->type != _tag_schema_terminator && !region->failed; field++)
	{
		short item;

		if (field->type != _tag_schema_struct)
			continue;
		for (item = 0; item < field->count; item++)
			swap_structure(region, base + field->offset + item * field->size, field->definition, depth + 1);
	}

	return !region->failed;
}

/* geometry */

/* each vertex type's layout: runs of integers or reals (size bytes, count of
them) from its start (rasterizer_geometry.c's and rasterizer_model_types.h's
structures, in rasterizer_vertex_type_sizes' sizes) */
struct vertex_run
{
	char size;
	char count;
};

static struct vertex_run const vertex_layouts[NUMBER_OF_RASTERIZER_VERTEX_TYPES][4] =
{
	/* environment uncompressed: position, normal, binormal, tangent, texcoord */
	{{4, 14}},
	/* environment compressed: position, packed normal, binormal and tangent,
	texcoord */
	{{4, 8}},
	/* environment lightmap uncompressed: incident radiosity, texcoord */
	{{4, 5}},
	/* environment lightmap compressed: packed incident radiosity, u, v */
	{{4, 1}, {2, 2}},
	/* model uncompressed: position, normal, binormal, tangent, texcoord, two
	node indices, two node weights */
	{{4, 14}, {2, 2}, {4, 2}},
	/* model compressed: position, packed normal, binormal and tangent, two
	short texcoords, two byte node indices, a node weight */
	{{4, 6}, {2, 2}, {1, 2}, {2, 1}},
	/* (the rest are the game's own, made as it runs: never in a map) */
};

static short const vertex_sizes[NUMBER_OF_RASTERIZER_VERTEX_TYPES] =
{
	56, 32, 20, 8, 68, 32, 24, 36, 20, 16, 16, 8
};

/* count vertices of a type at data, each turned around unless it already
was (a buffer two parts share). FALSE if they are not in the region or their
type is not one a map has */
static boolean swap_vertices(struct swap_region *region, byte *data, long count, short type)
{
	long size;
	long index;

	if (count <= 0)
		return TRUE;
	if (type < 0 || type >= NUMBER_OF_RASTERIZER_VERTEX_TYPES || !vertex_layouts[type][0].size)
		return FALSE;
	size = vertex_sizes[type];
	if ((unsigned long)count > region->size / (unsigned long)size || !region_contains(region, data, count * size))
		return FALSE;
	for (index = 0; index < count; index++)
	{
		byte *vertex = data + index * size;
		struct vertex_run const *run;

		if (!mark(region, vertex, (unsigned long)size))
			continue;
		for (run = vertex_layouts[type]; run < vertex_layouts[type] + 4 && run->size; run++)
		{
			swap_run(vertex, run->size, run->count);
			vertex += run->size * run->count;
		}
	}

	return TRUE;
}

/* count 16-bit triangle indices at data, each turned around unless it
already was */
static boolean swap_indices(struct swap_region *region, byte *data, long count)
{
	long index;

	if (count <= 0)
		return TRUE;
	if ((unsigned long)count > region->size / 2 || !region_contains(region, data, count * 2))
		return FALSE;
	for (index = 0; index < count; index++)
	{
		if (mark(region, data + index * 2, 2))
			swap_run(data + index * 2, 2, 1);
	}

	return TRUE;
}

/* the bytes a header's buffer (its D3DResource, turned around with the
header: swap_buffers) holds, or NULL if it is not one in the region */
static byte *buffer_data(struct swap_region const *region, void const *buffer)
{
	byte *data;

	if (!buffer || !region_contains(region, buffer, BUFFER_LONGS * 4))
		return NULL;
	memcpy(&data, (byte const *)buffer + 4, 4);

	return data;
}

static void geometry_report(char const *what, char const *name)
{
	report("tag_byte_swap: %s's %s are not in the map: left as read", what, name);

	return;
}

static void swap_animation(struct swap_region *region, struct animation *animation);

/* a model part's vertices and indices, or a structure bsp material's
vertices, as the part or material is turned around (its counts and types
now this machine's) */
static void swap_geometry(struct swap_region *region, byte *base, struct tag_schema_definition const *definition)
{
	if (!definition->type_name)
		return;
	if (!strcmp(definition->type_name, "struct animation"))
		swap_animation(region, (struct animation *)base);
	else if (!strcmp(definition->type_name, "struct model_geometry_part"))
	{
		struct swap_model_geometry_part *part = (struct swap_model_geometry_part *)base;
		struct triangle_buffer const *triangles = &part->triangle_buffer;
		long index_count = 0;
		byte *data;

		if (part->vertex_buffer.hardware_format)
		{
			data = buffer_data(region, part->vertex_buffer.hardware_format);
			if (!data || !swap_vertices(region, data, part->vertex_buffer.count, part->vertex_buffer.type))
				geometry_report("model_geometry_part", "vertices");
		}
		if (triangles->type == _triangle_buffer_type_triangles)
			index_count = triangles->count > 0 && triangles->count <= LONG_MAX / 3 ? 3 * triangles->count : 0;
		else if (triangles->type == _triangle_buffer_type_precompiled_strip)
			index_count = triangles->count > 0 && triangles->count <= LONG_MAX - 2 ? triangles->count + 2 : 0;
		if (triangles->hardware_format && index_count)
		{
			data = buffer_data(region, triangles->hardware_format);
			if (!data || !swap_indices(region, data, index_count))
				geometry_report("model_geometry_part", "indices");
		}
	}
	else if (!strcmp(definition->type_name, "struct structure_material"))
	{
		struct structure_material *material = (struct structure_material *)base;
		struct vertex_buffer const *vertices = &material->vertices;
		struct vertex_buffer const *lightmap_vertices = &material->lightmap_vertices;
		/* (its vertex data: each vertex, then each lightmap vertex, of the
		compressed types in the compressed data and the uncompressed types in
		the uncompressed) */
		struct
		{
			struct tag_data const *data;
			short type;
			short lightmap_type;
		} const datas[] =
		{
			{&material->compressed_vertex_data, _rasterizer_vertex_type_environment_compressed,
				_rasterizer_vertex_type_environment_lightmap_compressed},
			{&material->uncompressed_vertex_data, _rasterizer_vertex_type_environment_uncompressed,
				_rasterizer_vertex_type_environment_lightmap_uncompressed},
		};
		short index;

		for (index = 0; index < NUMBEROF(datas); index++)
		{
			struct tag_data const *data = datas[index].data;
			long vertex_count = vertices->count;
			long lightmap_count = lightmap_vertices->count;
			long size = data->size;

			if (size <= 0 || !data->address || !region_contains(region, data->address, (unsigned long)size))
				continue;
			/* (what of its counts the data holds) */
			vertex_count = PIN(vertex_count, 0, size / vertex_sizes[datas[index].type]);
			size -= vertex_count * vertex_sizes[datas[index].type];
			lightmap_count = PIN(lightmap_count, 0, size / vertex_sizes[datas[index].lightmap_type]);
			swap_vertices(region, data->address, vertex_count, datas[index].type);
			swap_vertices(region, (byte *)data->address + vertex_count * vertex_sizes[datas[index].type],
				lightmap_count, datas[index].lightmap_type);
		}
		/* (its buffers: the same bytes, or the bsp header's own) */
		if (vertices->hardware_format)
		{
			byte *data = buffer_data(region, vertices->hardware_format);

			if (!data || !swap_vertices(region, data, vertices->count, vertices->type))
				geometry_report("structure_material", "vertices");
		}
		if (lightmap_vertices->hardware_format && lightmap_vertices->count)
		{
			byte *data = buffer_data(region, lightmap_vertices->hardware_format);

			if (!data || !swap_vertices(region, data, lightmap_vertices->count, lightmap_vertices->type))
				geometry_report("structure_material", "lightmap vertices");
		}
	}

	return;
}

/* animations */

enum
{
	/* model_animations.c: a compressed animation's header (its offsets, up to
	its rotation node headers), and a node header's keyframe count bits (the
	first keyframe's index is above them) */
	COMPRESSED_ANIMATION_HEADER_LONGS = 11,
	KEYFRAME_COUNT_BITS = 12,
	/* a rotation: an uncompressed frame's (four shorts) and a compressed
	animation's (three) */
	ROTATION_SHORTS = 4,
	COMPRESSED_ROTATION_SHORTS = 3,
	TRANSLATION_REALS = 3,
};

/* count integers of size bytes at offset in the bytes from data to end,
each turned around unless it already was. FALSE if they are not all there */
static boolean swap_array(struct swap_region *region, byte *data, byte *end, long offset, long count, short size)
{
	byte *first = data + offset;
	long index;

	if (count <= 0)
		return TRUE;
	if (offset < 0 || offset > end - data || count > (end - first) / size ||
		!region_contains(region, first, (unsigned long)(count * size)))
		return FALSE;
	for (index = 0; index < count; index++)
	{
		if (mark(region, first + index * size, (unsigned long)size))
			swap_run(first + index * size, size, 1);
	}

	return TRUE;
}

/* whether the animation moves the node one way (its flags of that kind) */
static boolean node_flag(unsigned long const *flags, short node_index)
{
	return BIT_VECTOR_TEST_FLAG(flags, node_index) != 0;
}

/* a frame's (or the defaults') rotations, translations and scales, node by
node: those the flags name (want TRUE), or those they don't (FALSE). The
bytes it took, or NONE if they are not all there */
static long swap_animation_nodes(struct swap_region *region, struct animation const *animation, byte *data,
	byte *end, boolean want)
{
	byte *at = data;
	short node_index;

	for (node_index = 0; node_index < animation->node_count; node_index++)
	{
		if (node_flag(animation->nodes_with_rotation_flags, node_index) == want)
		{
			if (!swap_array(region, at, end, 0, ROTATION_SHORTS, 2))
				return NONE;
			at += ROTATION_SHORTS * 2;
		}
		if (node_flag(animation->nodes_with_translation_flags, node_index) == want)
		{
			if (!swap_array(region, at, end, 0, TRANSLATION_REALS, 4))
				return NONE;
			at += TRANSLATION_REALS * 4;
		}
		if (node_flag((unsigned long const *)animation->nodes_with_scale_flags, node_index) == want)
		{
			if (!swap_array(region, at, end, 0, 1, 4))
				return NONE;
			at += 4;
		}
	}

	return (long)(at - data);
}

/* a compressed animation's node headers (count of them at offset), turned
around, and how many keyframes they name (the most any reaches) */
static long swap_node_headers(struct swap_region *region, byte *data, byte *end, long offset, long count)
{
	long keyframe_total = 0;
	long index;

	if (!swap_array(region, data, end, offset, count, 4))
		return 0;
	for (index = 0; index < count; index++)
	{
		unsigned long header;
		long reach;

		memcpy(&header, data + offset + index * 4, 4);
		reach = (long)(header >> KEYFRAME_COUNT_BITS) + (long)(header & (FLAG(KEYFRAME_COUNT_BITS) - 1));
		if (reach > keyframe_total)
			keyframe_total = reach;
	}

	return keyframe_total;
}

/* a compressed animation's data, from its header at data (end is its data's
end): FALSE if a part of it is not there */
static boolean swap_compressed_animation(struct swap_region *region, struct animation const *animation, byte *data,
	byte *end, long rotation_count, long translation_count, long scale_count)
{
	long offsets[COMPRESSED_ANIMATION_HEADER_LONGS];
	long rotation_keyframes;
	long translation_keyframes;
	long scale_keyframes;
	boolean ok;

	if (!swap_array(region, data, end, 0, COMPRESSED_ANIMATION_HEADER_LONGS, 4))
		return FALSE;
	memcpy(offsets, data, sizeof(offsets));
	/* (the header's offsets, in its order: rotations' frame indices,
	defaults, keyframes; translations' node headers, frame indices,
	defaults, keyframes; scales' the same) */
	rotation_keyframes = swap_node_headers(region, data, end, COMPRESSED_ANIMATION_HEADER_LONGS * 4, rotation_count);
	translation_keyframes = swap_node_headers(region, data, end, offsets[3], translation_count);
	scale_keyframes = swap_node_headers(region, data, end, offsets[7], scale_count);
	ok = swap_array(region, data, end, offsets[0], rotation_keyframes, 2);
	ok = swap_array(region, data, end, offsets[1], animation->node_count * COMPRESSED_ROTATION_SHORTS, 2) && ok;
	ok = swap_array(region, data, end, offsets[2], rotation_keyframes * COMPRESSED_ROTATION_SHORTS, 2) && ok;
	ok = swap_array(region, data, end, offsets[4], translation_keyframes, 2) && ok;
	ok = swap_array(region, data, end, offsets[5], animation->node_count * TRANSLATION_REALS, 4) && ok;
	ok = swap_array(region, data, end, offsets[6], translation_keyframes * TRANSLATION_REALS, 4) && ok;
	ok = swap_array(region, data, end, offsets[8], scale_keyframes, 2) && ok;
	ok = swap_array(region, data, end, offsets[9], scale_count, 4) && ok;
	ok = swap_array(region, data, end, offsets[10], scale_keyframes, 4) && ok;

	return ok;
}

static long node_flag_count(unsigned long const *flags, short node_count)
{
	long count = 0;
	short node_index;

	for (node_index = 0; node_index < node_count; node_index++)
		count += node_flag(flags, node_index);

	return count;
}

/* an animation's frames, defaults and frame info, as it is turned around
(its counts and flags now this machine's) */
static void swap_animation(struct swap_region *region, struct animation *animation)
{
	byte *data = animation->data.address;
	long data_size = animation->data.size;
	byte *end;
	long uncompressed_size;
	long frame_index;
	boolean ok = TRUE;

	if (animation->node_count < 0 || animation->node_count > MAXIMUM_NODES_PER_ANIMATION)
		return;
	if (data && data_size > 0 && region_contains(region, data, (unsigned long)data_size))
	{
		end = data + data_size;
		uncompressed_size = data_size;
		if (TEST_FLAG(animation->flags, _animation_compressed_bit))
		{
			long rotation_count = node_flag_count(animation->nodes_with_rotation_flags, animation->node_count);
			long translation_count = node_flag_count(animation->nodes_with_translation_flags, animation->node_count);
			long scale_count = node_flag_count((unsigned long const *)animation->nodes_with_scale_flags,
				animation->node_count);

			if (animation->compressed_data_offset >= 0 && animation->compressed_data_offset < data_size)
			{
				ok = swap_compressed_animation(region, animation, data + animation->compressed_data_offset, end,
					rotation_count, translation_count, scale_count);
				uncompressed_size = animation->compressed_data_offset;
			}
			else
			{
				ok = FALSE;
			}
		}
		/* (the uncompressed frames, before any compressed data) */
		if (animation->frame_size > 0)
		{
			for (frame_index = 0; frame_index < animation->frame_count &&
				(frame_index + 1) * animation->frame_size <= uncompressed_size; frame_index++)
			{
				byte *frame = data + frame_index * animation->frame_size;

				if (swap_animation_nodes(region, animation, frame, frame + animation->frame_size, TRUE) !=
					animation->frame_size)
				{
					ok = FALSE;
					break;
				}
			}
		}
	}
	if (animation->default_data.address && animation->default_data.size > 0 &&
		region_contains(region, animation->default_data.address, (unsigned long)animation->default_data.size))
	{
		byte *defaults = animation->default_data.address;

		if (swap_animation_nodes(region, animation, defaults, defaults + animation->default_data.size, FALSE) == NONE)
			ok = FALSE;
	}
	if (animation->frame_info.address && animation->frame_info.size > 0)
	{
		byte *info = animation->frame_info.address;

		if (!swap_array(region, info, info + animation->frame_info.size, 0, animation->frame_info.size / 4, 4))
			ok = FALSE;
	}
	if (!ok)
		report("tag_byte_swap: animation '%.31s''s data is not all where it says: some left as read", animation->name);

	return;
}

/* the structure at base (its own integers and those of the structures in
it), then its blocks' elements */
static void swap_element(struct swap_region *region, byte *base, struct tag_schema_definition const *definition,
	short depth)
{
	if (!region_contains(region, base, (unsigned long)definition->size) ||
		!mark(region, base, (unsigned long)definition->size))
		return;
	if (swap_structure(region, base, definition, depth))
	{
		swap_geometry(region, base, definition);
		swap_children(region, base, definition, depth);
	}

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

				if (block->count <= 0 || !block->address ||
					(unsigned long)block->count > region->size / (unsigned long)element->size)
					break;
				for (element_index = 0; element_index < block->count && !region->failed; element_index++)
					swap_element(region, (byte *)block->address + element_index * element->size, element, depth + 1);
				break;
			}

			case _tag_schema_data:
			case _tag_schema_file_data:
				swap_data(region, (struct tag_data *)address, definition, field);
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
