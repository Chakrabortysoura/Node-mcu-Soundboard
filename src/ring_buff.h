//
// created by souranil on 1/5/2026
//
#ifndef RING_BUFF_H
#define RING_BUFF_H

#define _GNU_SOURCE
#include <pthread.h>
#define MAX_SIZE 6144
#define MIN_WRITE 1024

typedef struct{
  float buff[MAX_SIZE];
  int read_head, write_head; 
  pthread_mutex_t read_head_lock, write_head_lock;
}RingBuffer;

int write_buffer(RingBuffer *ring_buff, float *buff, int n);

int read_buffer(RingBuffer *ring_buff, float *buff, int n);

#endif
