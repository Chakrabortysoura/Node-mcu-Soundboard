//
// created by souranil on 1/5/2026
//
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>

#include "ring_buff.h"

int read_buffer(RingBuffer *ring_buff, float *buff, int n){
  /*
  *  Read upto n amount of data from the ring_buff. This function ensures that we don't read into the part of the buffer
  *  that was previously read by another process and is meant for write. Read only the amount of data so that the read_head doesn't go beyond
  *  the curr_write head. Return the number of elements read from the ring_buff.
  */
  if (buff==NULL || ring_buff==NULL || n==0){
    return 0;
  }
  pthread_mutex_lock(&ring_buff->read_head_lock);
  int curr_read=ring_buff->read_head;
  pthread_mutext_unlock(&ring_buff->read_head_lock);

  pthread_mutex_lock(&ring_buff->write_head_lock);
  int curr_write=ring_buff->write_head;
  pthread_mutext_unlock(&ring_buff->write_head_lock);
  
  n=((curr_read+n)%MAX_SIZE)<curr_write?n:curr_write-curr_read; // Get the available amount of data in the ringbuffer for reading
  
  memcpy(buff, ring_buffer->buff[curr_read], n);

  pthread_mutex_lock(&ring_buff->write_head_lock);
  ring_buff->read_head+=n;
  pthread_mutext_unlock(&ring_buff->write_head_lock);
  
  return n;
}

int write_buffer(RingBuffer *ring_buff, float *buff, int n){
  /*
  *  Write upto n amount of data to the ring_buff. This function is thread-safe and ensures that we don't write into the part of the buffer
  *  that is yet to be read. Write only the amount of data in the buff so that the write head doesn't go beyond the current read_head. 
  *  Return the number of elements written to the ring_buff.
  */
  if (buff==NULL || ring_buff==NULL || n==0){
    return 0;
  }
  pthread_mutex_lock(&ring_buff->read_head_lock);
  int curr_read=ring_buff->read_head;
  pthread_mutext_unlock(&ring_buff->read_head_lock);

  pthread_mutex_lock(&ring_buff->write_head_lock);
  int curr_write=ring_buff->write_head;
  pthread_mutext_unlock(&ring_buff->write_head_lock);

  n=((curr_write+n)%MAX_SIZE)<curr_read?n:curr_read-curr_write; // Get the available amount of data in the ringbuffer for write
  
  memcpy(ring_buffer->buff[curr_write], buff, n);
  
  pthread_mutex_lock(&ring_buff->write_head_lock);
  ring_buff->write_head+=n;
  pthread_mutext_unlock(&ring_buff->write_head_lock);
  
  return n;
}
