#include <rtthread.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/workbench.h>

#include "lcd.h"
#include "key.h"
#include "touch.h"

void workbench_panel1(void* parameter)
{
	rt_mq_t mq;
	rtgui_view_t* view;
	struct rtgui_workbench* workbench;

	mq = rt_mq_create("wmq1", 256, 8, RT_IPC_FLAG_FIFO);
	/* 注册当前线程为GUI线程 */
	rtgui_thread_register(rt_thread_self(), mq);
	/* 创建一个工作台 */
	workbench = rtgui_workbench_create("info", "workbench #1");
	if (workbench == RT_NULL) return;

	view = rtgui_view_create("view1");
	if (view == RT_NULL) return;
	/* 指定视图的背景色 */
	RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(view)) = white;

	/* 添加到父workbench中 */
	rtgui_workbench_add_view(workbench, view);
	/* 非模式方式显示视图 */
	rtgui_view_show(view, RT_FALSE);

	/* 执行工作台事件循环 */
	rtgui_workbench_event_loop(workbench);

	/* 去注册GUI线程 */
	rtgui_thread_deregister(rt_thread_self());

	/* delete message queue */
	rt_mq_delete(mq);
}

void workbench_panel2(void* parameter)
{
	rt_mq_t mq;
	rtgui_view_t* view;
	struct rtgui_workbench* workbench;

	mq = rt_mq_create("wmq2", 256, 8, RT_IPC_FLAG_FIFO);
	/* 注册当前线程为GUI线程 */
	rtgui_thread_register(rt_thread_self(), mq);
	/* 创建一个工作台 */
	workbench = rtgui_workbench_create("panel2", "workbench #2");
	if (workbench == RT_NULL) return;

	view = rtgui_view_create("view2");
	if (view == RT_NULL) return ;
	/* 指定视图的背景色 */
	RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(view)) = green;
	/* 添加到父workbench中 */
	rtgui_workbench_add_view(workbench, view);
	/* 非模式方式显示视图 */
	rtgui_view_show(view, RT_FALSE);

	/* 执行工作台事件循环 */
	rtgui_workbench_event_loop(workbench);
	/* 去注册GUI线程 */
	rtgui_thread_deregister(rt_thread_self());
	rt_mq_delete(mq);
}

void workbench_panel3(void* parameter)
{
	rt_mq_t mq;
	rtgui_view_t* view;
	struct rtgui_workbench* workbench;

	mq = rt_mq_create("wmq3", 256, 8, RT_IPC_FLAG_FIFO);
	/* 注册当前线程为GUI线程 */
	rtgui_thread_register(rt_thread_self(), mq);
	/* 创建一个工作台 */
	workbench = rtgui_workbench_create("panel3", "workbench #2");
	if (workbench == RT_NULL) return;

	view = rtgui_view_create("view2");
	if (view == RT_NULL) return ;
	/* 指定视图的背景色 */
	RTGUI_WIDGET_BACKGROUND(RTGUI_WIDGET(view)) = red;
	/* 添加到父workbench中 */
	rtgui_workbench_add_view(workbench, view);
	/* 非模式方式显示视图 */
	rtgui_view_show(view, RT_FALSE);

	/* 执行工作台事件循环 */
	rtgui_workbench_event_loop(workbench);
	/* 去注册GUI线程 */
	rtgui_thread_deregister(rt_thread_self());
	rt_mq_delete(mq);
}

/* 初始化两个workbench，分别位于info，panel2和panel3 */
void workbench_init()
{
	rt_thread_t tid;

	tid = rt_thread_create("wb1", workbench_panel1, RT_NULL, 2048, 20, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);

	tid = rt_thread_create("wb2", workbench_panel2, RT_NULL, 2048, 20, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);

	tid = rt_thread_create("wb3", workbench_panel3, RT_NULL, 2048, 20, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);
}

void gui_init()
{
	extern void rtgui_touch_hw_init(void);
	extern rt_err_t load_setup(void);
    rtgui_rect_t rect;
	rt_device_t lcd;

	/* 初始化LCD驱动 */
    rt_hw_lcd_init();
	lcd = rt_device_find("lcd");
	if (lcd != RT_NULL)
	{
		rt_device_init(lcd);
		rtgui_graphic_set_device(lcd);

		/* 初始化RT-Thread/GUI server */
	    rtgui_system_server_init();

    /* 注册面板1 */
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = 240;
    rect.y2 = 25;
    rtgui_panel_register("info", &rect);
    rtgui_panel_set_default_focused("info");

	/* 注册面板2 */
    rect.x1 = 0;
    rect.y1 = 25;
    rect.x2 = 120;
    rect.y2 = 320;
    rtgui_panel_register("panel2", &rect);

	/* 注册面板3 */
    rect.x1 = 120;
    rect.y1 = 25;
    rect.x2 = 240;
    rect.y2 = 320;
    rtgui_panel_register("panel3", &rect);

	/* 初始化键盘驱动 */
	rt_hw_key_init();

	/* 初始化触摸屏驱动 */
	load_setup(); //touch装载默认值
	rtgui_touch_hw_init();	
    rt_device_init_all();

	/* 初始化workbench应用 */
	workbench_init();
	}
}
