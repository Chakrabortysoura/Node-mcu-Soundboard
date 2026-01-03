//
// Created by souranil on 1/3/2026
//
#ifndef PW_CONFIG_H
#define PW_CONFIG_H

#include <stdint.h>

int8_t init_pipewire(const int pipe_read_fd);

void deinit_pipewire();

#endif
