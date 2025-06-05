# 手表界面

使用LVGL v8(后续会有v9版本)，包含的界面有：
- 蜂窝主菜单
- 表盘
- 立方体左右旋转

## 添加蜂窝功能
首先想要在蜂窝界面中添加一个新的蜂窝，则需要调```BUILTIN_APP_EXPORT()```这个接口进行一个app的注册。

例如：
``` 
LV_IMG_DECLARE(img_LiChuang);//声明一个图像资源，并转化为C数组，用于应用显示
#define APP_ID "hello_world"//定义应用的唯一标识符，用于系统识别和管理应用（每个APP都有唯一的一个标识ID）
static int app_main(intent_t i)//入口函数，用于系统启动时调用
{
    gui_app_regist_msg_handler(APP_ID, msg_handler);
    return 0;
}
BUILTIN_APP_EXPORT(LV_EXT_STR_ID(rotation3d), LV_EXT_IMG_GET(img_LiChuang), APP_ID, app_main);//注册消息处理函数，使应用能够响应系统发送的各种生命周期事件（如启动、暂停、恢复、停止）
```

然后在使用状态机```msg_handler() ```中的启动函数on_start()中添加跳转之后的页面的设置（例如需要点击进去之后显示hello world）。

```
static void on_start(void)
{
    //容器创建与布局
    lv_obj_t *lc = lv_obj_create(lc);//在当前活动屏幕上创建一个空白的基础容器对象（lc）,作为后面添加其他元素的父容器
    lv_obj_set_size(lc, LV_HOR_RES_MAX, LV_VER_RES_MAX);//设置屏幕的水平和垂直分辨率，确保容器铺满整个屏幕
    lv_obj_align(lc, LV_ALIGN_CENTER, 0, 0);//将容器居中对齐
    
    //文本标签创建与样式设置
    lv_obj_t *hello_label = lv_label_create(lv_scr_act());//在父容器（lc）上创建文本标签
    lv_label_set_text(hello_label, "Hello World");//设置需要显示的字体（hello world）
    lv_ext_set_local_font(hello_label, FONT_BIGL, LV_COLOR_WHITE);//设置字体样式大小（FONT_BIGL）和颜色白色（LV_COLOR_WHITE）
    lv_obj_center(hello_label); //居中显示

    lv_img_cache_invalidate_src(NULL);//清空LVGL的图像缓存，用于防止内存占用过高或图像更新不及时
}
```

## 指定字体
参考`src/resource/fonts/SConscript`，通过在CPPDEFINES中添加`FREETYPE_FONT_NAME`宏定义，可以注册对应TTF字体到LVGL中
```python
CPPDEFINES += ["FREETYPE_FONT_NAME={}".format(font_name)]
```

如果`font_name`是`DroidSansFallback`，相当于添加了如下宏定义
```c
#define FREETYPE_FONT_NAME   DroidSansFallback
```

编译时会在`freetype`子目录里查找以`.ttf`为后缀的字体文件，将其转换为C文件加入编译

```python
objs = Glob('freetype/{}.ttf'.format(font_name))
objs = Env.FontFile(objs)
```


`FREETYPE_TINY_FONT_FULL`这些宏是在工程目录下的`Kconfig.proj`中定义，类似下面这样

```kconfig
config FREETYPE_TINY_FONT_FULL
    bool
    default y
```

## 代码结构
```Bash
  src/
   ├── app_utils/                  # 工具类代码，如 main.c、资源加载等
   ├── gui_apps/                   # GUI 应用模块
   │   ├── clock/                  # 时钟应用模块
   │   ├── main/                   # 主菜单应用（核心分析对象）
   │   ├── mem/                    # 内存管理模块
   │   ├── rotation3d/             # 3D旋转动画效果
   │   ├── utils/                  # 自定义动画工具等
   │   └── watch_demo.c            # 主程序入口
   ├── resource/
   │   ├── fonts/                  # 字体资源（包括内置字体和 FreeType 支持）
   │   ├── images/                 # 图像资源（图标、背景图等）
   │   └── strings/                # 多语言字符串资源
   └── SConscript                  # 构建脚本
```
## 编译
* 切换到例程project目录，运行scons命令执行编译：
```c
scons --board=sf32lb52-ulp -j8
```

编译后可查看build_sf32lb52-ulp_hcpu\rtconfig.h中宏定义配置情况

