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
  *  the curr_write head and the total amount read is still under n. Return the number of elements read from the ring_buff.
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

  int i=0;
  for (i=0;curr_read<curr_write && i<n;i++){
    buff[i]=ring_buff[curr_read];
    curr_read=(curr_read+1)%MAX_SIZE;
  }

  pthread_mutex_lock(&ring_buff->write_head_lock);
  ring_buff->read_head=curr_read;
  pthread_mutext_unlock(&ring_buff->write_head_lock);
  
  return i;
}

int write_buffer(RingBuffer *ring_buff, float *buff, int n){
  /*
  *  Write upto n amount of data to the ring_buff. This function is thread-safe and ensures that we don't write into the part of the buffer
  *  that is yet to be read. Write only the amount of data in the buff so that the write head doesn't go beyond the current read_head and the amount written is 
  *  still under n. Return the number of elements written to the ring_buff.
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

  int i=0;
  for (i=0;curr_write<curr_read && i<n;i++){
    ring_buff[curr_write]=buff[i];
    curr_write=(curr_write+1)%MAX_SIZE;
  }
  
  pthread_mutex_lock(&ring_buff->write_head_lock);
  ring_buff->write_head=curr_write;
  pthread_mutext_unlock(&ring_buff->write_head_lock);
  
  return i;
}
