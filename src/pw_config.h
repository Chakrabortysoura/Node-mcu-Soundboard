//
// Created by souranil on 1/3/2026
//
#ifndef PW_CONFIG_H
#define PW_CONFIG_H

#include <stdint.h>

typedef struct{
  /*
  * The main struct to use to pass data to spawn a new thread to connect to pipewire deamon.
  */
  int8_t pipe_read_fd;
  uint8_t result;
}PwInput;

void * init_pipewire(void *args);

void *deinit_pipewire(PwInput *input);

#endif
