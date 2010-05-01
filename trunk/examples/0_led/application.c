/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <rtthread.h>
#include "led.h"

static void rt_thread_entry_led1(void* parameter)
{
    unsigned int count=0;
    while (1)
    {
        /* led1 on */
        rt_kprintf("led1 on,count : %d\r\n",count);
        count++;
        rt_hw_led_on(0);
        rt_thread_delay(50); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
        rt_kprintf("led1 off\r\n");
        rt_hw_led_off(0);
        rt_thread_delay(50);
    }
}

int rt_application_init()
{
    rt_thread_t thread;

    /* create led1 thread */
    thread = rt_thread_create("led1",
                              rt_thread_entry_led1, RT_NULL,
                              512,
                              20, 5);
    if (thread != RT_NULL)
        rt_thread_startup(thread);

    return 0;
}

/*@}*/
