#include "../lvgl/demos/widgets/lv_demo_widgets.h"
#include "../lvgl/lvgl.h"
#include "../platform/lvgl_vita.h"
#include "../lvgl/demos/lv_demos.h"

int main()
{
    psv_lv_init();

    lv_demo_widgets();

    psv_lv_mainloop();

    lv_demo_widgets_close();

    psv_lv_deinit();

    return 0;
}
