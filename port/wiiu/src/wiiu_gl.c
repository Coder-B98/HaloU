/*
WIIU_GL.C

OpenGL ES 3.2 on the Wii U's GPU: as much of it as the renderer uses (the
list in port/linux/src/gl.h), drawn through GX2, the Wii U's own graphics
library.

The renderer (port/linux/src/d3d8_gl.c and the files around it) turns the
Xbox's Direct3D into OpenGL ES, as it does on Android. The Wii U has no
OpenGL, so this file plays its part: it keeps the state GL calls set (what
is bound, enabled, blended...) and, at each draw, hands GX2 the same state
in GX2's terms. Shaders are GLSL, compiled on the Wii U itself by CafeGLSL
(a Wii U port of Mesa's compiler) into the GPU's own code.

How GL's ideas map onto GX2's:
- A texture is a GX2Texture over memory of our own. Texels the renderer
  uploads (32-bit ARGB words) are laid out row by row (LINEAR_ALIGNED), so
  they can be copied in as they are; textures the GPU draws into (render
  targets) are left tiled, as the GPU likes them.
- A framebuffer is just the textures it names: binding one sets GX2's
  color and depth buffers at the next draw. Framebuffer 0, the screen, is a
  1280x720 color buffer that platform_video_swap copies to the TV and the
  GamePad.
- A buffer is a block of memory the GPU reads directly. The game's own
  memory (the Xbox's "contiguous" window) can be a buffer as it is
  (wiiu_gl_buffer_alias), so most vertices are never copied.
- Uniforms live in GX2's uniform registers; each program keeps a copy of
  its registers and loads them whenever it is drawn after another.
- Clears and framebuffer blits are drawn as rectangles by small programs
  of this file's own (GX2's own clear calls ignore the scissor and masks).
- Occlusion queries always say "visible": GX2 has none that wut reaches.

Everything here runs on the game's thread, which wiiu_main.c keeps on one
CPU core: GX2 must be called from the core it was started on.

(No platform.h here: wut's headers and the XDK's both declare BOOL.)
*/

#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/time.h>
#include <gx2/clear.h>
#include <gx2/context.h>
#include <gx2/display.h>
#include <gx2/draw.h>
#include <gx2/enum.h>
#include <gx2/event.h>
#include <gx2/mem.h>
#include <gx2/registers.h>
#include <gx2/sampler.h>
#include <gx2/shaders.h>
#include <gx2/state.h>
#include <gx2/surface.h>
#include <gx2/swap.h>
#include <gx2/texture.h>
#include <gx2/utils.h>
#include <whb/gfx.h>
#include <cafeglsl/CafeGLSLCompiler.h>
#include <coreinit/thread.h>

#include <ctype.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the renderer's list of GL functions; with GL_FUNCTIONS_DEFINE, gl.h
declares them without renaming the calls */
#define GL_FUNCTIONS_DEFINE
#include "gl.h"

/* the game's log (port/linux/src/platform.h) */
void platform_log(const char *format, ...) __attribute__((format(printf, 1, 2)));
/* (port/linux/src/port_config.h) */
long config_integer(const char *name);

/* the screen: what the renderer is told it draws into (the game's picture
is letterboxed into it at presentation) */
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

/* attribute locations (XGPU_VERTEX_ATTRIBUTE_COUNT) and texture units the
renderer uses */
#define ATTRIBUTE_COUNT 16
#define TEXTURE_UNIT_COUNT 8

/* ---------- logging, once per kind of problem */

static void log_once(int *said, const char *format, ...) __attribute__((format(printf, 2, 3)));

static void log_once(int *said, const char *format, ...)
{
	char text[512];
	va_list arguments;

	if (*said)
		return;
	*said = 1;
	va_start(arguments, format);
	vsnprintf(text, sizeof(text), format, arguments);
	va_end(arguments);
	platform_log("GX2: %s", text);
}

/* ---------- memory the GPU reads

Memory handed to the GPU stays in use until the GPU has run the commands
that read it, which can be a frame or two after the CPU queued them. Freed
blocks therefore wait in a list until GX2 says it has finished everything
queued up to their release (GX2GetRetiredTimeStamp). */

struct deferred_free
{
	struct deferred_free *next;
	void *memory;
	/* the timestamp of the commands queued before the release: 0 until the
	frame they are in has been sent to the GPU */
	OSTime after;
};

static struct deferred_free *pending_frees;

static void *gpu_alloc(unsigned long alignment, unsigned long size)
{
	void *memory = memalign(alignment < 0x40 ? 0x40 : alignment, size ? size : 1);

	if (!memory)
		platform_log("GX2: out of memory for %lu bytes", size);
	return memory;
}

static void gpu_free(void *memory)
{
	struct deferred_free *entry;

	if (!memory)
		return;
	entry = malloc(sizeof(*entry));
	if (!entry)
		return;
	entry->memory = memory;
	entry->after = 0;
	entry->next = pending_frees;
	pending_frees = entry;
}

/* after a frame is sent: what was released during it waits for its
timestamp, and what the GPU is done with is freed */
static void gpu_free_collect(void)
{
	OSTime submitted = GX2GetLastSubmittedTimeStamp(), retired = GX2GetRetiredTimeStamp();
	struct deferred_free **link = &pending_frees, *entry;

	while ((entry = *link) != NULL)
	{
		if (!entry->after)
			entry->after = submitted;
		if (entry->after <= retired)
		{
			*link = entry->next;
			free(entry->memory);
			free(entry);
		}
		else
		{
			link = &entry->next;
		}
	}
}

/* ---------- object names

GL names objects with numbers from 1; each kind of object has a table of
pointers indexed by name. */

struct name_table
{
	void **objects;
	GLuint capacity;
	GLuint next;
};

static void *name_get(const struct name_table *table, GLuint name)
{
	return name && name < table->capacity ? table->objects[name] : NULL;
}

static GLuint name_new(struct name_table *table, void *object)
{
	GLuint name;

	if (!table->next)
		table->next = 1;
	name = table->next++;
	if (name >= table->capacity)
	{
		GLuint capacity = table->capacity ? table->capacity * 2 : 256;
		void **objects = realloc(table->objects, capacity * sizeof(void *));

		if (!objects)
			return 0;
		memset(objects + table->capacity, 0, (capacity - table->capacity) * sizeof(void *));
		table->objects = objects;
		table->capacity = capacity;
	}
	table->objects[name] = object;
	return name;
}

static void name_delete(struct name_table *table, GLuint name)
{
	if (name && name < table->capacity)
		table->objects[name] = NULL;
}

/* ---------- objects */

struct gl_buffer
{
	unsigned char *data;
	unsigned long size;
	/* data is someone else's memory (wiiu_gl_buffer_alias), not ours to free */
	int alias;
};

/* a sampler's state, as GL sets it on a sampler object or a texture */
struct sampler_state
{
	GLint min_filter, mag_filter;
	GLint wrap[3];
	float min_lod, max_lod;
	float anisotropy;
	float border[4];
	/* the GX2 sampler made from it, and whether it is out of date */
	GX2Sampler sampler;
	int dirty;
};

struct gl_texture
{
	/* GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP or GL_TEXTURE_3D, once bound */
	GLenum target;
	int allocated;
	/* drawn into by the GPU (tiled, a render target) rather than written by
	the CPU (linear) */
	int render;
	/* a depth buffer, not a color texture */
	int depth;
	GX2Texture texture;
	GX2DepthBuffer depth_buffer;
	/* a color buffer for each mip level drawn into, made when first needed */
	GX2ColorBuffer *color_views[16];
	/* how the texels in memory map onto red, green, blue and alpha before
	the GL swizzle: ARGB words for the renderer's uploads */
	uint32_t base_map;
	GLint swizzle[4];
	GLint base_level, max_level;
	struct sampler_state sampler;
	/* drawn into since it was last sampled: the GPU's caches must be
	written back first */
	int gpu_written;
	/* GX2InitTextureRegs is due (the swizzle changed) */
	int regs_dirty;
};

struct gl_framebuffer
{
	GLuint color;
	GLint color_level;
	GLuint depth;
};

struct gl_shader
{
	GLenum type;
	char *source;
	int compiled;
	char log[1024];
	GX2VertexShader *vertex;
	GX2PixelShader *pixel;
};

/* a uniform of a program: where it is in each stage's registers (-1: not
in that stage), and how many array elements it has */
struct uniform_entry
{
	char name[48];
	int vertex_offset;
	int pixel_offset;
	unsigned int count;
	/* a sampler: which texture unit it reads (glUniform1i), and the GX2
	sampler slot it is in the pixel shader, and its kind */
	int sampler;
	int unit;
	unsigned int slot;
	GX2SamplerVarType sampler_type;
};

struct gl_program
{
	GLuint vertex_name, pixel_name;
	int linked;
	char log[1024];
	GX2VertexShader *vertex;
	GX2PixelShader *pixel;
	struct uniform_entry *uniforms;
	unsigned int uniform_count;
	/* each stage's uniform registers as the program has them, in floats,
	and the range changed since they were last loaded */
	float *vertex_registers, *pixel_registers;
	unsigned int vertex_register_count, pixel_register_count;
	unsigned int vertex_dirty_first, vertex_dirty_last;
	unsigned int pixel_dirty_first, pixel_dirty_last;
};

/* a vertex attribute's array, or its current value when the array is off */
struct attribute
{
	int enabled;
	GLuint buffer;
	GLint size;
	GLenum type;
	int normalized;
	int integer;
	GLsizei stride;
	unsigned long offset;
	/* the value read when disabled: floats, or (integer_value) unsigned ints */
	int integer_value;
	float value[4];
	GLuint value_integer[4];
};

static struct name_table buffers, textures, framebuffers, samplers, shaders, programs;
static GLuint plain_names; /* vertex arrays, queries, renderbuffers */

/* ---------- GL state */

enum
{
	_dirty_targets = 1 << 0,
	_dirty_viewport = 1 << 1,
	_dirty_scissor = 1 << 2,
	_dirty_depth_stencil = 1 << 3,
	_dirty_blend = 1 << 4,
	_dirty_color_mask = 1 << 5,
	_dirty_polygon = 1 << 6,
	_dirty_program = 1 << 7,
	_dirty_all = 0xffff
};

static struct
{
	unsigned int dirty;
	int blend, cull_face, depth_test, stencil_test, scissor_test, polygon_offset_fill;
	GLint viewport[4];
	GLint scissor[4];
	float depth_range[2];
	float clear_color[4];
	float clear_depth;
	GLint clear_stencil;
	GLboolean color_mask[4];
	GLboolean depth_mask;
	GLenum depth_function;
	GLenum stencil_function;
	GLint stencil_reference;
	GLuint stencil_value_mask, stencil_write_mask;
	GLenum stencil_operations[3];
	GLenum blend_source, blend_destination, blend_equation;
	float blend_color[4];
	GLenum cull_mode, front_face;
	float polygon_offset[2];

	GLuint active_unit;
	/* per unit: the 2D, cube map and 3D textures bound */
	GLuint unit_textures[TEXTURE_UNIT_COUNT][3];
	GLuint unit_samplers[TEXTURE_UNIT_COUNT];
	GLuint array_buffer, element_buffer, copy_write_buffer, other_buffer;
	GLuint draw_framebuffer, read_framebuffer;
	GLuint program;
	struct attribute attributes[ATTRIBUTE_COUNT];
	GLenum error;
} gl;

/* the program whose registers GX2 has (its uniforms need loading in full
when another program is drawn) */
static struct gl_program *registers_program;

static void set_error(GLenum error)
{
	if (gl.error == GL_NO_ERROR)
		gl.error = error;
}

/* ---------- the screen and GX2 itself */

static GX2ContextState *context_state;
static GX2ColorBuffer screen;
static int video_ready, screen_shown;

/* the commands queued for the frames in flight, by d3d8_gl.c's ring slot
(host_gl_fence_frame) */
#define FENCE_SLOTS 4
static OSTime frame_fences[FENCE_SLOTS];

/* vertex values of disabled attributes: a few kilobytes a frame, in a ring
of per-frame parts that are reused only once their frame is done */
#define CONSTANT_RING_PART 0x40000
static unsigned char *constant_ring;
static unsigned long constant_ring_part, constant_ring_offset;

static void internal_programs_initialize(void);

/* ---------- texture helpers */

static int texture_slot(GLenum target)
{
	return target == GL_TEXTURE_CUBE_MAP ? 1 : target == GL_TEXTURE_3D ? 2 : 0;
}

static GLenum texture_binding_target(GLenum target)
{
	if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
		return GL_TEXTURE_CUBE_MAP;
	return target;
}

static struct gl_texture *bound_texture(GLenum target)
{
	GLenum binding = texture_binding_target(target);

	if (gl.active_unit >= TEXTURE_UNIT_COUNT)
		return NULL;
	return name_get(&textures, gl.unit_textures[gl.active_unit][texture_slot(binding)]);
}

static void sampler_state_default(struct sampler_state *state)
{
	state->min_filter = GL_NEAREST_MIPMAP_LINEAR;
	state->mag_filter = GL_LINEAR;
	state->wrap[0] = state->wrap[1] = state->wrap[2] = GL_REPEAT;
	state->min_lod = -1000.0f;
	state->max_lod = 1000.0f;
	state->anisotropy = 1.0f;
	memset(state->border, 0, sizeof(state->border));
	state->dirty = 1;
}

static GX2TexClampMode clamp_mode(GLint wrap)
{
	switch (wrap)
	{
	case GL_MIRRORED_REPEAT: return GX2_TEX_CLAMP_MODE_MIRROR;
	case GL_CLAMP_TO_EDGE: return GX2_TEX_CLAMP_MODE_CLAMP;
	case GL_CLAMP_TO_BORDER: return GX2_TEX_CLAMP_MODE_CLAMP_BORDER;
	default: return GX2_TEX_CLAMP_MODE_WRAP;
	}
}

/* the GX2 sampler for a sampler state. GX2's border colors are only black,
white or transparent black (wut has no call for others): the nearest. */
static GX2Sampler *sampler_state_get(struct sampler_state *state)
{
	GX2TexXYFilterMode minification, magnification;
	GX2TexMipFilterMode mip;
	GX2TexAnisoRatio ratio;
	GX2TexBorderType border;
	float lightness;

	if (!state->dirty)
		return &state->sampler;
	state->dirty = 0;
	magnification = state->mag_filter == GL_NEAREST ? GX2_TEX_XY_FILTER_MODE_POINT : GX2_TEX_XY_FILTER_MODE_LINEAR;
	minification = state->min_filter == GL_NEAREST || state->min_filter == GL_NEAREST_MIPMAP_NEAREST ||
		state->min_filter == GL_NEAREST_MIPMAP_LINEAR ? GX2_TEX_XY_FILTER_MODE_POINT : GX2_TEX_XY_FILTER_MODE_LINEAR;
	mip = state->min_filter == GL_NEAREST || state->min_filter == GL_LINEAR ? GX2_TEX_MIP_FILTER_MODE_NONE :
		state->min_filter == GL_NEAREST_MIPMAP_NEAREST || state->min_filter == GL_LINEAR_MIPMAP_NEAREST ?
		GX2_TEX_MIP_FILTER_MODE_POINT : GX2_TEX_MIP_FILTER_MODE_LINEAR;
	ratio = state->anisotropy >= 16.0f ? GX2_TEX_ANISO_RATIO_16_TO_1 : state->anisotropy >= 8.0f ?
		GX2_TEX_ANISO_RATIO_8_TO_1 : state->anisotropy >= 4.0f ? GX2_TEX_ANISO_RATIO_4_TO_1 :
		state->anisotropy >= 2.0f ? GX2_TEX_ANISO_RATIO_2_TO_1 : GX2_TEX_ANISO_RATIO_NONE;
	GX2InitSampler(&state->sampler, GX2_TEX_CLAMP_MODE_WRAP, GX2_TEX_XY_FILTER_MODE_LINEAR);
	GX2InitSamplerClamping(&state->sampler, clamp_mode(state->wrap[0]), clamp_mode(state->wrap[1]),
		clamp_mode(state->wrap[2]));
	GX2InitSamplerXYFilter(&state->sampler, magnification, minification, ratio);
	GX2InitSamplerZMFilter(&state->sampler,
		minification == GX2_TEX_XY_FILTER_MODE_POINT ? GX2_TEX_Z_FILTER_MODE_POINT : GX2_TEX_Z_FILTER_MODE_LINEAR, mip);
	GX2InitSamplerLOD(&state->sampler, state->min_lod < 0.0f ? 0.0f : state->min_lod,
		state->max_lod > 13.0f ? 13.0f : state->max_lod, 0.0f);
	lightness = (state->border[0] + state->border[1] + state->border[2]) / 3.0f;
	border = state->border[3] < 0.5f ? GX2_TEX_BORDER_TYPE_TRANSPARENT_BLACK :
		lightness >= 0.5f ? GX2_TEX_BORDER_TYPE_WHITE : GX2_TEX_BORDER_TYPE_BLACK;
	GX2InitSamplerBorderType(&state->sampler, border);
	return &state->sampler;
}

