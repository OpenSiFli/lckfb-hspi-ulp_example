#include "bf0_hal.h"
#include "drivers/rt_drv_pwm.h"
#include "drv_io.h"
#include "rtthread.h"
#include "stdio.h"
#include "string.h"

#define RGB_COLOR (0x00ff00)

#define RGBLED_NAME "rgbled"

struct rt_device *rgbled_device;
struct rt_color
{
    char *color_name;
    uint32_t color;
};

static struct rt_color rgb_color_arry[] = {
    {"black", 0x000000},  {"blue", 0x00000f}, {"green", 0x000f00},
    {"cyan", 0x000f0f},   {"red", 0x0f0000},  {"purple", 0x0f000f},
    {"yellow", 0x0f0f00}, {"white", 0x0f0f0f}};

static void rgb_led_init()
{
    HAL_PMU_ConfigPeriLdo(PMU_PERI_LDO3_3V3, true, true);
    HAL_PIN_Set(PAD_PA32, GPTIM2_CH1, PIN_NOPULL, 1);
    rgbled_device = rt_device_find(RGBLED_NAME); // find rgb
    if (!rgbled_device)
    {
        RT_ASSERT(0);
    }
}

static void rgb_led_set_color(uint32_t color)
{
    struct rt_rgbled_configuration configuration;
    configuration.color_rgb = color;
    rt_device_control(rgbled_device, PWM_CMD_SET_COLOR, &configuration);
}

/**
 * @brief  Main program
 * @param  None
 * @retval 0 if success, otherwise failure number
 */
int main(void)
{
    rgb_led_init();

    rt_kprintf("RGB LED example started!\n");
    
    // while(1) rgb_led_set_color(0xff00ff);
    /* Infinite loop */
    while (1)
    {
        for (int i = 0; i < sizeof(rgb_color_arry) / sizeof(rgb_color_arry[0]);
             i++)
        {
            rt_kprintf("-> %s\n", rgb_color_arry[i].color_name);
            rgb_led_set_color(rgb_color_arry[i].color);
            rt_thread_mdelay(5000);
        }
    }
    return 0;
}
