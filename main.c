//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "audio.h"

int main(int argc, char  *argv[]){
  int total_track_number=6;
  for(int i=1;i<argc;i++){ // Command line args parser to parse through the args
    if (strcmp(argv[i], "--total")==0 && i+1<argc){
      total_track_number=atoi(argv[i+1]);
    }
    if (strcmp(argv[i], "--err")==0 && i+1<argc){
      fclose(stderr);
      stderr=fopen(argv[i+1], "a");
    }
  }
  
  if (chdir("/home/souranil/")!=0){ //Always start at the users home directoryf;
    fprintf(stderr, "Error in changing the base directory: %s\n", strerror(errno));
    return 1;
  }
  char buffer[800];
  fprintf(stderr, "Currect directory: %s\n", getcwd(buffer, sizeof(buffer)));

  //pthread_t audio_thread;
  init_av_objects(total_track_number); // Initialize the objects for the package of the audio header package
  
  PlayInput inputs;
  while (true){
    fprintf(stderr, "\nGive Input: ");
    scanf("%d", &inputs.track_number);
    if (inputs.track_number<0 || inputs.track_number>=total_track_number){
      break;
    }
    play(&inputs);
    if (inputs.result<0)
      break;
  }

  free_av_objects(total_track_number); //free all the objects from the audio package 
  fprintf(stderr, "Closing the programme\n");
  return 0;
}
