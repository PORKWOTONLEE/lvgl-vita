/*********************
 *      INCLUDES
 *********************/
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>
#include <stdio.h>
#include "../lvgl/lvgl.h"

#include "lvgl_vita.h"
#include "psv_port_disp.h"
#include "psv_port_input.h"
#include "psv_port_fs.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
/*Event*/
static void psv_event_handler_loop_init(void);
static void psv_event_handler_loop_deinit(void);
static int32_t *psv_event_handler_loop(SceSize arg_size, void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/
/*Event*/
static SceUID event_handler_thread_uid;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/*Platform*/
void psv_lv_init()
{
    lv_init();

    display_init();

    input_init();

    filesystem_init();
}

void psv_lv_deinit()
{
    filesystem_deinit();

    input_deinit();

    display_deinit();

    lv_deinit();
}

void psv_lv_mainloop()
{
    psv_event_handler_loop_init();

    VITA_DEBUG("[%s] start\n", __FUNCTION__);

    while (1)
    {
        sceKernelDelayThread(1000); 
        lv_tick_inc(1);
    }

    VITA_DEBUG("[%s] end\n", __FUNCTION__);

    psv_event_handler_loop_deinit();
}

/*Log*/
void vita_debug_to_local_file(const char *format, ...)
{
#if DEBUG==0
    return;
#endif
    char buffer[DEBUG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    SceUID log_uid = sceIoOpen(DEBUG_FILE_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
    if (log_uid >= 0)
    {
        sceIoWrite(log_uid, buffer, strlen(buffer));
        sceIoClose(log_uid);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/*Event*/
static void psv_event_handler_loop_init(void)
{
    SceKernelThreadEntry event_handler_thread_entry = (SceKernelThreadEntry) psv_event_handler_loop;

    event_handler_thread_uid = sceKernelCreateThread("event_handler_thread", event_handler_thread_entry, 0x10000100, 0x10000, 0, 0, NULL);
    int32_t ret = sceKernelStartThread(event_handler_thread_uid, 0, NULL);

    VITA_DEBUG("%s %s\n", __FUNCTION__, DEBUG_CHECK_RESULT(ret==0));
}

static void psv_event_handler_loop_deinit(void)
{
    sceKernelExitThread(event_handler_thread_uid);
}

static int32_t *psv_event_handler_loop(SceSize arg_size, void *arg)
{
    while (1)
    {
        lv_task_handler();
    }

    return 0;
}
