/*********************
 *      INCLUDES
 *********************/
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/kernel/sysmem.h>
#include <stdlib.h>

#include "lvgl_vita.h"
#include "psv_port_disp.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Display*/
typedef struct
{
	void *address;

} vita2d_display_data;

typedef struct
{
	SceGxmFragmentProgram *color;
	SceGxmFragmentProgram *texture;
	SceGxmFragmentProgram *textureTint;

} vita2d_fragment_programs;

struct {
	vita2d_fragment_programs blend_mode_normal;
	vita2d_fragment_programs blend_mode_add;
} _vita2d_fragmentPrograms;

typedef struct vita2d_clear_vertex {
	float x;
	float y;
} vita2d_clear_vertex;

typedef struct vita2d_color_vertex {
	float x;
	float y;
	float z;
	unsigned int color;
} vita2d_color_vertex;

typedef struct vita2d_texture_vertex {
	float x;
	float y;
	float z;
	float u;
	float v;
} vita2d_texture_vertex;

typedef struct vita2d_texture {
	SceGxmTexture gxm_tex;
	SceUID data_UID;
	SceUID palette_UID;
	SceGxmRenderTarget *gxm_rtgt;
	SceGxmColorSurface gxm_sfc;
	SceGxmDepthStencilSurface gxm_sfd;
	SceUID depth_UID;
} vita2d_texture;

/**********************
 *  STATIC PROTOTYPES
 **********************/
/*Display*/
static void psv_display_init(void);
static void psv_display_deinit(void);
static void psv_display_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);
static void lv_display_init(void);

/**********************
 *  STATIC VARIABLES
 **********************/
/*Display*/
static lv_disp_draw_buf_t draw_buffer;
static lv_disp_drv_t display_drv;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/*Display*/
void display_init(void)
{
    VITA_DEBUG("[%s]\n", __FUNCTION__);

    psv_display_init();

    lv_display_init();
}

void display_deinit(void)
{
    psv_display_deinit();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static vita2d_texture *psv_texture;
static void *psv_texture_buf;


extern const SceGxmProgram clear_v_gxp_start;
extern const SceGxmProgram clear_f_gxp_start;
extern const SceGxmProgram color_v_gxp_start;
extern const SceGxmProgram color_f_gxp_start;
extern const SceGxmProgram texture_v_gxp_start;
extern const SceGxmProgram texture_f_gxp_start;
extern const SceGxmProgram texture_tint_f_gxp_start;

static const SceGxmProgram *const clearVertexProgramGxp         = &clear_v_gxp_start;
static const SceGxmProgram *const clearFragmentProgramGxp       = &clear_f_gxp_start;
static const SceGxmProgram *const colorVertexProgramGxp         = &color_v_gxp_start;
static const SceGxmProgram *const colorFragmentProgramGxp       = &color_f_gxp_start;
static const SceGxmProgram *const textureVertexProgramGxp       = &texture_v_gxp_start;
static const SceGxmProgram *const textureFragmentProgramGxp     = &texture_f_gxp_start;
static const SceGxmProgram *const textureTintFragmentProgramGxp = &texture_tint_f_gxp_start;


static SceUID vdmRingBufferUid;
static SceUID vertexRingBufferUid;
static SceUID fragmentRingBufferUid;
static SceUID fragmentUsseRingBufferUid;

static SceGxmContextParams contextParams;
static SceGxmRenderTarget *renderTarget = NULL;
static SceUID displayBufferUid[DISPLAY_BUFFER_COUNT];
static void *displayBufferData[DISPLAY_BUFFER_COUNT];
static SceUID displayBufferUid[DISPLAY_BUFFER_COUNT];
static SceGxmColorSurface displaySurface[DISPLAY_BUFFER_COUNT];
static SceGxmSyncObject *displayBufferSync[DISPLAY_BUFFER_COUNT];
static SceUID depthBufferUid;
static SceUID stencilBufferUid;
static SceGxmDepthStencilSurface depthSurface;
static void *depthBufferData = NULL;
static void *stencilBufferData = NULL;

static unsigned int backBufferIndex = 0;
static unsigned int frontBufferIndex = 0;

static SceGxmShaderPatcher *shaderPatcher = NULL;
static SceGxmVertexProgram *clearVertexProgram = NULL;
static SceGxmFragmentProgram *clearFragmentProgram = NULL;

static SceGxmShaderPatcherId clearVertexProgramId;
static SceGxmShaderPatcherId clearFragmentProgramId;
static SceGxmShaderPatcherId colorVertexProgramId;
static SceGxmShaderPatcherId colorFragmentProgramId;
static SceGxmShaderPatcherId textureVertexProgramId;
static SceGxmShaderPatcherId textureFragmentProgramId;
static SceGxmShaderPatcherId textureTintFragmentProgramId;

static SceUID patcherBufferUid;
static SceUID patcherVertexUsseUid;
static SceUID patcherFragmentUsseUid;

static SceUID clearVerticesUid;
static SceUID linearIndicesUid;
static vita2d_clear_vertex *clearVertices = NULL;
static uint16_t *linearIndices = NULL;

/* Shared with other .c */
float _vita2d_ortho_matrix[4*4];
SceGxmContext *_vita2d_context = NULL;
SceGxmVertexProgram *_vita2d_colorVertexProgram = NULL;
SceGxmFragmentProgram *_vita2d_colorFragmentProgram = NULL;
SceGxmVertexProgram *_vita2d_textureVertexProgram = NULL;
SceGxmFragmentProgram *_vita2d_textureFragmentProgram = NULL;
SceGxmFragmentProgram *_vita2d_textureTintFragmentProgram = NULL;
const SceGxmProgramParameter *_vita2d_clearClearColorParam = NULL;
const SceGxmProgramParameter *_vita2d_colorWvpParam = NULL;
const SceGxmProgramParameter *_vita2d_textureWvpParam = NULL;
const SceGxmProgramParameter *_vita2d_textureTintColorParam = NULL;

// Temporary memory pool
static void *pool_addr = NULL;
static SceUID poolUid;
static unsigned int pool_index = 0;
static unsigned int pool_size = 0;

// sceGxmInitialize
unsigned int vblank_wait = 0;

static void display_callback(const void *callback_data)
{
	SceDisplayFrameBuf framebuf;
	const vita2d_display_data *display_data = (const vita2d_display_data *)callback_data;

	memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
	framebuf.size        = sizeof(SceDisplayFrameBuf);
	framebuf.base        = display_data->address;
	framebuf.pitch       = PSV_DISP_HOR_RES;
	framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
	framebuf.width       = PSV_DISP_HOR_RES;
	framebuf.height      = PSV_DISP_VER_RES;
	sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);

	if (vblank_wait) {
		sceDisplayWaitVblankStart();
	}
}

