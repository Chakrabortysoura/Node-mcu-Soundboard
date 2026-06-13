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

/*
 * This function generates a default config file with the appropiate structure in the 
 * target config directory of the user.
 * Return: 0 for successfully generating the default config file template.
 *	  -1 for failure.
 */
uint8_t generate_config();

/*
 * Struct to hold the names of each audio file that were mapped to all the inputs numbers from the serial input device.
 * This struct also contains info about the last time the config file was read in programme. 
 */
typedef struct{
	String *filename; // Filename of the source config file provided for the audio mappings.
	uint8_t total_number_of_inputs;	
	// Array containing the filename in the target edirectory for the serial audio inputs
	String **audio_mapping_arr;
	// Boolean array to indicate whether audio file mapping for a particular serial input was modified
	bool *is_audio_map_changed;
	// to keep track of when the configs were last read from the file and updated in programme's memory
	time_t last_read;

}AudioMappings; 

/*
 * Return a new audiomappings struct with the config data parsed from the `config` file with the number
 * of inputs provided. 
 */
AudioMappings * init_audio_mapping(const char *config_filename, const uint8_t number_of_inputs);

/*
 * De-initialize the audiomapping struct and all it's internal data.
 */
void deinit_audio_mapping(AudioMappings *config_map);

/*
 * Add a new serial input to audio file mapping in the existing audio mapping struct
 */
int8_t add_new_mapping(AudioMappings *configs, char *line);

/*
 * Reads the config txt file and store config audio mappings in the configmap struct. This configmap
 * struct should be initialized with init_audio_mapping function before passing it to this function. 
 * Return: -ve for any error and 0 for succesfully parsing the whole file.
 */
int8_t parse_config_file(AudioMappings *configmap);

/*
* Functions to check if the config file was modified since the last time it was read. 
* Return value: -1 failure, 0 not modified, 1 modified.
*/
int8_t is_modified(const AudioMappings *configs);


#endif
