finsh shell使用UART3的例子（console输出也使用UART3），需要在原有串口硬件模块的基础上，把RX、TX接到PB10、PB11。

要点主要有几点：
1. rtconfig.h中定义RT_USING_UART3以使能串口3
2. rtconfig.h中定义finsh device name为uart3

3. 在board.c中，需要把console的设备设置到uart3 (rt_console_set_device("uart3");)

