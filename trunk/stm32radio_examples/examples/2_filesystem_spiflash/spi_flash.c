#include <stm32f10x.h>
#include "board.h"
#include "rtthread.h"
#include "spi_flash.h"

static uint8_t SPI_WriteByte(unsigned char data)
{
    //Wait until the transmit buffer is empty
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    // Send the byte
    SPI_I2S_SendData(SPI1, data);

    //Wait until a data is received
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    // Get the received data
    data = SPI_I2S_ReceiveData(SPI1);

    // Return the shifted data
    return data;
}

/********************** hardware *************************************/
/* SPI_FLASH_CS   PA4 */
/* SPI_FLASH_RST  PA3 */
#define FLASH_RST_0()    GPIO_ResetBits(GPIOA,GPIO_Pin_3)
#define FLASH_RST_1()    GPIO_SetBits(GPIOA,GPIO_Pin_3)

#define FLASH_CS_0()     GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define FLASH_CS_1()     GPIO_SetBits(GPIOA,GPIO_Pin_4)
/********************** hardware *************************************/

#if SPI_FLASH_USE_DMA
static uint8_t dummy = 0;
static uint8_t _spi_flash_buffer[ FLASH_SECTOR_SIZE ];
#endif

#if SST_WRITE_BUFFER
static rt_uint8_t buffer_use_flag = 0;//BUFFER是否已经使用的标致.
#endif

struct _flash_buffer
{
    rt_uint8_t   swap_buffer[4096];  // 交换区.
#if SST_WRITE_BUFFER
    rt_uint8_t   write_buffer[4096]; // 写缓冲区.
    rt_uint32_t  block_addr;         // 缓冲区的目标地址.
    rt_uint8_t   sector_flag[8];     // 缓冲区内各区间的使用情况.
#endif
};
struct _flash_buffer * flash_buffer = RT_NULL;

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    FLASH_RST_0(); // RESET
    FLASH_CS_1();
    FLASH_RST_1();
}

#if SPI_FLASH_USE_DMA
static void DMA_RxConfiguration(rt_uint32_t addr, rt_size_t size)
{
    DMA_InitTypeDef DMA_InitStructure;

    DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_TE2 | DMA1_FLAG_TC3 | DMA1_FLAG_TE3);

    /* DMA Channel configuration ----------------------------------------------*/
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(SPI1->DR));
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32) addr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel2, ENABLE);

    /* Dummy TX channel configuration */
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(SPI1->DR));
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(&dummy);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel3, ENABLE);
}
#endif

static uint8_t SPI_HostReadByte(void)
{
    //return SPI_WriteByte(0x00);
    //Wait until the transmit buffer is empty
    //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    while( (SPI1->SR & SPI_I2S_FLAG_TXE) == RESET);
    // Send the byte
    SPI_I2S_SendData(SPI1, 0);

    //Wait until a data is received
    //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    while( (SPI1->SR & SPI_I2S_FLAG_RXNE) == RESET);
    // Get the received data
    return SPI_I2S_ReceiveData(SPI1);

}

static void SPI_HostWriteByte(uint8_t wByte)
{
    SPI_WriteByte(wByte);
}

/*****************************************************************************/
/*Status Register Format:                                   */
/* ------------------------------------------------------------------------- */
/* | bit7   | bit6   | bit5   | bit4   | bit3   | bit2   | bit1   | bit0   | */
/* |--------|--------|--------|--------|--------|--------|--------|--------| */
/* |RDY/BUSY| COMP   |         device density            |   X    |   X    | */
/* ------------------------------------------------------------------------- */
/* 0:busy   |        |        AT45DB041:0111             | protect|page size */
/* 1:ready  |        |        AT45DB161:1011             |                   */
/* --------------------------------------------------------------------------*/
/*****************************************************************************/
static uint8_t AT45DB_StatusRegisterRead(void)
{
    uint8_t i;

    FLASH_CS_0();
    SPI_HostWriteByte(AT45DB_READ_STATE_REGISTER);
    i = SPI_HostReadByte();
    FLASH_CS_1();

    return i;
}

