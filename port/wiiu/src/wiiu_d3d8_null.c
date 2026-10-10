/*
WIIU_D3D8_NULL.C

Direct3D 8 with nothing drawn, for the Wii U build until it has a GX2
renderer (port/linux/src/d3d8_gl.c is the Linux build's, with OpenGL). The
game runs as it would with a picture: the device keeps the state the game
sets and reads back (render states, transforms, the viewport, its targets),
its frames are paced by a 60 Hz vertical blank as the Xbox's are, and every
draw is dropped.
*/

#include "xbox_texture_layout.h"
#include "halo_ui_pointer.h"

#include <errno.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

/* ---------- state the XDK header's inline functions read and write */

DWORD D3D__RenderState[D3DRS_MAX];
DWORD D3D__TextureState[D3DTSS_MAXSTAGES][D3DTSS_MAX];
WORD *D3D__IndexData;

/* the "XDK" device; the game only ever holds a pointer to it */
static struct
{
	BOOL created;
	D3DSurface back_buffer;
	D3DSurface depth_buffer;
	D3DSurface *render_target;
	D3DSurface *depth_stencil;
	D3DMATRIX transforms[D3DTS_MAX];
	D3DVIEWPORT8 viewport;
} device;

/* ---------- vertical blank: a thread ticking 60 times a second, as the
Xbox's display does; a presented frame is shown at the next one */

#define VERTICAL_BLANK_NANOSECONDS (1000000000L / 60)

static pthread_mutex_t vertical_blank_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t vertical_blank_condition = PTHREAD_COND_INITIALIZER;
static D3DCALLBACK vertical_blank_callback;
static unsigned long vertical_blank_count;
static volatile unsigned int flip_count;
static unsigned long pending_flips;
static BOOL vertical_blank_thread_started = FALSE;

static void *vertical_blank_thread(void *unused)
{
	(void)unused;
	for (;;)
	{
		struct timespec wait = { 0, VERTICAL_BLANK_NANOSECONDS };
		D3DCALLBACK callback;

		while (nanosleep(&wait, &wait) == -1 && errno == EINTR)
			;
		pthread_mutex_lock(&vertical_blank_lock);
		vertical_blank_count++;
		if (pending_flips)
		{
			pending_flips--;
			flip_count++;
		}
		callback = vertical_blank_callback;
		pthread_cond_broadcast(&vertical_blank_condition);
		pthread_mutex_unlock(&vertical_blank_lock);
		if (callback)
			callback(0);
	}
	return NULL;
}

static void vertical_blank_start(void)
{
	pthread_mutex_lock(&vertical_blank_lock);
	if (!vertical_blank_thread_started)
	{
		pthread_attr_t attributes;
		pthread_t thread;
		int created;

		/* (the game's vertical blank callback runs on it) */
		pthread_attr_init(&attributes);
		pthread_attr_setstacksize(&attributes, 0x40000);
		created = pthread_create(&thread, &attributes, vertical_blank_thread, NULL) == 0;
		pthread_attr_destroy(&attributes);
		if (created)
		{
			pthread_detach(thread);
			vertical_blank_thread_started = TRUE;
		}
		else
		{
			platform_log("cannot start the vertical blank thread");
		}
	}
	pthread_mutex_unlock(&vertical_blank_lock);
}

/* replaces main/d3d_intimacy.cpp, which reads the counter out of the Xbox
Direct3D runtime's private device structure */
volatile unsigned int *d3d_find_flipcount(void)
{
	return &flip_count;
}

void WINAPI D3DDevice_SetVerticalBlankCallback(D3DCALLBACK callback)
{
	pthread_mutex_lock(&vertical_blank_lock);
	vertical_blank_callback = callback;
	pthread_mutex_unlock(&vertical_blank_lock);
	vertical_blank_start();
}

void WINAPI D3DDevice_BlockUntilVerticalBlank(void)
{
	unsigned long count;

	vertical_blank_start();
	pthread_mutex_lock(&vertical_blank_lock);
	count = vertical_blank_count;
	while (vertical_blank_count == count)
		pthread_cond_wait(&vertical_blank_condition, &vertical_blank_lock);
	pthread_mutex_unlock(&vertical_blank_lock);
}

void WINAPI D3DDevice_Present(CONST RECT *source_rectangle, CONST RECT *destination_rectangle,
	void *unused, void *unused2)
{
	static unsigned long frame;

	(void)source_rectangle;
	(void)destination_rectangle;
	(void)unused;
	(void)unused2;
	/* (a sign of life in the log while nothing is on screen) */
	if (++frame % 600 == 1)
		platform_log("frame %lu", frame);
	vertical_blank_start();
	pthread_mutex_lock(&vertical_blank_lock);
	/* the Xbox keeps at most two frames queued behind its display */
	while (pending_flips >= 2)
		pthread_cond_wait(&vertical_blank_condition, &vertical_blank_lock);
	pending_flips++;
	pthread_mutex_unlock(&vertical_blank_lock);
}

