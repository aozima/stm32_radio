这是一个包含了finsh shell的工程例子。

在application.c文件中添加了两个函数到finsh shell上：
- fun_a
- int_is

在系统执行起来后，在finsh shell的命令行提示符
finsh>>

上输入fun_a()或int_is(10)即可调用相应的函数。

Note
----
void int_is(int a)
{
	rt_kprintf("int is: %d\n", a);
}
FINSH_FUNCTION_EXPORT(int_is, show a integer);

上面的语句中，FINSH_FUNCTION_EXPORT是关键的地方，第一个参数是函数名。这条宏定义在finsh.h头文件中定义。

在finsh shell中能够支持采用参数的方式调用系统函数，当前支持一些基本类型，例如int, long, char, char*以及其他指针。

在以后的一些例子中，会把一些例程函数直接输出到finsh shell中，当要运行时（或采用不同参数运行时），将在finsh shell中直接调用这些函数。

Note Driver
------------
finsh shell依赖于一定的设备，关键地方在rtthread_startup函数的
#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init();
	finsh_set_device("uart1");
#endif

即这里的finsh_set_device函数，这里需要设置finsh shell用到的设备，这个例程是串口1设备。这个设备仅仅和finsh shell的输入设备有关系，
与输出无关（在RT-Thread系统中，统一使用rt_kprintf函数进行输出）