/* GL's texture swizzle, on top of how the texels are stored */
static uint32_t texture_component_map(const struct gl_texture *texture)
{
	uint32_t map = 0;
	int channel;

	for (channel = 0; channel < 4; channel++)
	{
		GLint source = texture->swizzle[channel];
		uint32_t select;

		if (source >= GL_RED && source <= GL_ALPHA)
			select = (texture->base_map >> (24 - 8 * (source - GL_RED))) & 0xff;
		else if (source == GL_ZERO)
			select = GX2_SQ_SEL_0;
		else
			select = GX2_SQ_SEL_1;
		map |= select << (24 - 8 * channel);
	}
	return map;
}

static void texture_free_storage(struct gl_texture *texture)
{
	int level;

	for (level = 0; level < 16; level++)
	{
		free(texture->color_views[level]);
		texture->color_views[level] = NULL;
	}
	if (texture->allocated)
	{
		if (texture->depth)
		{
			gpu_free(texture->depth_buffer.surface.image);
		}
		else
		{
			gpu_free(texture->texture.surface.image);
			gpu_free(texture->texture.surface.mipmaps);
		}
	}
	texture->allocated = 0;
	memset(&texture->texture, 0, sizeof(texture->texture));
	memset(&texture->depth_buffer, 0, sizeof(texture->depth_buffer));
	/* a framebuffer may hold its old color buffers */
	gl.dirty |= _dirty_targets;
}

static unsigned int full_mip_count(unsigned int width, unsigned int height, unsigned int depth)
{
	unsigned int largest = width > height ? width : height, levels = 1;

	if (depth > largest)
		largest = depth;
	while (largest > 1)
	{
		largest >>= 1;
		levels++;
	}
	return levels;
}

/* gives a texture storage of this size; FALSE if there is no memory */
static int texture_allocate(struct gl_texture *texture, unsigned int width, unsigned int height,
	unsigned int depth, int render)
{
	GX2Surface *surface = &texture->texture.surface;
	unsigned int levels = full_mip_count(width, height, texture->target == GL_TEXTURE_3D ? depth : 1);

	if (texture->max_level + 1 < (GLint)levels)
		levels = texture->max_level < 0 ? 1 : (unsigned int)texture->max_level + 1;
	if (levels > 13)
		levels = 13;
	texture_free_storage(texture);
	texture->depth = 0;
	texture->render = render;
	surface->dim = texture->target == GL_TEXTURE_CUBE_MAP ? GX2_SURFACE_DIM_TEXTURE_CUBE :
		texture->target == GL_TEXTURE_3D ? GX2_SURFACE_DIM_TEXTURE_3D : GX2_SURFACE_DIM_TEXTURE_2D;
	surface->width = width;
	surface->height = height;
	surface->depth = depth;
	surface->mipLevels = levels;
	surface->format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
	surface->aa = GX2_AA_MODE1X;
	surface->use = render ? (GX2SurfaceUse)(GX2_SURFACE_USE_TEXTURE | GX2_SURFACE_USE_COLOR_BUFFER) :
		GX2_SURFACE_USE_TEXTURE;
	surface->tileMode = render ? GX2_TILE_MODE_DEFAULT : GX2_TILE_MODE_LINEAR_ALIGNED;
	surface->swizzle = 0;
	GX2CalcSurfaceSizeAndAlignment(surface);
	surface->image = gpu_alloc(surface->alignment, surface->imageSize);
	surface->mipmaps = surface->mipmapSize ? gpu_alloc(surface->alignment, surface->mipmapSize) : NULL;
	if (!surface->image || (surface->mipmapSize && !surface->mipmaps))
	{
		free(surface->image);
		free(surface->mipmaps);
		memset(surface, 0, sizeof(*surface));
		return 0;
	}
	if (render)
	{
		/* (a render target starts black, as a new GL texture reads) */
		memset(surface->image, 0, surface->imageSize);
		GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, surface->image, surface->imageSize);
	}
	texture->texture.viewFirstMip = 0;
	texture->texture.viewNumMips = levels;
	texture->texture.viewFirstSlice = 0;
	texture->texture.viewNumSlices = texture->target == GL_TEXTURE_CUBE_MAP ? 6 : 1;
	texture->allocated = 1;
	texture->regs_dirty = 1;
	return 1;
}

static int texture_allocate_depth(struct gl_texture *texture, unsigned int width, unsigned int height)
{
	GX2Surface *surface = &texture->depth_buffer.surface;

	texture_free_storage(texture);
	texture->depth = 1;
	texture->render = 1;
	surface->dim = GX2_SURFACE_DIM_TEXTURE_2D;
	surface->width = width;
	surface->height = height;
	surface->depth = 1;
	surface->mipLevels = 1;
	surface->format = GX2_SURFACE_FORMAT_UNORM_R24_X8;
	surface->aa = GX2_AA_MODE1X;
	surface->use = GX2_SURFACE_USE_DEPTH_BUFFER;
	surface->tileMode = GX2_TILE_MODE_DEFAULT;
	surface->swizzle = 0;
	GX2CalcSurfaceSizeAndAlignment(surface);
	surface->image = gpu_alloc(surface->alignment, surface->imageSize);
	if (!surface->image)
		return 0;
	texture->depth_buffer.viewMip = 0;
	texture->depth_buffer.viewFirstSlice = 0;
	texture->depth_buffer.viewNumSlices = 1;
	texture->depth_buffer.depthClear = 1.0f;
	texture->depth_buffer.stencilClear = 0;
	texture->depth_buffer.hiZPtr = NULL;
	texture->depth_buffer.hiZSize = 0;
	GX2InitDepthBufferRegs(&texture->depth_buffer);
	GX2InitDepthBufferHiZEnable(&texture->depth_buffer, FALSE);
	texture->allocated = 1;
	return 1;
}

/* where a mip level's texels are: its first byte, the bytes from one row
to the next, and from one slice (cube face, 3D layer) to the next */
static unsigned char *texture_level_layout(const GX2Surface *surface, unsigned int level,
	unsigned long *row_bytes, unsigned long *slice_bytes)
{
	unsigned char *base;
	unsigned long level_bytes, next;
	unsigned int slices = surface->dim == GX2_SURFACE_DIM_TEXTURE_CUBE ? 6 :
		surface->dim == GX2_SURFACE_DIM_TEXTURE_3D ? (surface->depth >> level ? surface->depth >> level : 1) : 1;
	GX2Surface level_surface;

	/* level 0 is the image; level 1 starts the mipmaps, and level n after
	that is mipLevelOffset[n - 1] into them */
	if (level == 0)
	{
		base = surface->image;
		level_bytes = surface->imageSize;
	}
	else
	{
		unsigned long offset = level == 1 ? 0 : surface->mipLevelOffset[level - 1];

		next = level + 1 < surface->mipLevels ? surface->mipLevelOffset[level] : surface->mipmapSize;
		base = (unsigned char *)surface->mipmaps + offset;
		level_bytes = next - offset;
	}
	/* the level's pitch, as GX2 lays out a texture of that size by itself */
	memset(&level_surface, 0, sizeof(level_surface));
	level_surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
	level_surface.width = surface->width >> level ? surface->width >> level : 1;
	level_surface.height = surface->height >> level ? surface->height >> level : 1;
	level_surface.depth = 1;
	level_surface.mipLevels = 1;
	level_surface.format = surface->format;
	level_surface.aa = GX2_AA_MODE1X;
	level_surface.use = GX2_SURFACE_USE_TEXTURE;
	level_surface.tileMode = surface->tileMode;
	GX2CalcSurfaceSizeAndAlignment(&level_surface);
	*row_bytes = level_surface.pitch * 4;
	*slice_bytes = level_bytes / slices;
	return base;
}

/* a texture's GX2 registers, brought up to date */
static GX2Texture *texture_for_sampling(struct gl_texture *texture)
{
	if (texture->regs_dirty)
	{
		texture->regs_dirty = 0;
		texture->texture.compMap = texture_component_map(texture);
		GX2InitTextureRegs(&texture->texture);
	}
	if (texture->gpu_written)
	{
		/* what the GPU drew is written back from its color cache and the
		texture cache forgets the old texels */
		texture->gpu_written = 0;
		GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_COLOR_BUFFER | GX2_INVALIDATE_MODE_TEXTURE),
			texture->texture.surface.image, texture->texture.surface.imageSize);
		if (texture->texture.surface.mipmaps)
			GX2Invalidate((GX2InvalidateMode)(GX2_INVALIDATE_MODE_COLOR_BUFFER | GX2_INVALIDATE_MODE_TEXTURE),
				texture->texture.surface.mipmaps, texture->texture.surface.mipmapSize);
	}
	return &texture->texture;
}

/* the color buffer that draws into a mip level of a texture */
static GX2ColorBuffer *texture_color_view(struct gl_texture *texture, unsigned int level)
{
	GX2ColorBuffer *view;

	if (!texture->allocated || texture->depth || level >= 16 || level >= texture->texture.surface.mipLevels)
		return NULL;
	if (texture->color_views[level])
		return texture->color_views[level];
	if (!texture->render)
	{
		static int said;

		log_once(&said, "a texture uploaded by the CPU is drawn into (it was not made a render target)");
	}
	view = calloc(1, sizeof(*view));
	if (!view)
		return NULL;
	view->surface = texture->texture.surface;
	view->viewMip = level;
	view->viewFirstSlice = 0;
	view->viewNumSlices = 1;
	GX2InitColorBufferRegs(view);
	texture->color_views[level] = view;
	return view;
}

/* ---------- the internal programs: rectangles for clears and blits */

struct internal_program
{
	GX2VertexShader *vertex;
	GX2PixelShader *pixel;
	GX2FetchShader fetch;
	/* the uniforms' register offsets (vertex: rectangle, source; pixel:
	color), and the blit's sampler slot */
	int rectangle, source, depth, color;
	unsigned int sampler_slot;
};

static struct internal_program clear_program, blit_program;
static int internal_ready;

/* the rectangle (corners in clip space: x0, y0, x1, y1) drawn from four
vertices numbered 0 to 3, with no vertex data */
static const char internal_vertex_clear[] =
	"#version 310 es\n"
	"precision highp float;\n"
	"uniform vec4 rectangle;\n"
	"uniform vec4 depth;\n"
	"void main()\n"
	"{\n"
	"\tint corner = gl_VertexID;\n"
	"\tgl_Position = vec4((corner & 1) != 0 ? rectangle.z : rectangle.x,\n"
	"\t\t(corner & 2) != 0 ? rectangle.w : rectangle.y, depth.x, 1.0);\n"
	"}\n";

static const char internal_pixel_clear[] =
	"#version 310 es\n"
	"precision highp float;\n"
	"uniform vec4 color;\n"
	"layout(location = 0) out vec4 result;\n"
	"void main()\n"
	"{\n"
	"\tresult = color;\n"
	"}\n";

static const char internal_vertex_blit[] =
	"#version 310 es\n"
	"precision highp float;\n"
	"uniform vec4 rectangle;\n"
	"uniform vec4 source;\n"
	"layout(location = 0) out vec2 coordinates;\n"
	"void main()\n"
	"{\n"
	"\tint corner = gl_VertexID;\n"
	"\tgl_Position = vec4((corner & 1) != 0 ? rectangle.z : rectangle.x,\n"
	"\t\t(corner & 2) != 0 ? rectangle.w : rectangle.y, 0.0, 1.0);\n"
	"\tcoordinates = vec2((corner & 1) != 0 ? source.z : source.x, (corner & 2) != 0 ? source.w : source.y);\n"
	"}\n";

static const char internal_pixel_blit[] =
	"#version 310 es\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"uniform sampler2D image;\n"
	"layout(location = 0) in vec2 coordinates;\n"
	"layout(location = 0) out vec4 result;\n"
	"void main()\n"
	"{\n"
	"\tresult = texture(image, coordinates);\n"
	"}\n";

static int uniform_offset(const GX2UniformVar *variables, unsigned int count, const char *name)
{
	unsigned int index;

	for (index = 0; index < count; index++)
	{
		if (!strcmp(variables[index].name, name))
			return (int)variables[index].offset;
	}
	return -1;
}

static int fetch_shader_make(GX2FetchShader *fetch, GX2AttribStream *streams, unsigned int count)
{
	uint32_t size = GX2CalcFetchShaderSizeEx(count, GX2_FETCH_SHADER_TESSELLATION_NONE,
		GX2_TESSELLATION_MODE_DISCRETE);
	void *program = gpu_alloc(GX2_SHADER_PROGRAM_ALIGNMENT, size);

	if (!program)
		return 0;
	GX2InitFetchShaderEx(fetch, program, count, streams, GX2_FETCH_SHADER_TESSELLATION_NONE,
		GX2_TESSELLATION_MODE_DISCRETE);
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, program, size);
	return 1;
}

/* the free memory, in the log: the shader compiler is large, and running
out inside it would look like a crash with no message. (wut gives newlib's
malloc all of the system's heap at start-up, so the system's own count of
free memory reads 0: newlib's is the one that counts.) The largest block is
found by asking for less and less, each block given back at once. */
void wiiu_alloc_quiet(int quiet);

static void log_free_memory(const char *when)
{
	struct mallinfo info = mallinfo();
	unsigned long low = 0, high = 1024UL * 1024 * 1024;

	wiiu_alloc_quiet(1);
	while (high - low > 64 * 1024)
	{
		unsigned long middle = low + (high - low) / 2;
		void *block = malloc(middle);

		if (block)
		{
			free(block);
			low = middle;
		}
		else
			high = middle;
	}
	wiiu_alloc_quiet(0);
	platform_log("GX2: memory %s: heap %lu KB, %lu KB in use, %lu KB free in it; largest block now %lu KB",
		when, (unsigned long)info.arena / 1024, (unsigned long)info.uordblks / 1024,
		(unsigned long)info.fordblks / 1024, low / 1024);
}

/* the stack of the thread that asks for a compile: the compiler goes deep */
static void log_stack(const char *when)
{
	OSThread *thread = OSGetCurrentThread();
	char here;

	platform_log("GX2: stack %s: thread %p (%s), %lu KB left of %lu KB", when, (void *)thread,
		OSGetThreadName(thread) ? OSGetThreadName(thread) : "no name",
		(unsigned long)((char *)&here - (char *)thread->stackEnd) / 1024,
		(unsigned long)((char *)thread->stackStart - (char *)thread->stackEnd) / 1024);
}

/* the shader compiler, started once, by the first to need it */
static void compiler_start(void)
{
	static int started;

	if (started)
		return;
	started = 1;
	InitGLSLCompiler();
	platform_log("GX2: shader compiler %s", GetGLSLCompilerVersion());
}

static int internal_program_make(struct internal_program *program, const char *vertex_source,
	const char *pixel_source, const char *what)
{
	char log[1024];

	log_free_memory("before compiling");
	log_stack("before compiling");
	platform_log("GX2: compiling the %s program", what);
	log[0] = 0;
	if (CompileShaderPair(vertex_source, pixel_source, GLSL_COMPILE_REGISTER, &program->vertex, &program->pixel,
		log, sizeof(log), GLSL_COMPILER_FLAG_NONE) == GLSL_SHADER_MODE_ERROR)
	{
		platform_log("GX2: cannot compile the %s program: %s", what, log);
		return 0;
	}
	platform_log("GX2: compiled the %s program (%u + %u bytes)", what, (unsigned)program->vertex->size,
		(unsigned)program->pixel->size);
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, program->vertex->program, program->vertex->size);
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, program->pixel->program, program->pixel->size);
	program->rectangle = uniform_offset(program->vertex->uniformVars, program->vertex->uniformVarCount, "rectangle");
	program->source = uniform_offset(program->vertex->uniformVars, program->vertex->uniformVarCount, "source");
	program->depth = uniform_offset(program->vertex->uniformVars, program->vertex->uniformVarCount, "depth");
	program->color = uniform_offset(program->pixel->uniformVars, program->pixel->uniformVarCount, "color");
	program->sampler_slot = program->pixel->samplerVarCount ? program->pixel->samplerVars[0].location : 0;
	return fetch_shader_make(&program->fetch, NULL, 0);
}

static void internal_programs_initialize(void)
{
	internal_ready = internal_program_make(&clear_program, internal_vertex_clear, internal_pixel_clear, "clear") &&
		internal_program_make(&blit_program, internal_vertex_blit, internal_pixel_blit, "blit");
}

/* ---------- framebuffers: what a draw draws into */

