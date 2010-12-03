/*
 * File      : led.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rtthread.h>
#include <stm32f10x.h>

#ifdef STM32_SIMULATOR
#define led_rcc                    RCC_APB2Periph_GPIOA
#define led_gpio                   GPIOA
#define led_pin                    (GPIO_Pin_5)
#else
#define led_rcc                    RCC_APB2Periph_GPIOE
#define led_gpio                   GPIOE
#define led_pin                    (GPIO_Pin_2)
#endif

static rt_uint8_t led_inited = 0;
void rt_hw_led_init(void)
{
	if (led_inited == 0)
	{
	    GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd(led_rcc, ENABLE);

		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_InitStructure.GPIO_Pin   = led_pin;
		GPIO_Init(led_gpio, &GPIO_InitStructure);

		led_inited = 1;
	}
}

void rt_hw_led_on(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        GPIO_SetBits(led_gpio, led_pin);
        break;
    default:
        break;
    }
}

void rt_hw_led_off(rt_uint32_t n)
{
    switch (n)
    {
    case 0:
        GPIO_ResetBits(led_gpio, led_pin);
        break;
    default:
        break;
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void led(rt_uint32_t led, rt_uint32_t value)
{
    /* init led configuration if it's not inited. */
    if (!led_inited)
        rt_hw_led_init();

    if ( led == 0 )
    {
        /* set led status */
        switch (value)
        {
        case 0:
            rt_hw_led_off(0);
            break;
        case 1:
            rt_hw_led_on(0);
            break;
        default:
            break;
        }
    }
}
FINSH_FUNCTION_EXPORT(led, set led on(1) or off(0))
#endif
