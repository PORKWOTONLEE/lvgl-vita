/*********************
 *      INCLUDES
 *********************/
#include <psp2/display.h>
#include <psp2/kernel/sysmem.h>
#include <psp2common/display.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/_types.h>

#include "lvgl_vita.h"
#include "psv_port_disp.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Display*/

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

static SceDisplayFrameBuf frame_buffer;
static void* frame_buffer_base;
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
/*Initialize display and the required peripherals.*/
static void psv_display_init(void)
{
    SceUID uid = sceKernelAllocMemBlock("Frame Buffer", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, PSV_DISP_HOR_RES*PSV_DISP_VER_RES*4, NULL);
    sceKernelGetMemBlockBase(uid, &frame_buffer_base);

    memset(&frame_buffer, 0, sizeof(frame_buffer));
    frame_buffer.size        = sizeof(frame_buffer);
    frame_buffer.base        = frame_buffer_base;
    frame_buffer.pitch       = PSV_DISP_HOR_RES;
    frame_buffer.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
    frame_buffer.width       = PSV_DISP_HOR_RES;
    frame_buffer.height      = PSV_DISP_VER_RES;

    sceDisplaySetFrameBuf(&frame_buffer, SCE_DISPLAY_SETBUF_NEXTFRAME);

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
    //lv_disp_draw_buf_init(&draw_buffer, buf_1_1, NULL, PSV_DISP_HOR_RES * PSV_DISP_VER_RES);
    lv_disp_draw_buf_init(&draw_buffer, frame_buffer_base, NULL, PSV_DISP_HOR_RES * PSV_DISP_VER_RES);

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
    display_drv.direct_mode = 1;
#if FULL_FRESH
    display_drv.full_refresh = 1;
#endif
    lv_disp_t* ret = lv_disp_drv_register(&display_drv);

    VITA_DEBUG("%s %s, %s full fresh, use %d buffer(s)\n", __FUNCTION__, DEBUG_CHECK_RESULT(ret!=NULL), display_drv.full_refresh==1?"enable":"disable", draw_buffer_num); 
}

static void psv_display_flush(lv_disp_drv_t* disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    int x1 = area->x1;
    int x2 = area->x2;
    int y1 = area->y1;
    int y2 = area->y2;

    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;

    //// line copy
    //for (int32_t y=y1; y<=y2; ++y)
    //{
    //    memcpy(&((lv_color_t *)frame_buffer_base[0])[x1+y*PSV_DISP_HOR_RES], color_p, w*4);
    //    color_p+=w;
    //}

    for (int32_t y=y1; y<=y2; ++y)
    {
        for (int32_t x=x1; x<=x2; ++x)
        {
            uint16_t ch; 
            ch = color_p->ch.red;
            color_p->ch.red = color_p->ch.blue;
            color_p->ch.blue = ch;
            ++color_p;
        }
    }

    sceDisplayWaitVblankStart();

    lv_disp_flush_ready(disp_drv);
}
