//
// Created by souranil on 11/1/25.
//
#define _GNU_SOURCE
#ifndef AUDIO_H 
#define AUDIO_H 

#include <stdint.h>
#include <pthread.h>

typedef struct play_input{
	// PlayInput struct now contains an is_running flag to notify if there is any thread running currently the play
	// function and using the struct reference. 
	
	pthread_mutex_t track_input_mutex;
	int8_t track_number;

	int result;

	pthread_mutex_t state_var_mutex; // Mutex to handle reading and writing of the is_running boolean that indicates the state of the threads 
	// as that is read and written to by both the main thread and the audio thread
	bool is_running;
}PlayInput;


void *play(void *args);

int init_av_objects(const int total_track_number);

void deinit_av_objects(const int total_track_number);


#endif
