//
// Created by souranil on 11/1/25.
//
#define _GNU_SOURCE

typedef struct play_input{
	// PlayInput struct now contains an is_running flag to notify if there is any thread running currently the play
	// function and using the struct reference. 
	int track_number;
	bool is_running;
	int result;
}PlayInput;

void *play(void *args);

void init_av_objects(const int total_track_number);

void free_av_objects(const int total_track_number);