HRESULT WINAPI D3DDevice_PersistDisplay(void)
{
	return S_OK;
}

/* ---------- the device */

Direct3D *WINAPI Direct3DCreate8(UINT sdk_version)
{
	(void)sdk_version;
	return (Direct3D *)1;
}

void WINAPI Direct3D_SetPushBufferSize(DWORD push_buffer_size, DWORD segment_count)
{
	(void)push_buffer_size;
	(void)segment_count;
}

HRESULT WINAPI Direct3D_CreateDevice(UINT adapter, D3DDEVTYPE device_type, void *unused, DWORD behavior_flags,
	D3DPRESENT_PARAMETERS *presentation_parameters, D3DDevice **returned_device)
{
	(void)adapter;
	(void)device_type;
	(void)unused;
	(void)behavior_flags;
	if (!device.created)
	{
		unsigned long width = presentation_parameters && presentation_parameters->BackBufferWidth ?
			presentation_parameters->BackBufferWidth : SCREEN_WIDTH;
		unsigned long height = presentation_parameters && presentation_parameters->BackBufferHeight ?
			presentation_parameters->BackBufferHeight : SCREEN_HEIGHT;
		int index;

		d3d8_surface_initialize(&device.back_buffer, D3DFMT_LIN_A8R8G8B8, width, height);
		d3d8_surface_initialize(&device.depth_buffer, D3DFMT_LIN_D24S8, width, height);
		device.render_target = &device.back_buffer;
		device.depth_stencil = &device.depth_buffer;
		for (index = 0; index < D3DTS_MAX; index++)
		{
			device.transforms[index]._11 = 1.0f;
			device.transforms[index]._22 = 1.0f;
			device.transforms[index]._33 = 1.0f;
			device.transforms[index]._44 = 1.0f;
		}
		device.viewport.Width = width;
		device.viewport.Height = height;
		device.viewport.MaxZ = 1.0f;
		D3D__RenderState[D3DRS_ZENABLE] = TRUE;
		D3D__RenderState[D3DRS_ZWRITEENABLE] = TRUE;
		D3D__RenderState[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL;
		D3D__RenderState[D3DRS_COLORWRITEENABLE] = D3DCOLORWRITEENABLE_ALL;
		D3D__RenderState[D3DRS_SRCBLEND] = D3DBLEND_ONE;
		D3D__RenderState[D3DRS_DESTBLEND] = D3DBLEND_ZERO;
		D3D__RenderState[D3DRS_BLENDOP] = D3DBLENDOP_ADD;
		D3D__RenderState[D3DRS_CULLMODE] = D3DCULL_CCW;
		D3D__RenderState[D3DRS_FRONTFACE] = D3DFRONT_CW;
		D3D__RenderState[D3DRS_FILLMODE] = D3DFILL_SOLID;
		D3D__RenderState[D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS;
		D3D__RenderState[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
		D3D__RenderState[D3DRS_STENCILMASK] = 0xff;
		D3D__RenderState[D3DRS_STENCILWRITEMASK] = 0xff;
		for (index = 0; index < D3DTSS_MAXSTAGES; index++)
		{
			D3D__TextureState[index][D3DTSS_ADDRESSU] = D3DTADDRESS_WRAP;
			D3D__TextureState[index][D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
			D3D__TextureState[index][D3DTSS_ADDRESSW] = D3DTADDRESS_WRAP;
			D3D__TextureState[index][D3DTSS_MAGFILTER] = D3DTEXF_POINT;
			D3D__TextureState[index][D3DTSS_MINFILTER] = D3DTEXF_POINT;
			D3D__TextureState[index][D3DTSS_MAXANISOTROPY] = 1;
		}
		platform_log("Direct3D: no renderer yet on the Wii U (nothing is displayed)");
		device.created = TRUE;
	}
	*returned_device = (D3DDevice *)&device;
	return S_OK;
}

ULONG WINAPI D3DDevice_Release(void)
{
	return 1;
}

void WINAPI D3DDevice_GetDeviceCaps(D3DCAPS8 *caps)
{
	memset(caps, 0, sizeof(*caps));
	caps->DeviceType = D3DDEVTYPE_HAL;
	caps->MaxTextureWidth = 4096;
	caps->MaxTextureHeight = 4096;
	caps->MaxVolumeExtent = 512;
	caps->MaxTextureRepeat = 8192;
	caps->MaxTextureAspectRatio = 4096;
	caps->MaxAnisotropy = 4;
	caps->MaxTextureBlendStages = 4;
	caps->MaxSimultaneousTextures = 4;
	caps->MaxActiveLights = 8;
	caps->MaxVertexBlendMatrices = 4;
	caps->MaxPointSize = 64.0f;
	caps->MaxPrimitiveCount = 0xfffff;
	caps->MaxVertexIndex = 0xffff;
	caps->MaxStreams = 16;
	caps->MaxStreamStride = 255;
	caps->VertexShaderVersion = D3DVS_VERSION(1, 1);
	caps->MaxVertexShaderConst = 192;
	caps->PixelShaderVersion = D3DPS_VERSION(1, 1);
	caps->MaxPixelShaderValue = 1.0f;
}

void WINAPI D3DDevice_GetBackBuffer(INT back_buffer, D3DBACKBUFFER_TYPE type, D3DSurface **result)
{
	(void)back_buffer;
	(void)type;
	/* like Direct3D, the caller gets a reference it must release */
	device.back_buffer.Common++;
	*result = &device.back_buffer;
}

HRESULT WINAPI D3DDevice_GetDepthStencilSurface(D3DSurface **result)
{
	*result = device.depth_stencil;
	if (!*result)
		return D3DERR_NOTFOUND;
	(*result)->Common++;
	return S_OK;
}

void WINAPI D3DDevice_SetRenderTarget(D3DSurface *render_target, D3DSurface *depth_stencil)
{
	if (render_target)
		device.render_target = render_target;
	device.depth_stencil = depth_stencil;
	/* like Direct3D, the viewport becomes the whole new target */
	if (device.render_target)
	{
		struct xgpu_texture_description description;

		xgpu_texture_describe(device.render_target->Format, device.render_target->Size, &description);
		device.viewport.X = 0;
		device.viewport.Y = 0;
		device.viewport.Width = description.width;
		device.viewport.Height = description.height;
		device.viewport.MinZ = 0.0f;
		device.viewport.MaxZ = 1.0f;
	}
}

void WINAPI D3DDevice_SetViewport(CONST D3DVIEWPORT8 *viewport)
{
	device.viewport = *viewport;
}

void WINAPI D3DDevice_SetTransform(D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX *matrix)
{
	if ((unsigned long)state < D3DTS_MAX)
		device.transforms[state] = *matrix;
}

void WINAPI D3DDevice_GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX *matrix)
{
	if ((unsigned long)state < D3DTS_MAX)
		*matrix = device.transforms[state];
}

void WINAPI D3DDevice_SetFlickerFilter(DWORD filter) { (void)filter; }
void WINAPI D3DDevice_SetSoftDisplayFilter(BOOL enable) { (void)enable; }
void WINAPI D3DDevice_SetShaderConstantMode(D3DSHADERCONSTANTMODE mode) { (void)mode; }

/* ---------- the "GPU": idle, and done with everything at once */

BOOL WINAPI D3DDevice_IsBusy(void)
{
	return FALSE;
}

void WINAPI D3DDevice_KickPushBuffer(void)
{
}

void WINAPI D3DDevice_InsertCallback(D3DCALLBACKTYPE type, D3DCALLBACK callback, DWORD context)
{
	(void)type;
	if (callback)
		callback(context);
}

void WINAPI D3DDevice_BeginVisibilityTest(void)
{
}

HRESULT WINAPI D3DDevice_EndVisibilityTest(DWORD index)
{
	(void)index;
	return S_OK;
}

HRESULT WINAPI D3DDevice_GetVisibilityTestResult(DWORD index, UINT *result, ULONGLONG *time_stamp)
{
	(void)index;
	if (result)
		*result = 0;
	if (time_stamp)
		*time_stamp = 0;
	return S_OK;
}

/* ---------- render and texture states: kept, for the game reads them back */

void D3DFASTCALL D3DDevice_SetRenderState_Simple(DWORD method, DWORD value)
{
	/* callers also store the value in D3D__RenderState themselves */
	(void)method;
	(void)value;
}

void D3DFASTCALL D3DDevice_SetRenderState_Deferred(D3DRENDERSTATETYPE state, DWORD value)
{
	if ((unsigned long)state < D3DRS_MAX)
		D3D__RenderState[state] = value;
}

void WINAPI D3DDevice_SetRenderState_ZBias(DWORD value)
{
	D3D__RenderState[D3DRS_ZBIAS] = value;
}

void WINAPI D3DDevice_SetRenderStateNotInline(D3DRENDERSTATETYPE state, DWORD value)
{
	if ((unsigned long)state < D3DRS_MAX)
		D3D__RenderState[state] = value;
}

#define COMPLEX_RENDER_STATE(name, state) \
	void WINAPI D3DDevice_SetRenderState_##name(DWORD value) { D3D__RenderState[state] = value; }

COMPLEX_RENDER_STATE(PSTextureModes, D3DRS_PSTEXTUREMODES)
COMPLEX_RENDER_STATE(VertexBlend, D3DRS_VERTEXBLEND)
COMPLEX_RENDER_STATE(FogColor, D3DRS_FOGCOLOR)
COMPLEX_RENDER_STATE(FillMode, D3DRS_FILLMODE)
COMPLEX_RENDER_STATE(BackFillMode, D3DRS_BACKFILLMODE)
COMPLEX_RENDER_STATE(TwoSidedLighting, D3DRS_TWOSIDEDLIGHTING)
COMPLEX_RENDER_STATE(NormalizeNormals, D3DRS_NORMALIZENORMALS)
COMPLEX_RENDER_STATE(ZEnable, D3DRS_ZENABLE)
COMPLEX_RENDER_STATE(StencilEnable, D3DRS_STENCILENABLE)
COMPLEX_RENDER_STATE(StencilFail, D3DRS_STENCILFAIL)
COMPLEX_RENDER_STATE(FrontFace, D3DRS_FRONTFACE)
COMPLEX_RENDER_STATE(CullMode, D3DRS_CULLMODE)
COMPLEX_RENDER_STATE(TextureFactor, D3DRS_TEXTUREFACTOR)
COMPLEX_RENDER_STATE(LogicOp, D3DRS_LOGICOP)
COMPLEX_RENDER_STATE(EdgeAntiAlias, D3DRS_EDGEANTIALIAS)
COMPLEX_RENDER_STATE(MultiSampleAntiAlias, D3DRS_MULTISAMPLEANTIALIAS)
COMPLEX_RENDER_STATE(MultiSampleMask, D3DRS_MULTISAMPLEMASK)
COMPLEX_RENDER_STATE(MultiSampleType, D3DRS_MULTISAMPLETYPE)
COMPLEX_RENDER_STATE(ShadowFunc, D3DRS_SHADOWFUNC)
COMPLEX_RENDER_STATE(LineWidth, D3DRS_LINEWIDTH)
COMPLEX_RENDER_STATE(Dxt1NoiseEnable, D3DRS_DXT1NOISEENABLE)
COMPLEX_RENDER_STATE(YuvEnable, D3DRS_YUVENABLE)
COMPLEX_RENDER_STATE(OcclusionCullEnable, D3DRS_OCCLUSIONCULLENABLE)
COMPLEX_RENDER_STATE(StencilCullEnable, D3DRS_STENCILCULLENABLE)
COMPLEX_RENDER_STATE(RopZCmpAlwaysRead, D3DRS_ROPZCMPALWAYSREAD)
COMPLEX_RENDER_STATE(RopZRead, D3DRS_ROPZREAD)
COMPLEX_RENDER_STATE(DoNotCullUncompressed, D3DRS_DONOTCULLUNCOMPRESSED)

void D3DFASTCALL D3DDevice_SetTextureState_Deferred(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES && (unsigned long)type < D3DTSS_MAX)
		D3D__TextureState[stage][type] = value;
}

void WINAPI D3DDevice_SetTextureState_TexCoordIndex(DWORD stage, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES)
		D3D__TextureState[stage][D3DTSS_TEXCOORDINDEX] = value;
}

void WINAPI D3DDevice_SetTextureState_BorderColor(DWORD stage, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES)
		D3D__TextureState[stage][D3DTSS_BORDERCOLOR] = value;
}