static void wait_busy(void)
{
    uint16_t i = 0;
    while (i++ < 10000)
    {
        if (AT45DB_StatusRegisterRead() & 0x80)
        {
            return;
        }
    }
    rt_kprintf("\r\nSPI_FLASH timeout!!!\r\n");
}

static void read_page(uint32_t page, uint8_t *pHeader)
{
#if SPI_FLASH_USE_DMA
    rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
    /* SPI1 configure */
    rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

    DMA_RxConfiguration((rt_uint32_t) pHeader, FLASH_SECTOR_SIZE);

    FLASH_CS_0();

    SPI_HostWriteByte(AT45DB_MM_PAGE_READ);
    SPI_HostWriteByte((uint8_t)(page >> 6));
    SPI_HostWriteByte((uint8_t)(page << 2));
    SPI_HostWriteByte(0x00);

    // 4 don't care bytes
    SPI_HostWriteByte(0x00);
    SPI_HostWriteByte(0x00);
    SPI_HostWriteByte(0x00);
    SPI_HostWriteByte(0x00);

    SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_RXNE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);
    while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);

    FLASH_CS_1();

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);

    rt_sem_release(&spi1_lock);
#else
    uint16_t i;

    rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
    /* SPI1 configure */
    rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

    FLASH_CS_0();

    SPI_HostWriteByte(AT45DB_MM_PAGE_READ);
    SPI_HostWriteByte((uint8_t)(page >> 6));
    SPI_HostWriteByte((uint8_t)(page << 2));
    SPI_HostWriteByte(0x00);

    // 4 don't care bytes
    SPI_HostWriteByte(0x00);
    SPI_HostWriteByte(0x00);
    SPI_HostWriteByte(0x00);
    SPI_HostWriteByte(0x00);

    for (i = 0; i < FLASH_SECTOR_SIZE; i++)
    {
        *pHeader++ = SPI_HostReadByte();
    }

    FLASH_CS_1();

    rt_sem_release(&spi1_lock);
#endif
}

static void write_page(uint32_t page, uint8_t *pHeader)
{
    uint16_t i;

    rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
    /* SPI1 configure */
    rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

    FLASH_CS_0();

    SPI_HostWriteByte(AT45DB_MM_PAGE_PROG_THRU_BUFFER1);
    SPI_HostWriteByte((uint8_t) (page >> 6));
    SPI_HostWriteByte((uint8_t) (page << 2));
    SPI_HostWriteByte(0x00);

    for (i = 0; i < FLASH_SECTOR_SIZE; i++)
    {
        SPI_HostWriteByte(*pHeader++);
    }

    FLASH_CS_1();

    wait_busy();

    rt_sem_release(&spi1_lock);
}


#include <rtthread.h>
/* SPI DEVICE */
static struct rt_device spi_flash_device;

/* RT-Thread Device Driver Interface */
static rt_err_t AT45DB_flash_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t AT45DB_flash_open(rt_device_t dev, rt_uint16_t oflag)
{

    return RT_EOK;
}

static rt_err_t AT45DB_flash_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t AT45DB_flash_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *)args;
        if (geometry == RT_NULL) return -RT_ERROR;

        geometry->bytes_per_sector = 512;
        geometry->sector_count = 4096;
        geometry->block_size = 4096; /* block erase: 4k */
    }

    return RT_EOK;
}

static rt_size_t AT45DB_flash_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_uint32_t index, nr;

    nr = size / FLASH_SECTOR_SIZE;

    for (index = 0; index < nr; index++)
    {
        /* only supply single block read: block size 512Byte */
#if SPI_FLASH_USE_DMA
        uint16_t *sp, *dp, *end;

        read_page((pos / FLASH_SECTOR_SIZE + index), _spi_flash_buffer);
//    	rt_memcpy(((rt_uint8_t *) buffer + index * FLASH_SECTOR_SIZE), _spi_flash_buffer, FLASH_SECTOR_SIZE);
        sp = (uint16_t *) _spi_flash_buffer;
        dp = (uint16_t *) buffer;
        end = sp + FLASH_SECTOR_SIZE / 2;
        while (sp < end)
        {
            *dp++ = *sp++;
        }
#else
        read_page((pos / FLASH_SECTOR_SIZE + index), ((rt_uint8_t *) buffer + index * FLASH_SECTOR_SIZE));
#endif
    }

    return nr * FLASH_SECTOR_SIZE;
}

