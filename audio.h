#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#define _GNU_SOURCE
#include <stdint.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

extern void free_av_objects(AVFormatContext **fileformatctx, AVPacket **datapacket, AVFrame **dataframe, AVCodecContext ***streamcodectx);

typedef struct dir_entry_names{
	char **entry_names;
	int count;
}DIRENT;

extern DIRENT * read_dir_contents(const char *dirname);

extern void free_direntry_obj(DIRENT **obj);

#endif
