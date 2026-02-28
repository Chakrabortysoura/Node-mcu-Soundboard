//
// created by souranil on 2/1/2026.
//
#define _GNU_SOURCE
#ifndef CONFIG_RELOAD_H
#define CONFIG_RELOAD_H 

#include <stdint.h>

#include "config_reader.h"

/*
  * Spawns a new thread to start monitoring the config file for audio mappings for any new edits. \
  * This is helper function that is used just to start the background thread. 
  * Return value: 1 on success, -1 on failure to launch the new thread.
*/
int8_t start_monitoring(const AudioMappings* global_config);

/*
  * Kill the background thread from monitoring the config file for any edits. 
*/
void stop_monitoring();

#endif
