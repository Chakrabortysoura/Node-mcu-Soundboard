//
// created by souranil on 28/2/2026.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

#include "config_reload.h"
#include "config_reader.h"

static const AudioMappings *config;

/*
  * This function monitors if there was any modifiction to the serial_io to audio_file 
  * map config file after the last time that file was accessed. If that happens then we pause the programme and
  * reload the new audio mappings into the global config data structure. This function runs in parallel in a seperate
  * thread altogether so we are not blocking the rest of the programme for most of the time.
*/
int8_t monitor_config_file(){
}

int8_t start_monitoring(const AudioMappings* global_config){
  config=global_config;
}

void stop_monitoring(){
}
