#ifndef _FLAC_DECODER_H
#define _FLAC_DECODER_H
 
#include "bitstreamf.h"

#define MAX_CHANNELS 2       /* Maximum supported channels */
#define MAX_BLOCKSIZE 1152   /* Maxsize in samples of one uncompressed frame */
#define MAX_FRAMESIZE 5*1024  /* Maxsize in bytes of one compressed frame */

#define FLAC_OUTPUT_DEPTH 16 /* Provide samples left-shifted to 28 bits+sign */


enum decorrelation_type {
    INDEPENDENT,
    LEFT_SIDE,
    RIGHT_SIDE,
    MID_SIDE,
};


//#define INDEPENDENT  0
//#define LEFT_SIDE    1
//#define RIGHT_SIDE   2
//#define MID_SIDE     3


typedef struct FLACContext {
    GetBitContext gb;

    int min_blocksize, max_blocksize;
    int min_framesize, max_framesize;
    int samplerate, channels;
    int blocksize;  // last_blocksize
    int bps, curr_bps;
    unsigned long samplenumber;
    unsigned long totalsamples;
    enum decorrelation_type decorrelation;
//	int decorrelation;

    int filesize;
    int length;
    int bitrate;
    int metadatalength;
    
	int seektable;
	int seekpoints;
	
    int bitstream_size;
    int bitstream_index;

    int sample_skip;
    int framesize;

    int *decoded0;  // channel 0
    int *decoded1;  // channel 1
} FLACContext;

int flac_decode_frame(FLACContext *s, uint8_t *buf, int buf_size, int16_t *wavbuf) ICODE_ATTR_FLAC;

#endif
