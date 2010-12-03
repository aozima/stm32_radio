这个是finsh shell做为telnet server的例子

默认时，finsh shell使用uart1，并且控制台输出也是uart1

telnet server（在telnet.c/.h中实现），当它获得客户端连接时(仅支持1个客户端)，它将重新设置系统中的console和finsh shell设备。

要想让finsh shell使用不同的输出端口，最关键的是把端口采用RT-Thread的设备驱动接口进行实现。