- 包含了下面宏（此处列举一些常用配置）
```c
#define LVGL_V8 1 //使用LVGLV8 版本
#define PKG_USING_LITTLEVGL2RTT 1 //启用LittlevGL图形库
#define LV_FONT_MONTSERRAT_12 1 //启用Montserrat 12号字体
#define LV_FB_TWO_NOT_SCREEN_SIZE 1 //启用双缓冲帧缓冲区，且不限制屏幕大小
#define IMAGE_CACHE_IN_PSRAM_SIZE 1100000 //设置PSRAM中的图像缓存大小为1100000字节
#define IMAGE_CACHE_IN_SRAM_SIZE 50000 //设置SRAM中的图像缓存大小为50000字节
#define USING_CELL_TRANSFORM 1 //启用蜂窝变换功能
...
```


先在watch_demo.c定义所需要的设备及参数宏定义
```c
#define APP_WATCH_GUI_TASK_STACK_SIZE 16*1024 //watch界面任务的堆栈大小为16KB（16 * 1024字节）
#define SLEEP_CTRL_PIN   (BSP_KEY1_PIN) //控制设备进入睡眠模式的引脚
#define LCD_DEVICE_NAME  "lcd" //LCD设备的名称
#define IDLE_TIME_LIMIT  (10000) //空闲时间限制为10000毫秒
```
在watch_demo.c中添加app_watch_init()创建一个RTOS线程app_watch_entry为线程函数，此函数是GUI 的主入口函数，负责初始化所有图形组件和运行主循环。

```c
//在独立线程中运行 GUI 主循环，避免阻塞系统其他功能。
int app_watch_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_thread_t thread = RT_NULL;

    ret = rt_thread_init(&watch_thread, "app_watch", app_watch_entry, RT_NULL, watch_thread_stack, APP_WATCH_GUI_TASK_STACK_SIZE,
                         RT_THREAD_PRIORITY_MIDDLE, RT_THREAD_TICK_DEFAULT);//初始化线程

    if (RT_EOK != ret)
    {
        return RT_ERROR;
    }
    rt_thread_startup(&watch_thread); //开启线程
    return RT_EOK;
}

INIT_APP_EXPORT(app_watch_init);// 注册为自动执行函数。
```

现在我们继续添加app_watch_entry线程函数
```c
void app_watch_entry(void *parameter)
{
    init_pin();//初始化物理按键，对应按键回调函数：button_event_handler
    lcd_device = rt_device_find(LCD_DEVICE_NAME);//寻找设备
    
    {
        rt_err_t r = littlevgl2rtt_init(LCD_DEVICE_NAME); //初始化 LVGL 的显示驱动、输入设备
        RT_ASSERT(RT_EOK == r);
    }
    lv_ex_data_pool_init(); //初始化数据池和资源
    resource_init();//初始化资源管理器
    gui_app_init();//初始化 GUI 应用框架（如消息队列、生命周期管理、页面切换动画等）
    
    gui_app_run("Main");//启动名为 "Main" 的应用，即 app_mainmenu.c 中注册的主菜单界面
    
    lv_disp_trig_activity(NULL);//触发活动事件，表示当前屏幕处于活动状态。防止刚启动时被误判为空闲状态而休眠。
    while (1)
    {
        int ms;
    
        rt_pm_request(PM_SLEEP_MODE_IDLE);
        ms = lv_timer_handler(); // 处理 LVGL 定时器事件
        rt_pm_release(PM_SLEEP_MODE_IDLE);
    
        if (ms > 0)
            rt_thread_mdelay(ms); // 延迟一段时间
    
        if (first_loop)
        {
            uint8_t brightness = 100;
            rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &brightness); // 打开背光
            first_loop = 0;
        }
    }
    
}
```
当我们调用gui_app_run("Main");就会触发app_mainmenu.c里的msg_handler函数进行初始化主菜单的界面创建页面并加载图标，以下是简化的调用链路
```c
//简化的调用链路：
+---------------------+
|     app_watch_init() |
+----------+----------+
           |
           v
+---------------------+
|  watch_thread_entry |
+----------+----------+
           |
           v
+---------------------+
|    GUI 初始化流程   |
| - init_pin()        |
| - littlevgl2rtt_init() |
| - gui_app_init()    |
| - keypad_default_handler_register() |
+----------+----------+
           |
           v
+---------------------+
| gui_app_run("Main") |
+----------+----------+
           |
           v
+---------------------+
| msg_handler(MSG_START) |
+----------+----------+
           |
           v
+---------------------+
|     on_start()      |
| - 创建页面          |
| - 加载图标资源      |
| - 设置图标位置      |
| - 注册事件回调      |
+----------+----------+
           |
           v
+---------------------+
| mainmenu_cell_icons_event_cb() |
| - 点击图标         |
| - gui_app_run(cmd) |
+--------------------+
```