struct draw_targets
{
	GX2ColorBuffer *color;
	struct gl_texture *color_texture;
	GX2DepthBuffer *depth;
	unsigned int width, height;
	/* framebuffer 0: GL's rows run from the bottom */
	int screen;
};

static int targets_get(GLuint name, struct draw_targets *targets)
{
	struct gl_framebuffer *framebuffer;

	memset(targets, 0, sizeof(*targets));
	if (!name)
	{
		targets->color = &screen;
		targets->width = SCREEN_WIDTH;
		targets->height = SCREEN_HEIGHT;
		targets->screen = 1;
		return 1;
	}
	framebuffer = name_get(&framebuffers, name);
	if (!framebuffer)
		return 0;
	if (framebuffer->color)
	{
		struct gl_texture *texture = name_get(&textures, framebuffer->color);

		if (texture && texture->allocated && !texture->depth)
		{
			targets->color = texture_color_view(texture, (unsigned int)framebuffer->color_level);
			targets->color_texture = texture;
			if (targets->color)
			{
				targets->width = targets->color->surface.width >> framebuffer->color_level;
				targets->height = targets->color->surface.height >> framebuffer->color_level;
				if (!targets->width)
					targets->width = 1;
				if (!targets->height)
					targets->height = 1;
			}
		}
	}
	if (framebuffer->depth)
	{
		struct gl_texture *texture = name_get(&textures, framebuffer->depth);

		if (texture && texture->allocated && texture->depth)
		{
			targets->depth = &texture->depth_buffer;
			if (!targets->color)
			{
				targets->width = texture->depth_buffer.surface.width;
				targets->height = texture->depth_buffer.surface.height;
			}
		}
	}
	return targets->color || targets->depth;
}

/* GL's window rectangle (x, y from the bottom for the screen, from row 0
otherwise) as GX2's (rows from the top), clamped to the target */
static void window_rectangle(const struct draw_targets *targets, GLint x, GLint y, GLint width, GLint height,
	uint32_t out[4])
{
	long x0 = x, y0 = targets->screen ? (long)targets->height - (y + height) : y;
	long x1 = x0 + width, y1 = y0 + height;

	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 > (long)targets->width)
		x1 = (long)targets->width;
	if (y1 > (long)targets->height)
		y1 = (long)targets->height;
	if (x1 < x0)
		x1 = x0;
	if (y1 < y0)
		y1 = y0;
	out[0] = (uint32_t)x0;
	out[1] = (uint32_t)y0;
	out[2] = (uint32_t)(x1 - x0);
	out[3] = (uint32_t)(y1 - y0);
}

/* the color and depth buffers GX2 last had */
static GX2ColorBuffer *current_color;
static GX2DepthBuffer *current_depth;

static void targets_bind(const struct draw_targets *targets)
{
	if (targets->color && targets->color != current_color)
	{
		current_color = targets->color;
		GX2SetColorBuffer(targets->color, GX2_RENDER_TARGET_0);
	}
	if (targets->depth && targets->depth != current_depth)
	{
		current_depth = targets->depth;
		GX2SetDepthBuffer(targets->depth);
	}
	if (targets->color_texture)
		targets->color_texture->gpu_written = 1;
}

/* ---------- uniform registers */

static void registers_mark(struct gl_program *program, int pixel, unsigned int first, unsigned int count)
{
	unsigned int *dirty_first = pixel ? &program->pixel_dirty_first : &program->vertex_dirty_first;
	unsigned int *dirty_last = pixel ? &program->pixel_dirty_last : &program->vertex_dirty_last;

	if (*dirty_first > first)
		*dirty_first = first;
	if (*dirty_last < first + count)
		*dirty_last = first + count;
}

/* loads what changed of the program's registers, or all of them when GX2
last had another program's */
static void registers_load(struct gl_program *program)
{
	if (registers_program != program)
	{
		registers_program = program;
		program->vertex_dirty_first = 0;
		program->vertex_dirty_last = program->vertex_register_count;
		program->pixel_dirty_first = 0;
		program->pixel_dirty_last = program->pixel_register_count;
	}
	if (program->vertex_dirty_last > program->vertex_dirty_first)
	{
		/* (whole registers, four floats each) */
		unsigned int first = program->vertex_dirty_first & ~3u;
		unsigned int last = (program->vertex_dirty_last + 3) & ~3u;

		GX2SetVertexUniformReg(first, last - first, program->vertex_registers + first);
	}
	if (program->pixel_dirty_last > program->pixel_dirty_first)
	{
		unsigned int first = program->pixel_dirty_first & ~3u;
		unsigned int last = (program->pixel_dirty_last + 3) & ~3u;

		GX2SetPixelUniformReg(first, last - first, program->pixel_registers + first);
	}
	program->vertex_dirty_first = program->pixel_dirty_first = ~0u;
	program->vertex_dirty_last = program->pixel_dirty_last = 0;
}

/* ---------- vertex fetch

GX2 reads vertices with a "fetch shader", a small program made for one
layout of attributes. The layouts a game draws with are few, so each is
made once and kept. Each attribute reads from an attribute buffer of its
own (numbered as its location). */

struct fetch_entry
{
	struct fetch_entry *next;
	unsigned int count;
	GX2AttribStream streams[ATTRIBUTE_COUNT];
	GX2FetchShader fetch;
};

#define FETCH_BUCKETS 64
static struct fetch_entry *fetch_buckets[FETCH_BUCKETS];

static GX2FetchShader *fetch_shader_get(const GX2AttribStream *streams, unsigned int count)
{
	const unsigned char *bytes = (const unsigned char *)streams;
	unsigned long hash = 2166136261UL, index;
	struct fetch_entry *entry;

	for (index = 0; index < count * sizeof(GX2AttribStream); index++)
		hash = (hash ^ bytes[index]) * 16777619UL;
	for (entry = fetch_buckets[hash % FETCH_BUCKETS]; entry; entry = entry->next)
	{
		if (entry->count == count && !memcmp(entry->streams, streams, count * sizeof(GX2AttribStream)))
			return &entry->fetch;
	}
	entry = calloc(1, sizeof(*entry));
	if (!entry)
		return NULL;
	entry->count = count;
	memcpy(entry->streams, streams, count * sizeof(GX2AttribStream));
	if (!fetch_shader_make(&entry->fetch, entry->streams, count))
	{
		free(entry);
		return NULL;
	}
	entry->next = fetch_buckets[hash % FETCH_BUCKETS];
	fetch_buckets[hash % FETCH_BUCKETS] = entry;
	return &entry->fetch;
}

#define SELECT(x, y, z, w) GX2_SEL_MASK(GX2_SQ_SEL_##x, GX2_SQ_SEL_##y, GX2_SQ_SEL_##z, GX2_SQ_SEL_##w)

/* GX2's format for a GL attribute array; FALSE if it has none */
static int attribute_stream_format(const struct attribute *attribute, GX2AttribStream *stream)
{
	static const uint32_t masks[4] =
	{
		SELECT(X, 0, 0, 1), SELECT(X, Y, 0, 1), SELECT(X, Y, Z, 1), SELECT(X, Y, Z, W)
	};
	GLint size = attribute->size;
	int components = size == GL_BGRA_EXT ? 4 : size;

	if (components < 1 || components > 4)
		return 0;
	stream->mask = masks[components - 1];
	stream->endianSwap = GX2_ENDIAN_SWAP_DEFAULT;
	switch (attribute->type)
	{
	case GL_FLOAT:
	{
		static const GX2AttribFormat formats[4] =
		{
			GX2_ATTRIB_FORMAT_FLOAT_32, GX2_ATTRIB_FORMAT_FLOAT_32_32,
			GX2_ATTRIB_FORMAT_FLOAT_32_32_32, GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32
		};

		stream->format = formats[components - 1];
		return 1;
	}
	case GL_UNSIGNED_BYTE:
	{
		static const GX2AttribFormat types[4] =
		{
			GX2_ATTRIB_TYPE_8, GX2_ATTRIB_TYPE_8_8, GX2_ATTRIB_TYPE_8_8_8_8, GX2_ATTRIB_TYPE_8_8_8_8
		};
		uint32_t flags = attribute->integer ? GX2_ATTRIB_FLAG_INTEGER :
			attribute->normalized ? 0 : GX2_ATTRIB_FLAG_SCALED;

		stream->format = (GX2AttribFormat)(types[components - 1] | flags);
		/* (bytes are read in the order they are in memory) */
		stream->endianSwap = GX2_ENDIAN_SWAP_NONE;
		if (size == GL_BGRA_EXT)
		{
			/* a D3DCOLOR: a 32-bit ARGB word, its bytes A, R, G, B in the
			Wii U's memory, read as four bytes */
			stream->endianSwap = GX2_ENDIAN_SWAP_NONE;
			stream->mask = SELECT(Y, Z, W, X);
		}
		return 1;
	}
	case GL_SHORT:
	{
		/* (no three-component 16-bit format: four, the fourth ignored) */
		static const GX2AttribFormat types[4] =
		{
			GX2_ATTRIB_TYPE_16, GX2_ATTRIB_TYPE_16_16, GX2_ATTRIB_TYPE_16_16_16_16, GX2_ATTRIB_TYPE_16_16_16_16
		};
		uint32_t flags = GX2_ATTRIB_FLAG_SIGNED | (attribute->integer ? GX2_ATTRIB_FLAG_INTEGER :
			attribute->normalized ? 0 : GX2_ATTRIB_FLAG_SCALED);

		stream->format = (GX2AttribFormat)(types[components - 1] | flags);
		return 1;
	}
	case GL_UNSIGNED_SHORT:
	{
		static const GX2AttribFormat types[4] =
		{
			GX2_ATTRIB_TYPE_16, GX2_ATTRIB_TYPE_16_16, GX2_ATTRIB_TYPE_16_16_16_16, GX2_ATTRIB_TYPE_16_16_16_16
		};
		uint32_t flags = attribute->integer ? GX2_ATTRIB_FLAG_INTEGER :
			attribute->normalized ? 0 : GX2_ATTRIB_FLAG_SCALED;

		stream->format = (GX2AttribFormat)(types[components - 1] | flags);
		return 1;
	}
	case GL_UNSIGNED_INT:
	case GL_INT:
	{
		static const GX2AttribFormat types[4] =
		{
			GX2_ATTRIB_TYPE_32, GX2_ATTRIB_TYPE_32_32, GX2_ATTRIB_TYPE_32_32_32, GX2_ATTRIB_TYPE_32_32_32_32
		};
		uint32_t flags = (attribute->type == GL_INT ? GX2_ATTRIB_FLAG_SIGNED : 0) |
			(attribute->integer ? GX2_ATTRIB_FLAG_INTEGER : attribute->normalized ? 0 : GX2_ATTRIB_FLAG_SCALED);

		stream->format = (GX2AttribFormat)(types[components - 1] | flags);
		return 1;
	}
	default:
		return 0;
	}
}

/* room in this frame's part of the constant ring */
static void *constant_ring_take(unsigned long size)
{
	void *memory;

	if (!constant_ring || constant_ring_offset + size > CONSTANT_RING_PART)
	{
		static int said;

		log_once(&said, "the ring for attribute values is full");
		return NULL;
	}
	memory = constant_ring + constant_ring_part * CONSTANT_RING_PART + constant_ring_offset;
	constant_ring_offset += (size + 63) & ~63UL;
	return memory;
}

/* the program's attributes pointed at their arrays, or their values; the
fetch shader for them */
static int attributes_bind(const struct gl_program *program)
{
	GX2AttribStream streams[ATTRIBUTE_COUNT];
	unsigned int count = 0, index;
	GX2FetchShader *fetch;

	for (index = 0; index < program->vertex->attribVarCount; index++)
	{
		const GX2AttribVar *variable = &program->vertex->attribVars[index];
		unsigned int location = variable->location;
		const struct attribute *attribute;
		GX2AttribStream *stream;
		struct gl_buffer *buffer;

		if (location >= ATTRIBUTE_COUNT)
			continue;
		attribute = &gl.attributes[location];
		stream = &streams[count];
		memset(stream, 0, sizeof(*stream));
		stream->location = location;
		stream->buffer = location;
		stream->offset = 0;
		stream->type = GX2_ATTRIB_INDEX_PER_VERTEX;
		stream->aluDivisor = 0;
		buffer = attribute->enabled ? name_get(&buffers, attribute->buffer) : NULL;
		if (buffer && buffer->data && attribute->offset < buffer->size && attribute_stream_format(attribute, stream))
		{
			GX2SetAttribBuffer(location, (uint32_t)(buffer->size - attribute->offset), (uint32_t)attribute->stride,
				buffer->data + attribute->offset);
		}
		else
		{
			/* the attribute's value, the same for every vertex */
			uint32_t *value = constant_ring_take(16);

			if (!value)
				return 0;
			if (attribute->integer_value)
			{
				memcpy(value, attribute->value_integer, 16);
				stream->format = (GX2AttribFormat)(GX2_ATTRIB_FLAG_INTEGER | GX2_ATTRIB_TYPE_32_32_32_32);
			}
			else
			{
				memcpy(value, attribute->value, 16);
				stream->format = GX2_ATTRIB_FORMAT_FLOAT_32_32_32_32;
			}
			stream->mask = SELECT(X, Y, Z, W);
			stream->endianSwap = GX2_ENDIAN_SWAP_DEFAULT;
			DCFlushRange(value, 16);
			GX2SetAttribBuffer(location, 16, 0, value);
		}
		count++;
	}
	fetch = fetch_shader_get(streams, count);
	if (!fetch)
		return 0;
	GX2SetFetchShader(fetch);
	return 1;
}

/* ---------- state for a draw */

static GX2CompareFunction compare_function(GLenum function)
{
	return function >= GL_NEVER && function <= GL_ALWAYS ? (GX2CompareFunction)(function - GL_NEVER) :
		GX2_COMPARE_FUNC_ALWAYS;
}

static GX2StencilFunction stencil_operation(GLenum operation)
{
	switch (operation)
	{
	case GL_ZERO: return GX2_STENCIL_FUNCTION_ZERO;
	case GL_REPLACE: return GX2_STENCIL_FUNCTION_REPLACE;
	case GL_INCR: return GX2_STENCIL_FUNCTION_INCR_CLAMP;
	case GL_DECR: return GX2_STENCIL_FUNCTION_DECR_CLAMP;
	case GL_INVERT: return GX2_STENCIL_FUNCTION_INV;
	case GL_INCR_WRAP: return GX2_STENCIL_FUNCTION_INCR_WRAP;
	case GL_DECR_WRAP: return GX2_STENCIL_FUNCTION_DECR_WRAP;
	default: return GX2_STENCIL_FUNCTION_KEEP;
	}
}

static GX2BlendMode blend_factor(GLenum factor)
{
	switch (factor)
	{
	case GL_ZERO: return GX2_BLEND_MODE_ZERO;
	case GL_ONE: return GX2_BLEND_MODE_ONE;
	case GL_SRC_COLOR: return GX2_BLEND_MODE_SRC_COLOR;
	case GL_ONE_MINUS_SRC_COLOR: return GX2_BLEND_MODE_INV_SRC_COLOR;
	case GL_SRC_ALPHA: return GX2_BLEND_MODE_SRC_ALPHA;
	case GL_ONE_MINUS_SRC_ALPHA: return GX2_BLEND_MODE_INV_SRC_ALPHA;
	case GL_DST_ALPHA: return GX2_BLEND_MODE_DST_ALPHA;
	case GL_ONE_MINUS_DST_ALPHA: return GX2_BLEND_MODE_INV_DST_ALPHA;
	case GL_DST_COLOR: return GX2_BLEND_MODE_DST_COLOR;
	case GL_ONE_MINUS_DST_COLOR: return GX2_BLEND_MODE_INV_DST_COLOR;
	case GL_SRC_ALPHA_SATURATE: return GX2_BLEND_MODE_SRC_ALPHA_SAT;
	case GL_CONSTANT_COLOR: return GX2_BLEND_MODE_BLEND_FACTOR;
	case GL_ONE_MINUS_CONSTANT_COLOR: return GX2_BLEND_MODE_INV_BLEND_FACTOR;
	case GL_CONSTANT_ALPHA: return GX2_BLEND_MODE_CONSTANT_ALPHA;
	case GL_ONE_MINUS_CONSTANT_ALPHA: return GX2_BLEND_MODE_INV_CONSTANT_ALPHA;
	default: return GX2_BLEND_MODE_ONE;
	}
}

static GX2BlendCombineMode blend_combine(GLenum equation)
{
	switch (equation)
	{
	case GL_FUNC_SUBTRACT: return GX2_BLEND_COMBINE_MODE_SUB;
	case GL_FUNC_REVERSE_SUBTRACT: return GX2_BLEND_COMBINE_MODE_REV_SUB;
	case GL_MIN: return GX2_BLEND_COMBINE_MODE_MIN;
	case GL_MAX: return GX2_BLEND_COMBINE_MODE_MAX;
	default: return GX2_BLEND_COMBINE_MODE_ADD;
	}
}