static rt_size_t AT45DB_flash_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    rt_uint32_t index, nr;

    nr = size / FLASH_SECTOR_SIZE;

    for (index = 0; index < nr; index++)
    {
        /* only supply single block write: block size 512Byte */
        write_page((pos / FLASH_SECTOR_SIZE + index), ((rt_uint8_t *) buffer + index * FLASH_SECTOR_SIZE));
    }

    return nr * FLASH_SECTOR_SIZE;
}

/******** SST25VF **************/
static uint8_t sst25vf_read_status(void)
{
    uint8_t tmp;

    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_RDSR );
    tmp=SPI_WriteByte(0XFF);
    FLASH_CS_1();
    return tmp;
}
static void sst25vf_wait_busy(void)
{
    while( sst25vf_read_status() & (0x01));
}

static void _buffer_write(rt_off_t pos)
{
    //数据写回FLASH
    rt_uint32_t index;
    rt_uint32_t des_sector = pos & ~0xFFF; //得到本次需要写的目标扇区号.
    rt_uint8_t * wp = flash_buffer->swap_buffer;

    sst25vf_wait_busy();//等待上次擦除完成

    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_WREN );//write en
    FLASH_CS_1();

    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_AAIP );
    SPI_WriteByte(  des_sector>>16 );
    SPI_WriteByte(  des_sector>>8 );
    SPI_WriteByte(  des_sector );

    SPI_WriteByte( *wp++ );
    SPI_WriteByte( *wp++ );
    FLASH_CS_1();
    sst25vf_wait_busy();

    for(index=0; index<2047; index++)
    {
        FLASH_CS_0();
        SPI_WriteByte( SPI_FLASH_CMD_AAIP );
        SPI_WriteByte( *wp++ );
        SPI_WriteByte( *wp++ );
        FLASH_CS_1();
        sst25vf_wait_busy();
    }
    FLASH_CS_1();

    //写操作完毕,关闭写使能.
    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_WRDI );
    FLASH_CS_1();
}

#if SST_WRITE_BUFFER
/* 定时器的控制块 */
static rt_timer_t timer_sst_write_buffer;

static void _buffer_update(void)
{
    rt_uint32_t index;
    for(index=0; index<4096/FLASH_SECTOR_SIZE; index++)
    {
        if( flash_buffer->sector_flag[index] )
        {
            rt_uint8_t * r_buf = flash_buffer->write_buffer + FLASH_SECTOR_SIZE*index;
            rt_uint8_t * w_buf = flash_buffer->swap_buffer  + FLASH_SECTOR_SIZE*index;

            rt_memcpy(w_buf,r_buf,FLASH_SECTOR_SIZE);

            flash_buffer->sector_flag[index] = 0;
        }
        else
        {
            rt_uint32_t read_index = flash_buffer->block_addr + FLASH_SECTOR_SIZE*index;
            rt_uint8_t * w_buf = flash_buffer->swap_buffer  + FLASH_SECTOR_SIZE*index;

            FLASH_CS_0();
            SPI_WriteByte( SPI_FLASH_CMD_WRDI );
            FLASH_CS_1();

            FLASH_CS_0();
            SPI_WriteByte( SPI_FLASH_CMD_READ);
            SPI_WriteByte(  read_index>>16 );
            SPI_WriteByte(  read_index>>8 );
            SPI_WriteByte(  read_index );

#if SPI_FLASH_USE_DMA
            // 从FLASH中读取数据:使用DMA读取
            {
                uint16_t *sp, *dp, *end;
                DMA_RxConfiguration((rt_uint32_t) _spi_flash_buffer, FLASH_SECTOR_SIZE);

                SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_RXNE);
                SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);
                while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);

                SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);

                //memcpy 使用16位方式提高性能
                sp = (uint16_t *) _spi_flash_buffer;
                dp = (uint16_t *) w_buf;
                end = sp + FLASH_SECTOR_SIZE / 2;
                while (sp < end)
                {
                    *dp++ = *sp++;
                }
            }
