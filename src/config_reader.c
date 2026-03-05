/ created by souranil on 2/1/2026.
//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "config_reader.h"
#include "String.h"

int64_t index(const char *str, const char search_term){
    /*This function searches for a particular search_term char in the given string
    * returns -1 on failure to find the search_term or index of the char in the string.
    * In order for this function to work properly the given string has to null-terminated.
    */
    int64_t total_len=strlen(str);
    char *idx=strchr(str, search_term);
    if (idx==NULL){ //Search term doesn't exist in the string
        return -1;
    }
    int64_t split_len=strlen(idx);
    return (int64_t) total_len-split_len;
}

uint8_t extract_right_side(const char *src, String **target_buffer, const char splitter){
    /*
     * Split the given src string at the splitter character and copy the right side of the split string in the target_buffer.
     * Return: -ve return value for any internal error and 0 when successful.
    */
    int64_t split_idx=index(src, splitter);
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

int8_t add_new_mapping(AudioMappings *configs, const char *line){
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
        fprintf(stderr, "Already a audio file mapping exist for this serial input.\n");
        return -1;
    }
    if (extract_right_side(line, &configs->audio_mapping_arr[input_number-1], ':')!=0){
        return -2;
    }
    return 0;
}

AudioMappings * init_audio_mapping(const char *config, const uint8_t number_of_inputs){
    //Initialize the audiomappings struct with the given number of total possible mappings
    AudioMappings *newobj=(AudioMappings *) calloc(1, sizeof(AudioMappings));
    if (newobj==NULL){
        fprintf(stderr, "AudioMapping allocation failed. Error: %s\n", strerror(errno));
        return NULL; 
    }
    newobj->filename=init_string_from_src(config);
    newobj->total_number_of_inputs=number_of_inputs;
    newobj->audio_mapping_arr=(String **)calloc(number_of_inputs, sizeof(String *));
    if (newobj->audio_mapping_arr==NULL){
        fprintf(stderr, "String buffer allocation failed during AudioMapping allocation. Error: %s\n", strerror(errno));
        free(newobj);
        return NULL;
    }
    newobj->is_audio_map_changed=(bool *)calloc(number_of_inputs, sizeof(bool));
    if (newobj->is_audio_map_changed==NULL){
        fprintf(stderr, "Unable to allocate the is_audio_map_changed flag buffer. Error: %s\n", strerror(errno));
        free(newobj->audio_mapping_arr);
        free(newobj);
        return NULL;
    }
    newobj->last_read=time(NULL);
    return newobj;
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