static GX2PrimitiveMode primitive_mode(GLenum mode)
{
	switch (mode)
	{
	case GL_POINTS: return GX2_PRIMITIVE_MODE_POINTS;
	case GL_LINES: return GX2_PRIMITIVE_MODE_LINES;
	case GL_LINE_STRIP: return GX2_PRIMITIVE_MODE_LINE_STRIP;
	case GL_LINE_LOOP: return GX2_PRIMITIVE_MODE_LINE_LOOP;
	case GL_TRIANGLE_STRIP: return GX2_PRIMITIVE_MODE_TRIANGLE_STRIP;
	case GL_TRIANGLE_FAN: return GX2_PRIMITIVE_MODE_TRIANGLE_FAN;
	default: return GX2_PRIMITIVE_MODE_TRIANGLES;
	}
}

static GX2ChannelMask color_mask_bits(void)
{
	return (GX2ChannelMask)((gl.color_mask[0] ? GX2_CHANNEL_MASK_R : 0) | (gl.color_mask[1] ? GX2_CHANNEL_MASK_G : 0) |
		(gl.color_mask[2] ? GX2_CHANNEL_MASK_B : 0) | (gl.color_mask[3] ? GX2_CHANNEL_MASK_A : 0));
}

/* the textures and samplers of the program's sampler uniforms */
static void textures_bind(const struct gl_program *program)
{
	unsigned int index;

	for (index = 0; index < program->uniform_count; index++)
	{
		const struct uniform_entry *uniform = &program->uniforms[index];
		struct gl_texture *texture;
		struct sampler_state *state;
		int slot;

		if (!uniform->sampler)
			continue;
		slot = uniform->sampler_type == GX2_SAMPLER_VAR_TYPE_SAMPLER_CUBE ? 1 :
			uniform->sampler_type == GX2_SAMPLER_VAR_TYPE_SAMPLER_3D ? 2 : 0;
		texture = uniform->unit >= 0 && uniform->unit < TEXTURE_UNIT_COUNT ?
			name_get(&textures, gl.unit_textures[uniform->unit][slot]) : NULL;
		if (!texture || !texture->allocated || texture->depth)
		{
			/* (an incomplete texture: GX2 is given none, and reads zeros) */
			continue;
		}
		state = uniform->unit < TEXTURE_UNIT_COUNT ? name_get(&samplers, gl.unit_samplers[uniform->unit]) : NULL;
		if (!state)
			state = &texture->sampler;
		GX2SetPixelTexture(texture_for_sampling(texture), uniform->slot);
		GX2SetPixelSampler(sampler_state_get(state), uniform->slot);
	}
}

/* the GL state GX2 needs for a draw into these targets */
static void state_apply(const struct draw_targets *targets)
{
	int has_depth = targets->depth != NULL;

	targets_bind(targets);
	if (gl.dirty & (_dirty_viewport | _dirty_targets))
	{
		long y = targets->screen ? (long)targets->height - (gl.viewport[1] + gl.viewport[3]) : gl.viewport[1];

		GX2SetViewport((float)gl.viewport[0], (float)y, (float)gl.viewport[2], (float)gl.viewport[3],
			gl.depth_range[0], gl.depth_range[1]);
	}
	if (gl.dirty & (_dirty_scissor | _dirty_targets))
	{
		uint32_t scissor[4];

		if (gl.scissor_test)
			window_rectangle(targets, gl.scissor[0], gl.scissor[1], gl.scissor[2], gl.scissor[3], scissor);
		else
			window_rectangle(targets, 0, targets->screen ? 0 : 0, (GLint)targets->width, (GLint)targets->height,
				scissor);
		GX2SetScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	}
	if (gl.dirty & (_dirty_depth_stencil | _dirty_targets))
	{
		int depth_test = has_depth && gl.depth_test;
		int stencil_test = has_depth && gl.stencil_test;
		GX2CompareFunction stencil = compare_function(gl.stencil_function);
		GX2StencilFunction fail = stencil_operation(gl.stencil_operations[0]);
		GX2StencilFunction depth_fail = stencil_operation(gl.stencil_operations[1]);
		GX2StencilFunction pass = stencil_operation(gl.stencil_operations[2]);

		GX2SetDepthStencilControl(depth_test, depth_test && gl.depth_mask, compare_function(gl.depth_function),
			stencil_test, FALSE, stencil, pass, depth_fail, fail, stencil, pass, depth_fail, fail);
		GX2SetStencilMask((uint8_t)gl.stencil_value_mask, (uint8_t)gl.stencil_write_mask, (uint8_t)gl.stencil_reference,
			(uint8_t)gl.stencil_value_mask, (uint8_t)gl.stencil_write_mask, (uint8_t)gl.stencil_reference);
	}
	if (gl.dirty & _dirty_blend)
	{
		GX2BlendMode source = blend_factor(gl.blend_source), destination = blend_factor(gl.blend_destination);
		GX2BlendCombineMode combine = blend_combine(gl.blend_equation);

		GX2SetColorControl(GX2_LOGIC_OP_COPY, gl.blend ? 1 : 0, FALSE, TRUE);
		GX2SetBlendControl(GX2_RENDER_TARGET_0, source, destination, combine, FALSE, source, destination, combine);
		GX2SetBlendConstantColor(gl.blend_color[0], gl.blend_color[1], gl.blend_color[2], gl.blend_color[3]);
	}
	if (gl.dirty & (_dirty_color_mask | _dirty_targets))
	{
		GX2SetTargetChannelMasks(targets->color ? color_mask_bits() : (GX2ChannelMask)0, (GX2ChannelMask)0,
			(GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0,
			(GX2ChannelMask)0);
	}
	if (gl.dirty & _dirty_polygon)
	{
		int cull_front = gl.cull_face && (gl.cull_mode == GL_FRONT || gl.cull_mode == GL_FRONT_AND_BACK);
		int cull_back = gl.cull_face && (gl.cull_mode == GL_BACK || gl.cull_mode == GL_FRONT_AND_BACK);

		GX2SetPolygonControl(gl.front_face == GL_CW ? GX2_FRONT_FACE_CW : GX2_FRONT_FACE_CCW, cull_front, cull_back,
			FALSE, GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, gl.polygon_offset_fill, gl.polygon_offset_fill,
			FALSE);
		/* GL's factor scales the depth slope, its units the depth buffer's
		smallest step: in the GPU's registers, sixteenths of the slope and
		(for a 24-bit depth buffer) half steps, as Mesa's driver for this
		GPU (r600) sets them */
		GX2SetPolygonOffset(gl.polygon_offset[1] * 2.0f, gl.polygon_offset[0] * 16.0f,
			gl.polygon_offset[1] * 2.0f, gl.polygon_offset[0] * 16.0f, 0.0f);
	}
	gl.dirty = 0;
}

/* port (debugging, temporary): debug.gpu_trace_frame also logs, for each
draw of that frame (counted here in presented frames), the state this
layer gives GX2 */
static unsigned long presented_frames;
static long trace_frame_number = -2;

static int tracing(void)
{
	if (trace_frame_number == -2)
		trace_frame_number = config_integer("debug.gpu_trace_frame");
	/* (and 400 frames later, by when the intro has given way to the menu) */
	return trace_frame_number >= 0 && (presented_frames == (unsigned long)trace_frame_number ||
		presented_frames == (unsigned long)trace_frame_number + 400);
}

static void trace_state(const struct gl_program *program, const struct draw_targets *targets)
{
	unsigned int index;

	platform_log("GX2 draw: program %u linked %d, target %ux%u screen %d color %p depth %p, viewport %d,%d %dx%d "
		"range %g-%g, scissor %d %d,%d %dx%d", gl.program, program->linked, targets->width, targets->height,
		targets->screen, (void *)targets->color, (void *)targets->depth, gl.viewport[0], gl.viewport[1],
		gl.viewport[2], gl.viewport[3], gl.depth_range[0], gl.depth_range[1], gl.scissor_test, gl.scissor[0],
		gl.scissor[1], gl.scissor[2], gl.scissor[3]);
	platform_log("GX2 draw:   depth test %d mask %d func %04x, stencil %d, blend %d %04x/%04x eq %04x, "
		"color mask %d%d%d%d, cull %d mode %04x front %04x, offset %d",
		gl.depth_test, gl.depth_mask, gl.depth_function, gl.stencil_test, gl.blend, gl.blend_source,
		gl.blend_destination, gl.blend_equation, gl.color_mask[0], gl.color_mask[1], gl.color_mask[2],
		gl.color_mask[3], gl.cull_face, gl.cull_mode, gl.front_face, gl.polygon_offset_fill);
	for (index = 0; index < program->vertex->attribVarCount; index++)
	{
		const GX2AttribVar *variable = &program->vertex->attribVars[index];
		const struct attribute *attribute = variable->location < ATTRIBUTE_COUNT ?
			&gl.attributes[variable->location] : NULL;
		struct gl_buffer *buffer = attribute && attribute->enabled ? name_get(&buffers, attribute->buffer) : NULL;

		if (!attribute)
			continue;
		if (buffer && buffer->data && attribute->offset + 16 <= buffer->size)
		{
			const float *first = (const float *)(buffer->data + attribute->offset);

			platform_log("GX2 draw:   %s at %u: buffer %u size %lu offset %lu stride %d, type %04x x%d, first %g %g %g %g",
				variable->name, variable->location, attribute->buffer, buffer->size, attribute->offset,
				attribute->stride, attribute->type, attribute->size, first[0], first[1], first[2], first[3]);
			if (variable->location == 0 && attribute->offset + 32 <= buffer->size)
			{
				const unsigned char *bytes = buffer->data + attribute->offset;
				char hex[3 * 32 + 1];
				unsigned int byte;

				for (byte = 0; byte < 32; byte++)
					snprintf(hex + 3 * byte, 4, "%02x ", bytes[byte]);
				platform_log("GX2 draw:     first vertex bytes: %s", hex);
			}
		}
		else
		{
			platform_log("GX2 draw:   %s at %u: enabled %d buffer %u (none usable), value %g %g %g %g",
				variable->name, variable->location, attribute->enabled, attribute->buffer, attribute->value[0],
				attribute->value[1], attribute->value[2], attribute->value[3]);
		}
	}
	for (index = 0; index < program->uniform_count; index++)
	{
		const struct uniform_entry *uniform = &program->uniforms[index];

		if (uniform->sampler)
			continue;
		if (uniform->vertex_offset >= 0)
		{
			const float *value = program->vertex_registers + uniform->vertex_offset;

			platform_log("GX2 draw:   vs %s at %d x%u: %g %g %g %g", uniform->name, uniform->vertex_offset,
				uniform->count, value[0], value[1], value[2], value[3]);
			if (!strcmp(uniform->name, "c"))
			{
				static const unsigned int shown[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
					18, 19, 28, 29, 30, 31, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72 };
				unsigned int element;

				for (element = 0; element < sizeof(shown) / sizeof(shown[0]); element++)
				{
					if (shown[element] < uniform->count)
					{
						value = program->vertex_registers + uniform->vertex_offset + shown[element] * 4;
						platform_log("GX2 draw:     c[%u] = %g %g %g %g", shown[element], value[0], value[1],
							value[2], value[3]);
					}
				}
			}
		}
		if (uniform->pixel_offset >= 0)
		{
			const float *value = program->pixel_registers + uniform->pixel_offset;

			platform_log("GX2 draw:   ps %s at %d x%u: %g %g %g %g", uniform->name, uniform->pixel_offset,
				uniform->count, value[0], value[1], value[2], value[3]);
		}
	}
	platform_log("GX2 draw:   registers vs %u ps %u; vertex shader %p: %u attributes, %u uniforms, %u bytes; "
		"pixel shader %p: %u uniforms, %u samplers", program->vertex_register_count, program->pixel_register_count,
		(void *)program->vertex, program->vertex->attribVarCount, program->vertex->uniformVarCount,
		program->vertex->size, (void *)program->pixel, program->pixel->uniformVarCount,
		program->pixel->samplerVarCount);
}

/* everything a draw of the current program needs set; FALSE: draw nothing */
static int draw_prepare(void)
{
	struct gl_program *program = name_get(&programs, gl.program);
	struct draw_targets targets;

	if (!video_ready || !program || !program->linked)
	{
		if (tracing())
			platform_log("GX2 draw: skipped: ready %d program %u %s", video_ready, gl.program,
				program ? "not linked" : "missing");
		return 0;
	}
	if (!targets_get(gl.draw_framebuffer, &targets))
	{
		if (tracing())
			platform_log("GX2 draw: skipped: no targets for framebuffer %u", gl.draw_framebuffer);
		return 0;
	}
	if (tracing())
		trace_state(program, &targets);
	state_apply(&targets);
	GX2SetVertexShader(program->vertex);
	GX2SetPixelShader(program->pixel);
	registers_load(program);
	textures_bind(program);
	return attributes_bind(program);
}

/* ---------- rectangles: clears, blits and mipmaps */

/* after an internal draw the GL state must all be set again */
static void internal_done(void)
{
	gl.dirty = _dirty_all;
	registers_program = NULL;
}

static void internal_targets_state(const struct draw_targets *targets, const uint32_t scissor[4])
{
	targets_bind(targets);
	GX2SetViewport(0.0f, 0.0f, (float)targets->width, (float)targets->height, 0.0f, 1.0f);
	GX2SetScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	GX2SetColorControl(GX2_LOGIC_OP_COPY, 0, FALSE, TRUE);
	GX2SetPolygonControl(GX2_FRONT_FACE_CCW, FALSE, FALSE, FALSE, GX2_POLYGON_MODE_TRIANGLE,
		GX2_POLYGON_MODE_TRIANGLE, FALSE, FALSE, FALSE);
}

/* a rectangle (clip space corners) from the internal program */
static void internal_rectangle(struct internal_program *program, const float rectangle[4], const float *source,
	float depth, const float *color)
{
	float values[4];

	GX2SetFetchShader(&program->fetch);
	GX2SetVertexShader(program->vertex);
	GX2SetPixelShader(program->pixel);
	if (program->rectangle >= 0)
		GX2SetVertexUniformReg((uint32_t)program->rectangle, 4, rectangle);
	if (program->source >= 0 && source)
		GX2SetVertexUniformReg((uint32_t)program->source, 4, source);
	if (program->depth >= 0)
	{
		values[0] = values[1] = values[2] = values[3] = depth;
		GX2SetVertexUniformReg((uint32_t)program->depth, 4, values);
	}
	if (program->color >= 0 && color)
		GX2SetPixelUniformReg((uint32_t)program->color, 4, color);
	GX2DrawEx(GX2_PRIMITIVE_MODE_TRIANGLE_STRIP, 4, 0, 1);
}

static void clear_targets(const struct draw_targets *targets, GLbitfield mask)
{
	static const float whole[4] = { -1.0f, -1.0f, 1.0f, 1.0f };
	uint32_t scissor[4];
	int color = (mask & GL_COLOR_BUFFER_BIT) && targets->color;
	int depth = (mask & GL_DEPTH_BUFFER_BIT) && targets->depth && gl.depth_mask;
	int stencil = (mask & GL_STENCIL_BUFFER_BIT) && targets->depth && (gl.stencil_write_mask & 0xff);
	float clear_depth = gl.clear_depth < 0.0f ? 0.0f : gl.clear_depth > 1.0f ? 1.0f : gl.clear_depth;

	if (!color && !depth && !stencil)
		return;
	if (gl.scissor_test)
		window_rectangle(targets, gl.scissor[0], gl.scissor[1], gl.scissor[2], gl.scissor[3], scissor);
	else
		window_rectangle(targets, 0, 0, (GLint)targets->width, (GLint)targets->height, scissor);
	if (!scissor[2] || !scissor[3])
		return;
	internal_targets_state(targets, scissor);
	GX2SetTargetChannelMasks(color ? color_mask_bits() : (GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0,
		(GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0);
	/* the depth written through the rectangle's own (clip space -1..1 to
	the viewport's 0..1), the stencil value by replacing */
	GX2SetDepthStencilControl(depth, depth, GX2_COMPARE_FUNC_ALWAYS, stencil, FALSE,
		GX2_COMPARE_FUNC_ALWAYS, GX2_STENCIL_FUNCTION_REPLACE, GX2_STENCIL_FUNCTION_REPLACE,
		GX2_STENCIL_FUNCTION_REPLACE, GX2_COMPARE_FUNC_ALWAYS, GX2_STENCIL_FUNCTION_REPLACE,
		GX2_STENCIL_FUNCTION_REPLACE, GX2_STENCIL_FUNCTION_REPLACE);
	GX2SetStencilMask(0xff, (uint8_t)gl.stencil_write_mask, (uint8_t)gl.clear_stencil, 0xff,
		(uint8_t)gl.stencil_write_mask, (uint8_t)gl.clear_stencil);
	internal_rectangle(&clear_program, whole, NULL, clear_depth * 2.0f - 1.0f, gl.clear_color);
	internal_done();
}

/* draws a level of a texture into the targets: source rectangle in its
texels (rows from row 0), destination in GX2's window pixels (rows from the
top); the corners may be swapped, which flips the picture */
static void blit_texture(struct gl_texture *texture, unsigned int level, const float source_pixels[4],
	const struct draw_targets *targets, const float destination_pixels[4], int linear)
{
	GX2Texture view;
	struct sampler_state state;
	uint32_t scissor[4];
	float rectangle[4], source[4];
	unsigned int width = texture->texture.surface.width >> level ? texture->texture.surface.width >> level : 1;
	unsigned int height = texture->texture.surface.height >> level ? texture->texture.surface.height >> level : 1;

	if (!internal_ready || !texture->allocated || texture->depth)
		return;
	texture_for_sampling(texture);
	/* the one level, as a texture of its own */
	view = texture->texture;
	view.viewFirstMip = level;
	view.viewNumMips = 1;
	view.viewFirstSlice = 0;
	view.viewNumSlices = 1;
	view.compMap = texture->render ? GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A) :
		texture->base_map;
	GX2InitTextureRegs(&view);
	sampler_state_default(&state);
	state.min_filter = state.mag_filter = linear ? GL_LINEAR : GL_NEAREST;
	state.wrap[0] = state.wrap[1] = state.wrap[2] = GL_CLAMP_TO_EDGE;
	source[0] = source_pixels[0] / (float)width;
	source[1] = source_pixels[1] / (float)height;
	source[2] = source_pixels[2] / (float)width;
	source[3] = source_pixels[3] / (float)height;
	/* window pixels to clip space: rows from the top are y from +1 down */
	rectangle[0] = destination_pixels[0] * 2.0f / (float)targets->width - 1.0f;
	rectangle[1] = 1.0f - destination_pixels[1] * 2.0f / (float)targets->height;
	rectangle[2] = destination_pixels[2] * 2.0f / (float)targets->width - 1.0f;
	rectangle[3] = 1.0f - destination_pixels[3] * 2.0f / (float)targets->height;
	if (gl.scissor_test)
		window_rectangle(targets, gl.scissor[0], gl.scissor[1], gl.scissor[2], gl.scissor[3], scissor);
	else
		window_rectangle(targets, 0, 0, (GLint)targets->width, (GLint)targets->height, scissor);
	internal_targets_state(targets, scissor);
	GX2SetTargetChannelMasks(GX2_CHANNEL_MASK_RGBA, (GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0,
		(GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0, (GX2ChannelMask)0);
	GX2SetDepthStencilControl(FALSE, FALSE, GX2_COMPARE_FUNC_ALWAYS, FALSE, FALSE, GX2_COMPARE_FUNC_ALWAYS,
		GX2_STENCIL_FUNCTION_KEEP, GX2_STENCIL_FUNCTION_KEEP, GX2_STENCIL_FUNCTION_KEEP, GX2_COMPARE_FUNC_ALWAYS,
		GX2_STENCIL_FUNCTION_KEEP, GX2_STENCIL_FUNCTION_KEEP, GX2_STENCIL_FUNCTION_KEEP);
	GX2SetPixelTexture(&view, blit_program.sampler_slot);
	GX2SetPixelSampler(sampler_state_get(&state), blit_program.sampler_slot);
	internal_rectangle(&blit_program, rectangle, source, 0.0f, NULL);
	internal_done();
}

/* ---------- GL: state */

static const GLubyte *GL_APIENTRY wiiu_glGetString(GLenum name)
{
	switch (name)
	{
	case GL_VENDOR: return (const GLubyte *)"Halo U";
	case GL_RENDERER: return (const GLubyte *)"Wii U GX2 (port/wiiu/src/wiiu_gl.c)";
	case GL_VERSION: return (const GLubyte *)"OpenGL ES 3.2 on GX2";
	case GL_SHADING_LANGUAGE_VERSION: return (const GLubyte *)"OpenGL ES GLSL ES 3.10 (CafeGLSL)";
	default: return (const GLubyte *)"";
	}
}

static void GL_APIENTRY wiiu_glGetIntegerv(GLenum name, GLint *data)
{
	switch (name)
	{
	case GL_MAJOR_VERSION: *data = 3; break;
	case GL_MINOR_VERSION: *data = 2; break;
	case GL_MAX_TEXTURE_SIZE: *data = 8192; break;
	case GL_MAX_RENDERBUFFER_SIZE: *data = 8192; break;
	/* (no multisampling, no atomic counters) */
	case GL_MAX_SAMPLES: *data = 0; break;
	case GL_MAX_FRAGMENT_ATOMIC_COUNTERS: *data = 0; break;
	case GL_VIEWPORT: memcpy(data, gl.viewport, sizeof(gl.viewport)); break;
	default: *data = 0; break;
	}
}

static GLenum GL_APIENTRY wiiu_glGetError(void)
{
	GLenum error = gl.error;

	gl.error = GL_NO_ERROR;
	return error;
}

static void capability_set(GLenum capability, int value)
{
	switch (capability)
	{
	case GL_BLEND: gl.blend = value; gl.dirty |= _dirty_blend; break;
	case GL_CULL_FACE: gl.cull_face = value; gl.dirty |= _dirty_polygon; break;
	case GL_DEPTH_TEST: gl.depth_test = value; gl.dirty |= _dirty_depth_stencil; break;
	case GL_STENCIL_TEST: gl.stencil_test = value; gl.dirty |= _dirty_depth_stencil; break;
	case GL_SCISSOR_TEST: gl.scissor_test = value; gl.dirty |= _dirty_scissor; break;
	case GL_POLYGON_OFFSET_FILL: gl.polygon_offset_fill = value; gl.dirty |= _dirty_polygon; break;
	default: break;
	}
}

static void GL_APIENTRY wiiu_glEnable(GLenum capability)
{
	capability_set(capability, 1);
}

static void GL_APIENTRY wiiu_glDisable(GLenum capability)
{
	capability_set(capability, 0);
}

static void GL_APIENTRY wiiu_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	gl.viewport[0] = x;
	gl.viewport[1] = y;
	gl.viewport[2] = width;
	gl.viewport[3] = height;
	gl.dirty |= _dirty_viewport;
}

static void GL_APIENTRY wiiu_glDepthRangef(GLfloat near_value, GLfloat far_value)
{
	gl.depth_range[0] = near_value;
	gl.depth_range[1] = far_value;
	gl.dirty |= _dirty_viewport;
}

static void GL_APIENTRY wiiu_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	gl.scissor[0] = x;
	gl.scissor[1] = y;
	gl.scissor[2] = width;
	gl.scissor[3] = height;
	gl.dirty |= _dirty_scissor;
}

static void GL_APIENTRY wiiu_glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	gl.clear_color[0] = red;
	gl.clear_color[1] = green;
	gl.clear_color[2] = blue;
	gl.clear_color[3] = alpha;
}

static void GL_APIENTRY wiiu_glClearDepthf(GLfloat depth)
{
	gl.clear_depth = depth;
}

static void GL_APIENTRY wiiu_glClearStencil(GLint stencil)
{
	gl.clear_stencil = stencil;
}

static void GL_APIENTRY wiiu_glClear(GLbitfield mask)
{
	struct draw_targets targets;

	if (!video_ready || !internal_ready || !targets_get(gl.draw_framebuffer, &targets))
		return;
	clear_targets(&targets, mask);
}

static void GL_APIENTRY wiiu_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	gl.color_mask[0] = red;
	gl.color_mask[1] = green;
	gl.color_mask[2] = blue;
	gl.color_mask[3] = alpha;
	gl.dirty |= _dirty_color_mask;
}

