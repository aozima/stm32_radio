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
#include <finsh.h>

#include <stm32f10x.h>
#include "board.h"
#include "lcd.h"
#include "setup.h"

#ifdef RT_USING_SPI
#include <spi_flash_at45dbxx.h>
#include <spi_flash_sst25vfxx.h>
#endif

#ifdef RT_USING_DFS
/* dfs init */
#include <dfs_init.h>
/* dfs filesystem:ELM FatFs filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/driver.h>
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#endif

/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
#ifdef RT_USING_SPI
    /* init hardware device */
    if(sst25vfxx_init("flash0", "spi10") != RT_EOK)
    {
        if(at45dbxx_init("flash0", "spi10") != RT_EOK)
        {
            rt_kprintf("[error] No such spi flash!\r\n");
        }
    }
#endif

#ifdef RT_USING_DFS
    /* Filesystem Initialization */
    {
		extern void ff_convert_init();

        /* init the device filesystem */
        dfs_init();

        /* init the elm FAT filesystam*/
        elm_init();

        /* mount spi flash fat as root directory */
        if (dfs_mount("flash0", "/", "elm", 0, 0) == 0)
        {
            rt_kprintf("SPI File System initialized!\n");

        }
        else
            rt_kprintf("SPI File System init failed!\n");

#ifdef RT_DFS_ELM_USE_LFN
		ff_convert_init();
#endif
    }
#endif


#ifdef RT_USING_RTGUI
	{
        extern void rt_hw_key_init(void);
	    extern rt_err_t rtgui_touch_hw_init(const char * spi_device_name);
		extern void rtgui_startup();
		rt_device_t lcd;

		/* init lcd */
		rt_hw_lcd_init();

		rt_hw_key_init();

		/* init touch panel */
		load_setup();
		rtgui_touch_hw_init("spi11");

		/* re-init device driver */
		rt_device_init_all();

		/* find lcd device */
		lcd = rt_device_find("lcd");

		/* set lcd device as rtgui graphic driver */
		rtgui_graphic_set_device(lcd);

		/* startup rtgui */
		rtgui_startup();
	}
#endif

}

int rt_application_init()
{
    rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif
    if (init_thread != RT_NULL) rt_thread_startup(init_thread);

    return 0;
}

/*@}*/