#else
            // 从FLASH中读取数据
            {
                rt_uint32_t i;
                for(i=0; i<512; i++)
                {
                    *w_buf++ = SPI_WriteByte(0xFF);
                }
            }
#endif
            FLASH_CS_1();
        }
    }
}

/* 定时器超时函数 */
static void timeout_sst_write_buffer(void* parameter)
{
    /* 停止自己 */
    rt_timer_stop(timer_sst_write_buffer);

    rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
    /* SPI1 configure */
    rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

    _buffer_update();
    //擦除扇区
    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_WREN );//write en
    FLASH_CS_1();
    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_ERASE_4K );
    SPI_WriteByte( flash_buffer->block_addr >> 16 );
    SPI_WriteByte( flash_buffer->block_addr >> 8 );
    SPI_WriteByte( flash_buffer->block_addr  );
    FLASH_CS_1();
    sst25vf_wait_busy();

    _buffer_write( flash_buffer->block_addr);

    FLASH_CS_1();
    rt_sem_release(&spi1_lock);
}

#endif

static rt_size_t rt_spi_flash_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
#if SST_WRITE_BUFFER
    //检查是否已经使用了写缓冲
    if( buffer_use_flag == 0)
    {
        // 如果还没有使用写缓冲
        rt_uint8_t * w_buf = flash_buffer->write_buffer + (rt_uint32_t)(pos&0x0FFF);

        buffer_use_flag = 1;
        flash_buffer->block_addr = pos&~0x0FFF ;
        flash_buffer->sector_flag[ (pos&0x0FFF) / FLASH_SECTOR_SIZE ] = 1;

        rt_memcpy(w_buf,buffer,size);

        //启动定时器
        {
            rt_uint32_t time_out = 100;
            rt_timer_control(timer_sst_write_buffer,RT_TIMER_CTRL_SET_TIME,&time_out);
            rt_timer_start(timer_sst_write_buffer);
        }
    }// 如果还没有使用写缓冲
    else
    {
        // 如果缓冲区里面已经有数据了.
        rt_uint8_t * w_buf = flash_buffer->write_buffer + (rt_uint32_t)(pos&0x0FFF);
        rt_uint32_t block_addr = pos&~0x0FFF ;

        //先停止定时器防止捣乱
        rt_timer_stop(timer_sst_write_buffer);

        //检查当前要写的数据是否和缓冲区里面的在同一块.
        if( block_addr == flash_buffer->block_addr)
        {
            //如果和上次数据在同一个块.
            flash_buffer->sector_flag[ (pos&0x0FFF) / FLASH_SECTOR_SIZE ] = 1;
            rt_memcpy(w_buf,buffer,size);

            //检查是否全部缓冲都已经写入.
            if(
                (flash_buffer->sector_flag[ 0 ] == 1) &&
                (flash_buffer->sector_flag[ 1 ] == 1) &&
                (flash_buffer->sector_flag[ 2 ] == 1) &&
                (flash_buffer->sector_flag[ 3 ] == 1) &&
                (flash_buffer->sector_flag[ 4 ] == 1) &&
                (flash_buffer->sector_flag[ 5 ] == 1) &&
                (flash_buffer->sector_flag[ 6 ] == 1) &&
                (flash_buffer->sector_flag[ 7 ] == 1) )
            {
                //已经是写满了.
                //把写缓冲的数据写入FLASH

                rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
                /* SPI1 configure */
                rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

                // 因为已经是一个块4096字节满了,所以不必读取FLASH.
                // _buffer_update();
                rt_memcpy(flash_buffer->swap_buffer,flash_buffer->write_buffer,4096);

                FLASH_CS_0();
                SPI_WriteByte( SPI_FLASH_CMD_WREN );//write en
                FLASH_CS_1();
                FLASH_CS_0();
                SPI_WriteByte( SPI_FLASH_CMD_ERASE_4K );
                SPI_WriteByte( flash_buffer->block_addr >> 16 );
                SPI_WriteByte( flash_buffer->block_addr >> 8 );
                SPI_WriteByte( flash_buffer->block_addr  );
                FLASH_CS_1();
                //本处不必等待.先把数据写入缓冲区以提高效率.
                sst25vf_wait_busy();

                _buffer_write(pos);

                // 清除所有 flag
                buffer_use_flag = 0;
                {
                    rt_uint32_t i;
                    for(i=0; i<8; i++)
                    {
                        flash_buffer->sector_flag[i] = 0;
                    }
                }
                // 清除所有 flag

                FLASH_CS_1();
                rt_sem_release(&spi1_lock);
            }//已经是写满了.
            else
            {
                //还没有写满
                //启动定时器
                rt_uint32_t time_out = 100;
                rt_timer_control(timer_sst_write_buffer,RT_TIMER_CTRL_SET_TIME,&time_out);
                rt_timer_start(timer_sst_write_buffer);
            }
        }//如果和上次数据在同一个块.
        else
        {
            //缓冲区已经有数据,并和当前的数据不在同一块.
            //则必须先把当前缓冲区内的数据写入FLASH并把本次数据写入缓冲区.

            _buffer_update();

            rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
            /* SPI1 configure */
            rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

            FLASH_CS_0();
            SPI_WriteByte( SPI_FLASH_CMD_WREN );//write en
            FLASH_CS_1();
            FLASH_CS_0();
            SPI_WriteByte( SPI_FLASH_CMD_ERASE_4K );
            SPI_WriteByte( flash_buffer->block_addr >> 16 );
            SPI_WriteByte( flash_buffer->block_addr >> 8 );
            SPI_WriteByte( flash_buffer->block_addr  );
            FLASH_CS_1();
            //本处不必等待.先把数据写入缓冲区以提高效率.
            sst25vf_wait_busy();
            _buffer_write( flash_buffer->block_addr );
            FLASH_CS_1();
            rt_sem_release(&spi1_lock);

            {
                rt_uint8_t * w_buf = flash_buffer->write_buffer + (rt_uint32_t)(pos&0x0FFF);

                buffer_use_flag = 1;
                flash_buffer->block_addr = pos&~0x0FFF ;
                flash_buffer->sector_flag[ (pos&0x0FFF) / FLASH_SECTOR_SIZE ] = 1;

                rt_memcpy(w_buf,buffer,size);
                //启动定时器
                {
                    rt_uint32_t time_out = 100;
                    rt_timer_control(timer_sst_write_buffer,RT_TIMER_CTRL_SET_TIME,&time_out);
                    rt_timer_start(timer_sst_write_buffer);
                }
            }
        }//缓冲区已经有数据,并和当前的数据不在同一块.
    }// 如果缓冲区里面已经有数据了.

    return size;
