//
// Created by souranil on 11/1/25.
//
#define _GNU_SOURCE
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

int play(char *track_name);

int init_av_objects();

void free_av_objects();