void WINAPI D3DDevice_SetTextureState_ColorKeyColor(DWORD stage, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES)
		D3D__TextureState[stage][D3DTSS_COLORKEYCOLOR] = value;
}

void WINAPI D3DDevice_SetTextureState_BumpEnv(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
	if (stage < D3DTSS_MAXSTAGES && (unsigned long)type < D3DTSS_MAX)
		D3D__TextureState[stage][type] = value;
}

/* ---------- vertex shaders: handles, and their sizes */

struct null_vertex_shader
{
	UINT instruction_count;
};

HRESULT WINAPI D3DDevice_CreateVertexShader(CONST DWORD *declaration, CONST DWORD *function, DWORD *handle, DWORD usage)
{
	struct null_vertex_shader *shader = calloc(1, sizeof(*shader));

	(void)declaration;
	(void)usage;
	if (!shader)
		return E_OUTOFMEMORY;
	/* header: program type in the low word, instruction count in the high */
	if (function)
		shader->instruction_count = function[0] >> 16;
	/* (odd handles are FVF codes; malloc's are even) */
	*handle = (DWORD)shader;
	return S_OK;
}

void WINAPI D3DDevice_DeleteVertexShader(DWORD handle)
{
	(void)handle;
}

void WINAPI D3DDevice_GetVertexShaderSize(DWORD handle, UINT *size)
{
	*size = handle && !(handle & 1) ? ((struct null_vertex_shader *)handle)->instruction_count : 0;
}

