/*********************
 *      INCLUDES
 *********************/
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>

#include "lvgl_vita.h"
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
/*File system*/
static void psv_filesystem_init(void);
static void lv_filesystem_init(void);
static void psv_filesystem_deinit(void);
static void lv_filesystem_deinit(void);
static void * psv_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t psv_close(lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t psv_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t psv_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
static lv_fs_res_t psv_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t psv_size(lv_fs_drv_t * drv, void * file_p, uint32_t * size_p);
static lv_fs_res_t psv_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
static void * psv_dir_open(lv_fs_drv_t * drv, const char * path);
static lv_fs_res_t psv_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn);
static lv_fs_res_t psv_dir_close(lv_fs_drv_t * drv, void * rddir_p);

/**********************
 *  STATIC VARIABLES
 **********************/
/*File system*/
static lv_fs_drv_t fs_drv;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/*File system*/
void filesystem_init(void)
{
    psv_filesystem_init();

    lv_filesystem_init();
}

void filesystem_deinit(void)
{
    psv_filesystem_deinit();

    lv_filesystem_deinit();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void psv_filesystem_init(void)
{
    // Do nothing
}

static void lv_filesystem_init(void)
{
    lv_fs_drv_init(&fs_drv);

    fs_drv.letter   = 'P';
    fs_drv.open_cb  = psv_open;
    fs_drv.close_cb = psv_close;
    fs_drv.read_cb  = psv_read;
    fs_drv.write_cb = psv_write;
    fs_drv.seek_cb  = psv_seek;
    fs_drv.tell_cb  = NULL;

    fs_drv.dir_close_cb = psv_dir_close;
    fs_drv.dir_open_cb  = psv_dir_open;
    fs_drv.dir_read_cb  = psv_dir_read;

    lv_fs_drv_register(&fs_drv);
}

static void psv_filesystem_deinit(void)
{
    // Do nothing
}

static void lv_filesystem_deinit(void)
{
    // Do nothing
}

static void * psv_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    SceUID file_uid;

    if(mode == LV_FS_MODE_WR)
    {
        file_uid = sceIoOpen(path, SCE_O_WRONLY, SCE_S_IRWXU);
    }
    else if(mode == LV_FS_MODE_RD)
    {
        file_uid = sceIoOpen(path, SCE_O_RDONLY, SCE_S_IRWXU);
    }
    else if(mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
    {
        file_uid = sceIoOpen(path, SCE_O_RDWR, SCE_S_IRWXU);
    }

    if (file_uid > 0)
    {
        return (void *)file_uid;
    }
    else 
    {
        return NULL;
    }
}

static lv_fs_res_t psv_close(lv_fs_drv_t * drv, void * file_p)
{
    int32_t ret = sceIoClose((SceUID)file_p);

    if (ret >= 0)
    {
        return LV_FS_RES_OK;
    }
    else
    {
        return LV_FS_RES_FS_ERR;
    }
}

static lv_fs_res_t psv_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    int32_t ret = sceIoRead((SceUID)file_p, buf, btr);
    *br = ret;

    if (ret >= 0)
    {
        return LV_FS_RES_OK;
    }
    else
    {
        return LV_FS_RES_FS_ERR;
    }
}

static lv_fs_res_t psv_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    int32_t ret = sceIoWrite((SceUID)file_p, buf, btw);
    *bw = ret;

    if (ret >= 0)
    {
        return LV_FS_RES_OK;
    }
    else
    {
        return LV_FS_RES_FS_ERR;
    }
}

static lv_fs_res_t psv_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    int32_t ret;

    if(whence == LV_FS_SEEK_CUR)
    {
       ret = sceIoLseek((SceUID)file_p, pos, SCE_SEEK_CUR);
    }
    else if(whence == LV_FS_SEEK_END)
    {
        sceIoLseek((SceUID)file_p, pos, SCE_SEEK_END);
    }
    else if(whence == LV_FS_SEEK_SET)
    {
        sceIoLseek((SceUID)file_p, pos, SCE_SEEK_SET);
    }

    if (ret >= 0) 
    {
        return LV_FS_RES_OK;
    }
    else
    {
        return LV_FS_RES_FS_ERR;;
    }
}

static void * psv_dir_open(lv_fs_drv_t * drv, const char * path)
{
    SceUID dir_uid = sceIoDopen(path); 

    if (dir_uid >= 0)
    {
        return (void *)drv;
    }
    else 
    {
        return NULL;
    }
}

static lv_fs_res_t psv_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn)
{
    SceIoDirent *file_info;
    int32_t ret = sceIoDread((SceUID)rddir_p, file_info);

    if (ret == 0)
    {
        strcpy(fn, "");

        return LV_FS_RES_OK;
    }
    else if (ret > 0)
    {
        strcpy(fn, file_info->d_name);

        return LV_FS_RES_OK;
    }
    else
    {
        return LV_FS_RES_FS_ERR;
    }
}

static lv_fs_res_t psv_dir_close(lv_fs_drv_t * drv, void * rddir_p)
{
    uint32_t ret = sceIoDclose((SceUID)rddir_p);

    if (ret >= 0)
    {
        return LV_FS_RES_OK;
    }
    else
    {
        return LV_FS_RES_FS_ERR;;
    }
}
