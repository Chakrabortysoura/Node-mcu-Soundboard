#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#define _GNU_SOURCE
#include <stdint.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

extern void free_av_objects(AVFormatContext **fileformatctx, AVPacket **datapacket, AVFrame **dataframe, AVCodecContext ***streamcodectx);

typedef struct direntry{ 
	/* 
	 * A wrapper struct around a char buffer to store the names 
	 * of the directory entries and keeps a count of the total number of directory entries
	*/
	char *direntrynames;
	int8_t count;
}DIRENTRY;

extern int8_t read_dir_contents(const char *dirname, DIRENTRY *contents);

#endif
