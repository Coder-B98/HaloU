/*
TAG_BYTE_SWAP.H

A map's tags turned from the Xbox's byte order (little-endian) to a big-endian
machine's (the Wii U's), as they load and before the validator
(tag_validate.c) reads them. See tag_byte_swap.c.
*/

#ifndef __TAG_BYTE_SWAP_H
#define __TAG_BYTE_SWAP_H
#pragma once

/* ---------- structures */

/* count integers of size bytes (2, 4 or 8), one after another, at offset in a
structure */
struct tag_swap_run
{
	long offset;
	short size;
	short count;
};

/* where the integers are in a structure a tag schema names
(tools/tag_swap_codes.py writes them: tag_swap_codes.c) */
struct tag_swap_type
{
	char const *name;
	long size;
	long run_count;
	struct tag_swap_run const *runs;
};

/* ---------- globals */

extern struct tag_swap_type const tag_swap_types[];
extern long const tag_swap_type_count;

/* ---------- prototypes/TAG_BYTE_SWAP.C */

/* the cache file's header (its 0x800 bytes), as it is read */
void tag_byte_swap_cache_file_header(void *header);

/* the tags just read at tag_header (tag_data_size bytes): their header, their
table and every tag but the structure bsps (which load later); every pointer
in them is moved by relocation bytes (where the tag cache is, less where the
Xbox had it). FALSE if the tags cannot be turned around (a damaged header or
a schema without its structure's integers), which refuses the map */
boolean tag_byte_swap_tags(void *tag_header, long tag_data_size, long relocation);

/* a structure bsp just read at base (size bytes): its header and its tag */
boolean tag_byte_swap_structure_bsp(void *base, long size, long relocation);

/* how many tags of a group without a schema were left as they were read, and
how many tag data fields whose bytes have their own layout were (both are
logged once each as they are met) */
long tag_byte_swap_unswapped_tags(void);
long tag_byte_swap_unswapped_data(void);

#endif // __TAG_BYTE_SWAP_H
