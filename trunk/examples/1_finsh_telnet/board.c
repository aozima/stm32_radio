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

#include "stm32f10x.h"
#include "board.h"
#include "usart.h"

/**
 * @addtogroup STM32
 */

/*@{*/

///* init console to support rt_kprintf */
//static void rt_hw_console_init()
//{
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
//
//    /* Enable USART1 and GPIOA clocks */
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
//
//    /* GPIO configuration */
//    {
//        GPIO_InitTypeDef GPIO_InitStructure;
//
//        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//
//        /* Configure USART Tx as alternate function push-pull */
//        GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;
//        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//        GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//        /* Configure USART Rx as input floating */
//        GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
//        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//        GPIO_Init(GPIOA, &GPIO_InitStructure);
//    }
//
//    /* USART configuration */
//    {
//        USART_InitTypeDef USART_InitStructure;
//
//        /* USART configured as follow:
//        - BaudRate = 115200 baud
//        - Word Length = 8 Bits
//        - One Stop Bit
//        - No parity
//        - Hardware flow control disabled (RTS and CTS signals)
//        - Receive and transmit enabled
//        - USART Clock disabled
//        - USART CPOL: Clock is active low
//        - USART CPHA: Data is captured on the middle
//        - USART LastBit: The clock pulse of the last data bit is not output to
//        the SCLK pin
//        */
//        USART_InitStructure.USART_BaudRate = 115200;
//        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//        USART_InitStructure.USART_StopBits = USART_StopBits_1;
//        USART_InitStructure.USART_Parity = USART_Parity_No;
//        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//        USART_Init(USART1, &USART_InitStructure);
//        /* Enable USART */
//        USART_Cmd(USART1, ENABLE);
//    }
//}
//
///* write one character to serial, must not trigger interrupt */
//static void rt_hw_console_putc(const char c)
//{
//    /*
//    	to be polite with serial console add a line feed
//    	to the carriage return character
//    */
//    if (c=='\n')rt_hw_console_putc('\r');
//
//    while (!(USART1->SR & USART_FLAG_TXE));
//    USART1->DR = (c & 0x1FF);
//}
//
///**
// * This function is used by rt_kprintf to display a string on console.
// *
// * @param str the displayed string
// */
//void rt_hw_console_output(const char* str)
//{
//    while (*str)
//    {
//        rt_hw_console_putc (*str++);
//    }
//}

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

/**
 * This function will initial STM32 board.
 */
void rt_hw_board_init()
{
    /* 配置系统时钟并启动PLL,让系统工作在72M,此函数由库中提供 */
    SystemInit();

    /* 配置systick分频器 */
    /* SystemCoreClock为系统主时钟 由库中提供,在system_stm32f10x.c中 */
    /* RT_TICK_PER_SECOND 为系统节拍,由rtconfig.h中定义 */
    SysTick_Config( SystemCoreClock / RT_TICK_PER_SECOND );

//    /*  初始化串口 */
//    rt_hw_console_init();

    rt_hw_usart_init();
    rt_console_set_device("uart1");
}

/*@}*/
