//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "audio.h"
#include "pw_config.h"

static int8_t total_track_number;
static int pipeline[2];

void termination_handler(int sign){
  fprintf(stderr, "\nTerminating signal handler invoked\n");
  fclose(stderr);
  fclose(stdout);
  //pw_main_loop_quit(main_loop); 
  //deinit_av_objects(total_track_number); // deinitialize the audio.h package level objects for easy cleanup at the time of exit. 
  fprintf(stderr, "Closing the programme\n");
  exit(0);
}


int main(int argc, char  *argv[]){
  signal(SIGINT, termination_handler); // Registering some basic signal handlers for the programme. 
  signal(SIGTERM, termination_handler);
  signal(SIGKILL, termination_handler);
  signal(SIGSTOP, termination_handler);
  
  if (chdir("/home/souranil/Music")!=0){ //Always start at the users home directoryf;
    fprintf(stderr, "Error in changing the base directory: %s\n", strerror(errno));
    return 1;
  }

  total_track_number=6;
  char *serial_port;
  for(int i=1;i<argc;i++){ // Command line args parser to parse through the args
    if (strcmp(argv[i], "--err")==0 && i+1<argc){
      fclose(stderr);
      stderr=fopen(argv[i+1], "a");
    }else if (strcmp(argv[i], "--serial")==0){
      if (i+1<argc){
        serial_port=argv[i+1]; 
      }else{
        fprintf(stderr, "Please provide the address to the serial port.\n");
        return 1;
      }
    }else if (strcmp(argv[i], "--track")==0){
      if (i+1<argc){
        total_track_number=atoi(argv[i+1]);
      }else{
        fprintf(stderr, "Default total track mapping to 6 as no input was provided.\n");
        return 1;
      }
    }
  }
  if (init_av_objects(total_track_number)<0){
    fprintf(stderr, "Error in initalizing all the internal audio buffers\n");
    return 1;
  }

  if (pipe2(pipeline, O_NONBLOCK)!=0){ // Initiate the pipe file descriptor
    fprintf(stderr, "Error while creating pipe for sending the pipewire server data");
    return 1;
  }
  
  //PlayInput audio_play_input={.pipe_write_head=pipeline[1],.track_number=1};
  //printf("track number:");
  //scanf("%" SCNd8, &audio_play_input.track_number);
  //while (audio_play_input.track_number>=total_track_number){
    //printf("Target track_number should be within the total_track_number: %d\n", total_track_number);
    //printf("track number:");
    //scanf("%c", &audio_play_input.track_number);
  //}
  //play(&audio_play_input);
  //pthread_t audio_thread;
  //pthread_create(&audio_thread, 0,play, &audio_play_input);
  
  init_pipewire(&pipeline[0]);

  fprintf(stderr, "Closing the programme\n");
  return 0;
}
