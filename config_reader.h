//
// created by souranil on 2/1/2026.
//
#define _GNU_SOURCE
#ifndef CONFIG_READER_H
#define CONFIG_READER_H 

#include <stdint.h>

int64_t index(const char *str, const char search_term);

int split_in_two(const char *src, char * target_buffer[2], const char splitter);

#endif
