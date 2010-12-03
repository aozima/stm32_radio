#include <rtthread.h>
#include <stm32f10x.h>

int rt_application_init()
{
    return 0;
}

#include <finsh.h>
void fun_a(void)
{
    rt_kprintf("fun_a done.\r\n");
}
FINSH_FUNCTION_EXPORT(fun_a, fun_a desc);

void int_is(int a)
{
	rt_kprintf("int is: %d\n", a);
}
FINSH_FUNCTION_EXPORT(int_is, show a integer);

/*@}*/