// sceGxmCreateContext
unsigned int get_aligned_size(SceKernelMemBlockType type, unsigned int size)
{
	switch (type) {
	case SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW:
		return ALIGN(size, 256*1024);
	case SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW:
	case SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW:
		return ALIGN(size, 1024*1024);
	default:
		return ALIGN(size, 4*1024);
	}
}

void *gpu_alloc(SceKernelMemBlockType type, unsigned int size, unsigned int alignment, unsigned int attribs, SceUID *uid)
{
	void *mem;
	unsigned int aligned_size = get_aligned_size(type, size);

	*uid = sceKernelAllocMemBlock("gpu_mem", type, aligned_size, NULL);

	// Fallbacking to other mem types if out of mem
	if (*uid < 0) {
		type = type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW ? SCE_KERNEL_MEMBLOCK_TYPE_USER_RW : SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW;
		aligned_size = get_aligned_size(type, size);
		*uid = sceKernelAllocMemBlock("gpu_mem", type, aligned_size, NULL);
		if (*uid < 0) {
			type = SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW;
			aligned_size = get_aligned_size(type, size);
			*uid = sceKernelAllocMemBlock("gpu_mem", type, aligned_size, NULL);
			if (*uid < 0)
				return NULL;
		}
	}

	if (sceKernelGetMemBlockBase(*uid, &mem) < 0)
		return NULL;

	if (sceGxmMapMemory(mem, aligned_size, attribs) < 0)
		return NULL;

	return mem;
}

void *fragment_usse_alloc(unsigned int size, SceUID *uid, unsigned int *usse_offset)
{
	void *mem = NULL;

	size = ALIGN(size, 4096);
	*uid = sceKernelAllocMemBlock("fragment_usse", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, size, NULL);

	if (sceKernelGetMemBlockBase(*uid, &mem) < 0)
		return NULL;
	if (sceGxmMapFragmentUsseMemory(mem, size, usse_offset) < 0)
		return NULL;

	return mem;
}

void *vertex_usse_alloc(unsigned int size, SceUID *uid, unsigned int *usse_offset)
{
	void *mem = NULL;

	size = ALIGN(size, 4096);
	*uid = sceKernelAllocMemBlock("vertex_usse", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, size, NULL);

	if (sceKernelGetMemBlockBase(*uid, &mem) < 0)
		return NULL;
	if (sceGxmMapVertexUsseMemory(mem, size, usse_offset) < 0)
		return NULL;

	return mem;
}

static void *patcher_host_alloc(void *user_data, unsigned int size)
{
	return malloc(size);
}

static void patcher_host_free(void *user_data, void *mem)
{
	free(mem);
}

