/*********************
 *      INCLUDES
 *********************/
#include <psp2/ctrl.h>
#include <psp2/touch.h>

#include "lvgl_vita.h"
#include "psv_port_input.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      MACROS
 **********************/
#define LERP(value,from_max,to_max) ((((value*10)*(to_max*10))/(from_max*10))/10)

/**********************
 *      TYPEDEFS
 **********************/
/*Input*/
typedef struct
{
    // keypad
    uint8_t is_keypad_pressed;
    int32_t keycode;
    // touchpad
    uint8_t is_touchpad_pressed;
    int32_t touch_point_x;
    int32_t touch_point_y;

}PsvInputData;

/**********************
 *  STATIC PROTOTYPES
 **********************/
/*Input*/
static void psv_input_init(void);
static void lv_input_init(void);
/*Input touchpad*/
static void psv_touchpad_init(void);
static void lv_touchpad_init(void);
static void psv_touchpad_peek(void);
static void lv_touchpad_peek(lv_indev_drv_t * indev, lv_indev_data_t * data);
/*Input button*/
static void psv_button_init(void);
static void lv_button_init(void);
static void psv_button_peek(void);
static void lv_button_peek(lv_indev_drv_t * indev, lv_indev_data_t * data);

/**********************
 *  STATIC VARIABLES
 **********************/
/*Input*/
static PsvInputData psv_input_data;
static lv_indev_drv_t input_touchpad;
static lv_indev_drv_t input_button;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/*Input*/
void input_init(void)
{
    VITA_DEBUG("[%s]\n", __FUNCTION__);

    psv_input_init();

    lv_input_init();
}

void input_deinit(void)
{
    // Do nothing
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/*Input*/
static void psv_input_init(void)
{
    psv_touchpad_init();

    psv_button_init();
}

static void lv_input_init(void)
{
    lv_touchpad_init();

    lv_button_init();
}

/*Touchpad*/
static void psv_touchpad_init(void)
{
    int32_t ret = sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    VITA_DEBUG("%s %s\n", __FUNCTION__, DEBUG_CHECK_RESULT(!ret));
}

static void lv_touchpad_init(void)
{
    lv_indev_drv_init(&input_touchpad);

    input_touchpad.type = LV_INDEV_TYPE_POINTER;
    input_touchpad.read_cb = lv_touchpad_peek;

    lv_indev_t *ret = lv_indev_drv_register(&input_touchpad);

    VITA_DEBUG("%s %s\n", __FUNCTION__, DEBUG_CHECK_RESULT(ret!=NULL));
}

static void lv_touchpad_peek(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    psv_touchpad_peek();
    
    if(psv_input_data.is_touchpad_pressed) 
    {
        data->state = LV_INDEV_STATE_PR;
    }
    else 
    {
        data->state = LV_INDEV_STATE_REL;
    }

    data->point.x = LERP(psv_input_data.touch_point_x, 1919, PSV_DISP_HOR_RES);
    data->point.y = LERP(psv_input_data.touch_point_y, 1087, PSV_DISP_VER_RES);
}

static void psv_touchpad_peek(void)
{
    SceTouchData touch[SCE_TOUCH_PORT_MAX_NUM];

    for(uint32_t port=0; port < SCE_TOUCH_PORT_MAX_NUM; ++port)
    {
        sceTouchPeek(port, &touch[port], 1);
    }

    if (touch[SCE_TOUCH_PORT_FRONT].reportNum>0 &&
        (touch[SCE_TOUCH_PORT_FRONT].report[0].x!=-1 || touch[SCE_TOUCH_PORT_FRONT].report[0].y!=-1))
    {
        psv_input_data.is_touchpad_pressed = true;
    }
    else 
    {
        psv_input_data.is_touchpad_pressed = false;
    }

    psv_input_data.touch_point_x = touch[SCE_TOUCH_PORT_FRONT].report[0].x;
    psv_input_data.touch_point_y = touch[SCE_TOUCH_PORT_FRONT].report[0].y;
}

/*Button*/
static void psv_button_init(void)
{
	int32_t ret = sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    VITA_DEBUG("%s %s\n", __FUNCTION__, DEBUG_CHECK_RESULT(!ret));
}

static void lv_button_init(void)
{
    lv_indev_drv_init(&input_button);

    input_button.type = LV_INDEV_TYPE_KEYPAD;
    input_button.read_cb = lv_button_peek;

    lv_indev_t *ret = lv_indev_drv_register(&input_button);

    VITA_DEBUG("%s %s\n", __FUNCTION__, DEBUG_CHECK_RESULT(ret!=NULL));
}

static void psv_button_peek(void)
{
    SceCtrlData ctrl;

    sceCtrlPeekBufferPositive(0, &ctrl, 1);

    if (ctrl.buttons != 0)
    {
        psv_input_data.is_keypad_pressed = true;
    }
    else
    {
        psv_input_data.is_keypad_pressed = false;
    }

    psv_input_data.keycode = ctrl.buttons;
}

static void lv_button_peek(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    psv_button_peek();

    if (psv_input_data.is_keypad_pressed)
    {
        data->state = LV_INDEV_STATE_PR;
    }
    else 
    {
        data->state = LV_INDEV_STATE_REL;
    }

    switch(psv_input_data.keycode) 
    {
        case SCE_CTRL_PSBUTTON:
            data->key = LV_KEY_HOME;
            break;
        case SCE_CTRL_UP:
            data->key = LV_KEY_UP;
            break;
        case SCE_CTRL_DOWN:
            data->key = LV_KEY_DOWN;
            break;
        case SCE_CTRL_LEFT:
            data->key = LV_KEY_LEFT;
            break;
        case SCE_CTRL_RIGHT:
            data->key = LV_KEY_RIGHT;
            break;
        case SCE_CTRL_CIRCLE:
            data->key = LV_KEY_ENTER;
            break;
        case SCE_CTRL_CROSS:
            data->key = LV_KEY_BACKSPACE;
            break;
        case SCE_CTRL_SQUARE:
            data->key = LV_KEY_END;
            break;
        case SCE_CTRL_L1:
            data->key = LV_KEY_PREV;
            break;
        case SCE_CTRL_R1:
            data->key = LV_KEY_NEXT;
            break;
    }
}
