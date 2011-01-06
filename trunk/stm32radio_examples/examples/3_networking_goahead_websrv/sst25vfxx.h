#ifndef SST25VFXX_H_INCLUDED
#define SST25VFXX_H_INCLUDED

#include <stdint.h>

/* function list */
extern void sst25vfxx_init(void);
extern uint32_t sst25vfxx_read(uint32_t offset,uint8_t * buffer,uint32_t size);
extern uint32_t sst25vfxx_page_write(uint32_t page,const uint8_t * buffer,uint32_t size);

/* command list */
#define CMD_RDSR                    0x05  /* 读状态寄存器     */
#define CMD_WRSR                    0x01  /* 写状态寄存器     */
#define CMD_EWSR                    0x50  /* 使能写状态寄存器 */
#define CMD_WRDI                    0x04  /* 关闭写使能       */
#define CMD_WREN                    0x06  /* 打开写使能       */
#define CMD_READ                    0x03  /* 读数据           */
#define CMD_FAST_READ               0x0B  /* 快速读           */
#define CMD_BP                      0x02  /* 字节编程         */
#define CMD_AAIP                    0xAD  /* 自动地址增量编程 */
#define CMD_ERASE_4K                0x20  /* 扇区擦除:4K      */
#define CMD_ERASE_32K               0x52  /* 扇区擦除:32K     */
#define CMD_ERASE_64K               0xD8  /* 扇区擦除:64K     */
#define CMD_ERASE_full              0xC7  /* 全片擦除         */
#define CMD_JEDEC_ID                0x9F  /* 读 JEDEC_ID      */
#define CMD_EBSY                    0x70  /* 打开SO忙输出指示 */
#define CMD_DBSY                    0x80  /* 关闭SO忙输出指示 */

/* device id define */
enum _sst25vfxx_id
{
    unknow     = 0,
    SST25VF016 = 0x004125BF,
};


#endif // SST25VFXX_H_INCLUDED