#else // #if SST_WRITE_BUFFER
    //先读取本扇区数据
    {
        rt_uint32_t index;
        rt_uint8_t * w_buf = flash_buffer->swap_buffer;
        rt_uint32_t des_sector = pos&~0x0FFF ;//得到本次需要写的目标扇区号.

        rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
        /* SPI1 configure */
        rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

        FLASH_CS_0();
        SPI_WriteByte( SPI_FLASH_CMD_WRDI );
        FLASH_CS_1();

        FLASH_CS_0();
        SPI_WriteByte( SPI_FLASH_CMD_READ);
        SPI_WriteByte(  des_sector>>16 );
        SPI_WriteByte(  des_sector>>8 );
        SPI_WriteByte(  des_sector );
#if SPI_FLASH_USE_DMA
        //每次读取512字节
        for(index=0; index<(4096/FLASH_SECTOR_SIZE); index++)
        {
            uint16_t *sp, *dp, *end;
            DMA_RxConfiguration((rt_uint32_t) _spi_flash_buffer, FLASH_SECTOR_SIZE);

            SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_RXNE);
            SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);
            while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);

            SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);

            //memcpy 使用16位方式提高性能
            sp = (uint16_t *) _spi_flash_buffer;
            dp = (uint16_t *) w_buf;
            end = sp + FLASH_SECTOR_SIZE / 2;
            while (sp < end)
            {
                *dp++ = *sp++;
            }
            w_buf += FLASH_SECTOR_SIZE;
        }
