这是一个文件系统使用SD卡的例程

SD的驱动在sdcard.c/.h中实现，要实现一个块设备类型的驱动，需要实现相应的五个接口：
static rt_err_t rt_sdcard_init(rt_device_t dev);
static rt_err_t rt_sdcard_open(rt_device_t dev, rt_uint16_t oflag);
static rt_err_t rt_sdcard_close(rt_device_t dev);
static rt_size_t rt_sdcard_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size);
static rt_size_t rt_sdcard_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size);
static rt_err_t rt_sdcard_control(rt_device_t dev, rt_uint8_t cmd, void *args);

当一个新的块设备驱动被实现时，RT-Thread/Device Filesystem就可以在上面使用自己的文件系统了，
而不用关心底层是什么具体硬件(这个在SPI flash上也能够明显体现出来)。

文件系统
--------
文件系统在application.c的初始化线程中进行装载：
/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
    /* Filesystem Initialization */
#ifdef RT_USING_DFS
    {
        /* init the device filesystem */
        dfs_init();

        /* init the elm FAT filesystam*/
        elm_init();

        /* mount sd card fat partition 1 as root directory */
        if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
            rt_kprintf("File System initialized!\n");
        else
            rt_kprintf("File System init failed!\n");
    }
#endif
}

dfs_init()和elm_init()分别是device filesystem和elm fatfs文件系统的初始化，当它们初始化完毕后就可以装载文件系统：
dfs_mount("sd0", "/", "elm", 0, 0)

第一个参数是设备名，对于elm fatfs文件系统而言，这个设备一定要存在否则不能够装载成功。
第二个参数是装载的目录，这里是根目录，如果要装载到不同的目录，那么必须要确保这个目录在现在的文件系统上已经存在。
后面两个参数可忽略不计。

-----------
最简单的测试方法:
finsh>>ls("/")
更多文件系统相关命令请在finsh中输入 list()