static void GL_APIENTRY wiiu_glDepthMask(GLboolean flag)
{
	gl.depth_mask = flag;
	gl.dirty |= _dirty_depth_stencil;
}

static void GL_APIENTRY wiiu_glDepthFunc(GLenum function)
{
	gl.depth_function = function;
	gl.dirty |= _dirty_depth_stencil;
}

static void GL_APIENTRY wiiu_glStencilFunc(GLenum function, GLint reference, GLuint mask)
{
	gl.stencil_function = function;
	gl.stencil_reference = reference;
	gl.stencil_value_mask = mask;
	gl.dirty |= _dirty_depth_stencil;
}

static void GL_APIENTRY wiiu_glStencilOp(GLenum fail, GLenum depth_fail, GLenum pass)
{
	gl.stencil_operations[0] = fail;
	gl.stencil_operations[1] = depth_fail;
	gl.stencil_operations[2] = pass;
	gl.dirty |= _dirty_depth_stencil;
}

static void GL_APIENTRY wiiu_glStencilMask(GLuint mask)
{
	gl.stencil_write_mask = mask;
	gl.dirty |= _dirty_depth_stencil;
}

static void GL_APIENTRY wiiu_glBlendFunc(GLenum source, GLenum destination)
{
	gl.blend_source = source;
	gl.blend_destination = destination;
	gl.dirty |= _dirty_blend;
}

static void GL_APIENTRY wiiu_glBlendEquation(GLenum equation)
{
	gl.blend_equation = equation;
	gl.dirty |= _dirty_blend;
}

static void GL_APIENTRY wiiu_glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	gl.blend_color[0] = red;
	gl.blend_color[1] = green;
	gl.blend_color[2] = blue;
	gl.blend_color[3] = alpha;
	gl.dirty |= _dirty_blend;
}

static void GL_APIENTRY wiiu_glCullFace(GLenum mode)
{
	gl.cull_mode = mode;
	gl.dirty |= _dirty_polygon;
}

static void GL_APIENTRY wiiu_glFrontFace(GLenum mode)
{
	gl.front_face = mode;
	gl.dirty |= _dirty_polygon;
}

static void GL_APIENTRY wiiu_glPolygonOffset(GLfloat factor, GLfloat units)
{
	gl.polygon_offset[0] = factor;
	gl.polygon_offset[1] = units;
	gl.dirty |= _dirty_polygon;
}

static void GL_APIENTRY wiiu_glLineWidth(GLfloat width)
{
	GX2SetLineWidth(width);
}

static void GL_APIENTRY wiiu_glPixelStorei(GLenum name, GLint value)
{
	/* (uploads are 32-bit texels: rows are always whole words) */
	(void)name;
	(void)value;
}

/* The read framebuffer's pixels, as 32-bit RGBA (only what the renderer's
screenshots ask for): the GPU copies its tiled colour buffer into a linear
one, and the CPU reads that once the GPU is done. Rows come out as they are
in the buffer. */
static void GL_APIENTRY wiiu_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
	GLenum type, void *pixels)
{
	struct draw_targets targets;
	GX2Surface *source;
	GX2Surface staging;
	GX2Rect rectangle;
	GX2Point point = { 0, 0 };
	GLsizei row;

	(void)format; (void)type;
	memset(pixels, 0, (size_t)width * (size_t)height * 4);
	if (!video_ready || width <= 0 || height <= 0 || !targets_get(gl.read_framebuffer, &targets) ||
		!targets.color)
		return;
	source = &targets.color->surface;
	if (x < 0 || y < 0 || (uint32_t)(x + width) > source->width || (uint32_t)(y + height) > source->height)
		return;
	memset(&staging, 0, sizeof(staging));
	staging.dim = GX2_SURFACE_DIM_TEXTURE_2D;
	staging.width = (uint32_t)width;
	staging.height = (uint32_t)height;
	staging.depth = 1;
	staging.mipLevels = 1;
	staging.format = source->format;
	staging.aa = GX2_AA_MODE1X;
	staging.use = GX2_SURFACE_USE_TEXTURE;
	staging.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
	GX2CalcSurfaceSizeAndAlignment(&staging);
	staging.image = gpu_alloc(staging.alignment, staging.imageSize);
	if (!staging.image)
		return;
	rectangle.left = x;
	rectangle.top = y;
	rectangle.right = x + width;
	rectangle.bottom = y + height;
	GX2CopySurfaceEx(source, targets.color->viewMip, targets.color->viewFirstSlice, &staging, 0, 0, 1,
		&rectangle, &point);
	GX2DrawDone();
	DCInvalidateRange(staging.image, staging.imageSize);
	for (row = 0; row < height; row++)
		memcpy((unsigned char *)pixels + (size_t)row * (size_t)width * 4,
			(const unsigned char *)staging.image + (size_t)row * staging.pitch * 4, (size_t)width * 4);
	gpu_free(staging.image);
	internal_done();
}

static void GL_APIENTRY wiiu_glFinish(void)
{
	if (video_ready)
		GX2DrawDone();
}

static void GL_APIENTRY wiiu_glFlush(void)
{
	if (video_ready)
		GX2Flush();
}

/* ---------- GL: textures */

static void GL_APIENTRY wiiu_glGenTextures(GLsizei count, GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
	{
		struct gl_texture *texture = calloc(1, sizeof(*texture));

		names[index] = 0;
		if (!texture)
			continue;
		texture->swizzle[0] = GL_RED;
		texture->swizzle[1] = GL_GREEN;
		texture->swizzle[2] = GL_BLUE;
		texture->swizzle[3] = GL_ALPHA;
		texture->base_map = GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);
		texture->max_level = 1000;
		sampler_state_default(&texture->sampler);
		names[index] = name_new(&textures, texture);
	}
}

static void GL_APIENTRY wiiu_glDeleteTextures(GLsizei count, const GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
	{
		struct gl_texture *texture = name_get(&textures, names[index]);
		int unit, slot;

		if (!texture)
			continue;
		texture_free_storage(texture);
		for (unit = 0; unit < TEXTURE_UNIT_COUNT; unit++)
		{
			for (slot = 0; slot < 3; slot++)
			{
				if (gl.unit_textures[unit][slot] == names[index])
					gl.unit_textures[unit][slot] = 0;
			}
		}
		if (current_color && current_color->surface.image == NULL)
			current_color = NULL;
		/* (GX2 may still have its buffers: set them again at the next draw) */
		current_color = NULL;
		current_depth = NULL;
		name_delete(&textures, names[index]);
		free(texture);
	}
}

static void GL_APIENTRY wiiu_glBindTexture(GLenum target, GLuint name)
{
	struct gl_texture *texture = name_get(&textures, name);

	if (gl.active_unit >= TEXTURE_UNIT_COUNT)
		return;
	if (texture && !texture->target)
		texture->target = target;
	gl.unit_textures[gl.active_unit][texture_slot(target)] = name;
}

static void GL_APIENTRY wiiu_glActiveTexture(GLenum unit)
{
	gl.active_unit = unit - GL_TEXTURE0;
}

/* writes texels (32-bit words, rows one after another) into a level of a
texture, at x, y of slice; tiled textures by way of a linear copy */
static void texture_write(struct gl_texture *texture, unsigned int level, unsigned int slice, unsigned int x,
	unsigned int y, unsigned int width, unsigned int height, const void *pixels)
{
	GX2Surface *surface = &texture->texture.surface;
	unsigned long row_bytes, slice_bytes;
	unsigned char *base;
	unsigned int row;

	if (!texture->allocated || texture->depth || level >= surface->mipLevels)
		return;
	if (texture->render)
	{
		/* a texture the GPU draws into is tiled: the texels go into a
		linear texture of their own, which the GPU copies over */
		GX2Surface staging;
		GX2Rect rectangle = { 0, 0, (int32_t)width, (int32_t)height };
		GX2Point point = { (int32_t)x, (int32_t)y };

		memset(&staging, 0, sizeof(staging));
		staging.dim = GX2_SURFACE_DIM_TEXTURE_2D;
		staging.width = width;
		staging.height = height;
		staging.depth = 1;
		staging.mipLevels = 1;
		staging.format = surface->format;
		staging.aa = GX2_AA_MODE1X;
		staging.use = GX2_SURFACE_USE_TEXTURE;
		staging.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
		GX2CalcSurfaceSizeAndAlignment(&staging);
		staging.image = gpu_alloc(staging.alignment, staging.imageSize);
		if (!staging.image)
			return;
		for (row = 0; row < height; row++)
			memcpy((unsigned char *)staging.image + row * staging.pitch * 4,
				(const unsigned char *)pixels + row * width * 4, width * 4);
		GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, staging.image, staging.imageSize);
		GX2CopySurfaceEx(&staging, 0, 0, surface, level, slice, 1, &rectangle, &point);
		gpu_free(staging.image);
		texture->gpu_written = 1;
		return;
	}
	base = texture_level_layout(surface, level, &row_bytes, &slice_bytes) + slice * slice_bytes;
	for (row = 0; row < height; row++)
		memcpy(base + (y + row) * row_bytes + x * 4, (const unsigned char *)pixels + row * width * 4, width * 4);
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU_TEXTURE, base + y * row_bytes, height * row_bytes);
}