#else
    for(index=0; index<4096; index++)
    {
        *w_buf++ = SPI_WriteByte(0xFF);
    }
#endif
        FLASH_CS_1();

        //读取完毕,就擦除本扇区
        FLASH_CS_0();
        SPI_WriteByte( SPI_FLASH_CMD_WREN );//write en
        FLASH_CS_1();
        //--------------
        FLASH_CS_0();
        SPI_WriteByte( SPI_FLASH_CMD_ERASE_4K );
        SPI_WriteByte( des_sector>>16 );
        SPI_WriteByte( des_sector>>8 );
        SPI_WriteByte( des_sector );
        FLASH_CS_1();
        //本处不必等待.先把数据写入缓冲区以提高效率.
    }//先读取本扇区数据

    //数据写入缓冲区
    {
        rt_uint32_t index;
        rt_uint32_t des_sector = (pos&0x0FFF) ;
        rt_uint8_t * w_buf = flash_buffer->swap_buffer + des_sector ;
        const char * r_buf = buffer;

        for(index=0; index<512; index++)
        {
            *w_buf++ = *r_buf++;
        }
    }

    //将缓冲区的数据写入FLASH.
    sst25vf_wait_busy();
    _buffer_write(pos);

    rt_sem_release(&spi1_lock);

    return size;
#endif
}

static rt_size_t rt_spi_flash_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_uint32_t index;
#if SST_WRITE_BUFFER
    // 如果使用了写缓冲,则检查需要读取的数据是否已在缓冲中.
    if(
        buffer_use_flag &&
        (flash_buffer->block_addr == (pos&~0x0FFF)) &&
        (flash_buffer->sector_flag[ (pos&0x0FFF) / FLASH_SECTOR_SIZE ]) )
    {
        uint16_t *sp, *dp, *end;
        sp = (uint16_t *)(flash_buffer->write_buffer + (rt_uint32_t)(pos&0x0FFF) );
        dp = buffer;
        end = sp + FLASH_SECTOR_SIZE / 2;
        while (sp < end)
        {
            *dp++ = *sp++;
        }
        return size;
    }
#endif

    rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
    /* SPI1 configure */
    rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_WRDI );
    FLASH_CS_1();

    FLASH_CS_0();
    SPI_WriteByte( SPI_FLASH_CMD_READ);
    SPI_WriteByte(  pos>>16 );
    SPI_WriteByte(  pos>>8 );
    SPI_WriteByte(  pos );
#if SPI_FLASH_USE_DMA
    //每次读取512字节
    for (index = 0; index < (size / FLASH_SECTOR_SIZE); index++)
    {
        uint16_t *sp, *dp, *end;

        DMA_RxConfiguration((rt_uint32_t) _spi_flash_buffer, FLASH_SECTOR_SIZE);

        SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_RXNE);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);
        while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);

        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);

        //memcpy 使用16位方式提高性能
        sp = (uint16_t *) _spi_flash_buffer;
        dp = (uint16_t *) buffer + index*FLASH_SECTOR_SIZE;
        end = sp + FLASH_SECTOR_SIZE / 2;
        while (sp < end)
        {
            *dp++ = *sp++;
        }
    }
#else
    //读取全部数据
    {
        char * wp = buffer;
        for(index=0; index<size; index++)
        {
            *wp++ = SPI_WriteByte(0xFF);
        }
    }
#endif
    FLASH_CS_1();
    rt_sem_release(&spi1_lock);

    return size;
}
/******** SST25VF **************/

