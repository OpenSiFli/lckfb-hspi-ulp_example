/*********************
 *      INCLUDES
 *********************/


#include <rtthread.h>
#include <rtdevice.h>
#include "littlevgl2rtt.h"
#include "lvgl.h"
#include "lvsf_comp.h"
#include "gui_app_fwk.h"
#include "lv_ext_resource_manager.h"
#include "lv_ex_data.h"
#include "app_mem.h"


#if (1 == LV_USE_GPU) && (!defined(SF32LB55X))
#include "drv_epic.h"
#if (IMAGE_CACHE_IN_SRAM_SIZE >= 258*1000)
    #define ROTATE_BG_IMG_VARIABLE clock_rotate_bg_bg   //Image size 257766 bytes, 359x359
#elif (IMAGE_CACHE_IN_SRAM_SIZE >= 81*1000)
    #define ROTATE_BG_IMG_VARIABLE clock_rotate_bg_bg_small //Image size 80000 bytes, 200x200
#else
    #define ROTATE_BG_IMG_VARIABLE clock_rotate_bg_bg_tiny //Image size 45000 bytes, 150x150
#endif




static void on_start(void)
{
   
    lv_obj_t *lc = lv_obj_create(lv_scr_act());
    // lv_obj_set_size(lc, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_size(lc, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_align(lc, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *hello_label = lv_label_create(lc);
    lv_label_set_text(hello_label, "Hello World");
    lv_ext_set_local_font(hello_label, FONT_BIGL, LV_COLOR_WHITE);
    lv_obj_center(hello_label); 

    lv_img_cache_invalidate_src(NULL);
}

static void on_pause(void)
{
}

static void on_resume(void)
{
}

static void on_stop(void)
{
}


static void msg_handler(gui_app_msg_type_t msg, void *param)
{
    switch (msg)
    {
    case GUI_APP_MSG_ONSTART:
        on_start();
        break;

    case GUI_APP_MSG_ONRESUME:
        on_resume();
        break;

    case GUI_APP_MSG_ONPAUSE:
        on_pause();
        break;

    case GUI_APP_MSG_ONSTOP:
        on_stop();
        break;
    default:
        break;
    }
}

LV_IMG_DECLARE(img_LiChuang);
#define APP_ID "hello_world"
static int app_main(intent_t i)
{

    gui_app_regist_msg_handler(APP_ID, msg_handler);

    return 0;
}

BUILTIN_APP_EXPORT(LV_EXT_STR_ID(lckfb), LV_EXT_IMG_GET(img_LiChuang), APP_ID, app_main);

#endif