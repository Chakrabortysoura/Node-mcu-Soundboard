//
// Created by souranil on 11/1/25.
//
#define _GNU_SOURCE

typedef struct play_input{
	int track_number;
	int result;
}PlayInput;

void *play(void *args);

int init_av_objects(int total_track_number);

void free_av_objects(int total_track_number);