void rt_hw_spi_flash_init(void)
{
    GPIO_Configuration();

#if SPI_FLASH_USE_DMA
    /* Enable the DMA1 Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#endif

    //读取芯片ID以区分型号
    {
        rt_uint8_t id1,id2,id3;

        rt_sem_take(&spi1_lock, RT_WAITING_FOREVER);
        /* SPI1 configure */
        rt_hw_spi1_baud_rate(SPI_BaudRatePrescaler_4);/* 72M/4=18M */

        //读取前都要先关闭写使能. <针对SST25VF**,于AT45DB无效>
        FLASH_CS_0();
        SPI_WriteByte( SPI_FLASH_CMD_WRDI );
        FLASH_CS_1();

        FLASH_CS_0();
        SPI_WriteByte( SPI_FLASH_CMD_JEDEC_ID );
        id1 = SPI_WriteByte(0xFF);
        id2 = SPI_WriteByte(0xFF);
        id3 = SPI_WriteByte(0xFF);
        FLASH_CS_1();

        if( id1==0xBF && id2==0x25 && id3==0x41) //SST25VF016
        {
            rt_kprintf("FLASH ID: %02X %02X %02X",id1,id2,id3);
            rt_kprintf("  match SST25VF016\r\n");

#if SST_WRITE_BUFFER
            //为驱动创建一个定时器
            {
                /* 创建定时器1 */
                timer_sst_write_buffer = rt_timer_create("t_spi",  /* 定时器名字是 timer1 */
                                         timeout_sst_write_buffer, /* 超时时回调的处理函数 */
                                         RT_NULL, /* 超时函数的入口参数 */
                                         100, /* 定时长度，以OS Tick为单位，即10个OS Tick */
                                         RT_TIMER_FLAG_PERIODIC); /* 周期性定时器 */
                if( timer_sst_write_buffer == RT_NULL )
                {
                    rt_kprintf("create timer_sst_write_buffer fail..\r\n");
                }
            }

            // 为SST25VF016申请交换区
            //读缓冲区4096,写缓冲区4096,外加缓冲控制区
            flash_buffer = rt_malloc( sizeof(struct _flash_buffer) );
            if( flash_buffer == RT_NULL )
            {
                rt_kprintf("\r\n flash_buffer no memory!\r\n");
            }
            else
            {
                rt_memset(flash_buffer,0,sizeof(struct _flash_buffer));
                buffer_use_flag = 0;//初始状态还没有使用写缓冲.
            }
#else
            // 为SST25VF016申请交换区
            flash_buffer = rt_malloc( sizeof(struct _flash_buffer) );
            if( flash_buffer == RT_NULL )
            {
                rt_kprintf("\r\n flash_buf no memory!\r\n");
            }
#endif

            //------ 初始化SST25VF** -----------
            FLASH_CS_0();
            SPI_WriteByte( SPI_FLASH_CMD_DBSY );
            FLASH_CS_1();

            FLASH_CS_0();
            SPI_WriteByte( SPI_FLASH_CMD_EWSR );
            FLASH_CS_1();

            FLASH_CS_0();
            SPI_WriteByte( SPI_FLASH_CMD_WRSR );
            SPI_WriteByte( 0 );
            FLASH_CS_1();
            //------ 初始化SST25VF** -----------

            /* register spi_flash device */
            spi_flash_device.type    = RT_Device_Class_Block;
            spi_flash_device.init    = AT45DB_flash_init;
            spi_flash_device.open    = AT45DB_flash_open;
            spi_flash_device.close   = AT45DB_flash_close;
            spi_flash_device.read 	 = rt_spi_flash_read;
            spi_flash_device.write   = rt_spi_flash_write;
            spi_flash_device.control = AT45DB_flash_control;
        }
        else //AT45DB161D
        {
            rt_uint8_t id = AT45DB_StatusRegisterRead();
            if( (id&0x3C) == 0x2C  )
            {
                rt_kprintf("FLASH status: %02X match AT45DB161\r\n",id);
                /* register spi_flash device */
                spi_flash_device.type    = RT_Device_Class_Block;
                spi_flash_device.init    = AT45DB_flash_init;
                spi_flash_device.open    = AT45DB_flash_open;
                spi_flash_device.close   = AT45DB_flash_close;
                spi_flash_device.read 	 = AT45DB_flash_read;
                spi_flash_device.write   = AT45DB_flash_write;
                spi_flash_device.control = AT45DB_flash_control;
            }
            else
            {
                rt_kprintf("No match FLASH!\r\n");
            }
        }

        FLASH_CS_1();
        rt_sem_release(&spi1_lock);
    }//读取芯片ID以区分型号

    /* no private */
    spi_flash_device.private = RT_NULL;

    rt_device_register(&spi_flash_device, "spi0",
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}