void halo_vertex_shader_lighting(unsigned long handle) { (void)handle; }
void WINAPI D3DDevice_SetVertexShader(DWORD handle) { (void)handle; }
void WINAPI D3DDevice_LoadVertexShader(DWORD handle, DWORD address) { (void)handle; (void)address; }
void WINAPI D3DDevice_SelectVertexShader(DWORD handle, DWORD address) { (void)handle; (void)address; }
void WINAPI D3DDevice_SetVertexShaderConstant(INT reg, CONST void *constant_data, DWORD constant_count)
{
	(void)reg;
	(void)constant_data;
	(void)constant_count;
}

/* ---------- drawing: dropped */

void WINAPI D3DDevice_Clear(DWORD count, CONST D3DRECT *rectangles, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	(void)count; (void)rectangles; (void)flags; (void)color; (void)z; (void)stencil;
}

void WINAPI D3DDevice_SetPixelShaderProgram(D3DPIXELSHADERDEF *definition) { (void)definition; }
void WINAPI D3DDevice_SetTexture(DWORD stage, D3DBaseTexture *texture) { (void)stage; (void)texture; }
void WINAPI D3DDevice_SetPalette(DWORD stage, D3DPalette *palette) { (void)stage; (void)palette; }

void WINAPI D3DDevice_SetStreamSource(UINT stream_number, D3DVertexBuffer *stream_data, UINT stride)
{
	(void)stream_number; (void)stream_data; (void)stride;
}

