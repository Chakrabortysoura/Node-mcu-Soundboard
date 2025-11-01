#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#define _GNU_SOURCE
#include <stdint.h>

typedef struct strarray{
	char **array;
	int count;
}StringArray;

extern StringArray read_dir_contents(const char *dirname);

extern void free_direntry_obj(StringArray *obj);

#endif
