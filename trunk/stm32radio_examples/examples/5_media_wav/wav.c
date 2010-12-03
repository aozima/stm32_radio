#include <finsh.h>
#include <dfs_posix.h>
#include "stm32f10x.h"

//定义mempool块大小.
#define  mempll_block_size      16384
//我们共申请两块mempool,并留出4字节做为控制块.
static rt_uint8_t mempool[ (mempll_block_size+4) *2];
static struct rt_mempool _mp;
//内存池初始化标识
static rt_bool_t is_inited = RT_FALSE;

static rt_err_t wav_tx_done(rt_device_t dev, void *buffer)
{
    /* release memory block */
    rt_mp_free(buffer);

    return RT_EOK;
}

void wav(char* filename)
{
    int fd;

    //检查mempool是否被初始化,否则进行初始化.
    if (is_inited == RT_FALSE)
    {
        rt_mp_init(&_mp, "wav_buf", &mempool[0], sizeof(mempool), mempll_block_size);
        is_inited = RT_TRUE;
    }


    //打开文件
    fd = open(filename, O_RDONLY, 0);
    if (fd >= 0)
    {
        rt_uint8_t* buf;
        rt_size_t 	len;
        rt_device_t device;

        /* open audio device and set tx done call back */
        device = rt_device_find("snd");
        //设置发送完成回调函数,让DAC数据发完时执行wav_tx_done函数释放空间.
        rt_device_set_tx_complete(device, wav_tx_done);
        rt_device_open(device, RT_DEVICE_OFLAG_WRONLY);

        do
        {
            //向mempoll申请空间,如果申请不成功则一直在此等待.
            buf = rt_mp_alloc(&_mp, RT_WAITING_FOREVER);
            //从文件读取数据
            len = read(fd, (char*)buf, mempll_block_size);
            //读取成功就把数据写入设备
            if (len > 0)
            {
                rt_device_write(device, 0, buf, len);
            }
            //否则释放刚才申请的空间,正常情况下是读到文件尾时.
            else
            {
                rt_mp_free(buf);
            }
        }
        while (len != 0);

        /* close device and file */
        rt_device_close(device);
        close(fd);
    }
}
FINSH_FUNCTION_EXPORT(wav, wav test. e.g: wav("/test.wav"))

