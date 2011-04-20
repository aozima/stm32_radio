/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: main.c 25844 2010-05-06 17:35:13Z kugel $
 *
 * Copyright (C) 2005 Dave Chapman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#include <inttypes.h>
#include <rtthread.h>
#include <dfs_posix.h>
#include <string.h>

#include "board.h"
#include "codec.h"
#include "decoder.h"

#define true RT_TRUE
#define false RT_FALSE
#define bool rt_bool_t

#define MAX_CHANNELS 2         /* Maximum supported channels */
#define MAX_BLOCKSIZE 1152     /* Maxsize in samples of one uncompressed frame */
#define MAX_FRAMESIZE 5*1024   /* Maxsize in bytes of one compressed frame */
#define FLAC_OUTPUT_DEPTH 16   /* Provide samples left-shifted to 28 bits+sign */

unsigned char filebuf[MAX_FRAMESIZE]; /* The input buffer */
int8_t decoded0[4 * MAX_BLOCKSIZE];
int8_t decoded1[4 * MAX_BLOCKSIZE];


/*
 * 定义mempool块大小.
 * mempll_block_size = fc.max_blocksize * 4
 * 现在只支持fc.max_blocksize=1152的情况
 */
#define  mempll_block_size  (4 * MAX_BLOCKSIZE) 

//我们共申请两块mempool,并留出4字节做为控制块,每块大小为:mempll_block_size
static rt_uint8_t mempool[ (mempll_block_size+4) *2];
static struct rt_mempool _mp;
//内存池初始化标识
static rt_bool_t is_inited = RT_FALSE;



static void dump_headers(FLACContext *s)
{
    rt_kprintf("\n\r  Blocksize: %d .. %d\n\r", s->min_blocksize, 
                   s->max_blocksize);
    rt_kprintf("  Framesize: %d .. %d\n\r", s->min_framesize, 
                   s->max_framesize);
    rt_kprintf("  Samplerate: %d\n\r", s->samplerate);
    rt_kprintf("  Channels: %d\n\r", s->channels);
    rt_kprintf("  Bits per sample: %d\n\r", s->bps);
    rt_kprintf("  Metadata length: %d\n\r", s->metadatalength);
    rt_kprintf("  Total Samples: %lu\n\r",s->totalsamples);
    rt_kprintf("  Duration: %d ms\n\r",s->length);
    rt_kprintf("  Bitrate: %d kbps\n\r",s->bitrate);
}

static bool flac_init(int fd, FLACContext* fc)
{
    unsigned char buf[255];
    struct stat statbuf;
    bool found_streaminfo=false;
    int endofmetadata=0;
    int blocklength;
    uint32_t* p;
    uint32_t seekpoint_lo,seekpoint_hi;
   	uint32_t offset_lo,offset_hi;
	int n;

    if (lseek(fd, 0, SEEK_SET) < 0) 
    {
        return false;
    }

    if (read(fd, buf, 4) < 4)
    {
        return false;
    }

    if (rt_memcmp(buf,"fLaC",4) != 0) 
    {
        return false;
    }
    fc->metadatalength = 4;

    while (!endofmetadata) 
	{
        if (read(fd, buf, 4) < 4)
        {
            return false;
        }

        endofmetadata=(buf[0]&0x80);
        blocklength = (buf[1] << 16) | (buf[2] << 8) | buf[3];
        fc->metadatalength+=blocklength+4;

        if ((buf[0] & 0x7f) == 0)       /* 0 is the STREAMINFO block */
        {
            /* FIXME: Don't trust the value of blocklength */
            if (read(fd, buf, blocklength) < 0)
            {
                return false;
            }
          
            fstat(fd,&statbuf);
            fc->filesize = statbuf.st_size;
            fc->min_blocksize = (buf[0] << 8) | buf[1];
            fc->max_blocksize = (buf[2] << 8) | buf[3];
            fc->min_framesize = (buf[4] << 16) | (buf[5] << 8) | buf[6];
            fc->max_framesize = (buf[7] << 16) | (buf[8] << 8) | buf[9];
            fc->samplerate = (buf[10] << 12) | (buf[11] << 4) 
                             | ((buf[12] & 0xf0) >> 4);
            fc->channels = ((buf[12]&0x0e)>>1) + 1;
            fc->bps = (((buf[12]&0x01) << 4) | ((buf[13]&0xf0)>>4) ) + 1;

            /* totalsamples is a 36-bit field, but we assume <= 32 bits are 
               used */
            fc->totalsamples = (buf[14] << 24) | (buf[15] << 16) 
                               | (buf[16] << 8) | buf[17];

            /* Calculate track length (in ms) and estimate the bitrate 
               (in kbit/s) */
            fc->length = (fc->totalsamples / fc->samplerate) * 1000;

            found_streaminfo=true;
        }
		else 
		    if ((buf[0] & 0x7f) == 3) 	/* 3 is the SEEKTABLE block */
		    { 
               // rt_kprintf("Seektable length = %d bytes\n",blocklength);
                while (blocklength >= 18) {
                n=read(fd,buf,18);
                if (n < 18) return false;
                blocklength-=n;

                p=(uint32_t*)buf;
                seekpoint_hi=betoh32(*(p++));
                seekpoint_lo=betoh32(*(p++));
                offset_hi=betoh32(*(p++));
                offset_lo=betoh32(*(p++));
            
                if ((seekpoint_hi != 0xffffffff) && (seekpoint_lo != 0xffffffff))
				{
         	         //rt_kprintf("Seekpoint: %u, Offset=%u\n",seekpoint_lo,offset_lo);
                }
            }
            lseek(fd, blocklength, SEEK_CUR);
        } else 
		  {
            /* Skip to next metadata block */
            if (lseek(fd, blocklength, SEEK_CUR) < 0)
            {
                return false;
            }
          }
    }

   if (found_streaminfo)
   {
       fc->bitrate = ((fc->filesize-fc->metadatalength) * 8) / fc->length;
       return true;
   } 
   else 
   {
       return false;
   }
}

	
static rt_err_t flac_decoder_tx_done(rt_device_t dev, void *buffer)
{
    /* release memory block */
    rt_mp_free(buffer);
	return RT_EOK;
}


