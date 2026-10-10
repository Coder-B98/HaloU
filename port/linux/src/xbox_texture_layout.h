/*
XBOX_TEXTURE_LAYOUT.H

Where an Xbox texture's texels are in memory (xbox_texture_layout.c): what
creates, locks and draws textures shares it, with or without OpenGL.
*/

#ifndef __HALO_LINUX_XBOX_TEXTURE_LAYOUT_H
#define __HALO_LINUX_XBOX_TEXTURE_LAYOUT_H

#include "platform.h"

/* ---------- formats */

enum texel_kind
{
	_texel_unknown,
	_texel_a8r8g8b8, _texel_x8r8g8b8, _texel_r5g6b5, _texel_a1r5g5b5, _texel_x1r5g5b5, _texel_a4r4g4b4,
	_texel_l8, _texel_al8, _texel_a8, _texel_a8l8, _texel_p8, _texel_g8b8, _texel_r8b8, _texel_r6g5b5,
	_texel_l16, _texel_v16u16, _texel_a8b8g8r8, _texel_b8g8r8a8, _texel_r8g8b8a8, _texel_r5g5b5a1,
	_texel_r4g4b4a4, _texel_yuy2, _texel_uyvy, _texel_d24s8, _texel_d16,
	_texel_dxt1, _texel_dxt3, _texel_dxt5,
};

struct format_information
{
	unsigned char kind;
	unsigned char bytes; /* per texel; per 4x4 block for DXT */
	unsigned char linear;
};

/* a D3DFMT_*'s (an unknown one's are A8R8G8B8's) */
struct format_information xgpu_format_information(DWORD format);

/* ---------- textures */

struct xgpu_texture_description
{
	DWORD format;       /* D3DFMT_* */
	unsigned long width, height, depth, levels;
	BOOL cube_map;
	BOOL linear;        /* not swizzled; addressed with texel coordinates */
	BOOL compressed;
	unsigned long pitch; /* linear textures */
	BOOL hires;         /* a high-res HUD texture drawn in the texture's place (hud_hires.h) */
	BOOL hires_coverage; /* ... whose green is its coverage (a meter's) */
};

void xgpu_texture_describe(DWORD format_word, DWORD size_word, struct xgpu_texture_description *description);
/* bytes of one face, mip levels included (cube faces are padded) */
unsigned long xgpu_texture_face_size(const struct xgpu_texture_description *description);
unsigned long xgpu_texture_level_offset(const struct xgpu_texture_description *description, unsigned long level);
unsigned long xgpu_texture_level_pitch(const struct xgpu_texture_description *description, unsigned long level);
/* a level's width, height or depth (at least 1), and its bytes */
unsigned long xgpu_level_dimension(unsigned long base, unsigned long level);
unsigned long xgpu_level_bytes(const struct xgpu_texture_description *description, unsigned long level);
/* whether the size is one D3DDevice_GetDeviceCaps allows */
BOOL xgpu_texture_size_supported(const struct xgpu_texture_description *description);

/* ---------- surfaces (d3d8_resources.c) */

void d3d8_surface_initialize(D3DSurface *surface, D3DFORMAT format, unsigned long width, unsigned long height);
/* holds the new size (the back buffer, when the screen's width changes) */
void d3d8_surface_resize(D3DSurface *surface, D3DFORMAT format, unsigned long width, unsigned long height);

#endif
