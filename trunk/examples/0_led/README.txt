这是一个简单的点灯程序。

入口地方在application.c文件的rt_application_init函数中，在这个函数中创建了一个动态线程。

这个线程的入口在rt_thread_entry_led1函数：
这个函数是一个简单的while(1)循环，
先点亮灯，
然后过50个OS时钟节拍后熄灭
再过50个OS时钟节拍后再点亮
。。。

Note Appliation: 
- 点灯是在单独的线程中执行；

Note Drivers:
- 点灯调用的是rt_hw_led_on/rt_hw_led_off函数，参数1是灯的序号，因为Radio上只有一个灯，所以序号是0
- rt_kprintf是往串口输出信息，它依赖于console设备。

Note System:
- RT-Thread系统的入口是startup.c文件中的main函数；
- 在main函数中，因为默认Keil MDK环境下中断是开启的，所以第一件事是关闭系统总中断。
- 系统的总中断会在第一个线程被调度执行时自动打开（因为机器状态字在栈的初始化时被置上打开中断）
- main函数中调用真正的RT-Thread系统入口函数：rtthread_startup
- rtthread_startup函数主要分为几个部分：
  * rt_hw_board_init硬件相关的初始化；
  * rt_system_heap_init系统堆初始化，会用于系统的动态内存分配；
  * rt_application_init用户应用初始化；
  * rt_system_scheduler_start启动系统调度器。
