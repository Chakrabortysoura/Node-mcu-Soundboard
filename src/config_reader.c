//
// created by souranil on 2/1/2026.
//
#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config_reader.h"
#include "String.h"

ssize_t split_index(char *str, const char search_term){
    /*This function searches for a particular search_term char in the given string
    * returns -1 on failure to find the search_term or index of the char in the string.
    * In order for this function to work properly the given string has to null-terminated.
    */
    for (ssize_t i=0;i<strlen(str);i++){
        if (str[i]==search_term){
            return i;
        }
    }
    return -1;
}

int8_t copy_str_from_split(char *src, String **target_buffer, const char splitter){
    /*
     * Split the given src string at the splitter character and copy the right side of the split string in the target_buffer.
     * Return: -ve return value for any internal error and 0 when successful.
    */
    ssize_t split_idx=split_index(src, splitter);
    if (split_idx<0){
        fprintf(stderr, "The splitting character was not found in the source string.\n");
        return -1;
    }
    *target_buffer=init_string_from_src(src+split_idx+1);
    if (target_buffer==NULL){
        fprintf(stderr, "Unable to copy the split string.\n");
        return -2;
    }
    return 0;
}

int8_t add_new_mapping(AudioMappings *configs, char *line){
    if (configs==NULL){
        fprintf(stderr, "Uninitialized configs data structure.\n");
        return -1;
    }
    int8_t input_number=atoi(line); 
    if (input_number>configs->total_number_of_inputs || input_number<=0){
        fprintf(stderr, "Please provide valid number to map audio file to: %s\n", line);
        return -1;
    }
    if (configs->audio_mapping_arr[input_number-1]!=NULL){
        fprintf(stderr, "Changed detected for the audio file map for the serial input: %d\n", input_number);
        configs->is_audio_map_changed[input_number-1]=true;
    }
    if (copy_str_from_split(line, &configs->audio_mapping_arr[input_number-1], ':')!=0){
        return -2;
    }
    return 0;
}

AudioMappings * init_audio_mapping(const char *config_filename, const uint8_t number_of_inputs){
    //Initialize the audiomappings struct with the given number of total possible mappings
    AudioMappings *newobj=(AudioMappings *) calloc(1, sizeof(AudioMappings));
    if (newobj==NULL){
        fprintf(stderr, "AudioMapping allocation failed. Error: %s\n", strerror(errno));
        return NULL; 
    }
    newobj->filename=init_string_from_src(config_filename);
    if (newobj->filename==nullptr){
        free(newobj);
        return NULL;
    }
    newobj->total_number_of_inputs=number_of_inputs;
    newobj->audio_mapping_arr=(String **)calloc(number_of_inputs, sizeof(String *));
    if (newobj->audio_mapping_arr==NULL){
        fprintf(stderr, "String buffer allocation failed during AudioMapping allocation. Error: %s\n", strerror(errno));
        deinit_string(newobj->filename);
        free(newobj);
        return NULL;
    }
    newobj->is_audio_map_changed=(bool *)calloc(number_of_inputs, sizeof(bool));
    if (newobj->is_audio_map_changed==NULL){
        fprintf(stderr, "Unable to allocate the is_audio_map_changed flag buffer. Error: %s\n", strerror(errno));
        deinit_string(newobj->filename);
        free(newobj->audio_mapping_arr);
        free(newobj);
        return NULL;
    }
    newobj->last_read=time(NULL);
    return newobj;
}

void deinit_audio_mapping(AudioMappings *config_map){
    for(uint8_t i=0;i<config_map->total_number_of_inputs;i++){
       deinit_string(config_map->audio_mapping_arr[i]); 
    }
    deinit_string(config_map->filename);
    free(config_map->audio_mapping_arr);
    free(config_map);
}

uint8_t generate_config(){
	const char *default_config="1 : [example.mp3]\n"
                                   "2 :\n"
                                   "3 :\n"
                                   "4 :\n"
                                   "5 :\n"
                                   "6 :\n"
                                   "Provide a path to a valid audio file for each of the input. Remove this line";
	
	FILE *baseconfigfile=fopen("config.txt", "w");
	if (fputs(default_config, baseconfigfile)<0){
		fprintf(stderr, "Issue generating the default template for the config file: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int8_t parse_config_file(AudioMappings *configmap){
    FILE *config_file=fopen(configmap->filename->str, "r");
    if (config_file==NULL){
        fprintf(stderr, "Opening config failed. Error: %s\n", strerror(errno));
        return -1;
    }
    size_t len=1024; // Read each line from the text file and parse those line to store the audio mappings for each of the inputs from the serial device.
    char *buffer=calloc(len, sizeof(char));
    if (buffer==NULL){
        fprintf(stderr, "Error allocating string buffer for reading context file. Error: %s\n", strerror(errno));
        return -1;
    }
    ssize_t linesize=0;
    while ((linesize=getline(&buffer, &len, config_file))>0){
        buffer[linesize-1]='\0';
        add_new_mapping(configmap, buffer); // Ignoring any error occuring in add_new_mapping as any error related to non-existent audio mapping is handled in the audio module's play function. 
    }
    fclose(config_file);
    free(buffer);
    return 0; 
}

int8_t is_modified(const AudioMappings *configs){
    if (configs==NULL){
        fprintf(stderr, "Uninitialized configs data structure.\n");
        return -1;
    }
    struct stat fileparams;
    if (stat(configs->filename->str, &fileparams)!=0){ // Get the file attributes for the target config filename
        fprintf(stderr, "Unable to get the file attributes of the config file: %s\n", configs->filename->str);
        return -1;
    }
    const time_t last_modified=fileparams.st_mtime;
    if (difftime(last_modified, configs->last_read)>0){ // Compare to check if the file was modified since the last read of the config data
        return 1;
    }
    return 0;
}
