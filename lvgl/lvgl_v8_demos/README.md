# LVGL v8 Demos
## 一、项目介绍
本示例用来演示LVGL V8的官方示例，使用官方提供的demo应用程序。
可以使用menuconfig来选择演示的demo应用程序。包含的应用程序有：
- Show some widget 演示lvgl widget的使用
- Demonstrate the usage of encoder and keyboard 演示键盘
- Benchmark your system 演示benchmark
- Stress test for LVGL 压力测试
- Music player demo 演示音乐播放

## 工程编译及下载：
- 执行`scons --board=sf32lb52-lchspi-ulp -j8`即可生成工程
- 下载可以执行build_sf32lb52-lchspi-ulp_hcpu\uart_download.bat输入下载UART的端口号执行

            
使用宏定义来选择启动的demo
```c
#if  LV_USE_DEMO_WIDGETS
    #if  LV_USE_DEMO_BENCHMARK
        #define lv_demo_main lv_demo_benchmark
    #else
        #define lv_demo_main lv_demo_widgets
    #endif
#elif   LV_USE_DEMO_KEYPAD_AND_ENCODER
    #define lv_demo_main lv_demo_keypad_encoder
#elif   LV_USE_DEMO_MUSIC
    #define lv_demo_main lv_demo_music
#elif   LV_USE_DEMO_STRESS
    #define lv_demo_main  lv_demo_stress
#else
    #error "Select a demo application to start"
#endif
```
我们可以在menuconfig中选择想使用的demo

![](assets/demo_pick.png)

## 二、main.c 分析 —— 程序入口与主循环

1. main 函数
```c

int main(void)
{
int main(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t ms;

    /* init littlevGL */
    ret = littlevgl2rtt_init("lcd"); // 初始化显示驱动和输入设备
    if (ret != RT_EOK) {
        return ret;
    }

    lv_ex_data_pool_init(); // 初始化数据池

    lv_demo_main();         // 启动指定的 demo（根据宏定义）

    while (1)
    {
        ms = lv_task_handler();       // 处理 LVGL 内部任务调度器
        rt_thread_mdelay(ms);         // 按帧率延迟
    }
    return RT_EOK;
}
}
```
## 三、lv_demo_benchmark.c 分析 —— Benchmark 核心逻辑
1. 入口函数：lv_demo_benchmark()
```c
void lv_demo_benchmark(void)
{
    benchmark_init(); // 初始化界面和监控回调

    next_scene_timer_cb(NULL); // 手动触发第一个场景
}
- benchmark_init()：
   设置屏幕样式
   注册性能监控回调
   创建标题、副标题、背景对象
- next_scene_timer_cb(NULL)：
   触发第一个场景加载
```
2. 场景切换机制
```c
lv_timer_create(next_scene_timer_cb, SCENE_TIME, NULL);//创建定时器切换场景
```
定时器回调函数
```c
static void next_scene_timer_cb(lv_timer_t *timer)
{
    scene_act++; // 场景索引递增
    load_scene(scene_act);

    if (scenes[scene_act].create_cb == NULL) {
        lv_timer_delete(timer);
        generate_report(); // 最后一个场景结束，生成报告
    } else {
        lv_timer_set_period(timer, scenes[scene_act].scene_time);
    }
}
```
 加载场景函数：load_scene(uint32_t scene)
```c
static void load_scene(uint32_t scene)
{
    lv_obj_clean(scene_bg); // 清除当前场景内容
    if (scenes[scene].create_cb) {
        scenes[scene].create_cb(); // 执行创建回调
    }
}
```
3. 场景描述结构体：scene_dsc_t
```c
typedef struct
{
    const char *name;
    void (*create_cb)(void);
    uint32_t time_sum_normal;   // 正常模式总耗时
    uint32_t time_sum_opa;      // 半透明模式总耗时
    uint32_t refr_cnt_normal;   // 正常模式刷新次数
    uint32_t refr_cnt_opa;      // 半透明模式刷新次数
    uint32_t fps_normal;        // 正常 FPS
    uint32_t fps_opa;           // 半透明 FPS
    uint8_t weight;             // 权重，用于加权计算
} scene_dsc_t;
```
示例场景定义
```c
static scene_dsc_t scenes[] =
{
    {.name = "Rectangle", .weight = 30, .create_cb = rectangle_cb},
    {.name = "Circle border", .weight = 10, .create_cb = border_circle_cb},
    ...
    {.name = "", .create_cb = NULL}
};
```
4. 性能监控模块（LV_USE_PERF_MONITOR）
```c
static scene_dsc_t scenes[] =
{
    {.name = "Rectangle", .weight = 30, .create_cb = rectangle_cb},
    {.name = "Circle border", .weight = 10, .create_cb = border_circle_cb},
    ...
    {.name = "", .create_cb = NULL}
};
```
注册观察者：
```c
disp->driver->monitor_cb = monitor_cb;
```
回调函数：monitor_cb(lv_disp_drv_t *drv, uint32_t time, uint32_t px)
```c
static void monitor_cb(lv_disp_drv_t *drv, uint32_t time, uint32_t px)
{
    if (opa_mode) {
        scenes[scene_act].time_sum_opa += time;
        scenes[scene_act].refr_cnt_opa++;
    } else {
        scenes[scene_act].time_sum_normal += time;
        scenes[scene_act].refr_cnt_normal++;
    }
}
```
5. 动画系统详解
```c
lv_anim_t a;
lv_anim_init(&a);
lv_anim_set_exec_cb(&a, fall_anim_y_cb); // 绑定执行回调
lv_anim_set_values(&a, 0, target_y);     // 设置起始/目标值
lv_anim_set_time(&a, duration);          // 设置持续时间
lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); // 设置无限循环
lv_anim_start(&a);                       // 启动动画
```
动画回调函数示例：
```c
static void fall_anim_y_cb(void *var, int32_t v)
{
    lv_obj_set_y(var, v); // 改变 Y 坐标
}
```
动画类型示例：

颜色变化    color_anim_cb(回调函数：改变背景或文本颜色)	         
移动动画	   fall_anim_y_cb(回调函数：对象从上往下移动)	      
弧线动画	   arc_anim_end_angle_cb(回调函数：弧形进度条动画)	


6. UI 组件构建示例：card_create()

```c
static lv_obj_t *card_create(void)
{
    lv_obj_t *panel = lv_obj_create(scene_bg); // 创建容器
    lv_obj_remove_style_all(panel);
    lv_obj_add_style(panel, &style_common, 0);

    lv_obj_t *img = lv_image_create(panel); // 添加图片
    lv_image_set_src(img, &img_benchmark_cogwheel_rgb);

    lv_obj_t *label = lv_label_create(panel); // 添加标签
    lv_label_set_text(label, "John Smith");

    return panel;
}
```

完整调用链路图

```c
main()
│
├── littlevgl2rtt_init()            → 初始化 LVGL 运行环境
│
├── lv_demo_benchmark()             → 启动 benchmark demo
│   │
│   ├── benchmark_init()            → 初始化界面、样式、性能监控
│   │
│   ├── load_scene(0)               → 加载第一个场景
│   │   └── scenes[0].create_cb() => rectangle_cb()
│   │
│   ├── lv_timer_create(...)        → 创建定时器，定时切换场景
│   │
│   └── monitor_cb                  → 性能数据采集回调
│
└── while (1)
    └── lv_task_handler()           → LVGL 主任务调度
```