#include "stm32f10x.h"

#include "board.h"
#include "touch.h"

#include <rtthread.h>
#include <rtgui/event.h>
#include <rtgui/kbddef.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>

#if (LCD_VERSION == 2)
#include "ili9325/ili9325.h"
/*
MISO PA6
MOSI PA7
CLK  PA5
CS   PC4
*/

#define   CS_0()          GPIO_ResetBits(GPIOC,GPIO_Pin_4)
#define   CS_1()          GPIO_SetBits(GPIOC,GPIO_Pin_4)

/*
7  6 - 4  3      2     1-0
s  A2-A0 MODE SER/DFR PD1-PD0
*/
#define TOUCH_MSR_Y  0x90   //读X轴坐标指令 addr:1
#define TOUCH_MSR_X  0xD0   //读Y轴坐标指令 addr:3

struct rtgui_touch_device
{
    struct rt_device parent;

    rt_timer_t poll_timer;
    rt_uint16_t x, y;

    rt_bool_t calibrating;
    rt_touch_calibration_func_t calibration_func;

    rt_uint16_t min_x, max_x;
    rt_uint16_t min_y, max_y;
};
static struct rtgui_touch_device *touch = RT_NULL;

extern unsigned char SPI_WriteByte(unsigned char data);
rt_inline void EXTI_Enable(rt_uint32_t enable);

rt_inline uint8_t SPI_WriteByte(unsigned char data)
{
	//Wait until the transmit buffer is empty
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	// Send the byte
	SPI_I2S_SendData(SPI1, data);

	//Wait until a data is received
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	// Get the received data
	data = SPI_I2S_ReceiveData(SPI1);

	// Return the shifted data
	return data;
}

//SPI写数据
static void WriteDataTo7843(unsigned char num)
{
    SPI_WriteByte(num);
}

#define X_WIDTH 240
#define Y_WIDTH 320

static void rtgui_touch_calculate()
{
    if (touch != RT_NULL)
    {
        unsigned int touch_hw_tmp_x[10];
        unsigned int touch_hw_tmp_y[10];
        unsigned int i;

        CS_0();
        for(i=0; i<10; i++)
        {
            WriteDataTo7843(TOUCH_MSR_X);                                    /* read X */
            touch_hw_tmp_x[i] = SPI_WriteByte(0x00)<<4;                      /* read MSB bit[11:8] */
            touch_hw_tmp_x[i] |= ((SPI_WriteByte(TOUCH_MSR_Y)>>4)&0x0F );    /* read LSB bit[7:0] */
            touch_hw_tmp_y[i] = SPI_WriteByte(0x00)<<4;                      /* read MSB bit[11:8] */
            touch_hw_tmp_y[i] |= ((SPI_WriteByte(0x00)>>4)&0x0F );           /* read LSB bit[7:0] */
        }
        WriteDataTo7843( 1<<7 ); /* 打开中断 */
        CS_1();


        {
            unsigned int temp_x = 0;
            unsigned int temp_y = 0;
            unsigned int max_x = 0;
            unsigned int min_x = 0xffff;
            unsigned int max_y = 0;
            unsigned int min_y = 0xffff;

            for(i=0; i<10; i++)
            {
                temp_x += touch_hw_tmp_x[i];
                temp_y += touch_hw_tmp_y[i];
                if(touch_hw_tmp_x[i] > max_x) max_x = touch_hw_tmp_x[i];
                if(touch_hw_tmp_x[i] < min_x) min_x = touch_hw_tmp_x[i];
                if(touch_hw_tmp_y[i] > max_y) max_y = touch_hw_tmp_y[i];
                if(touch_hw_tmp_y[i] < min_y) min_y = touch_hw_tmp_y[i];
            }
            touch->x = (temp_x-max_x-min_x) / 8;
            touch->y = (temp_y-max_y-min_y) / 8;
        }

        /* if it's not in calibration status  */
        if (touch->calibrating != RT_TRUE)
        {
            if (touch->max_x > touch->min_x)
            {
                touch->x = (touch->x - touch->min_x) * X_WIDTH/(touch->max_x - touch->min_x);
            }
            else
            {
                touch->x = (touch->min_x - touch->x) * X_WIDTH/(touch->min_x - touch->max_x);
            }

            if (touch->max_y > touch->min_y)
            {
                touch->y = (touch->y - touch->min_y) * Y_WIDTH /(touch->max_y - touch->min_y);
            }
            else
            {
                touch->y = (touch->min_y - touch->y) * Y_WIDTH /(touch->min_y - touch->max_y);
            }
        }
    }
}