static void texture_image(GLenum target, GLint level, GLint internal_format, GLsizei width, GLsizei height,
	GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
	struct gl_texture *texture = bound_texture(target);
	unsigned int slice = 0;
	GLenum binding = texture_binding_target(target);

	if (!texture || level < 0 || width <= 0 || height <= 0)
		return;
	if (!texture->target)
		texture->target = binding;
	if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	{
		slice = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		depth = 6;
	}
	if (internal_format == GL_DEPTH24_STENCIL8 || format == GL_DEPTH_STENCIL)
	{
		if (level == 0 && !(texture->allocated && texture->depth &&
			texture->depth_buffer.surface.width == (uint32_t)width &&
			texture->depth_buffer.surface.height == (uint32_t)height))
		{
			texture_allocate_depth(texture, (unsigned int)width, (unsigned int)height);
		}
		return;
	}
	if (type != GL_UNSIGNED_BYTE || (format != GL_BGRA_EXT && format != GL_RGBA))
	{
		static int said;

		log_once(&said, "a texture of format %04x, type %04x is not supported", (unsigned)format, (unsigned)type);
		return;
	}
	/* level 0 decides the size: a texture drawn into by the GPU when it
	comes with no texels, else one the CPU writes */
	if (level == 0 && (slice == 0 || !texture->allocated))
	{
		int render = pixels == NULL;
		const GX2Surface *surface = &texture->texture.surface;
		unsigned int wanted_levels = full_mip_count((unsigned)width, (unsigned)height,
			binding == GL_TEXTURE_3D ? (unsigned)depth : 1);

		if (texture->max_level + 1 < (GLint)wanted_levels)
			wanted_levels = texture->max_level < 0 ? 1 : (unsigned int)texture->max_level + 1;
		if (wanted_levels > 13)
			wanted_levels = 13;
		if (!texture->allocated || texture->depth || texture->render != render || surface->width != (uint32_t)width ||
			surface->height != (uint32_t)height || surface->depth != (uint32_t)depth ||
			surface->mipLevels != wanted_levels)
		{
			if (!texture_allocate(texture, (unsigned int)width, (unsigned int)height, (unsigned int)depth, render))
				return;
		}
	}
	/* 32-bit ARGB words (GL_BGRA, as the renderer uploads its texels): in
	memory, alpha, red, green, blue */
	if (pixels)
	{
		uint32_t base_map = format == GL_BGRA_EXT ?
			GX2_COMP_MAP(GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A, GX2_SQ_SEL_R) :
			GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);

		if (texture->base_map != base_map)
		{
			texture->base_map = base_map;
			texture->regs_dirty = 1;
		}
		if (binding == GL_TEXTURE_3D)
		{
			GLsizei layer;

			for (layer = 0; layer < depth; layer++)
				texture_write(texture, (unsigned int)level, (unsigned int)layer, 0, 0, (unsigned int)width,
					(unsigned int)height, (const unsigned char *)pixels + (size_t)layer * width * height * 4);
		}
		else
		{
			texture_write(texture, (unsigned int)level, slice, 0, 0, (unsigned int)width, (unsigned int)height, pixels);
		}
	}
	else if (texture->render && texture->base_map != GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A))
	{
		texture->base_map = GX2_COMP_MAP(GX2_SQ_SEL_R, GX2_SQ_SEL_G, GX2_SQ_SEL_B, GX2_SQ_SEL_A);
		texture->regs_dirty = 1;
	}
}

static void GL_APIENTRY wiiu_glTexImage2D(GLenum target, GLint level, GLint internal_format, GLsizei width,
	GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
	(void)border;
	texture_image(target, level, internal_format, width, height, 1, format, type, pixels);
}

static void GL_APIENTRY wiiu_glTexImage3D(GLenum target, GLint level, GLint internal_format, GLsizei width,
	GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
	(void)border;
	texture_image(target, level, internal_format, width, height, depth, format, type, pixels);
}

static void GL_APIENTRY wiiu_glTexSubImage2D(GLenum target, GLint level, GLint x, GLint y, GLsizei width,
	GLsizei height, GLenum format, GLenum type, const void *pixels)
{
	struct gl_texture *texture = bound_texture(target);
	unsigned int slice = target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ?
		target - GL_TEXTURE_CUBE_MAP_POSITIVE_X : 0;

	(void)format;
	if (!texture || !pixels || type != GL_UNSIGNED_BYTE || x < 0 || y < 0 || width <= 0 || height <= 0)
		return;
	texture_write(texture, (unsigned int)level, slice, (unsigned int)x, (unsigned int)y, (unsigned int)width,
		(unsigned int)height, pixels);
}

static void GL_APIENTRY wiiu_glCompressedTexImage2D(GLenum target, GLint level, GLenum internal_format,
	GLsizei width, GLsizei height, GLint border, GLsizei size, const void *data)
{
	static int said;

	(void)target; (void)level; (void)internal_format; (void)width; (void)height; (void)border; (void)size; (void)data;
	log_once(&said, "compressed textures are decoded by the CPU instead (none should come here)");
}

static void GL_APIENTRY wiiu_glCompressedTexImage3D(GLenum target, GLint level, GLenum internal_format,
	GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei size, const void *data)
{
	(void)depth;
	wiiu_glCompressedTexImage2D(target, level, internal_format, width, height, border, size, data);
}

static void sampler_parameter(struct sampler_state *state, GLenum name, const GLfloat *values)
{
	switch (name)
	{
	case GL_TEXTURE_MIN_FILTER: state->min_filter = (GLint)values[0]; break;
	case GL_TEXTURE_MAG_FILTER: state->mag_filter = (GLint)values[0]; break;
	case GL_TEXTURE_WRAP_S: state->wrap[0] = (GLint)values[0]; break;
	case GL_TEXTURE_WRAP_T: state->wrap[1] = (GLint)values[0]; break;
	case GL_TEXTURE_WRAP_R: state->wrap[2] = (GLint)values[0]; break;
	case GL_TEXTURE_MIN_LOD: state->min_lod = values[0]; break;
	case GL_TEXTURE_MAX_LOD: state->max_lod = values[0]; break;
	case GL_TEXTURE_MAX_ANISOTROPY_EXT: state->anisotropy = values[0]; break;
	case GL_TEXTURE_BORDER_COLOR: memcpy(state->border, values, sizeof(state->border)); break;
	default: return;
	}
	state->dirty = 1;
}

static void texture_parameter(GLenum target, GLenum name, const GLfloat *values, GLint integer)
{
	struct gl_texture *texture = bound_texture(target);

	if (!texture)
		return;
	switch (name)
	{
	case GL_TEXTURE_SWIZZLE_R: texture->swizzle[0] = integer; texture->regs_dirty = 1; break;
	case GL_TEXTURE_SWIZZLE_G: texture->swizzle[1] = integer; texture->regs_dirty = 1; break;
	case GL_TEXTURE_SWIZZLE_B: texture->swizzle[2] = integer; texture->regs_dirty = 1; break;
	case GL_TEXTURE_SWIZZLE_A: texture->swizzle[3] = integer; texture->regs_dirty = 1; break;
	case GL_TEXTURE_BASE_LEVEL: texture->base_level = integer; break;
	case GL_TEXTURE_MAX_LEVEL: texture->max_level = integer; break;
	default: sampler_parameter(&texture->sampler, name, values); break;
	}
}

static void GL_APIENTRY wiiu_glTexParameteri(GLenum target, GLenum name, GLint value)
{
	GLfloat values[4] = { (GLfloat)value, 0.0f, 0.0f, 0.0f };

	texture_parameter(target, name, values, value);
}

static void GL_APIENTRY wiiu_glTexParameteriv(GLenum target, GLenum name, const GLint *values)
{
	GLfloat converted[4] = { (GLfloat)values[0], 0.0f, 0.0f, 0.0f };

	if (name == GL_TEXTURE_BORDER_COLOR)
	{
		int index;

		for (index = 0; index < 4; index++)
			converted[index] = (GLfloat)values[index] / 2147483647.0f;
	}
	texture_parameter(target, name, converted, values[0]);
}

static void GL_APIENTRY wiiu_glTexParameterf(GLenum target, GLenum name, GLfloat value)
{
	GLfloat values[4] = { value, 0.0f, 0.0f, 0.0f };

	texture_parameter(target, name, values, (GLint)value);
}

static void GL_APIENTRY wiiu_glTexParameterfv(GLenum target, GLenum name, const GLfloat *values)
{
	texture_parameter(target, name, values, (GLint)values[0]);
}

static void GL_APIENTRY wiiu_glGenerateMipmap(GLenum target)
{
	struct gl_texture *texture = bound_texture(target);
	unsigned int level;

	if (!texture || !texture->allocated || texture->depth || target != GL_TEXTURE_2D)
		return;
	if (!texture->render)
	{
		static int said;

		log_once(&said, "mipmaps of a texture the CPU wrote are not made");
		return;
	}
	/* each level drawn from the one above it, filtered */
	for (level = (unsigned int)(texture->base_level < 0 ? 0 : texture->base_level) + 1;
		level < texture->texture.surface.mipLevels; level++)
	{
		struct draw_targets targets;
		float source[4], destination[4];

		memset(&targets, 0, sizeof(targets));
		targets.color = texture_color_view(texture, level);
		targets.color_texture = texture;
		if (!targets.color)
			return;
		targets.width = texture->texture.surface.width >> level ? texture->texture.surface.width >> level : 1;
		targets.height = texture->texture.surface.height >> level ? texture->texture.surface.height >> level : 1;
		source[0] = 0.0f;
		source[1] = 0.0f;
		source[2] = (float)(texture->texture.surface.width >> (level - 1) ? texture->texture.surface.width >> (level - 1) : 1);
		source[3] = (float)(texture->texture.surface.height >> (level - 1) ? texture->texture.surface.height >> (level - 1) : 1);
		destination[0] = 0.0f;
		destination[1] = 0.0f;
		destination[2] = (float)targets.width;
		destination[3] = (float)targets.height;
		{
			/* (the scissor is GL's, which mipmaps ignore) */
			int scissor_test = gl.scissor_test;

			gl.scissor_test = 0;
			blit_texture(texture, level - 1, source, &targets, destination, 1);
			gl.scissor_test = scissor_test;
		}
	}
}

static void GL_APIENTRY wiiu_glCopyImageSubData(GLuint source_name, GLenum source_target, GLint source_level,
	GLint source_x, GLint source_y, GLint source_z, GLuint destination_name, GLenum destination_target,
	GLint destination_level, GLint destination_x, GLint destination_y, GLint destination_z, GLsizei width,
	GLsizei height, GLsizei depth)
{
	struct gl_texture *source = name_get(&textures, source_name);
	struct gl_texture *destination = name_get(&textures, destination_name);
	GX2Rect rectangle;
	GX2Point point;

	(void)source_target; (void)destination_target; (void)depth;
	if (!video_ready || !source || !destination || !source->allocated || !destination->allocated ||
		source->depth || destination->depth)
	{
		return;
	}
	texture_for_sampling(source);
	rectangle.left = source_x;
	rectangle.top = source_y;
	rectangle.right = source_x + width;
	rectangle.bottom = source_y + height;
	point.x = destination_x;
	point.y = destination_y;
	GX2CopySurfaceEx(&source->texture.surface, (uint32_t)source_level, (uint32_t)source_z,
		&destination->texture.surface, (uint32_t)destination_level, (uint32_t)destination_z, 1, &rectangle, &point);
	destination->gpu_written = 1;
	internal_done();
}

/* ---------- GL: samplers */

static void GL_APIENTRY wiiu_glGenSamplers(GLsizei count, GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
	{
		struct sampler_state *state = calloc(1, sizeof(*state));

		names[index] = 0;
		if (!state)
			continue;
		sampler_state_default(state);
		names[index] = name_new(&samplers, state);
	}
}

static void GL_APIENTRY wiiu_glBindSampler(GLuint unit, GLuint sampler)
{
	if (unit < TEXTURE_UNIT_COUNT)
		gl.unit_samplers[unit] = sampler;
}

static void GL_APIENTRY wiiu_glSamplerParameteri(GLuint sampler, GLenum name, GLint value)
{
	struct sampler_state *state = name_get(&samplers, sampler);
	GLfloat values[4] = { (GLfloat)value, 0.0f, 0.0f, 0.0f };

	if (state)
		sampler_parameter(state, name, values);
}

static void GL_APIENTRY wiiu_glSamplerParameterf(GLuint sampler, GLenum name, GLfloat value)
{
	struct sampler_state *state = name_get(&samplers, sampler);
	GLfloat values[4] = { value, 0.0f, 0.0f, 0.0f };

	if (state)
		sampler_parameter(state, name, values);
}

static void GL_APIENTRY wiiu_glSamplerParameterfv(GLuint sampler, GLenum name, const GLfloat *values)
{
	struct sampler_state *state = name_get(&samplers, sampler);

	if (state)
		sampler_parameter(state, name, values);
}

/* ---------- GL: framebuffers */

static void GL_APIENTRY wiiu_glGenFramebuffers(GLsizei count, GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
	{
		struct gl_framebuffer *framebuffer = calloc(1, sizeof(*framebuffer));

		names[index] = framebuffer ? name_new(&framebuffers, framebuffer) : 0;
	}
}

static void GL_APIENTRY wiiu_glDeleteFramebuffers(GLsizei count, const GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
	{
		struct gl_framebuffer *framebuffer = name_get(&framebuffers, names[index]);

		if (!framebuffer)
			continue;
		if (gl.draw_framebuffer == names[index])
			gl.draw_framebuffer = 0;
		if (gl.read_framebuffer == names[index])
			gl.read_framebuffer = 0;
		name_delete(&framebuffers, names[index]);
		free(framebuffer);
	}
}

static void GL_APIENTRY wiiu_glBindFramebuffer(GLenum target, GLuint name)
{
	if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER)
	{
		if (gl.draw_framebuffer != name)
			gl.dirty |= _dirty_targets;
		gl.draw_framebuffer = name;
	}
	if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER)
		gl.read_framebuffer = name;
}

static void GL_APIENTRY wiiu_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum texture_target,
	GLuint texture, GLint level)
{
	struct gl_framebuffer *framebuffer = name_get(&framebuffers,
		target == GL_READ_FRAMEBUFFER ? gl.read_framebuffer : gl.draw_framebuffer);

	(void)texture_target;
	if (!framebuffer)
		return;
	if (attachment == GL_COLOR_ATTACHMENT0)
	{
		framebuffer->color = texture;
		framebuffer->color_level = level;
	}
	else if (attachment == GL_DEPTH_STENCIL_ATTACHMENT || attachment == GL_DEPTH_ATTACHMENT)
	{
		framebuffer->depth = texture;
	}
	gl.dirty |= _dirty_targets;
}

static GLenum GL_APIENTRY wiiu_glCheckFramebufferStatus(GLenum target)
{
	(void)target;
	return GL_FRAMEBUFFER_COMPLETE;
}

static void GL_APIENTRY wiiu_glBlitFramebuffer(GLint source_x0, GLint source_y0, GLint source_x1, GLint source_y1,
	GLint destination_x0, GLint destination_y0, GLint destination_x1, GLint destination_y1, GLbitfield mask,
	GLenum filter)
{
	struct gl_framebuffer *read = name_get(&framebuffers, gl.read_framebuffer);
	struct gl_texture *texture;
	struct draw_targets targets;
	float source[4], destination[4];

	if (!video_ready || !(mask & GL_COLOR_BUFFER_BIT) || !read || !read->color)
		return;
	texture = name_get(&textures, read->color);
	if (!texture || !targets_get(gl.draw_framebuffer, &targets) || !targets.color)
		return;
	source[0] = (float)source_x0;
	source[1] = (float)source_y0;
	source[2] = (float)source_x1;
	source[3] = (float)source_y1;
	destination[0] = (float)destination_x0;
	destination[2] = (float)destination_x1;
	if (targets.screen)
	{
		/* GL's rows on the screen run from the bottom */
		destination[1] = (float)targets.height - (float)destination_y0;
		destination[3] = (float)targets.height - (float)destination_y1;
	}
	else
	{
		destination[1] = (float)destination_y0;
		destination[3] = (float)destination_y1;
	}
	blit_texture(texture, (unsigned int)read->color_level, source, &targets, destination, filter == GL_LINEAR);
}

static void GL_APIENTRY wiiu_glDrawBuffers(GLsizei count, const GLenum *buffers_)
{
	(void)count;
	(void)buffers_;
}

