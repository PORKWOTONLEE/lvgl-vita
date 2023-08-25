#ifndef PSV_PORT_FS_H 
#define PSV_PORT_FS_H 

#ifdef __cplusplus
extern "C" {
#endif
/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

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
/*File system*/
void filesystem_init(void);
void filesystem_deinit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
