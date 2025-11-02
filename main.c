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
  if (chdir("/home/souranil/")!=0){ //Always start at the users home directoryf;
    fprintf(stderr, "Error in changing the base directory: %s\n", strerror(errno));
    return 1;
  }
  char buffer[800];
  fprintf(stderr, "Currect directory: %s\n", getcwd(buffer, sizeof(buffer)));

  char *track_name=calloc(256, sizeof(char));
  if (track_name==NULL){
    fprintf(stderr, "Initial allocation for input buffer failed\n");
    return 1;
  }
  //pthread_t audio_thread;
  init_av_objects(); // Initialize the objects for the package of the audio header package

  while (true){
    fprintf(stderr, "Track path: ");
    if ((fgets_unlocked(track_name, 256, stdin))==NULL){
      fprintf(stderr, "Failed to take input\n");
      continue;
    }else{
      track_name[strcspn(track_name, "\n")]='\0';
    }
    if (strcmp(track_name, "Q")==0 || strcmp(track_name, "q")==0){
      fprintf(stderr, "Exiting the programme\n");
      break;
    }
    play(track_name);
  }

  free(track_name);
  free_av_objects(); //free all the objects from the audio package 
  return 0;
}