static void GL_APIENTRY wiiu_glGenRenderbuffers(GLsizei count, GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
		names[index] = ++plain_names;
}

static void GL_APIENTRY wiiu_glBindRenderbuffer(GLenum target, GLuint name)
{
	(void)target;
	(void)name;
}

static void GL_APIENTRY wiiu_glRenderbufferStorageMultisample(GLenum target, GLsizei samples,
	GLenum internal_format, GLsizei width, GLsizei height)
{
	/* (multisampling is off: GL_MAX_SAMPLES is 0) */
	(void)target; (void)samples; (void)internal_format; (void)width; (void)height;
}

static void GL_APIENTRY wiiu_glFramebufferRenderbuffer(GLenum target, GLenum attachment,
	GLenum renderbuffer_target, GLuint renderbuffer)
{
	(void)target; (void)attachment; (void)renderbuffer_target; (void)renderbuffer;
}

static void GL_APIENTRY wiiu_glReadBuffer(GLenum source)
{
	(void)source;
}

static void GL_APIENTRY wiiu_glInvalidateFramebuffer(GLenum target, GLsizei count, const GLenum *attachments)
{
	(void)target; (void)count; (void)attachments;
}

/* ---------- GL: buffers */

static GLuint *buffer_binding(GLenum target)
{
	switch (target)
	{
	case GL_ARRAY_BUFFER: return &gl.array_buffer;
	case GL_ELEMENT_ARRAY_BUFFER: return &gl.element_buffer;
	case GL_COPY_WRITE_BUFFER: return &gl.copy_write_buffer;
	default: return &gl.other_buffer;
	}
}

static struct gl_buffer *bound_buffer(GLenum target)
{
	return name_get(&buffers, *buffer_binding(target));
}

static void GL_APIENTRY wiiu_glGenBuffers(GLsizei count, GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
	{
		struct gl_buffer *buffer = calloc(1, sizeof(*buffer));

		names[index] = buffer ? name_new(&buffers, buffer) : 0;
	}
}

static void GL_APIENTRY wiiu_glDeleteBuffers(GLsizei count, const GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
	{
		struct gl_buffer *buffer = name_get(&buffers, names[index]);

		if (!buffer)
			continue;
		if (!buffer->alias)
			gpu_free(buffer->data);
		name_delete(&buffers, names[index]);
		free(buffer);
	}
}

static void GL_APIENTRY wiiu_glBindBuffer(GLenum target, GLuint name)
{
	*buffer_binding(target) = name;
}

/* new storage for the buffer; the old stays until the GPU is done with it
(the renderer's "orphaning") */
static void GL_APIENTRY wiiu_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
	struct gl_buffer *buffer = bound_buffer(target);

	(void)usage;
	if (!buffer)
		return;
	if (!buffer->alias)
		gpu_free(buffer->data);
	buffer->alias = 0;
	buffer->data = gpu_alloc(GX2_VERTEX_BUFFER_ALIGNMENT, (unsigned long)size);
	buffer->size = buffer->data ? (unsigned long)size : 0;
	if (buffer->data && data)
	{
		memcpy(buffer->data, data, (size_t)size);
		DCFlushRange(buffer->data, (uint32_t)size);
	}
}

static void buffer_write(struct gl_buffer *buffer, unsigned long offset, unsigned long size, const void *data)
{
	if (!buffer || !buffer->data || offset + size > buffer->size)
		return;
	memcpy(buffer->data + offset, data, size);
	DCFlushRange(buffer->data + offset, (uint32_t)size);
}

static void GL_APIENTRY wiiu_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
	buffer_write(bound_buffer(target), (unsigned long)offset, (unsigned long)size, data);
}

static void GL_APIENTRY wiiu_glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	(void)target; (void)index; (void)buffer;
}

static void GL_APIENTRY wiiu_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset,
	GLsizeiptr size)
{
	(void)target; (void)index; (void)buffer; (void)offset; (void)size;
}

/* the game's own memory as a buffer, read by the GPU where it is
(d3d8_gl.c, mirror_range) */
void wiiu_gl_buffer_alias(GLuint name, void *memory, unsigned long size)
{
	struct gl_buffer *buffer = name_get(&buffers, name);

	if (!buffer)
		return;
	if (!buffer->alias)
		gpu_free(buffer->data);
	buffer->data = memory;
	buffer->size = size;
	buffer->alias = 1;
}

/* what the CPU wrote to a range is written back from its cache, for the GPU
to read (the GPU's own caches are cleared once a frame: platform_video_swap) */
void wiiu_gl_buffer_flush(const void *memory, unsigned long size)
{
	DCFlushRange((void *)memory, (uint32_t)size);
}

/* ---------- GL: vertex arrays and attributes */

static void GL_APIENTRY wiiu_glGenVertexArrays(GLsizei count, GLuint *names)
{
	GLsizei index;

	/* (one set of attributes for all: the renderer binds one vertex array
	and keeps it) */
	for (index = 0; index < count; index++)
		names[index] = ++plain_names;
}

static void GL_APIENTRY wiiu_glBindVertexArray(GLuint name)
{
	(void)name;
}

static void GL_APIENTRY wiiu_glEnableVertexAttribArray(GLuint index)
{
	if (index < ATTRIBUTE_COUNT)
		gl.attributes[index].enabled = 1;
}

static void GL_APIENTRY wiiu_glDisableVertexAttribArray(GLuint index)
{
	if (index < ATTRIBUTE_COUNT)
		gl.attributes[index].enabled = 0;
}

static void attribute_pointer(GLuint index, GLint size, GLenum type, int normalized, int integer, GLsizei stride,
	const void *pointer)
{
	struct attribute *attribute;

	if (index >= ATTRIBUTE_COUNT)
		return;
	attribute = &gl.attributes[index];
	attribute->buffer = gl.array_buffer;
	attribute->size = size;
	attribute->type = type;
	attribute->normalized = normalized;
	attribute->integer = integer;
	attribute->offset = (unsigned long)pointer;
	/* (0: the elements packed one after another) */
	if (!stride)
	{
		int components = size == GL_BGRA_EXT ? 4 : size;
		int bytes = type == GL_FLOAT || type == GL_INT || type == GL_UNSIGNED_INT ? 4 :
			type == GL_SHORT || type == GL_UNSIGNED_SHORT ? 2 : 1;

		stride = components * bytes;
	}
	attribute->stride = stride;
}

static void GL_APIENTRY wiiu_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
	GLsizei stride, const void *pointer)
{
	attribute_pointer(index, size, type, normalized != GL_FALSE, 0, stride, pointer);
}

static void GL_APIENTRY wiiu_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride,
	const void *pointer)
{
	attribute_pointer(index, size, type, 0, 1, stride, pointer);
}

static void GL_APIENTRY wiiu_glVertexAttrib4fv(GLuint index, const GLfloat *value)
{
	if (index >= ATTRIBUTE_COUNT)
		return;
	memcpy(gl.attributes[index].value, value, sizeof(gl.attributes[index].value));
	gl.attributes[index].integer_value = 0;
}

static void GL_APIENTRY wiiu_glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	if (index >= ATTRIBUTE_COUNT)
		return;
	gl.attributes[index].value_integer[0] = x;
	gl.attributes[index].value_integer[1] = y;
	gl.attributes[index].value_integer[2] = z;
	gl.attributes[index].value_integer[3] = w;
	gl.attributes[index].integer_value = 1;
}

/* ---------- GL: drawing */

static void GL_APIENTRY wiiu_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	if (count <= 0 || !draw_prepare())
		return;
	if (tracing())
		platform_log("GX2 draw: arrays mode %04x count %d first %d", mode, count, first);
	GX2DrawEx(primitive_mode(mode), (uint32_t)count, (uint32_t)first, 1);
}

static void draw_elements(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint base_vertex)
{
	struct gl_buffer *buffer = name_get(&buffers, gl.element_buffer);
	const unsigned char *pointer;

	if (count <= 0 || !buffer || !buffer->data || (unsigned long)indices >= buffer->size || !draw_prepare())
		return;
	pointer = buffer->data + (unsigned long)indices;
	/* (the indices are the CPU's 16-bit numbers, in its byte order; the
	base vertex wraps around, as a negative one must) */
	GX2DrawIndexedEx(primitive_mode(mode), (uint32_t)count,
		type == GL_UNSIGNED_INT ? GX2_INDEX_TYPE_U32 : GX2_INDEX_TYPE_U16, pointer, (uint32_t)base_vertex, 1);
}

static void GL_APIENTRY wiiu_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
	draw_elements(mode, count, type, indices, 0);
}

static void GL_APIENTRY wiiu_glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices,
	GLint base_vertex)
{
	draw_elements(mode, count, type, indices, base_vertex);
}

/* ---------- GL: shaders and programs */

/* Varyings (a vertex shader's outputs, a pixel shader's inputs) are
matched by location, which CafeGLSL gives each shader by itself: a varying
declared without one is given one here, the same for the same name in
every shader. */
#define VARYING_NAMES 32
static char varying_names[VARYING_NAMES][32];
static int varying_count;

static int varying_location(const char *name)
{
	int index;

	for (index = 0; index < varying_count; index++)
	{
		if (!strcmp(varying_names[index], name))
			return index;
	}
	if (varying_count == VARYING_NAMES)
		return -1;
	snprintf(varying_names[varying_count], sizeof(varying_names[0]), "%s", name);
	return varying_count++;
}

/* the source with "layout(location = n)" before each of the varyings
declared without one: lines "out <type> <name>;" in a vertex shader, "in
<type> <name>;" in a pixel shader */
static char *varyings_located(const char *source, int pixel)
{
	const char *keyword = pixel ? "in " : "out ";
	size_t keyword_length = strlen(keyword);
	size_t size = strlen(source) * 2 + 64;
	char *result = malloc(size);
	char *out = result;
	const char *line = source;

	if (!result)
		return NULL;
	while (*line)
	{
		const char *end = strchr(line, '\n');
		size_t length = end ? (size_t)(end - line + 1) : strlen(line);
		const char *text = line;

		while (*text == ' ' || *text == '\t')
			text++;
		if (!strncmp(text, keyword, keyword_length))
		{
			/* the name: the last word before the semicolon */
			const char *semicolon = memchr(text, ';', length - (size_t)(text - line));
			const char *name_end = semicolon, *name_start;

			if (semicolon)
			{
				char name[32];
				int location;

				while (name_end > text && (name_end[-1] == ' ' || name_end[-1] == ']' || name_end[-1] == '['))
					name_end--;
				name_start = name_end;
				while (name_start > text && (isalnum((unsigned char)name_start[-1]) || name_start[-1] == '_'))
					name_start--;
				if (name_end - name_start > 0 && (size_t)(name_end - name_start) < sizeof(name))
				{
					memcpy(name, name_start, (size_t)(name_end - name_start));
					name[name_end - name_start] = 0;
					location = varying_location(name);
					if (location >= 0)
						out += sprintf(out, "layout(location = %d) ", location);
				}
			}
		}
		memcpy(out, line, length);
		out += length;
		line += length;
	}
	*out = 0;
	return result;
}

static GLuint GL_APIENTRY wiiu_glCreateShader(GLenum type)
{
	struct gl_shader *shader = calloc(1, sizeof(*shader));

	if (!shader)
		return 0;
	shader->type = type;
	return name_new(&shaders, shader);
}

static void GL_APIENTRY wiiu_glShaderSource(GLuint name, GLsizei count, const GLchar *const *strings,
	const GLint *lengths)
{
	struct gl_shader *shader = name_get(&shaders, name);
	size_t total = 0, length;
	GLsizei index;

	if (!shader)
		return;
	for (index = 0; index < count; index++)
		total += lengths && lengths[index] >= 0 ? (size_t)lengths[index] : strlen(strings[index]);
	free(shader->source);
	shader->source = malloc(total + 1);
	if (!shader->source)
		return;
	total = 0;
	for (index = 0; index < count; index++)
	{
		length = lengths && lengths[index] >= 0 ? (size_t)lengths[index] : strlen(strings[index]);
		memcpy(shader->source + total, strings[index], length);
		total += length;
	}
	shader->source[total] = 0;
}

static void GL_APIENTRY wiiu_glCompileShader(GLuint name)
{
	struct gl_shader *shader = name_get(&shaders, name);
	int pixel;
	char *source;
	GX2ShaderMode mode;
	int attempt;

	if (!shader || !shader->source)
		return;
	pixel = shader->type == GL_FRAGMENT_SHADER;
	source = varyings_located(shader->source, pixel);
	if (!source)
		return;
	shader->log[0] = 0;
	if (pixel)
		mode = CompilePixelShader(source, GLSL_COMPILE_REGISTER, &shader->pixel, shader->log, sizeof(shader->log),
			GLSL_COMPILER_FLAG_NONE);
	else
		mode = CompileVertexShader(source, GLSL_COMPILE_REGISTER, &shader->vertex, shader->log, sizeof(shader->log),
			GLSL_COMPILER_FLAG_NONE);
	shader->compiled = mode != GLSL_SHADER_MODE_ERROR;
	/* CafeGLSL now and then hands back an empty shell for a whole vertex
	shader: 16 bytes, no inputs, no uniforms, with no error (the game's very
	first one, compiled as the intro starts; r18 and r19). A vertex shader
	of the game's always reads an input, so one with none is compiled again;
	the second try has come back whole. */
	for (attempt = 0; attempt < 2 && shader->compiled && !pixel && !shader->vertex->attribVarCount; attempt++)
	{
		platform_log("GX2: vertex shader %u came back empty (%u bytes); compiling it again", name,
			(unsigned)shader->vertex->size);
		shader->log[0] = 0;
		mode = CompileVertexShader(source, GLSL_COMPILE_REGISTER, &shader->vertex, shader->log, sizeof(shader->log),
			GLSL_COMPILER_FLAG_NONE);
		shader->compiled = mode != GLSL_SHADER_MODE_ERROR;
	}
	free(source);
	/* the GPU reads the compiled code from memory, past the CPU's cache */
	if (shader->compiled && pixel)
		GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, shader->pixel->program, shader->pixel->size);
	else if (shader->compiled)
		GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, shader->vertex->program, shader->vertex->size);
}

static void GL_APIENTRY wiiu_glGetShaderiv(GLuint name, GLenum parameter, GLint *value)
{
	struct gl_shader *shader = name_get(&shaders, name);

	*value = 0;
	if (!shader)
		return;
	if (parameter == GL_COMPILE_STATUS)
		*value = shader->compiled;
	else if (parameter == GL_INFO_LOG_LENGTH)
		*value = (GLint)strlen(shader->log) + 1;
	else if (parameter == GL_SHADER_TYPE)
		*value = (GLint)shader->type;
}

static void GL_APIENTRY wiiu_glGetShaderInfoLog(GLuint name, GLsizei size, GLsizei *length, GLchar *log)
{
	struct gl_shader *shader = name_get(&shaders, name);
	int written = size > 0 ? snprintf(log, (size_t)size, "%s", shader ? shader->log : "") : 0;

	if (length)
		*length = written < size ? written : size - 1;
}

static void GL_APIENTRY wiiu_glDeleteShader(GLuint name)
{
	struct gl_shader *shader = name_get(&shaders, name);

	/* (the compiled code stays: programs made from it go on using it) */
	if (shader)
	{
		free(shader->source);
		shader->source = NULL;
	}
}

static GLuint GL_APIENTRY wiiu_glCreateProgram(void)
{
	struct gl_program *program = calloc(1, sizeof(*program));

	return program ? name_new(&programs, program) : 0;
}

static void GL_APIENTRY wiiu_glAttachShader(GLuint program_name, GLuint shader_name)
{
	struct gl_program *program = name_get(&programs, program_name);
	struct gl_shader *shader = name_get(&shaders, shader_name);

	if (!program || !shader)
		return;
	if (shader->type == GL_FRAGMENT_SHADER)
		program->pixel_name = shader_name;
	else
		program->vertex_name = shader_name;
}

static void GL_APIENTRY wiiu_glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
	/* (the renderer's attributes have their locations in the shader) */
	(void)program; (void)index; (void)name;
}

/* a uniform's entry in the program by its name (without "[0]"), made the
first time */
static struct uniform_entry *uniform_entry_get(struct gl_program *program, const char *name)
{
	unsigned int index;
	struct uniform_entry *entry, *uniforms;

	for (index = 0; index < program->uniform_count; index++)
	{
		if (!strcmp(program->uniforms[index].name, name))
			return &program->uniforms[index];
	}
	uniforms = realloc(program->uniforms, (program->uniform_count + 1) * sizeof(*uniforms));
	if (!uniforms)
		return NULL;
	program->uniforms = uniforms;
	entry = &uniforms[program->uniform_count++];
	memset(entry, 0, sizeof(*entry));
	snprintf(entry->name, sizeof(entry->name), "%s", name);
	entry->vertex_offset = entry->pixel_offset = -1;
	entry->unit = 0;
	return entry;
}

