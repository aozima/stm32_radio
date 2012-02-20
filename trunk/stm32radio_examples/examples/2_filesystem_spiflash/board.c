/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "stm32f10x.h"

#ifdef RT_USING_SPI
#include "stm32f10x_spi.h"
#include "rt_stm32f10x_spi.h"
#endif

/**
 * @addtogroup STM32
 */

/*@{*/

/**
 * This is the timer interrupt service routine.
 *
 */
void rt_hw_timer_handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

static void delay(void)
{
    volatile unsigned int dl;
    for(dl=0; dl<2500000; dl++);
}

static void all_device_reset(void)
{
    /* RESET */
    /* DM9000A          PE5  */
    /* LCD              PF10 */
    /* SPI-FLASH        PA3  */

    /*  CS */
    /* DM9000A FSMC_NE4 PG12 */
    /* LCD     FSMC_NE2 PG9  */
    /* SPI_FLASH        PA4  */
    /* CODEC            PC5  */
    /* TOUCH            PC4  */

    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE
                           | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    /* SDIO POWER */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOC,&GPIO_InitStructure);
    GPIO_SetBits(GPIOC,GPIO_Pin_6); /* SD card power down */

    /* SPI_FLASH CS */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_SetBits(GPIOA,GPIO_Pin_4);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

    /* CODEC && TOUCH CS */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOC,&GPIO_InitStructure);
    GPIO_SetBits(GPIOC,GPIO_Pin_4 | GPIO_Pin_5);

    /*  DM9000A RESET */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOE,&GPIO_InitStructure);
    GPIO_ResetBits(GPIOE,GPIO_Pin_5);

    /* LCD RESET */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOF,&GPIO_InitStructure);
    GPIO_ResetBits(GPIOF,GPIO_Pin_10);

    /* SPI_FLASH RESET */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_ResetBits(GPIOA,GPIO_Pin_3);

    /* FSMC GPIO configure */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF
                               | RCC_APB2Periph_GPIOG, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        /*
        FSMC_D0 ~ FSMC_D3
        PD14 FSMC_D0   PD15 FSMC_D1   PD0  FSMC_D2   PD1  FSMC_D3
        */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /*
        FSMC_D4 ~ FSMC_D12
        PE7 ~ PE15  FSMC_D4 ~ FSMC_D12
        */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
                                      | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_Init(GPIOE,&GPIO_InitStructure);

        /* FSMC_D13 ~ FSMC_D15   PD8 ~ PD10 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /*
        FSMC_A0 ~ FSMC_A5   FSMC_A6 ~ FSMC_A9
        PF0     ~ PF5       PF12    ~ PF15
        */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3
                                      | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_Init(GPIOF,&GPIO_InitStructure);

        /* FSMC_A10 ~ FSMC_A15  PG0 ~ PG5 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_Init(GPIOG,&GPIO_InitStructure);

        /* FSMC_A16 ~ FSMC_A18  PD11 ~ PD13 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /* RD-PD4 WR-PD5 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /* NBL0-PE0 NBL1-PE1 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
        GPIO_Init(GPIOE,&GPIO_InitStructure);

        /* NE1/NCE2 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_Init(GPIOD,&GPIO_InitStructure);
        /* NE2 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOG,&GPIO_InitStructure);
        /* NE3 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOG,&GPIO_InitStructure);
        /* NE4 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_Init(GPIOG,&GPIO_InitStructure);
    }
    /* FSMC GPIO configure */

    delay();
    GPIO_SetBits(GPIOE,GPIO_Pin_5);   /* DM9000A          */
    GPIO_SetBits(GPIOF,GPIO_Pin_10);  /* LCD              */
    GPIO_SetBits(GPIOA,GPIO_Pin_3);   /* SPI_FLASH        */
}

/**
 * This function will initial STM32 board.
 */
void rt_hw_board_init()
{
    /* 配置系统时钟并启动PLL,让系统工作在72M,此函数由库中提供 */
    SystemInit();

	/* 复位所有外设 */
	all_device_reset();

    /*
     * set priority group:
     * 2 bits for pre-emption priority
     * 2 bits for subpriority
     */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 配置systick分频器 */
    /* SystemCoreClock为系统主时钟 由库中提供,在system_stm32f10x.c中 */
    /* RT_TICK_PER_SECOND 为系统节拍,由rtconfig.h中定义 */
    SysTick_Config( SystemCoreClock / RT_TICK_PER_SECOND );

    /*  初始化串口 */
    rt_hw_usart_init();
#ifdef RT_USING_FINSH
    rt_console_set_device("uart1");
#endif

#ifdef RT_USING_SPI
    rt_stm32f10x_spi_init();

#   ifdef USING_SPI1
    /* attach spi10 : CS PA4 */
    {
        static struct rt_spi_device rt_spi_device;
        static struct stm32_spi_cs  stm32_spi_cs;
        GPIO_InitTypeDef GPIO_InitStructure;

        stm32_spi_cs.GPIOx = GPIOA;
        stm32_spi_cs.GPIO_Pin = GPIO_Pin_4;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_SetBits(GPIOA, GPIO_Pin_4);

        rt_spi_bus_attach_device(&rt_spi_device, "spi10", "spi1", (void*)&stm32_spi_cs);
    }
#   endif
#endif
}

void rt_hw_spi1_baud_rate(uint16_t SPI_BaudRatePrescaler)
{
	SPI1->CR1 &= ~SPI_BaudRatePrescaler_256;
	SPI1->CR1 |= SPI_BaudRatePrescaler;
}

/*@}*/