static void _vita2d_make_fragment_programs(vita2d_fragment_programs *out,
	const SceGxmBlendInfo *blend_info, SceGxmMultisampleMode msaa)
{
	int err;
	(void)err;

	err = sceGxmShaderPatcherCreateFragmentProgram(
		shaderPatcher,
		colorFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		msaa,
		blend_info,
		colorVertexProgramGxp,
		&out->color);

	VITA_DEBUG("color sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherCreateFragmentProgram(
		shaderPatcher,
		textureFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		msaa,
		blend_info,
		textureVertexProgramGxp,
		&out->texture);

	VITA_DEBUG("texture sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherCreateFragmentProgram(
		shaderPatcher,
		textureTintFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		msaa,
		blend_info,
		textureVertexProgramGxp,
		&out->textureTint);

	VITA_DEBUG("texture_tint sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);
}

void vita2d_set_blend_mode_add(int enable)
{
	vita2d_fragment_programs *in = enable ? &_vita2d_fragmentPrograms.blend_mode_add
	    : &_vita2d_fragmentPrograms.blend_mode_normal;

	_vita2d_colorFragmentProgram = in->color;
	_vita2d_textureFragmentProgram = in->texture;
	_vita2d_textureTintFragmentProgram = in->textureTint;
}

void matrix_init_orthographic(float *m, float left, float right, float bottom, float top, float near, float far)
{
	m[0x0] = 2.0f/(right-left);
	m[0x4] = 0.0f;
	m[0x8] = 0.0f;
	m[0xC] = -(right+left)/(right-left);

	m[0x1] = 0.0f;
	m[0x5] = 2.0f/(top-bottom);
	m[0x9] = 0.0f;
	m[0xD] = -(top+bottom)/(top-bottom);

	m[0x2] = 0.0f;
	m[0x6] = 0.0f;
	m[0xA] = -2.0f/(far-near);
	m[0xE] = (far+near)/(far-near);

	m[0x3] = 0.0f;
	m[0x7] = 0.0f;
	m[0xB] = 0.0f;
	m[0xF] = 1.0f;
}

static int tex_format_to_bytespp(SceGxmTextureFormat format)
{
	switch (format & 0x9f000000U) {
	case SCE_GXM_TEXTURE_BASE_FORMAT_U8:
	case SCE_GXM_TEXTURE_BASE_FORMAT_S8:
	case SCE_GXM_TEXTURE_BASE_FORMAT_P8:
		return 1;
	case SCE_GXM_TEXTURE_BASE_FORMAT_U4U4U4U4:
	case SCE_GXM_TEXTURE_BASE_FORMAT_U8U3U3U2:
	case SCE_GXM_TEXTURE_BASE_FORMAT_U1U5U5U5:
	case SCE_GXM_TEXTURE_BASE_FORMAT_U5U6U5:
	case SCE_GXM_TEXTURE_BASE_FORMAT_S5S5U6:
	case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8:
	case SCE_GXM_TEXTURE_BASE_FORMAT_S8S8:
		return 2;
	case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8:
	case SCE_GXM_TEXTURE_BASE_FORMAT_S8S8S8:
		return 3;
	case SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8U8:
	case SCE_GXM_TEXTURE_BASE_FORMAT_S8S8S8S8:
	case SCE_GXM_TEXTURE_BASE_FORMAT_F32:
	case SCE_GXM_TEXTURE_BASE_FORMAT_U32:
	case SCE_GXM_TEXTURE_BASE_FORMAT_S32:
	default:
		return 4;
	}
}

static vita2d_texture *vita2d_create_empty_texture_format(unsigned int w, unsigned int h, SceGxmTextureFormat format)
{
	vita2d_texture *texture = malloc(sizeof(*texture));
	if (!texture)
		return NULL;

	const int tex_size =  ((w + 7) & ~ 7) * h * tex_format_to_bytespp(format);

	/* Allocate a GPU buffer for the texture */
	void *texture_data = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
		tex_size,
		SCE_GXM_TEXTURE_ALIGNMENT,
		SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
		&texture->data_UID);

	if (!texture_data) {
		free(texture);
		return NULL;
	}

	/* Clear the texture */
	memset(texture_data, 0, tex_size);

	/* Create the gxm texture */
	sceGxmTextureInitLinear(
		&texture->gxm_tex,
		texture_data,
		format,
		w,
		h,
		0);

	if ((format & 0x9f000000U) == SCE_GXM_TEXTURE_BASE_FORMAT_P8) {

		const int pal_size = 256 * sizeof(uint32_t);

		void *texture_palette = gpu_alloc(
			SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
			pal_size,
			SCE_GXM_PALETTE_ALIGNMENT,
			SCE_GXM_MEMORY_ATTRIB_READ,
			&texture->palette_UID);

		if (!texture_palette) {
			texture->palette_UID = 0;
			return NULL;
		}

		memset(texture_palette, 0, pal_size);

		sceGxmTextureSetPalette(&texture->gxm_tex, texture_palette);
	} else {
		texture->palette_UID = 0;
	}

	return texture;
}

void *vita2d_pool_memalign(unsigned int size, unsigned int alignment)
{
	unsigned int new_index = (pool_index + alignment - 1) & ~(alignment - 1);
	if ((new_index + size) < pool_size) {
		void *addr = (void *)((unsigned int)pool_addr + new_index);
		pool_index = new_index + size;
		return addr;
	}
	return NULL;
}