static unsigned int flag = 0;
void touch_timeout(void* parameter)
{
    struct rtgui_event_mouse emouse;

    if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) != 0)
    {
        EXTI_Enable(1);
        emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
        emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_UP);

        /* use old value */
        emouse.x = touch->x;
        emouse.y = touch->y;

        /* stop timer */
        rt_timer_stop(touch->poll_timer);
        rt_kprintf("touch up: (%d, %d)\n", emouse.x, emouse.y);
        flag = 0;

        if ((touch->calibrating == RT_TRUE) && (touch->calibration_func != RT_NULL))
        {
            /* callback function */
            touch->calibration_func(emouse.x, emouse.y);
        }
    }
    else
    {
        if(flag == 0)
        {
            /* calculation */
            rtgui_touch_calculate();

            /* send mouse event */
            emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
            emouse.parent.sender = RT_NULL;

            emouse.x = touch->x;
            emouse.y = touch->y;

            /* init mouse button */
            emouse.button = (RTGUI_MOUSE_BUTTON_LEFT |RTGUI_MOUSE_BUTTON_DOWN);

            rt_kprintf("touch down: (%d, %d)\n", emouse.x, emouse.y);
            flag = 1;
        }
        else
        {
            /* send mouse event */
            emouse.parent.type = RTGUI_EVENT_MOUSE_MOTION;
            emouse.parent.sender = RT_NULL;

            /* calculation */
            rtgui_touch_calculate();

            emouse.x = touch->x;
            emouse.y = touch->y;

            /* init mouse button */
            emouse.button = 0;
            rt_kprintf("touch motion: (%d, %d)\n", emouse.x, emouse.y);

        }
    }

    /* send event to server */
    if (touch->calibrating != RT_TRUE)
        rtgui_server_post_event(&emouse.parent, sizeof(struct rtgui_event_mouse));
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

rt_inline void EXTI_Enable(rt_uint32_t enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Configure  EXTI  */
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升

    if (enable)
    {
        /* enable */
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    }
    else
    {
        /* disable */
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    }

    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line1);
}

static void EXTI_Configuration(void)
{
    /* PB1 touch INT */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(GPIOB,&GPIO_InitStructure);
    }

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);

    /* Configure  EXTI  */
    EXTI_Enable(1);
}

/* RT-Thread Device Interface */
static rt_err_t rtgui_touch_init (rt_device_t dev)
{
    NVIC_Configuration();
    EXTI_Configuration();

    /* PC4 touch CS */
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
        GPIO_Init(GPIOC,&GPIO_InitStructure);
        CS_1();
    }

    CS_0();
    WriteDataTo7843( 1<<7 ); /* 打开中断 */
    CS_1();

    return RT_EOK;
}

static rt_err_t rtgui_touch_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
    switch (cmd)
    {
    case RT_TOUCH_CALIBRATION:
        touch->calibrating = RT_TRUE;
        touch->calibration_func = (rt_touch_calibration_func_t)args;
        break;

    case RT_TOUCH_NORMAL:
        touch->calibrating = RT_FALSE;
        break;

    case RT_TOUCH_CALIBRATION_DATA:
    {
        struct calibration_data* data;

        data = (struct calibration_data*) args;

        //update
        touch->min_x = data->min_x;
        touch->max_x = data->max_x;
        touch->min_y = data->min_y;
        touch->max_y = data->max_y;
    }
    break;
    }

    return RT_EOK;
}

void EXTI1_IRQHandler(void)
{
    /* disable interrupt */
    EXTI_Enable(0);

    /* start timer */
    rt_timer_start(touch->poll_timer);

    EXTI_ClearITPendingBit(EXTI_Line1);
}
#endif

void rtgui_touch_hw_init(void)
{
#if (LCD_VERSION == 2)
    touch = (struct rtgui_touch_device*)rt_malloc (sizeof(struct rtgui_touch_device));
    if (touch == RT_NULL) return; /* no memory yet */

    /* clear device structure */
    rt_memset(&(touch->parent), 0, sizeof(struct rt_device));
    touch->calibrating = FALSE;
    touch->min_x = 0;
    touch->max_x = 240;
    touch->min_y = 0;
    touch->max_y = 320;

    /* init device structure */
    touch->parent.type = RT_Device_Class_Unknown;
    touch->parent.init = rtgui_touch_init;
    touch->parent.control = rtgui_touch_control;
    touch->parent.private = RT_NULL;

    /* create 1/8 second timer */
    touch->poll_timer = rt_timer_create("touch", touch_timeout, RT_NULL,
                                        RT_TICK_PER_SECOND/8, RT_TIMER_FLAG_PERIODIC);

    /* register touch device to RT-Thread */
    rt_device_register(&(touch->parent), "touch", RT_DEVICE_FLAG_RDWR);
#endif
}
