#include <rtthread.h>
#include "led.h"

static struct rt_thread led_thread;
static rt_uint32_t led_stack[512/4];
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;
    while (1)
    {
        /* led1 on */
#ifndef RT_USING_FINSH
        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        rt_hw_led_on(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        rt_kprintf("led2 off\r\n");
#endif
        rt_hw_led_off(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}

int rt_application_init()
{
	rt_err_t result;

    /* init led thread */
	result = rt_thread_init(&led_thread,
		"led",
		led_thread_entry, RT_NULL,
		(rt_uint8_t*)&led_stack[0], sizeof(led_stack), 20, 5);
	if (result == RT_EOK)
	{
        rt_thread_startup(&led_thread);
	}

    return 0;
}
