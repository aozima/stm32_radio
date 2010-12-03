这是一个空的RT-Thread/GUI工程，它仅仅初始化了RT-Thread/GUI Server、LCD驱动、触摸屏驱动、键盘驱动。

在LCD驱动中包含了一些基本测试例程(lcd.c，通过finsh shell导出)：
hline(x1, x2, y, pixel)
绘制水平线

vline(x, y1, y2, pixel)
绘制水平线

cls()
清屏