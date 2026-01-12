//
// created by souranil on 2/1/2026.
//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int split_in_two(const char *src, String **target_buffer, const char splitter){
    /*
    * Split the given src string in two parts at the splitter char idx. This function does check if the splitter
    * char is in the string or not. Destination string buffer should be a 2d buffer containing enough space for two 300 char strings.
    */
    int64_t split_idx=index(src, splitter);
    if (split_idx<0){
        fprintf(stderr, "The splitting character was not found in the source string.\n");
        return -1;
    }
    uint8_t key=atoi(src);
    *target_buffer=init_string_from_src(src+split_idx+1);
    if (target_buffer==NULL){
        return -1;
    }
    return key;
}

AudioMappings * init_audio_mapping(const uint8_t number_of_inputs){
    //Initialize the audiomappings struct with the given number of total possible mappings
    AudioMappings *newobj=(AudioMappings *) calloc(1, sizeof(AudioMappings));
    if (newobj==NULL){
        fprintf(stderr, "AudioMapping allocation failed. Error: %s\n", strerror(errno));
        return NULL; 
    }
    newobj->total_number_of_inputs=&number_of_inputs;
    newobj->filename_arr=(String **)calloc(number_of_inputs, sizeof(String *));
    if (newobj->filename_arr==NULL){
        fprintf(stderr, "String buffer allocation failed during AudioMapping allocation. Error: %s\n", strerror(errno));
        free(newobj);
        return NULL;
    }
    return newobj;
}

uint8_t add_new_mapping(AudioMappings *config_data, const char *line){
     
    return 0;
}
