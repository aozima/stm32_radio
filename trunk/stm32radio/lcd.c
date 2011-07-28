#include "stm32f10x.h"
#include "rtthread.h"
#include "board.h"

void lcd_backlight_init(void);
void brightness_set(unsigned int value);

struct rt_device _lcd_device;
static rt_err_t lcd_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t lcd_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch (cmd)
	{
	case RTGRAPHIC_CTRL_GET_INFO:
		{
			struct rt_device_graphic_info *info;

			info = (struct rt_device_graphic_info*) args;
			RT_ASSERT(info != RT_NULL);

			info->bits_per_pixel = 16;
			info->pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565P;
			info->framebuffer = RT_NULL;
			info->width = 240;
			info->height = 320;
		}
		break;

	case RTGRAPHIC_CTRL_RECT_UPDATE:
		/* nothong to be done */
		break;

	default:
		break;
	}

	return RT_EOK;
}

#include "fmt0371/FMT0371.h"
#include "ili_lcd_general.h"
#include "ssd1289.h"

extern struct rt_device_graphic_ops lcd_fmt_ops;
extern struct rt_device_graphic_ops lcd_ili_ops;
extern struct rt_device_graphic_ops ssd1289_ops;

void rt_hw_lcd_init(void)
{

	/* register lcd device */
	_lcd_device.type  = RT_Device_Class_Graphic;
	_lcd_device.init  = lcd_init;
	_lcd_device.open  = lcd_open;
	_lcd_device.close = lcd_close;
	_lcd_device.control = lcd_control;
	_lcd_device.read  = RT_NULL;
	_lcd_device.write = RT_NULL;

	/* set user privated data */
#if LCD_VERSION == 1
	_lcd_device.user_data = &lcd_fmt_ops;
    fmt_lcd_init();
#elif LCD_VERSION == 2
	_lcd_device.user_data = &lcd_ili_ops;
    lcd_Initializtion();
#elif LCD_VERSION == 3
	_lcd_device.user_data = &ssd1289_ops;
    ssd1289_init();
#endif

    /* register graphic device driver */
	rt_device_register(&_lcd_device, "lcd",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

    lcd_backlight_init();
}

#if ( LCD_USE_PWM == 1) || ( LCD_USE_PWM == 2 )
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef  TIM_OCInitStructure;
#define ARR  12000
#endif

void lcd_backlight_init(void)
{
    /* for old version */
    GPIO_InitTypeDef GPIO_InitStructure;
#if ( LCD_USE_PWM == 0)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOF,&GPIO_InitStructure);
    GPIO_SetBits(GPIOF,GPIO_Pin_9);
    /* for old version */
#endif

#if ( LCD_USE_PWM == 1)
    /* new version: use pwm in PB9 TIM4_CH4 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIOB Configuration:TIM4 Channel4 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_9;//GPIO_Pin_8 |
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = ARR;//12000
    TIM_TimeBaseStructure.TIM_Prescaler = 2;//预分频2,频率36M
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    /* PWM1 Mode configuration: Channel4 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*50; /* 设置初始背光亮度 */
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);

    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
#elif ( LCD_USE_PWM == 2 )
    /* new version: use pwm in PB6 TIM4_CH1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    /* TIM4 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIOB Configuration:TIM4 Channel4 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_6;//GPIO_Pin_8 |
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = ARR;//12000
    TIM_TimeBaseStructure.TIM_Prescaler = 2;//预分频2,频率36M
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    /* PWM1 Mode configuration: Channel4 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*50; /* 设置初始背光亮度 */
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
#endif
}

void brightness_set(unsigned int value)
{
#if ( LCD_USE_PWM == 0)
    if(value)
    {
        GPIO_SetBits(GPIOF,GPIO_Pin_9);
    }
    else
    {
        GPIO_ResetBits(GPIOF,GPIO_Pin_9);
    }
#endif

#if ( LCD_USE_PWM == 1 )
    if(value>100)value=50;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*value;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
#elif ( LCD_USE_PWM == 2 )
    if(value>100)value=50;
    TIM_OCInitStructure.TIM_Pulse = (ARR/100)*value;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
#endif
}
