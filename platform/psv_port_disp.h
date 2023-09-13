#ifndef PSV_PORT_DISP_H
#define PSV_PORT_DISP_H

#ifdef __cplusplus
extern "C" {
#endif
/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
/*Display*/
#define PSV_DRAW_SINGLE_BUFFER    (1)
#define PSV_DRAW_DOUBLE_BUFFER    (2) 
#define PSV_DRAW_BUFFER_TYPE      PSV_DRAW_SINGLE_BUFFER
#define FULL_FRESH                (0) 

/*Display*/
#define ARGB_TO_ABGR(color) ((color)&0xFF00FF00)|(((color)&0x00FF0000)>>16)|(((color)&0x000000FF)<<16)
#define DISPLAY_BUFFER_COUNT 3
#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))

/*********************
 *      MACRO 
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  EXTERN VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/*Display*/
void display_init(void);
void display_deinit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
