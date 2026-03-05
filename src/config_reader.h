//
// created by souranil on 2/1/2026.
//
#define _GNU_SOURCE
#ifndef CONFIG_READER_H
#define CONFIG_READER_H 

#include <stdint.h>
#include <pthread.h>
#include <time.h>

#include "String.h"

typedef struct{
	// Struct to hold the provided names for each input coming from the serial input device
	String *filename;
	uint8_t total_number_of_inputs;	
	String **audio_mapping_arr;
	//
	bool *is_audio_map_chaged;
	// mutex lock for synchronizing live reaload of new config data
	pthread_mutex_t config_file_lock;
	// to keep track of when the configs were last read from the file and updated in programme's memory
	time_t last_read;

}AudioMappings; 
/*
 * Return a new audiomappings struct with the config data parsed from the `config` file with the number
 * of inputs provided. 
 */
AudioMappings * init_audio_mapping(const char *config, const uint8_t number_of_inputs);

/*
 * Add a new serial input to audio file mapping in the existing audio mapping struct
 */
int8_t add_new_mapping(AudioMappings *configs, const char *line);

/*
* Functions to check if the config file was modified since the last time it was read. 
* Return value: -1 failure, 0 not modified, 1 modified.
*/
int8_t is_modified(const AudioMappings *configs);

#endif
