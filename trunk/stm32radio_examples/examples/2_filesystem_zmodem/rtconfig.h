/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

/* RT_NAME_MAX*/
#define RT_NAME_MAX	8

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	4

/* PRIORITY_MAX*/
#define RT_THREAD_PRIORITY_MAX	32

/* Tick per Second*/
#define RT_TICK_PER_SECOND	100

/* SECTION: RT_DEBUG */
/* Thread Debug*/
#define RT_DEBUG
/* #define RT_THREAD_DEBUG */

#define RT_USING_OVERFLOW_CHECK

/* Using Hook*/
#define RT_USING_HOOK

/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex*/
#define RT_USING_MUTEX

/* Using Event*/
#define RT_USING_EVENT

/* Using MailBox*/
#define RT_USING_MAILBOX

/* Using Message Queue*/
#define RT_USING_MESSAGEQUEUE

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
#define RT_USING_MEMPOOL

/* Using Dynamic Heap Management*/
#define RT_USING_HEAP

/* Using Small MM*/
#define RT_USING_SMALL_MEM

/* Using SLAB Allocator*/
/* #define RT_USING_SLAB */

/* SECTION: Device System */
/* Using Device System*/
#define RT_USING_DEVICE
#define RT_USING_SPI

/* SECTION: Console options */

#define RT_USING_CONSOLE
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	128

/* SECTION: FinSH shell options */
/* Using FinSH as Shell*/
#define RT_USING_FINSH
/* Using symbol table */
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_DEVICE_NAME   "uart1"

/* SECTION: C++ support */
/* Using C++ support*/
/* #define RT_USING_CPLUSPLUS */


#define RT_USING_DFS
#define DFS_USING_WORKDIR
/* #define RT_USING_DFS_EFSL */
/* byte alignment for EFSL */
#define BYTE_ALIGNMENT

#define RT_USING_DFS_ELMFAT
//#define RT_DFS_ELM_REENTRANT
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_DRIVES			2
//#define RT_DFS_ELM_USE_LFN
#define RT_DFS_ELM_MAX_LFN			255
#define RT_DFS_ELM_MAX_SECTOR_SIZE  2048

/* SECTION: DFS options */
/* the max number of mounted filesystem */
#define DFS_FILESYSTEMS_MAX			2
/* the max number of opened files 		*/
#define DFS_FD_MAX					8
/* the max number of cached sector 		*/
#define DFS_CACHE_MAX_NUM   		4

#endif
