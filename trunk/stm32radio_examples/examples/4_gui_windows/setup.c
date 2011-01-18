#include <rtthread.h>
#include <dfs_posix.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "setup.h"     // RADIO参数配置


#define setup_fn    "/setup.ini"

setup_TypeDef radio_setup;

static const char  kn_volume[]      = "default_volume";
static const char  kn_brightness[]  = "lcd_brightness";
static const char  kn_touch_min_x[] = "touch_min_x";
static const char  kn_touch_max_x[] = "touch_max_x";
static const char  kn_touch_min_y[] = "touch_min_y";
static const char  kn_touch_max_y[] = "touch_max_y";

void load_default(void)
{
    rt_kprintf("load_default!\r\n");
    radio_setup.default_volume = 25;
    radio_setup.lcd_brightness = 50;

    radio_setup.touch_min_x = 194;
    radio_setup.touch_max_x = 1810;
    radio_setup.touch_min_y = 147;
    radio_setup.touch_max_y = 1850;

    save_setup();
}

extern  rt_uint32_t read_line(int fd, char* line, rt_uint32_t line_size);
rt_err_t load_setup(void)
{
    int fd, length;
    char line[64];

    fd = open(setup_fn, O_RDONLY, 0);
    if (fd >= 0)
    {
        length = read_line(fd, line, sizeof(line));
        if (strcmp(line, "[config]") == 0)
        {
            char* begin;

            // default_volume
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_volume, sizeof(kn_volume) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.default_volume = atoi(begin);
            }

            // lcd_brightness
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_brightness, sizeof(kn_brightness) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.lcd_brightness = atoi(begin);
            }

            // touch_min_x
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_min_x, sizeof(kn_touch_min_x) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_min_x = atoi(begin);
            }

            // touch_max_x
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_max_x, sizeof(kn_touch_max_x) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_max_x = atoi(begin);
            }

            // touch_min_y
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_min_y, sizeof(kn_touch_min_y) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_min_y = atoi(begin);
            }

            // touch_max_y
            length = read_line(fd, line, sizeof(line));
            if (length == 0)
            {
                close(fd);
                load_default();
                return RT_EOK;
            }
            if (strncmp(line, kn_touch_max_y, sizeof(kn_touch_max_y) - 1) == 0)
            {
                begin = strchr(line, '=');
                begin++;
                radio_setup.touch_max_y = atoi(begin);
            }

        }
        else
        {
            close(fd);
            load_default();
            return RT_EOK;
        }
    }
    else
    {
        load_default();
    }

    close(fd);
    return RT_EOK;
}

rt_err_t save_setup(void)
{
    int fd, size;
    char* p_str;
    char* buf = rt_malloc(1024);

    if (buf == RT_NULL)
    {
        rt_kprintf("no memory\r\n");
        return RT_ENOMEM;
    }

    p_str = buf;

    //参数有效性检查,防止全黑或无声.
    if (radio_setup.default_volume < 5)radio_setup.default_volume = 5;
    if (radio_setup.lcd_brightness < 5)radio_setup.lcd_brightness = 5;

    fd = open(setup_fn, O_WRONLY | O_TRUNC, 0);
    if (fd >= 0)
    {
        size = sprintf(p_str, "[config]\r\n"); // [config] sprintf(p_str,"")
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_volume, radio_setup.default_volume); //default_volume
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_brightness, radio_setup.lcd_brightness); //lcd_brightness
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_min_x, radio_setup.touch_min_x); //touch_min_x
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_max_x, radio_setup.touch_max_x); //touch_max_x
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_min_y, radio_setup.touch_min_y); //touch_min_y
        p_str += size;

        size = sprintf(p_str, "%s=%d\r\n", kn_touch_max_y, radio_setup.touch_max_y); //touch_max_y
        p_str += size;
    }

    size = write(fd, buf, p_str - buf);
    if (size == (p_str - buf))
    {
        rt_kprintf("file write succeed:\r\n");
    }

    close(fd);
    rt_free(buf);

    return RT_EOK;
}

