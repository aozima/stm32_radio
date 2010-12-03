#include <rtthread.h>
#include <finsh.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/window.h>

/* 在finsh shell中创建的独立窗口 */

void window_entry(void* parameter)
{
	rt_mq_t mq;
	char name[32];
	rtgui_win_t *win;
	rtgui_rect_t rect = {10, 30, 110, 130};

	mq = rt_mq_create("wmq", 256, 8, RT_IPC_FLAG_FIFO);
	/* 注册当前线程为GUI线程 */
	rtgui_thread_register(rt_thread_self(), mq);

	snprintf(name, sizeof(name), "win %d", (rt_uint32_t)parameter);
	win = rtgui_win_create(RT_NULL, name, &rect, RTGUI_WIN_STYLE_DEFAULT);
	/* 显示窗口 */
	rtgui_win_show(win, RT_FALSE);

	/* 执行窗口的事件循环 */
	rtgui_win_event_loop(win);

	/* 去注册GUI线程 */
	rtgui_thread_deregister(rt_thread_self());

	/* delete message queue */
	rt_mq_delete(mq);
}

void window(void)
{
	rt_thread_t tid;
	static rt_uint32_t no = 0;

	tid = rt_thread_create("win", window_entry, (void*)no, 2048, 20, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);

	no ++;
}
FINSH_FUNCTION_EXPORT(window, startup a standalone window);
