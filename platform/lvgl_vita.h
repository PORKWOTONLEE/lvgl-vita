#ifndef LVGL_VITA_H
#define LVGL_VITA_H

#ifdef __cplusplus
extern "C" {
#endif
/*********************
 *      INCLUDES
 *********************/
#include "../lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
/*Platform*/
#define PSV_DISP_HOR_RES    960
#define PSV_DISP_VER_RES    544

/*Log*/
#define DEBUG                (1)
#define DEBUG_BUFFER_SIZE    (1024)
#define DEBUG_FILE_PATH      ("ux0:data/LVGL-Vita.txt")
#define VITA_DEBUG(...)      vita_debug_to_local_file(__VA_ARGS__)

/*********************
 *      MACRO 
 *********************/
/*Log*/
#define DEBUG_CHECK_RESULT(condition) ((condition)?"success":"fail")

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  EXTERN VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/*Platform*/
void psv_lv_init();
void psv_lv_deinit();
void psv_lv_mainloop();

/*Log*/
void vita_debug_to_local_file(const char *format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