void WINAPI D3DDevice_SetIndices(D3DIndexBuffer *index_data, UINT base_vertex_index)
{
	(void)index_data; (void)base_vertex_index;
}

void WINAPI D3DDevice_DrawVertices(D3DPRIMITIVETYPE primitive_type, UINT start_vertex, UINT vertex_count)
{
	(void)primitive_type; (void)start_vertex; (void)vertex_count;
}

void WINAPI D3DDevice_DrawIndexedVertices(D3DPRIMITIVETYPE primitive_type, UINT vertex_count, CONST WORD *index_data)
{
	(void)primitive_type; (void)vertex_count; (void)index_data;
}

void WINAPI D3DDevice_Begin(D3DPRIMITIVETYPE primitive_type) { (void)primitive_type; }
void WINAPI D3DDevice_End(void) { }
void WINAPI D3DDevice_SetVertexData2f(INT reg, FLOAT a, FLOAT b) { (void)reg; (void)a; (void)b; }
void WINAPI D3DDevice_SetVertexData4f(INT reg, FLOAT a, FLOAT b, FLOAT c, FLOAT d)
{
	(void)reg; (void)a; (void)b; (void)c; (void)d;
}
void WINAPI D3DDevice_SetVertexData2s(INT reg, SHORT a, SHORT b) { (void)reg; (void)a; (void)b; }
void WINAPI D3DDevice_SetVertexData4ub(INT reg, BYTE a, BYTE b, BYTE c, BYTE d)
{
	(void)reg; (void)a; (void)b; (void)c; (void)d;
}
void WINAPI D3DDevice_SetVertexDataColor(INT reg, D3DCOLOR color) { (void)reg; (void)color; }

/* ---------- the port's screen settings: the Xbox's 640x480 */

long halo_screen_width(void)
{
	return SCREEN_WIDTH;
}

long halo_screen_commit(void)
{
	return SCREEN_WIDTH;
}

float halo_screen_pixel_scale(void)
{
	return 1.0f;
}

long halo_shadow_map_scale(void)
{
	return 1;
}

void halo_screen_ui_offset(unsigned char centered) { (void)centered; }
void halo_screen_anti_alias(short x0, short y0, short x1, short y1) { (void)x0; (void)y0; (void)x1; (void)y1; }

int halo_ui_pointer_update(int menus_active, struct halo_ui_pointer *pointer)
{
	(void)menus_active;
	(void)pointer;
	return 0;
}
