#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "drv_gpio.h"

#include "sensor_liteon_ltr303.h"
#include "sensor_memsic_mmc56x3.h"
#include "st_lsm6dsl_sensor_v1.h"


/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
// 初始化硬件IO
static void board_io_init(void)
{
    HAL_PIN_Set(PAD_PA40, I2C1_SCL, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA39, I2C1_SDA, PIN_PULLUP, 1);
}

// 初始化所有传感器
static void sensors_init(struct rt_sensor_config *cfg)
{
    cfg->intf.dev_name = "i2c1";
    rt_hw_ltr303_init("ltr303", cfg);
    rt_hw_mmc56x3_init("mmc56x3", cfg);

    cfg->intf.user_data = (void *)LSM6DSL_ADDR_DEFAULT;
    cfg->irq_pin.pin = RT_PIN_NONE;
    rt_hw_lsm6dsl_init("lsm6d", cfg);
}
// 打开并配置传感器设备
static void open_and_config_devices(rt_device_t *ltr303_dev, rt_device_t *mmc56x3_dev,
                                    rt_device_t *lsm6d_dev, rt_device_t *lsm6d_gyro_dev, rt_device_t *lsm6d_step_dev)
{
    *ltr303_dev = rt_device_find("li_ltr303");
    if (*ltr303_dev == RT_NULL)
        rt_kprintf("Can't find device:%s\n", "ltr303");
    else {
        rt_err_t ret = rt_device_open(*ltr303_dev, RT_DEVICE_FLAG_RDONLY);
        if (ret != RT_EOK)
            rt_kprintf("open device failed! err: %d\n", ret);
        rt_device_control(*ltr303_dev, RT_SENSOR_CTRL_SET_POWER, (void *)RT_SENSOR_POWER_NORMAL);
    }

    *mmc56x3_dev = rt_device_find("mag_mmc56x3");
    if (*mmc56x3_dev == RT_NULL)
        rt_kprintf("Can't find device:%s\n", "mmc56x3");
    else {
        rt_err_t ret = rt_device_open(*mmc56x3_dev, RT_DEVICE_FLAG_RDONLY);
        if (ret != RT_EOK)
            rt_kprintf("open device failed! err: %d\n", ret);
    }

    *lsm6d_dev = rt_device_find("acce_lsm");
    *lsm6d_gyro_dev = rt_device_find("gyro_lsm");
    *lsm6d_step_dev = rt_device_find("step_lsm");
    if (*lsm6d_dev == RT_NULL || *lsm6d_gyro_dev == RT_NULL || *lsm6d_step_dev == RT_NULL)
        rt_kprintf("Can't find device:%s\n", "lsm6d");
    else {
        rt_err_t ret = rt_device_open(*lsm6d_dev, RT_DEVICE_FLAG_RDONLY);
        ret += rt_device_open(*lsm6d_gyro_dev, RT_DEVICE_FLAG_RDONLY);
        ret += rt_device_open(*lsm6d_step_dev, RT_DEVICE_FLAG_RDONLY);
        if (ret != RT_EOK)
            rt_kprintf("open device failed! err: %d\n", ret);
        rt_device_control(*lsm6d_dev, RT_SENSOR_CTRL_SET_ODR, (void *)1660);
        rt_device_control(*lsm6d_gyro_dev, RT_SENSOR_CTRL_SET_ODR, (void *)1660);
    }
}

// 传感器数据采集与打印
static void sensors_loop(rt_device_t ltr303_dev, rt_device_t mmc56x3_dev,
                        rt_device_t lsm6d_dev, rt_device_t lsm6d_gyro_dev, rt_device_t lsm6d_step_dev)
{
    struct rt_sensor_data ltr303, mmc56x3, lsm6d_acce, lsm6d_gyro, lsm6d_step;
    rt_size_t res;

    res = rt_device_read(ltr303_dev, 0, &ltr303, 1);
    if (res != 1)
        rt_kprintf("read data failed!size is %d\n", res);
    else
        rt_kprintf("light: %d lux\n", ltr303.data.light);

    res = rt_device_read(mmc56x3_dev, 0, &mmc56x3, 1);
    if (res != 1)
        rt_kprintf("read data failed!size is %d\n", res);
    else
        rt_kprintf("mag, x: %d, y: %d, z: %d\n", mmc56x3.data.mag.x, mmc56x3.data.mag.y, mmc56x3.data.mag.z);

    res = rt_device_read(lsm6d_dev, 0, &lsm6d_acce, 1);
    if (res != 1)
        rt_kprintf("read data failed!size is %d\n", res);
    else
        rt_kprintf("acce, x: %d, y: %d, z: %d\n", lsm6d_acce.data.acce.x, lsm6d_acce.data.acce.y, lsm6d_acce.data.acce.z);

    res = rt_device_read(lsm6d_gyro_dev, 0, &lsm6d_gyro, 1);
    if (res != 1)
        rt_kprintf("read data failed!size is %d\n", res);
    else
        rt_kprintf("gyro, x: %d, y: %d, z: %d\n", lsm6d_gyro.data.gyro.x, lsm6d_gyro.data.gyro.y, lsm6d_gyro.data.gyro.z);

    res = rt_device_read(lsm6d_step_dev, 0, &lsm6d_step, 1);
    if (res != 1)
        rt_kprintf("read data failed!size is %d\n", res);
    else
        rt_kprintf("lsm6d step, step: %d\n", lsm6d_step.data.step);

    rt_thread_mdelay(100);
    rt_kprintf("\n");
}
// 主函数
int main(void)
{
    rt_kprintf("Hello world!\n");

    board_io_init();

    struct rt_sensor_config cfg;
    sensors_init(&cfg);

    rt_device_t ltr303_dev, mmc56x3_dev, lsm6d_dev, lsm6d_gyro_dev, lsm6d_step_dev;
    open_and_config_devices(&ltr303_dev, &mmc56x3_dev, &lsm6d_dev, &lsm6d_gyro_dev, &lsm6d_step_dev);
    while (1)
    {
        sensors_loop(ltr303_dev, mmc56x3_dev, lsm6d_dev, lsm6d_gyro_dev, lsm6d_step_dev);
    }
    return 0;
}

