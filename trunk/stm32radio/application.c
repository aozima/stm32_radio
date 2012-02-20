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
#include "netbuffer.h"
#include "lcd.h"
#include "rtc.h"
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
#include <rtgui/rtgui.h>
#include <rtgui/driver.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>

#include <touch.h>
#include <codec.h>

extern void radio_rtgui_init(void);

#endif

/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
    codec_hw_init("spi12");

#ifdef RT_USING_DFS
    /* init hardware device */
    rt_hw_sdcard_init();
#   ifdef RT_USING_SPI
    /* init hardware device */
    if(sst25vfxx_init("flash0", "spi10") != RT_EOK)
    {
        if(at45dbxx_init("flash0", "spi10") != RT_EOK)
        {
            rt_kprintf("[error] No such spi flash!\r\n");
        }
    }
#   endif

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

            /* mount sd card fat partition 1 as SD directory */
            if (dfs_mount("sd0", "/SD", "elm", 0, 0) == 0)
                rt_kprintf("SD File System initialized!\n");
            else
                rt_kprintf("SD File System init failed!\n");
        }
        else
            rt_kprintf("SPI File System init failed!\n");

#ifdef RT_DFS_ELM_USE_LFN
        ff_convert_init();
#endif
    }
#endif

    load_setup();

    /* RTGUI Initialization */
#ifdef RT_USING_RTGUI
    {
        extern void rt_hw_key_init(void);
        extern void remote_init(void);

        rt_device_t lcd;
        rtgui_rect_t rect;

        //radio_rtgui_init();
        rt_hw_lcd_init();

        lcd = rt_device_find("lcd");
        if (lcd != RT_NULL)
        {
            rt_device_init(lcd);
            rtgui_graphic_set_device(lcd);

            /* init RT-Thread/GUI server */
            rtgui_system_server_init();

            /* register dock panel */
            rect.x1 = 0;
            rect.y1 = 0;
            rect.x2 = 240;
            rect.y2 = 25;
            rtgui_panel_register("info", &rect);
            rtgui_panel_set_nofocused("info");

            /* register main panel */
            rect.x1 = 0;
            rect.y1 = 25;
            rect.x2 = 240;
            rect.y2 = 320;
            rtgui_panel_register("main", &rect);
            rtgui_panel_set_default_focused("main");

            info_init();
            player_init();
        }

        rt_hw_key_init();
        rtgui_touch_hw_init("spi11");
        remote_init();
    }
#endif

    /* set default setup */
    {
        extern void vol(uint16_t v);
        extern void brightness_set(unsigned int value);
        vol(radio_setup.default_volume);
        brightness_set(radio_setup.lcd_brightness);
    }

    /* start RTC */
    rt_hw_rtc_init();

    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);
        extern void rt_hw_dm9000_init(void);

        eth_system_device_init();

        /* register ethernetif device */
        rt_hw_dm9000_init();
        /* init all device */
        rt_device_init_all();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");
    }
#endif

#if STM32_EXT_SRAM
    /* init netbuf worker */
    net_buf_init(320 * 1024);
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
