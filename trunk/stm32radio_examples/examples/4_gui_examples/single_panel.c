#include <rtthread.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/workbench.h>

#include "lcd.h"
#include "key.h"
#include "touch.h"

void gui_init()
{
	extern void workbench_init(void);
    rtgui_rect_t rect;

	/* 初始化RT-Thread/GUI server */
    rtgui_system_server_init();

    /* 注册面板 */
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = 240;
    rect.y2 = 320;
    rtgui_panel_register("main", &rect);
    rtgui_panel_set_default_focused("main");

	/* 初始化LCD驱动 */
    rt_hw_lcd_init();

	/* 初始化键盘驱动 */
	rt_hw_key_init();

	/* 初始化触摸屏驱动 */
	rt_hw_touch_init();

	/* 初始化workbench */
	workbench_init();
}