/*Initialize display and the required peripherals.*/
static void psv_display_init(void)
{
	int err;
	unsigned int i, x, y;

	SceGxmInitializeParams initializeParams;
	memset(&initializeParams, 0, sizeof(SceGxmInitializeParams));
	initializeParams.flags				            = 0;
	initializeParams.displayQueueMaxPendingCount	= 2;
	initializeParams.displayQueueCallback		    = display_callback;
	initializeParams.displayQueueCallbackDataSize	= sizeof(vita2d_display_data);
	initializeParams.parameterBufferSize		    = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;

	err = sceGxmInitialize(&initializeParams);
	VITA_DEBUG("sceGxmInitialize(): 0x%08X\n", err);

	// allocate ring buffer memory using default sizes
	void *vdmRingBuffer = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&vdmRingBufferUid);

	void *vertexRingBuffer = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&vertexRingBufferUid);

	void *fragmentRingBuffer = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&fragmentRingBufferUid);

	unsigned int fragmentUsseRingBufferOffset;
	void *fragmentUsseRingBuffer = fragment_usse_alloc(
		SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE,
		&fragmentUsseRingBufferUid,
		&fragmentUsseRingBufferOffset);

	memset(&contextParams, 0, sizeof(SceGxmContextParams));
	contextParams.hostMem				= malloc(SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE);
	contextParams.hostMemSize			= SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE;
	contextParams.vdmRingBufferMem			= vdmRingBuffer;
	contextParams.vdmRingBufferMemSize		= SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE;
	contextParams.vertexRingBufferMem		= vertexRingBuffer;
	contextParams.vertexRingBufferMemSize		= SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE;
	contextParams.fragmentRingBufferMem		= fragmentRingBuffer;
	contextParams.fragmentRingBufferMemSize		= SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE;
	contextParams.fragmentUsseRingBufferMem		= fragmentUsseRingBuffer;
	contextParams.fragmentUsseRingBufferMemSize	= SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE;
	contextParams.fragmentUsseRingBufferOffset	= fragmentUsseRingBufferOffset;

	err = sceGxmCreateContext(&contextParams, &_vita2d_context);
	VITA_DEBUG("sceGxmCreateContext(): 0x%08X\n", err);

	// set up parameters
	SceGxmRenderTargetParams renderTargetParams;
	memset(&renderTargetParams, 0, sizeof(SceGxmRenderTargetParams));
	renderTargetParams.flags			= 0;
	renderTargetParams.width			= PSV_DISP_HOR_RES;
	renderTargetParams.height			= PSV_DISP_VER_RES;
	renderTargetParams.scenesPerFrame		= 1;
	renderTargetParams.multisampleMode		= SCE_GXM_MULTISAMPLE_NONE;
	renderTargetParams.multisampleLocations		= 0;
	renderTargetParams.driverMemBlock		= -1; // Invalid UID

	// create the render target
	err = sceGxmCreateRenderTarget(&renderTargetParams, &renderTarget);
	VITA_DEBUG("sceGxmCreateRenderTarget(): 0x%08X\n", err);

	// allocate memory and sync objects for display buffers
	for (i = 0; i < DISPLAY_BUFFER_COUNT; i++) {

		// allocate memory for display
			displayBufferData[i] = gpu_alloc(
				SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
				4*PSV_DISP_HOR_RES*PSV_DISP_VER_RES,
				SCE_GXM_COLOR_SURFACE_ALIGNMENT,
				SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
				&displayBufferUid[i]);

			// memset the buffer to black
			for (y = 0; y < PSV_DISP_VER_RES; y++) {
				unsigned int *row = (unsigned int *)displayBufferData[i] + y*PSV_DISP_HOR_RES;
				for (x = 0; x < PSV_DISP_HOR_RES; x++) {
					row[x] = 0xff000000;
				}
			}

		// initialize a color surface for this display buffer
		err = sceGxmColorSurfaceInit(
			&displaySurface[i],
			SCE_GXM_COLOR_FORMAT_A8B8G8R8,
			SCE_GXM_COLOR_SURFACE_LINEAR,
			SCE_GXM_COLOR_SURFACE_SCALE_NONE,
			SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
			PSV_DISP_HOR_RES,
			PSV_DISP_VER_RES,
			PSV_DISP_HOR_RES,
			displayBufferData[i]);

		// create a sync object that we will associate with this buffer
		err = sceGxmSyncObjectCreate(&displayBufferSync[i]);
	}

	// compute the memory footprint of the depth buffer
	const unsigned int alignedWidth = ALIGN(PSV_DISP_HOR_RES, SCE_GXM_TILE_SIZEX);
	const unsigned int alignedHeight = ALIGN(PSV_DISP_VER_RES, SCE_GXM_TILE_SIZEY);
	unsigned int sampleCount = alignedWidth*alignedHeight;
	unsigned int depthStrideInSamples = alignedWidth;
    // msaa = SCE_GXM_MULTISAMPLE_NONE
	//if (msaa == SCE_GXM_MULTISAMPLE_4X) {
	//	// samples increase in X and Y
	//	sampleCount *= 4;
	//	depthStrideInSamples *= 2;
	//} else if (msaa == SCE_GXM_MULTISAMPLE_2X) {
	//	// samples increase in Y only
	//	sampleCount *= 2;
	//}

	// allocate the depth buffer
	depthBufferData = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		4*sampleCount,
		SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT,
		SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
		&depthBufferUid);

	// allocate the stencil buffer
	stencilBufferData = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		4*sampleCount,
		SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT,
		SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
		&stencilBufferUid);

	// create the SceGxmDepthStencilSurface structure
	err = sceGxmDepthStencilSurfaceInit(
		&depthSurface,
		SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24,
		SCE_GXM_DEPTH_STENCIL_SURFACE_TILED,
		depthStrideInSamples,
		depthBufferData,
		stencilBufferData);

	// set the stencil test reference (this is currently assumed to always remain 1 after here for region clipping)
	sceGxmSetFrontStencilRef(_vita2d_context, 1);
	// set the stencil function (this wouldn't actually be needed, as the set clip rectangle function has to call this at the begginning of every scene)
	sceGxmSetFrontStencilFunc(
		_vita2d_context,
		SCE_GXM_STENCIL_FUNC_ALWAYS,
		SCE_GXM_STENCIL_OP_KEEP,
		SCE_GXM_STENCIL_OP_KEEP,
		SCE_GXM_STENCIL_OP_KEEP,
		0xFF,
		0xFF);

	// set buffer sizes for this sample
	const unsigned int patcherBufferSize		= 64*1024;
	const unsigned int patcherVertexUsseSize	= 64*1024;
	const unsigned int patcherFragmentUsseSize	= 64*1024;

	// allocate memory for buffers and USSE code
	void *patcherBuffer = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		patcherBufferSize,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
		&patcherBufferUid);

	unsigned int patcherVertexUsseOffset;
	void *patcherVertexUsse = vertex_usse_alloc(
		patcherVertexUsseSize,
		&patcherVertexUsseUid,
		&patcherVertexUsseOffset);

	unsigned int patcherFragmentUsseOffset;
	void *patcherFragmentUsse = fragment_usse_alloc(
		patcherFragmentUsseSize,
		&patcherFragmentUsseUid,
		&patcherFragmentUsseOffset);

	// create a shader patcher
	SceGxmShaderPatcherParams patcherParams;
	memset(&patcherParams, 0, sizeof(SceGxmShaderPatcherParams));
	patcherParams.userData			= NULL;
	patcherParams.hostAllocCallback		= &patcher_host_alloc;
	patcherParams.hostFreeCallback		= &patcher_host_free;
	patcherParams.bufferAllocCallback	= NULL;
	patcherParams.bufferFreeCallback	= NULL;
	patcherParams.bufferMem			= patcherBuffer;
	patcherParams.bufferMemSize		= patcherBufferSize;
	patcherParams.vertexUsseAllocCallback	= NULL;
	patcherParams.vertexUsseFreeCallback	= NULL;
	patcherParams.vertexUsseMem		= patcherVertexUsse;
	patcherParams.vertexUsseMemSize		= patcherVertexUsseSize;
	patcherParams.vertexUsseOffset		= patcherVertexUsseOffset;
	patcherParams.fragmentUsseAllocCallback	= NULL;
	patcherParams.fragmentUsseFreeCallback	= NULL;
	patcherParams.fragmentUsseMem		= patcherFragmentUsse;
	patcherParams.fragmentUsseMemSize	= patcherFragmentUsseSize;
	patcherParams.fragmentUsseOffset	= patcherFragmentUsseOffset;

	err = sceGxmShaderPatcherCreate(&patcherParams, &shaderPatcher);
	VITA_DEBUG("sceGxmShaderPatcherCreate(): 0x%08X\n", err);

	// check the shaders
	err = sceGxmProgramCheck(clearVertexProgramGxp);
	VITA_DEBUG("clear_v sceGxmProgramCheck(): 0x%08X\n", err);
	err = sceGxmProgramCheck(clearFragmentProgramGxp);
	VITA_DEBUG("clear_f sceGxmProgramCheck(): 0x%08X\n", err);
	err = sceGxmProgramCheck(colorVertexProgramGxp);
	VITA_DEBUG("color_v sceGxmProgramCheck(): 0x%08X\n", err);
	err = sceGxmProgramCheck(colorFragmentProgramGxp);
	VITA_DEBUG("color_f sceGxmProgramCheck(): 0x%08X\n", err);
	err = sceGxmProgramCheck(textureVertexProgramGxp);
	VITA_DEBUG("texture_v sceGxmProgramCheck(): 0x%08X\n", err);
	err = sceGxmProgramCheck(textureFragmentProgramGxp);
	VITA_DEBUG("texture_f sceGxmProgramCheck(): 0x%08X\n", err);
	err = sceGxmProgramCheck(textureTintFragmentProgramGxp);
	VITA_DEBUG("texture_tint_f sceGxmProgramCheck(): 0x%08X\n", err);

	// register programs with the patcher
	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, clearVertexProgramGxp, &clearVertexProgramId);
	VITA_DEBUG("clear_v sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, clearFragmentProgramGxp, &clearFragmentProgramId);
	VITA_DEBUG("clear_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, colorVertexProgramGxp, &colorVertexProgramId);
	VITA_DEBUG("color_v sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, colorFragmentProgramGxp, &colorFragmentProgramId);
	VITA_DEBUG("color_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, textureVertexProgramGxp, &textureVertexProgramId);
	VITA_DEBUG("texture_v sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, textureFragmentProgramGxp, &textureFragmentProgramId);
	VITA_DEBUG("texture_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, textureTintFragmentProgramGxp, &textureTintFragmentProgramId);
	VITA_DEBUG("texture_tint_f sceGxmShaderPatcherRegisterProgram(): 0x%08X\n", err);

	// Fill SceGxmBlendInfo
	static const SceGxmBlendInfo blend_info = {
		.colorFunc = SCE_GXM_BLEND_FUNC_ADD,
		.alphaFunc = SCE_GXM_BLEND_FUNC_ADD,
		.colorSrc  = SCE_GXM_BLEND_FACTOR_SRC_ALPHA,
		.colorDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.alphaSrc  = SCE_GXM_BLEND_FACTOR_SRC_ALPHA,
		.alphaDst  = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorMask = SCE_GXM_COLOR_MASK_ALL
	};

	static const SceGxmBlendInfo blend_info_add = {
		.colorFunc = SCE_GXM_BLEND_FUNC_ADD,
		.alphaFunc = SCE_GXM_BLEND_FUNC_ADD,
		.colorSrc  = SCE_GXM_BLEND_FACTOR_ONE,
		.colorDst  = SCE_GXM_BLEND_FACTOR_ONE,
		.alphaSrc  = SCE_GXM_BLEND_FACTOR_ONE,
		.alphaDst  = SCE_GXM_BLEND_FACTOR_ONE,
		.colorMask = SCE_GXM_COLOR_MASK_ALL
	};

	// get attributes by name to create vertex format bindings
	const SceGxmProgramParameter *paramClearPositionAttribute = sceGxmProgramFindParameterByName(clearVertexProgramGxp, "aPosition");

	// create clear vertex format
	SceGxmVertexAttribute clearVertexAttributes[1];
	SceGxmVertexStream clearVertexStreams[1];
	clearVertexAttributes[0].streamIndex	= 0;
	clearVertexAttributes[0].offset		= 0;
	clearVertexAttributes[0].format		= SCE_GXM_ATTRIBUTE_FORMAT_F32;
	clearVertexAttributes[0].componentCount	= 2;
	clearVertexAttributes[0].regIndex	= sceGxmProgramParameterGetResourceIndex(paramClearPositionAttribute);
	clearVertexStreams[0].stride		= sizeof(vita2d_clear_vertex);
	clearVertexStreams[0].indexSource	= SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	// create clear programs
	err = sceGxmShaderPatcherCreateVertexProgram(
		shaderPatcher,
		clearVertexProgramId,
		clearVertexAttributes,
		1,
		clearVertexStreams,
		1,
		&clearVertexProgram);

	VITA_DEBUG("clear sceGxmShaderPatcherCreateVertexProgram(): 0x%08X\n", err);

	err = sceGxmShaderPatcherCreateFragmentProgram(
		shaderPatcher,
		clearFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		SCE_GXM_MULTISAMPLE_NONE,
		NULL,
		clearVertexProgramGxp,
		&clearFragmentProgram);

	VITA_DEBUG("clear sceGxmShaderPatcherCreateFragmentProgram(): 0x%08X\n", err);

	// create the clear triangle vertex/index data
	clearVertices = (vita2d_clear_vertex *)gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		3*sizeof(vita2d_clear_vertex),
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&clearVerticesUid);

	// Allocate a 64k * 2 bytes = 128 KiB buffer and store all possible
	// 16-bit indices in linear ascending order, so we can use this for
	// all drawing operations where we don't want to use indexing.
	linearIndices = (uint16_t *)gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
		UINT16_MAX*sizeof(uint16_t),
		sizeof(uint16_t),
		SCE_GXM_MEMORY_ATTRIB_READ,
		&linearIndicesUid);

        // Range of i must be greater than uint16_t, this doesn't endless-loop
	for (uint32_t i=0; i<=UINT16_MAX; ++i) {
		linearIndices[i] = i;
	}

	clearVertices[0].x = -1.0f;
	clearVertices[0].y = -1.0f;
	clearVertices[1].x =  3.0f;
	clearVertices[1].y = -1.0f;
	clearVertices[2].x = -1.0f;
	clearVertices[2].y =  3.0f;

	const SceGxmProgramParameter *paramColorPositionAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "aPosition");
	VITA_DEBUG("aPosition sceGxmProgramFindParameterByName(): %p\n", paramColorPositionAttribute);

	const SceGxmProgramParameter *paramColorColorAttribute = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "aColor");
	VITA_DEBUG("aColor sceGxmProgramFindParameterByName(): %p\n", paramColorColorAttribute);

	// create color vertex format
	SceGxmVertexAttribute colorVertexAttributes[2];
	SceGxmVertexStream colorVertexStreams[1];
	/* x,y,z: 3 float 32 bits */
	colorVertexAttributes[0].streamIndex = 0;
	colorVertexAttributes[0].offset = 0;
	colorVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	colorVertexAttributes[0].componentCount = 3; // (x, y, z)
	colorVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorPositionAttribute);
	/* color: 4 unsigned char  = 32 bits */
	colorVertexAttributes[1].streamIndex = 0;
	colorVertexAttributes[1].offset = 12; // (x, y, z) * 4 = 12 bytes
	colorVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N;
	colorVertexAttributes[1].componentCount = 4; // (color)
	colorVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramColorColorAttribute);
	// 16 bit (short) indices
	colorVertexStreams[0].stride = sizeof(vita2d_color_vertex);
	colorVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	// create color shaders
	err = sceGxmShaderPatcherCreateVertexProgram(
		shaderPatcher,
		colorVertexProgramId,
		colorVertexAttributes,
		2,
		colorVertexStreams,
		1,
		&_vita2d_colorVertexProgram);

	VITA_DEBUG("color sceGxmShaderPatcherCreateVertexProgram(): 0x%08X\n", err);


	const SceGxmProgramParameter *paramTexturePositionAttribute = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "aPosition");
	VITA_DEBUG("aPosition sceGxmProgramFindParameterByName(): %p\n", paramTexturePositionAttribute);

	const SceGxmProgramParameter *paramTextureTexcoordAttribute = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "aTexcoord");
	VITA_DEBUG("aTexcoord sceGxmProgramFindParameterByName(): %p\n", paramTextureTexcoordAttribute);

	// create texture vertex format
	SceGxmVertexAttribute textureVertexAttributes[2];
	SceGxmVertexStream textureVertexStreams[1];
	/* x,y,z: 3 float 32 bits */
	textureVertexAttributes[0].streamIndex = 0;
	textureVertexAttributes[0].offset = 0;
	textureVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	textureVertexAttributes[0].componentCount = 3; // (x, y, z)
	textureVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramTexturePositionAttribute);
	/* u,v: 2 floats 32 bits */
	textureVertexAttributes[1].streamIndex = 0;
	textureVertexAttributes[1].offset = 12; // (x, y, z) * 4 = 12 bytes
	textureVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	textureVertexAttributes[1].componentCount = 2; // (u, v)
	textureVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramTextureTexcoordAttribute);
	// 16 bit (short) indices
	textureVertexStreams[0].stride = sizeof(vita2d_texture_vertex);
	textureVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	// create texture shaders
	err = sceGxmShaderPatcherCreateVertexProgram(
		shaderPatcher,
		textureVertexProgramId,
		textureVertexAttributes,
		2,
		textureVertexStreams,
		1,
		&_vita2d_textureVertexProgram);

	VITA_DEBUG("texture sceGxmShaderPatcherCreateVertexProgram(): 0x%08X\n", err);

	// Create variations of the fragment program based on blending mode
	_vita2d_make_fragment_programs(&_vita2d_fragmentPrograms.blend_mode_normal, &blend_info, SCE_GXM_MULTISAMPLE_NONE);
	_vita2d_make_fragment_programs(&_vita2d_fragmentPrograms.blend_mode_add, &blend_info_add, SCE_GXM_MULTISAMPLE_NONE);

	// Default to "normal" blending mode (non-additive)
	vita2d_set_blend_mode_add(0);

	// find vertex uniforms by name and cache parameter information
	_vita2d_clearClearColorParam = sceGxmProgramFindParameterByName(clearFragmentProgramGxp, "uClearColor");
	VITA_DEBUG("_vita2d_clearClearColorParam sceGxmProgramFindParameterByName(): %p\n", _vita2d_clearClearColorParam);

	_vita2d_colorWvpParam = sceGxmProgramFindParameterByName(colorVertexProgramGxp, "wvp");
	VITA_DEBUG("color wvp sceGxmProgramFindParameterByName(): %p\n", _vita2d_colorWvpParam);

	_vita2d_textureWvpParam = sceGxmProgramFindParameterByName(textureVertexProgramGxp, "wvp");
	VITA_DEBUG("texture wvp sceGxmProgramFindParameterByName(): %p\n", _vita2d_textureWvpParam);

	_vita2d_textureTintColorParam = sceGxmProgramFindParameterByName(textureTintFragmentProgramGxp, "uTintColor");
	VITA_DEBUG("texture wvp sceGxmProgramFindParameterByName(): %p\n", _vita2d_textureWvpParam);

	// Allocate memory for the memory pool
	pool_size = (1 * 1024 * 1024);
	pool_addr = gpu_alloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW,
		pool_size,
		sizeof(void *),
		SCE_GXM_MEMORY_ATTRIB_READ,
		&poolUid);

	matrix_init_orthographic(_vita2d_ortho_matrix, 0.0f, PSV_DISP_HOR_RES, PSV_DISP_VER_RES, 0.0f, 0.0f, 1.0f);

	backBufferIndex = 0;
	frontBufferIndex = 0;

    psv_texture = vita2d_create_empty_texture_format(PSV_DISP_HOR_RES, PSV_DISP_VER_RES, SCE_GXM_TEXTURE_FORMAT_A8R8G8B8);
    psv_texture_buf = sceGxmTextureGetData(&psv_texture->gxm_tex);

    VITA_DEBUG("%s %s\n", __FUNCTION__, DEBUG_CHECK_RESULT(1));
}

