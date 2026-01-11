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
#include "config_reader.h"

#define SERIAL_INPUT_MAPPING_FILE "serial_config.txt"

static int8_t total_track_number;
static int pipeline[2];

void termination_handler(int sign){
  fprintf(stderr, "\nTerminating signal handler invoked\n");
  deinit_av_objects(total_track_number); // deinitialize the audio.h package level objects for easy cleanup at the time of exit. 
  fprintf(stderr, "Closing the programme\n");
  exit(0);
}

int main(int argc, char  *argv[]){
  // Registering some basic signal handlers for the programme. 
  signal(SIGINT, termination_handler);  
  signal(SIGTERM, termination_handler);
  signal(SIGKILL, termination_handler);
  signal(SIGSTOP, termination_handler);
  
  char homepath[1024];
  if (sprintf(homepath, "%s/.config/soundboard", getenv("HOME"))<=0){ // Get the designated config folder in the current user's config directory
    fprintf(stderr, "Error getting home path of the current user\n");
    return 1;
  }
  if (chdir(homepath)!=0){ //Always start at the users home directoryf;
    fprintf(stderr, "Error in changing the base directory: %s\n", strerror(errno));
    fprintf(stderr, "Provided path: %s\n", homepath);
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

  if (pipe2(pipeline, O_NONBLOCK)!=0){ // Initiate the pipe file descriptor
    fprintf(stderr, "Error while creating pipe for sending the pipewire server data");
    return 1;
  }
  
  const char *teststr="1:audio.mp3";
  String *str1=NULL;
  uint8_t input=split_in_two(teststr, &str1, ':');
  fprintf(stderr, "Key: %d, Filepath: %s\n", input, str1->str);   

  fprintf(stderr, "Closing the programme\n");
  return 0;
}
