这是一个空的RT-Thread/GUI工程，它仅仅初始化了RT-Thread/GUI Server、LCD驱动、触摸屏驱动、键盘驱动。

在LCD驱动中包含了一些基本测试例程(lcd.c，通过finsh shell导出)：
hline(x1, x2, y, pixel)
绘制水平线

vline(x, y1, y2, pixel)
绘制水平线

cls()
清屏
---------------------------------------------------------------------------------------------
2011-1-12 add:
这是一个0.4版的GUI Demo 初始化了RT-Thread/GUI Server、LCD驱动、触摸屏驱动、键盘驱动
新增了MENU、List、item等
请确保SPI Flash里面存有资源文件,这些文件本身自带

触屏因为只能保证本机准确 所以如果触屏不准确请在finish中运行calibration()校准(逆时针点击校准点) 
触屏时注意请时间力度相对加大 也可以在finish中键入touch_t(x,y)来模式点击,例如touch_t(179,305)就是点击"下一个"按钮

同时可以通过键盘左右键来实现切换

此GUI Demo基于VER 4的STM32 Radio 如用之前版本请在board.h的向导模式中修改相应选项

