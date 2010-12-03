#include <rtthread.h>
#include <finsh.h>
#include "board.h"

#define BENCHMARK_SIZE	(1024 * 8)
#define BENCHMARK_LOOP	0xfff

rt_uint8_t benchmark_buffer[BENCHMARK_SIZE];

void benchmark()
{
	rt_uint32_t tick;
	rt_uint32_t index, loop;

	/* benchmark for 8bit access */
	{
		rt_uint8_t *ptr;

		ptr = (rt_uint8_t*) benchmark_buffer;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE; index ++)
			{
				*ptr = (index & 0xff);
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("internal sram[8bit]: %d\n", tick);

		ptr = (rt_uint8_t*) STM32_EXT_SRAM_BEGIN;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE; index ++)
			{
				*ptr = (index & 0xff);
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("external sram[8bit]: %d\n", tick);
	}

	/* benchmark for 16bit access */
	{
		rt_uint16_t *ptr;

		ptr = (rt_uint16_t*) benchmark_buffer;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP/2; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE; index ++)
			{
				*ptr = (index & 0xffff);
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("internal sram[16bit]: %d\n", tick);

		ptr = (rt_uint16_t*) STM32_EXT_SRAM_BEGIN;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE/2; index ++)
			{
				*ptr = (index & 0xffff);
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("external sram[16bit]: %d\n", tick);
	}

	/* benchmark for 32bit access */
	{
		rt_uint32_t *ptr;

		ptr = (rt_uint32_t*) benchmark_buffer;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE/4; index ++)
			{
				*ptr = index;
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("internal sram[32bit]: %d\n", tick);

		ptr = (rt_uint32_t*) STM32_EXT_SRAM_BEGIN;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			for (index = 0; index < BENCHMARK_SIZE/4; index ++)
			{
				*ptr = index;
			}
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("external sram[32bit]: %d\n", tick);
	}

	/* benchmark for memset access */
	{
		rt_uint32_t *ptr;

		ptr = (rt_uint32_t*) benchmark_buffer;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			memset(ptr, 0xae, BENCHMARK_SIZE);
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("internal sram[memset]: %d\n", tick);

		ptr = (rt_uint32_t*) STM32_EXT_SRAM_BEGIN;
		tick = rt_tick_get();
		for (loop = 0; loop < BENCHMARK_LOOP; loop ++)
		{
			memset(ptr, 0xae, BENCHMARK_SIZE);
		}
		tick = rt_tick_get() - tick;
		rt_kprintf("external sram[memset]: %d\n", tick);
	}
}
FINSH_FUNCTION_EXPORT(benchmark, an internal and external SRAM benchmark);