int flac(char* path) 
{
    FLACContext fc;
    int fd;
    int n;
    int bytesleft;
    int consumed;
	int16_t *decoded;

	/* audio device */
	rt_device_t snd_device;

	extern void vol(uint16_t v) ;
	vol(50);

    //检查mempool是否被初始化,否则进行初始化.
    if (is_inited == RT_FALSE)
    {
        rt_mp_init(&_mp, "flac_buf", &mempool[0], sizeof(mempool), mempll_block_size);
        is_inited = RT_TRUE;
    }


    fd=open(path,O_RDONLY,0);

    if (fd < 0) {
        rt_kprintf("Can not parse %s\n",path);
        return(1);
    }

	/* open audio device */
	snd_device = rt_device_find("snd");
	if (snd_device != RT_NULL)
	{
		/*  set tx complete call back function 
		 *  设置回调函数,当DMA传输完毕时,会执行flac_decoder_tx_done
		 */
		rt_device_set_tx_complete(snd_device, flac_decoder_tx_done);
		rt_device_open(snd_device, RT_DEVICE_OFLAG_WRONLY);
	}


    /* Read the metadata and position the file pointer at the start of the 
       first audio frame */
    flac_init(fd,&fc); 
    dump_headers(&fc);

	if((fc.min_blocksize != fc.max_blocksize) || (fc.max_blocksize > MAX_BLOCKSIZE ) || (fc.max_framesize > MAX_FRAMESIZE))
	{
	  rt_kprintf("\n\rOo Do not support this file!!\n\r"); 
	  rt_kprintf("Please make sure that your file is compressed under level 2 ^_^\n\r"); 
	  return false;
	}

	//decoded0,decoded1用来存放临时PCM数据
	fc.decoded0 = (int32_t *)decoded0;
	fc.decoded1 = (int32_t *)decoded1;

	//set CODEC's samplerate
	rt_device_control(snd_device, CODEC_CMD_SAMPLERATE, &(fc.samplerate));

    bytesleft=read(fd,filebuf,MAX_FRAMESIZE);


       while (bytesleft) 
	   {
	   	//向mempoll申请空间,如果申请不成功则一直在此等待.
		//这里申请的内存用来放置真正送CODEC的PCM数据
        decoded = rt_mp_alloc(&_mp, RT_WAITING_FOREVER);

        if(flac_decode_frame(&fc,filebuf,bytesleft,decoded) < 0) 
		{
             rt_kprintf("DECODE ERROR, ABORTING\n");
             break;
        }

		rt_device_write(snd_device, 0, decoded, ((fc.max_blocksize * 4) ));

        consumed=fc.gb.index/8;
        rt_memmove(filebuf,&filebuf[consumed],bytesleft-consumed);
        bytesleft-=consumed;

        n=read(fd,&filebuf[bytesleft],MAX_FRAMESIZE-bytesleft);
        if (n > 0) 
		{
            bytesleft+=n;
        }
		 
		
      }

	/* close device and file */
    rt_device_close(snd_device);
    close(fd);
    return(0);
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(flac, flac(char* path) );
#endif
