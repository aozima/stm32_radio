这是一个空的RT-Thread/GUI工程，它仅仅初始化了RT-Thread/GUI Server、LCD驱动、触摸屏驱动、键盘驱动。

在LCD驱动中包含了一些基本测试例程(lcd.c，通过finsh shell导出)：
hline(x1, x2, y, pixel)
绘制水平线

vline(x, y1, y2, pixel)
绘制水平线

cls()
清屏
---------------------------------------------------------------------------------------------
2011-1-13 add:
这是一个基于label的Hello World(在single_panel.c实现) 初始化了LCD驱动、触摸屏驱动、键盘驱动、SPI Flash等
请确保SPI Flash里面存有资源文件,这些文件本身自带

触屏因为只能保证本机准确 所以如果触屏不准确请在finish中运行calibration()校准(逆时针点击校准点) 
触屏时注意请时间力度相对加大 也可以在finish中键入touch_t(x,y)来模式点击

此Hello World 基于VER 4的STM32 Radio 如用之前版本请在board.h的向导模式中修改相应选项

