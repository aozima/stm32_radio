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

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#endif

/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);
        extern void rt_hw_dm9000_init(void);
		extern void telnet_srv(void);

        eth_system_device_init();

        /* register ethernetif device */
        rt_hw_dm9000_init();
        /* init all device */
        rt_device_init_all();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");

		/* start telnet server */
		telnet_srv();
    }
#endif
}

int rt_application_init()
{
	rt_thread_t init_thread;

	init_thread = rt_thread_create("init",
								rt_init_thread_entry, RT_NULL,
								1024, 21, 20);
	rt_thread_startup(init_thread);

	return 0;
}

/*@}*/