/* a stage's uniform variables into the program's table: the arrays once,
by their names alone (CafeGLSL also lists each element, "c[1]") */
static unsigned int uniforms_take(struct gl_program *program, const GX2UniformVar *variables, unsigned int count,
	int pixel)
{
	unsigned int index, registers = 0;

	for (index = 0; index < count; index++)
	{
		const GX2UniformVar *variable = &variables[index];
		struct uniform_entry *entry;
		unsigned int elements = variable->count ? variable->count : 1;
		char name[48];
		const char *bracket = strchr(variable->name, '[');

		if (bracket && strcmp(bracket, "[0]"))
			continue;
		snprintf(name, sizeof(name), "%.*s", bracket ? (int)(bracket - variable->name) : (int)strlen(variable->name),
			variable->name);
		entry = uniform_entry_get(program, name);
		if (!entry)
			continue;
		if (pixel)
			entry->pixel_offset = (int)variable->offset;
		else
			entry->vertex_offset = (int)variable->offset;
		if (entry->count < elements)
			entry->count = elements;
		if (registers < variable->offset + elements * 4)
			registers = variable->offset + elements * 4;
	}
	return registers;
}

static void GL_APIENTRY wiiu_glLinkProgram(GLuint name)
{
	struct gl_program *program = name_get(&programs, name);
	struct gl_shader *vertex, *pixel;
	unsigned int index;

	if (!program)
		return;
	vertex = name_get(&shaders, program->vertex_name);
	pixel = name_get(&shaders, program->pixel_name);
	program->linked = 0;
	if (!vertex || !pixel || !vertex->compiled || !pixel->compiled)
	{
		snprintf(program->log, sizeof(program->log), "a shader did not compile");
		return;
	}
	program->vertex = vertex->vertex;
	program->pixel = pixel->pixel;
	free(program->uniforms);
	program->uniforms = NULL;
	program->uniform_count = 0;
	program->vertex_register_count = uniforms_take(program, program->vertex->uniformVars,
		program->vertex->uniformVarCount, 0);
	program->pixel_register_count = uniforms_take(program, program->pixel->uniformVars,
		program->pixel->uniformVarCount, 1);
	for (index = 0; index < program->pixel->samplerVarCount; index++)
	{
		const GX2SamplerVar *variable = &program->pixel->samplerVars[index];
		struct uniform_entry *entry = uniform_entry_get(program, variable->name);

		if (!entry)
			continue;
		entry->sampler = 1;
		entry->slot = variable->location;
		entry->sampler_type = variable->type;
		entry->count = 1;
	}
	free(program->vertex_registers);
	free(program->pixel_registers);
	program->vertex_registers = calloc(program->vertex_register_count + 4, sizeof(float));
	program->pixel_registers = calloc(program->pixel_register_count + 4, sizeof(float));
	if (!program->vertex_registers || !program->pixel_registers)
	{
		snprintf(program->log, sizeof(program->log), "no memory");
		return;
	}
	program->vertex_dirty_first = program->pixel_dirty_first = ~0u;
	program->vertex_dirty_last = program->pixel_dirty_last = 0;
	program->linked = 1;
	/* (DEBUGGING, TEMPORARY: what each program's compiled shaders hold) */
	if (config_integer("debug.gpu_trace_frame") >= 0)
		platform_log("GX2 link: program %u: vertex shader %u at %p: %u attributes, %u uniforms, %u bytes, "
			"%u registers; pixel shader %u at %p: %u uniforms, %u samplers, %u registers", name,
			program->vertex_name, (void *)program->vertex, program->vertex->attribVarCount,
			program->vertex->uniformVarCount, program->vertex->size, program->vertex_register_count,
			program->pixel_name, (void *)program->pixel, program->pixel->uniformVarCount,
			program->pixel->samplerVarCount, program->pixel_register_count);
	if (registers_program == program)
		registers_program = NULL;
}

static void GL_APIENTRY wiiu_glGetProgramiv(GLuint name, GLenum parameter, GLint *value)
{
	struct gl_program *program = name_get(&programs, name);

	*value = 0;
	if (!program)
		return;
	if (parameter == GL_LINK_STATUS)
		*value = program->linked;
	else if (parameter == GL_INFO_LOG_LENGTH)
		*value = (GLint)strlen(program->log) + 1;
}

static void GL_APIENTRY wiiu_glGetProgramInfoLog(GLuint name, GLsizei size, GLsizei *length, GLchar *log)
{
	struct gl_program *program = name_get(&programs, name);
	int written = size > 0 ? snprintf(log, (size_t)size, "%s", program ? program->log : "") : 0;

	if (length)
		*length = written < size ? written : size - 1;
}

static void GL_APIENTRY wiiu_glUseProgram(GLuint name)
{
	gl.program = name;
}

/* A uniform's location is its entry's number times 256, plus the array
element: c[i] is c's location plus i, as the renderer expects. */
static GLint GL_APIENTRY wiiu_glGetUniformLocation(GLuint name, const GLchar *uniform_name)
{
	struct gl_program *program = name_get(&programs, name);
	char base[48];
	const char *bracket = strchr(uniform_name, '[');
	unsigned long element = bracket ? strtoul(bracket + 1, NULL, 10) : 0;
	unsigned int index;

	if (!program || !program->linked)
		return -1;
	snprintf(base, sizeof(base), "%.*s", bracket ? (int)(bracket - uniform_name) : (int)strlen(uniform_name),
		uniform_name);
	for (index = 0; index < program->uniform_count; index++)
	{
		if (!strcmp(program->uniforms[index].name, base))
			return element < program->uniforms[index].count && element < 256 ? (GLint)(index * 256 + element) : -1;
	}
	return -1;
}

/* the current program's uniform at a location, and the element there */
static struct uniform_entry *uniform_at(GLint location, struct gl_program **program_out, unsigned int *element)
{
	struct gl_program *program = name_get(&programs, gl.program);
	unsigned int index;

	if (!program || location < 0)
		return NULL;
	index = (unsigned int)location / 256;
	if (index >= program->uniform_count)
		return NULL;
	*program_out = program;
	*element = (unsigned int)location % 256;
	return &program->uniforms[index];
}

/* values for elements of a uniform, components floats each (an element is
a whole register) */
static void uniform_store(GLint location, GLsizei count, const float *values, unsigned int components)
{
	struct gl_program *program;
	unsigned int element, index;
	struct uniform_entry *entry = uniform_at(location, &program, &element);

	if (!entry || entry->sampler || element >= entry->count)
		return;
	if ((unsigned int)count > entry->count - element)
		count = (GLsizei)(entry->count - element);
	for (index = 0; index < (unsigned int)count; index++)
	{
		if (entry->vertex_offset >= 0)
		{
			unsigned int offset = (unsigned int)entry->vertex_offset + (element + index) * 4;

			memcpy(program->vertex_registers + offset, values + index * components, components * sizeof(float));
			registers_mark(program, 0, offset, components);
		}
		if (entry->pixel_offset >= 0)
		{
			unsigned int offset = (unsigned int)entry->pixel_offset + (element + index) * 4;

			memcpy(program->pixel_registers + offset, values + index * components, components * sizeof(float));
			registers_mark(program, 1, offset, components);
		}
	}
}

static void GL_APIENTRY wiiu_glUniform1i(GLint location, GLint value)
{
	struct gl_program *program;
	unsigned int element;
	struct uniform_entry *entry = uniform_at(location, &program, &element);

	if (!entry)
		return;
	if (entry->sampler)
	{
		entry->unit = value;
	}
	else
	{
		/* (an int uniform: the bits of the int in its register) */
		float bits;

		memcpy(&bits, &value, sizeof(bits));
		uniform_store(location, 1, &bits, 1);
	}
}

static void GL_APIENTRY wiiu_glUniform1iv(GLint location, GLsizei count, const GLint *values)
{
	GLsizei index;

	for (index = 0; index < count; index++)
		wiiu_glUniform1i(location + index, values[index]);
}

static void GL_APIENTRY wiiu_glUniform1f(GLint location, GLfloat value)
{
	uniform_store(location, 1, &value, 1);
}

static void GL_APIENTRY wiiu_glUniform2f(GLint location, GLfloat x, GLfloat y)
{
	float values[2] = { x, y };

	uniform_store(location, 1, values, 2);
}

static void GL_APIENTRY wiiu_glUniform4fv(GLint location, GLsizei count, const GLfloat *values)
{
	uniform_store(location, count, values, 4);
}

/* ---------- GL: queries (always "visible": wut has no GX2 query calls) */

static void GL_APIENTRY wiiu_glGenQueries(GLsizei count, GLuint *names)
{
	GLsizei index;

	for (index = 0; index < count; index++)
		names[index] = ++plain_names;
}

static void GL_APIENTRY wiiu_glBeginQuery(GLenum target, GLuint name)
{
	(void)target;
	(void)name;
}

static void GL_APIENTRY wiiu_glEndQuery(GLenum target)
{
	(void)target;
}

static void GL_APIENTRY wiiu_glGetQueryObjectuiv(GLuint name, GLenum parameter, GLuint *value)
{
	(void)name;
	*value = parameter == GL_QUERY_RESULT_AVAILABLE ? 1 : 1;
}

/* ---------- the function pointers gl.h declares */

#define GL_DEFINE_FUNCTION(name) __typeof__(&name) halo_##name = wiiu_##name;
GL_FUNCTIONS(GL_DEFINE_FUNCTION)
#undef GL_DEFINE_FUNCTION

/* ---------- what the renderer asks of the host (guest_host.h on Android) */

int host_gl_has_extension(const char *name)
{
	return !strcmp(name, "GL_EXT_texture_filter_anisotropic") || !strcmp(name, "GL_EXT_texture_border_clamp") ||
		!strcmp(name, "GL_EXT_copy_image");
}

unsigned int host_gl_read_buffer_word(unsigned int name, unsigned int offset)
{
	struct gl_buffer *buffer = name_get(&buffers, name);
	unsigned int word = 0;

	if (buffer && buffer->data && offset + 4 <= buffer->size)
	{
		DCInvalidateRange(buffer->data + offset, 4);
		memcpy(&word, buffer->data + offset, 4);
	}
	return word;
}

void host_gl_buffer_write(unsigned int target, unsigned int offset, unsigned int size, const void *data)
{
	buffer_write(bound_buffer(target), offset, size, data);
}

/* the frame just sent to the GPU used the stream buffers of this slot */
void host_gl_fence_frame(unsigned int slot)
{
	if (video_ready)
		frame_fences[slot % FENCE_SLOTS] = GX2GetLastSubmittedTimeStamp();
}

/* the stream buffers of this slot are about to be written again: the GPU
must be done with the frame that last used them */
void host_gl_wait_frame(unsigned int slot)
{
	slot %= FENCE_SLOTS;
	if (video_ready && frame_fences[slot])
		GX2WaitTimeStamp(frame_fences[slot]);
	/* (the attribute values' ring follows the same frames) */
	constant_ring_part = slot;
	constant_ring_offset = 0;
}

/* ---------- the platform's video (sdl_platform.h) */

int platform_video_initialize(unsigned long width, unsigned long height)
{
	GLsizei unit;

	(void)width;
	(void)height;
	if (video_ready)
		return 1;
	platform_log("GX2: starting");
	if (!WHBGfxInit())
	{
		platform_log("GX2: WHBGfxInit failed");
		return 0;
	}
	platform_log("GX2: WHBGfxInit done");
	/* a context of our own: WHBGfx's are for its own drawing */
	context_state = gpu_alloc(GX2_CONTEXT_STATE_ALIGNMENT, sizeof(GX2ContextState));
	if (!context_state)
		return 0;
	GX2SetupContextStateEx(context_state, TRUE);
	GX2SetContextState(context_state);
	GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_REGISTER);
	/* depth in clip space runs -1 to 1, as in OpenGL (nv2a_vsh.c maps the
	Xbox's 0 to 1 onto it) */
	GX2SetRasterizerClipControlEx(TRUE, TRUE, FALSE);
	GX2SetAlphaTest(FALSE, GX2_COMPARE_FUNC_ALWAYS, 0.0f);
	GX2SetPointSize(1.0f, 1.0f);
	GX2SetPointLimits(0.0f, 64.0f);

	/* the screen: framebuffer 0 */
	memset(&screen, 0, sizeof(screen));
	screen.surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
	screen.surface.width = SCREEN_WIDTH;
	screen.surface.height = SCREEN_HEIGHT;
	screen.surface.depth = 1;
	screen.surface.mipLevels = 1;
	screen.surface.format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;
	screen.surface.aa = GX2_AA_MODE1X;
	screen.surface.use = GX2_SURFACE_USE_TEXTURE_COLOR_BUFFER_TV;
	screen.surface.tileMode = GX2_TILE_MODE_DEFAULT;
	screen.viewNumSlices = 1;
	GX2CalcSurfaceSizeAndAlignment(&screen.surface);
	GX2InitColorBufferRegs(&screen);
	screen.surface.image = gpu_alloc(screen.surface.alignment, screen.surface.imageSize);
	constant_ring = gpu_alloc(0x100, CONSTANT_RING_PART * FENCE_SLOTS);
	if (!screen.surface.image || !constant_ring)
		return 0;
	memset(screen.surface.image, 0, screen.surface.imageSize);
	GX2Invalidate(GX2_INVALIDATE_MODE_CPU, screen.surface.image, screen.surface.imageSize);

	/* GL's initial state */
	memset(&gl, 0, sizeof(gl));
	gl.dirty = _dirty_all;
	gl.viewport[2] = SCREEN_WIDTH;
	gl.viewport[3] = SCREEN_HEIGHT;
	gl.scissor[2] = SCREEN_WIDTH;
	gl.scissor[3] = SCREEN_HEIGHT;
	gl.depth_range[1] = 1.0f;
	gl.clear_depth = 1.0f;
	gl.color_mask[0] = gl.color_mask[1] = gl.color_mask[2] = gl.color_mask[3] = GL_TRUE;
	gl.depth_mask = GL_TRUE;
	gl.depth_function = GL_LESS;
	gl.stencil_function = GL_ALWAYS;
	gl.stencil_value_mask = 0xff;
	gl.stencil_write_mask = 0xff;
	gl.stencil_operations[0] = gl.stencil_operations[1] = gl.stencil_operations[2] = GL_KEEP;
	gl.blend_source = GL_ONE;
	gl.blend_destination = GL_ZERO;
	gl.blend_equation = GL_FUNC_ADD;
	gl.cull_mode = GL_BACK;
	gl.front_face = GL_CCW;
	for (unit = 0; unit < ATTRIBUTE_COUNT; unit++)
		gl.attributes[unit].value[3] = 1.0f;

	platform_log("GX2: screen and context set up");
	log_free_memory("before the shader compiler starts");
	compiler_start();
	internal_programs_initialize();
	video_ready = 1;
	platform_log("GX2: ready, drawing at %dx%d", SCREEN_WIDTH, SCREEN_HEIGHT);
	return internal_ready;
}

void platform_video_drawable_size(int *width, int *height)
{
	*width = SCREEN_WIDTH;
	*height = SCREEN_HEIGHT;
}

void platform_video_window_size(int *width, int *height)
{
	platform_video_drawable_size(width, height);
}

/* the screen to the TV and the GamePad, at the next vertical blank */
void platform_video_swap(void)
{
	uint32_t swaps, flips;
	OSTime last_flip, last_vsync;

	if (!video_ready)
		return;
	GX2CopyColorBufferToScanBuffer(&screen, GX2_SCAN_TARGET_TV);
	GX2CopyColorBufferToScanBuffer(&screen, GX2_SCAN_TARGET_DRC);
	GX2SwapScanBuffers();
	GX2Flush();
	if (!screen_shown)
	{
		screen_shown = 1;
		GX2SetTVEnable(TRUE);
		GX2SetDRCEnable(TRUE);
	}
	/* no more than two frames waiting to be shown */
	GX2GetSwapStatus(&swaps, &flips, &last_flip, &last_vsync);
	if (swaps - flips > 2)
		GX2WaitForFlip();
	/* the copies set state of their own: ours again, all of it, and the
	GPU's vertex caches cleared of last frame's vertices */
	GX2SetContextState(context_state);
	current_color = NULL;
	current_depth = NULL;
	internal_done();
	GX2Invalidate(GX2_INVALIDATE_MODE_ATTRIBUTE_BUFFER, (void *)0, 0xffffffff);
	presented_frames++;
	gpu_free_collect();
}

/* (the HOME button and the rest of the Wii U's system events: later) */
void platform_pump_events(void)
{
}

void platform_menus_set_active(int active)
{
	(void)active;
}