static void psv_display_deinit(void)
{
}

static void lv_display_init(void)
{
    // Create draw buffer(s)
    uint32_t draw_buffer_num;
#if PSV_DRAW_BUFFER_TYPE == PSV_DRAW_SINGLE_BUFFER
    static lv_color_t buf_1_1[PSV_DISP_HOR_RES * PSV_DISP_VER_RES];
    lv_disp_draw_buf_init(&draw_buffer, buf_1_1, NULL, PSV_DISP_HOR_RES * PSV_DISP_VER_RES);

    draw_buffer_num = 1;
#elif PSV_DRAW_BUFFER_TYPE == PSV_DRAW_DOUBLE_BUFFER
    static lv_color_t buf_2_1[PSV_DISP_HOR_RES * PSV_DISP_VER_RES];
    static lv_color_t buf_2_2[PSV_DISP_HOR_RES * PSV_DISP_VER_RES];
    lv_disp_draw_buf_init(&draw_buffer, buf_2_1, buf_2_2, PSV_DISP_HOR_RES * PSV_DISP_VER_RES);

    draw_buffer_num = 2;
#endif
    // Create a display and set a flush_cb
    lv_disp_drv_init(&display_drv);

    display_drv.hor_res = PSV_DISP_HOR_RES;
    display_drv.ver_res = PSV_DISP_VER_RES;
    display_drv.flush_cb = psv_display_flush;
    display_drv.draw_buf = &draw_buffer;
#if FULL_FRESH
    display_drv.full_refresh = 1;
#endif
    lv_disp_t* ret = lv_disp_drv_register(&display_drv);

    VITA_DEBUG("%s %s, %s full fresh, use %d buffer(s)\n", __FUNCTION__, DEBUG_CHECK_RESULT(ret!=NULL), display_drv.full_refresh==1?"enable":"disable", draw_buffer_num); 
}

