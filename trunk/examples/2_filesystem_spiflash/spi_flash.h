#ifndef SPI_FLASH_H_INCLUDED
#define SPI_FLASH_H_INCLUDED

/*
user for AT45DB161.
copy form : http://www.ourdev.cn/bbs/bbs_content.jsp?bbs_sn=737106
thanks to gxlujd.
*/
#define AT45DB_BUFFER_1_WRITE                 0x84	/* 写入第一缓冲区 */
#define AT45DB_BUFFER_2_WRITE                 0x87	/* 写入第二缓冲区 */
#define AT45DB_BUFFER_1_READ                  0xD4	/* 读取第一缓冲区 */
#define AT45DB_BUFFER_2_READ                  0xD6	/* 读取第二缓冲区 */
#define AT45DB_B1_TO_MM_PAGE_PROG_WITH_ERASE  0x83	/* 将第一缓冲区的数据写入主存储器（擦除模式）*/
#define AT45DB_B2_TO_MM_PAGE_PROG_WITH_ERASE  0x86	/* 将第二缓冲区的数据写入主存储器（擦除模式）*/
#define AT45DB_MM_PAGE_TO_B1_XFER             0x53	/* 将主存储器的指定页数据加载到第一缓冲区    */
#define AT45DB_MM_PAGE_TO_B2_XFER             0x55	/* 将主存储器的指定页数据加载到第二缓冲区    */
#define AT45DB_PAGE_ERASE                     0x81	/* 页删除（每页512/528字节） */
#define AT45DB_SECTOR_ERASE                   0x7C	/* 扇区擦除（每扇区128K字节）*/
#define AT45DB_READ_STATE_REGISTER            0xD7	/* 读取状态寄存器 */
#define AT45DB_MM_PAGE_READ                   0xD2	/* 读取主储存器的指定页 */
#define AT45DB_MM_PAGE_PROG_THRU_BUFFER1      0x82  /* 通过缓冲区写入主储存器 */

/*
user for SST25VF**
*/
#define SPI_FLASH_CMD_RDSR                    0x05  /* 读状态寄存器     */
#define SPI_FLASH_CMD_WRSR                    0x01  /* 写状态寄存器     */
#define SPI_FLASH_CMD_EWSR                    0x50  /* 使能写状态寄存器 */
#define SPI_FLASH_CMD_WRDI                    0x04  /* 关闭写使能       */
#define SPI_FLASH_CMD_WREN                    0x06  /* 打开写使能       */
#define SPI_FLASH_CMD_READ                    0x03  /* 读数据           */
#define SPI_FLASH_CMD_FAST_READ               0x0B  /* 快速读           */
#define SPI_FLASH_CMD_BP                      0x02  /* 字节编程         */
#define SPI_FLASH_CMD_AAIP                    0xAD  /* 自动地址增量编程 */
#define SPI_FLASH_CMD_ERASE_4K                0x20  /* 扇区擦除:4K      */
#define SPI_FLASH_CMD_ERASE_32K               0x52  /* 扇区擦除:32K     */
#define SPI_FLASH_CMD_ERASE_64K               0xD8  /* 扇区擦除:64K     */
#define SPI_FLASH_CMD_ERASE_full              0xC7  /* 全片擦除         */
#define SPI_FLASH_CMD_JEDEC_ID                0x9F  /* 读 JEDEC_ID      */
#define SPI_FLASH_CMD_EBSY                    0x70  /* 打开SO忙输出指示 */
#define SPI_FLASH_CMD_DBSY                    0x80  /* 关闭SO忙输出指示 */

//---------------------------------

/*
 * WARNING: !!! ENABLING DMA MODE MAY DESTROY YOUR DATA IN THE SPI FLASH !!!
 * Don't set SPI_FLASH_USE_DMA to 1 unless you know what you're doing!
 * However, readonly access is just fine. :)
 */
#define SPI_FLASH_USE_DMA         1
#define FLASH_SECTOR_SIZE	      512
#define SST_WRITE_BUFFER          1

extern void rt_hw_spi_flash_init(void);

#endif /* SPI_FLASH_H_INCLUDED */
