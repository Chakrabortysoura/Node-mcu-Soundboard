//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "audio.h"
#include "serial_com.h"

int main(int argc, char  *argv[]){
  if (chdir("/home/souranil/Music")!=0){ //Always start at the users home directoryf;
    fprintf(stderr, "Error in changing the base directory: %s\n", strerror(errno));
    return 1;
  }

  int total_track_number=6;
  char *serial_port;
  for(int i=1;i<argc;i++){ // Command line args parser to parse through the args
    if (strcmp(argv[i], "--total")==0 && i+1<argc){
      total_track_number=atoi(argv[i+1]);
    }else if (strcmp(argv[i], "--err")==0 && i+1<argc){
      fclose(stderr);
      stderr=fopen(argv[i+1], "a");
    }else if (strcmp(argv[i], "--serial")==0 && i+1<argc){
     serial_port=argv[i+1]; 
    }
  }
  if (serial_port==nullptr){
    fprintf(stderr, "Didn't provide a path to the serial port\n");
    exit(1);
  }
  int serial_dev=init_serial_port(serial_port);
  init_av_objects(total_track_number); // Initialize the objects for the package of the audio header package
  
  PlayInput inputs={.track_number=-1, .result=0, .is_running=false, .track_input_mutex= PTHREAD_MUTEX_INITIALIZER, .state_var_mutex=PTHREAD_MUTEX_INITIALIZER};
  pthread_t audio_thread;
  while (true){
    pthread_mutex_lock(&inputs.state_var_mutex);
    int read_ret=read(serial_dev, &inputs.track_number, 1);
    pthread_mutex_unlock(&inputs.state_var_mutex);

    if (read_ret>0){
      inputs.track_number-=(int) '0';
      printf("Serial input track: %d\n", inputs.track_number);
      if (inputs.track_number>total_track_number){
        break;
      }
      if (pthread_create(&audio_thread, NULL, play, &inputs)!=0){
        fprintf(stderr, "Some error in spawning a new thread\n");
        continue;
      }
    }
  }
  free_av_objects(total_track_number); //free all the objects from the audio package 
  fprintf(stderr, "Closing the programme\n");
  return 0;
}