static void psv_display_flush(lv_disp_drv_t* disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    // start drawing
    pool_index = 0;
    sceGxmBeginScene(
            _vita2d_context,
            0,
            renderTarget,
            NULL,
            NULL,
            displayBufferSync[backBufferIndex],
            &displaySurface[backBufferIndex],
            &depthSurface);

    // partial fresh, still buggy
    int x1 = area->x1;
    int x2 = area->x2;
    int y1 = area->y1;
    int y2 = area->y2;

    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;

    // pixel copy
    for (int32_t y=y1; y<=y2; ++y)
    {
        for (int32_t x=x1; x<=x2; ++x)
        {
            ((uint32_t *)(psv_texture_buf))[x+y*PSV_DISP_HOR_RES] = *(uint32_t *)color_p;
            ++color_p;
        }
    }
    // block copy
    //for (int y=y1; y<=y2; ++y)
    //{
    //    lv_memcpy(&((lv_color_t *)psv_texture_buf)[y * PSV_DISP_HOR_RES + x1], color_p, w * sizeof(lv_color_t));
    //    color_p+=w;
    //}

    // draw texture
    sceGxmSetVertexProgram(_vita2d_context, _vita2d_textureVertexProgram);
    sceGxmSetFragmentProgram(_vita2d_context, _vita2d_textureFragmentProgram);

    void *vertex_wvp_buffer;
	sceGxmReserveVertexDefaultUniformBuffer(_vita2d_context, &vertex_wvp_buffer);
	sceGxmSetUniformDataF(vertex_wvp_buffer, _vita2d_textureWvpParam, 0, 16, _vita2d_ortho_matrix);
    
    vita2d_texture_vertex *vertices = (vita2d_texture_vertex *)vita2d_pool_memalign(
		4 * sizeof(vita2d_texture_vertex), // 4 vertices
		sizeof(vita2d_texture_vertex));

	const float width  = sceGxmTextureGetWidth(&psv_texture->gxm_tex);
	const float height = sceGxmTextureGetHeight(&psv_texture->gxm_tex);

	vertices[0].x = 0;
	vertices[0].y = 0;
	vertices[0].z = +0.5f;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	vertices[1].x = width;
	vertices[1].y = 0;
	vertices[1].z = +0.5f;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	vertices[2].x = 0;
	vertices[2].y = height;
	vertices[2].z = +0.5f;
	vertices[2].u = 0.0f;
	vertices[2].v = 1.0f;

	vertices[3].x = PSV_DISP_HOR_RES;
	vertices[3].y = PSV_DISP_VER_RES;
	vertices[3].z = +0.5f;
	vertices[3].u = 1.0f;
	vertices[3].v = 1.0f;

	// Set the texture to the TEXUNIT0
	sceGxmSetFragmentTexture(_vita2d_context, 0, &psv_texture->gxm_tex);

	sceGxmSetVertexStream(_vita2d_context, 0, vertices);
	sceGxmDraw(_vita2d_context, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, SCE_GXM_INDEX_FORMAT_U16, linearIndices, 4);

    // end drawing
    sceGxmEndScene(_vita2d_context, NULL, NULL);

    // swap buffers
    // queue the display swap for this frame
    vita2d_display_data displayData;
    displayData.address = displayBufferData[backBufferIndex];
    sceGxmDisplayQueueAddEntry(
            displayBufferSync[frontBufferIndex],	// OLD fb
            displayBufferSync[backBufferIndex],	// NEW fb
            &displayData);
    // update buffer indices
    frontBufferIndex = backBufferIndex;
    backBufferIndex = (backBufferIndex + 1) % DISPLAY_BUFFER_COUNT;

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

