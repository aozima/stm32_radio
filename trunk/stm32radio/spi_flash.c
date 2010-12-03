#include <stm32f10x.h>
#include "board.h"
#include "rtthread.h"
#include "spi_flash.h"

#if (SPI_FLASH_TYPE == 1)
#include "at45dbxx.h"
#endif

#if (SPI_FLASH_TYPE == 2)
#include "sst25vfxx.h"
#endif

void rt_hw_spi_flash_init(void)
{
#if (SPI_FLASH_TYPE == 1)
    at45dbxx_init();
#endif

#if (SPI_FLASH_TYPE == 2)
    sst25vfxx_init();
#endif
}
