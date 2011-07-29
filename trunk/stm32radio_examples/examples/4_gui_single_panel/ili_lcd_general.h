#ifndef ILI_LCD_GENERAL_H_INCLUDED
#define ILI_LCD_GENERAL_H_INCLUDED

/*
 Compatible list:
 ili9320 ili9325 ili9328
 LG4531
*/

/* LCD color */
#define White            0xFFFF
#define Black            0x0000
#define Grey             0xF7DE
#define Blue             0x001F
#define Blue2            0x051F
#define Red              0xF800
#define Magenta          0xF81F
#define Green            0x07E0
#define Cyan             0x7FFF
#define Yellow           0xFFE0

/*---------------------- Graphic LCD size definitions ------------------------*/
#define LCD_WIDTH       240                 /* Screen Width (in pixels)           */
#define LCD_HEIGHT      320                 /* Screen Hight (in pixels)           */
#define BPP             16                  /* Bits per pixel                     */
#define BYPP            ((BPP+7)/8)         /* Bytes per pixel                    */

extern void lcd_Initializtion(void);
extern unsigned int lcd_getdeviceid(void);

//#define _ILI_REVERSE_DIRECTION_

#include "rtthread.h"
#include <rtgui/rtgui.h>
#include <rtgui/driver.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
extern void rt_hw_lcd_set_pixel(const char* pixel, int x, int y);
extern void rt_hw_lcd_get_pixel(char* pixel, int x, int y);
extern void rt_hw_lcd_draw_hline(const char* pixel, int x1, int x2, int y);
extern void rt_hw_lcd_draw_vline(const char* pixel, int x, int y1, int y2);
extern void rt_hw_lcd_draw_blit_line(const char* pixels, int x, int y, rt_size_t size);

#endif // ILI_LCD_GENERAL_H_INCLUDED
