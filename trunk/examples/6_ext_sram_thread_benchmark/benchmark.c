#include <rtthread.h>
#include <finsh.h>
#include "board.h"

#define BENCHMARK_SIZE	(1024 * 8)
#define BENCHMARK_LOOP	0xfff

rt_uint8_t benchmark_buffer[BENCHMARK_SIZE];

struct rt_thread benchmark_thread;
rt_uint8_t benchmark_thread_stack[512];
struct rt_semaphore ack;

void benchmark_thread_entry(void* parameter)
{
	rt_uint32_t tick;
	rt_uint32_t index, loop;

	/* benchmark for 8bit access */
	{
		rt_uint8_t *ptr;

		ptr = (rt_uint8_t*) parameter;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE; index ++)
			{
				*ptr = (index & 0xff);
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("sram[8bit]: %d\n", tick);
	}

	/* benchmark for 16bit access */
	{
		rt_uint16_t *ptr;

		ptr = (rt_uint16_t*) parameter;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE/2; index ++)
			{
				*ptr = (index & 0xffff);
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("sram[16bit]: %d\n", tick);
	}

	/* benchmark for 32bit access */
	{
		rt_uint32_t *ptr;

		ptr = (rt_uint32_t*) parameter;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE/4; index ++)
			{
				*ptr = index;
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("sram[32bit]: %d\n", tick);
	}

	/* benchmark for memset access */
	{
		rt_uint32_t *ptr;

		ptr = (rt_uint32_t*) parameter;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			memset(ptr, 0xae, BENCHMARK_SIZE);
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("external sram[memset]: %d\n", tick);
	}

	rt_sem_release(&ack);
}

void benchmark()
{
	rt_err_t result;
	rt_thread_t tid;
	rt_uint8_t* ptr;

	result = rt_sem_init(&ack, "ack", 0, RT_IPC_FLAG_FIFO);
	if (result != RT_EOK)
	{
		rt_kprintf("init ack semaphore failed\n");
		return;
	}

	rt_kprintf("internal SRAM benchmark\n");
	result = rt_thread_init(&benchmark_thread,
		"t1",
		benchmark_thread_entry, benchmark_buffer,
		&benchmark_thread_stack[0], sizeof(benchmark_thread_stack),
		20, 10);

	if (result == RT_EOK)
		rt_thread_startup(&benchmark_thread);

	/* wait thread complete */
	rt_sem_take(&ack, RT_WAITING_FOREVER);

	rt_thread_detach(&benchmark_thread);

	rt_kprintf("external SRAM benchmark\n");
	ptr = rt_malloc(BENCHMARK_SIZE);
	/* create benchmark thread */
	tid = rt_thread_create("t2",
		benchmark_thread_entry, ptr,
		512, 20, 10);
	if (tid != RT_NULL)
		rt_thread_startup(tid);

	/* wait thread complete */
	rt_sem_take(&ack, RT_WAITING_FOREVER);

	/* release memory */
	rt_free(ptr);

	/* detach semaphore */
	rt_sem_detach(&ack);
}
FINSH_FUNCTION_EXPORT(benchmark, an internal and external SRAM benchmark through thread);
