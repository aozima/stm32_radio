/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <rtthread.h>
#include <serial.h>
#include "board.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

extern void rt_hw_timer_handler(void);
extern void rt_hw_interrupt_thread_switch(void);

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    rt_kprintf("NMI_Handler\r\n");
    while(1);
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    rt_kprintf("MemManage_Handler\r\n");
    while(1);
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    rt_kprintf("BusFault_Handler\r\n");
    while(1);
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    rt_kprintf("UsageFault_Handler\r\n");
    while(1);
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
    rt_kprintf("SVC_Handler\r\n");
    while(1);
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
    rt_kprintf("DebugMon_Handler\r\n");
    while(1);
}

void SysTick_Handler(void)
{
    extern void rt_hw_timer_handler(void);
    rt_hw_timer_handler();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @}
  */

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
#ifdef RT_USING_DFS
    extern int SD_ProcessIRQSrc(void);

    /* enter interrupt */
    rt_interrupt_enter();

    /* Process All SDIO Interrupt Sources */
    SD_ProcessIRQSrc();

    /* leave interrupt */
    rt_interrupt_leave();
    rt_hw_interrupt_thread_switch();
#endif
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
void USART1_IRQHandler(void)
{
#ifdef RT_USING_UART1
    extern struct rt_device uart1_device;
    extern void rt_hw_serial_isr(struct rt_device *device);

    /* enter interrupt */
    rt_interrupt_enter();

    rt_hw_serial_isr(&uart1_device);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

void USART2_IRQHandler(void)
{
#ifdef RT_USING_UART2
    extern struct rt_device uart2_device;

    /* enter interrupt */
    rt_interrupt_enter();

    rt_hw_serial_isr(&uart2_device);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

void USART3_IRQHandler(void)
{
#ifdef RT_USING_UART3
    extern struct rt_device uart3_device;

    /* enter interrupt */
    rt_interrupt_enter();

    rt_hw_serial_isr(&uart3_device);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#include <stm32f10x_exti.h>
void EXTI4_IRQHandler(void)
{
#ifdef RT_USING_LWIP
    extern void rt_dm9000_isr(void);

    /* enter interrupt */
    rt_interrupt_enter();

    rt_dm9000_isr();

    /* Clear the Key Button EXTI line pending bit */
    EXTI_ClearITPendingBit(EXTI_Line4);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

/** \brief
 *
 * \param void
 * \return void
 *
 */
void DMA2_Channel2_IRQHandler(void)
{
#if CODEC_USE_SPI3
    extern void codec_dma_isr(void);

    /* enter interrupt */
    rt_interrupt_enter();

    if (DMA_GetITStatus(DMA2_IT_TC2))
    {
        /* clear DMA flag */
        DMA_ClearFlag(DMA2_FLAG_TC2 | DMA2_FLAG_TE2);

        /* transmission complete, invoke serial dma tx isr */
        codec_dma_isr();
    }

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
