2011.6.7:
用法:  FINSH下键入 ogg("/xxx.ogg")

这个Decoder来自Tremor lowmemory 可以在下面地址SVN到 
http://svn.xiph.org/branches/lowmem-branch/Tremor/

移植主要是vorbisfile.c中的ov_callbacks的函数填充
虽然
"The C source in this package will build on any ANSI C compiler "
但是却用上了alloca,而RT-Thread也没有此接口,只能自己试验得出个所以然.

内存占用70k++,所以用上了外部SRAM,导致听着像鬼叫.

注意:
1.'_ARM_ASSEM_' is not #defined,所以asm_arm.h没用上.
2.优化级别为O1

代码注释不多,当然修改的也不多,感兴趣的童鞋自己diff ^-^
