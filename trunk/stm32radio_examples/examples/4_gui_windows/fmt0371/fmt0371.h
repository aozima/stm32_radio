#ifndef FMT0371_H_INCLUDED
#define FMT0371_H_INCLUDED

extern void ftm0371_port_init(void);
extern void ftm0371_init(void);

/*
16位(R5G6B5)
内存范围
0x02   D7:D0  X起始地址
0x03   D8:D0  Y起始地址
0x04   D7:D0  X结束地址
0x05   D8:D0  Y结束地址
*/

#endif // FMT0371_H_INCLUDED
