//
// created by souranil on 2/1/2026.
//
#define _GNU_SOURCE
#ifndef CONFIG_READER_H
#define CONFIG_READER_H 

#include <stdint.h>
#include <pthread.h>

#include "String.h"

typedef struct{
	// Struct to hold the provided names for each input coming from the serial input device
	uint8_t total_number_of_inputs;	
	String **filename_arr;
	
	// mutex lock for synchronizing live reaload of new config data
	pthread_mutex_t config_file_lock;

}AudioMappings; 

AudioMappings * init_audio_mapping(const uint8_t number_of_inputs);

int8_t add_new_mapping(AudioMappings *configs, const char *line);

#endif
