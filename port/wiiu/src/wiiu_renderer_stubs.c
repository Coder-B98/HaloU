/*
WIIU_RENDERER_STUBS.C

The renderer's extras the Wii U build leaves out for now: the PC version's
anti-aliasing passes (xgpu_post.c), its high-resolution HUD, text and menu
art (hud_hires.c, text_hires.c, menu_files.c). Each says "none", and the
renderer draws the game's own art instead, as the Xbox did.
*/

#include "platform.h"
#include "xgpu.h"
#include "hud_hires.h"
#include "menu_files.h"
#include "text_hires.h"

BOOL xgpu_post_prepare(BOOL smaa)
{
	(void)smaa;
	return FALSE;
}

BOOL xgpu_post_anti_alias(BOOL smaa, GLuint framebuffer, unsigned long width, unsigned long height,
	const GLint corners[4])
{
	(void)smaa;
	(void)framebuffer;
	(void)width;
	(void)height;
	(void)corners;
	return FALSE;
}

long hud_hires_override_find(unsigned long address, unsigned long width, unsigned long height,
	unsigned long level0_size)
{
	(void)address;
	(void)width;
	(void)height;
	(void)level0_size;
	return -1;
}

unsigned int hud_hires_override_texture(long asset, unsigned long *levels)
{
	(void)asset;
	if (levels)
		*levels = 0;
	return 0;
}

int hud_hires_override_coverage(long asset)
{
	(void)asset;
	return 0;
}

unsigned int menu_art_texture(unsigned long data, unsigned long *levels)
{
	(void)data;
	if (levels)
		*levels = 0;
	return 0;
}

unsigned int text_hires_atlas_texture(unsigned long data)
{
	(void)data;
	return 0;
}
