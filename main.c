//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "audio.h"

int main(int argc, char  *argv[]){
  int total_track_number=6;
  if (argc==2){ // If the user provides a specific number of tracks that will be mapped to soundboard serial input otherwise defaults to the basic 6 inputs for nodemcu. 
    total_track_number=atoi(argv[1]);
  }
  
  if (chdir("/home/souranil/")!=0){ //Always start at the users home directoryf;
    fprintf(stderr, "Error in changing the base directory: %s\n", strerror(errno));
    return 1;
  }
  char buffer[800];
  fprintf(stderr, "Currect directory: %s\n", getcwd(buffer, sizeof(buffer)));

  //pthread_t audio_thread;
  init_av_objects(total_track_number); // Initialize the objects for the package of the audio header package

  while (true){
    int track_number;
    fprintf(stderr, "\nInput track number: ");
    scanf("%d", &track_number);
    play(track_number);
  }

  free_av_objects(total_track_number); //free all the objects from the audio package 
  return 0;
}
