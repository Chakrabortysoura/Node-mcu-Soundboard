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
#include "pw_config.h"
#include "serial_com.h"

#define SERIAL_INPUT_MAPPING_FILE "config.txt"

static int8_t total_track_number;
static int pipeline[2];

void termination_handler(int signal){
  fprintf(stderr, "Closing the programme\n");
  deinit_av_objects(total_track_number); // deinitialize the audio.h package level objects for easy cleanup at the time of exit. 
  deinit_pipewire();
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
    fprintf(stderr, "Error in changing the base directory: %s\nProvided path: %s\n", strerror(errno), homepath);
    return 1;
  }
  
  total_track_number=6;
  char *serial_port=NULL;
  for(int i=1;i<argc;i++){ // Command line args parser to initilize the necessary configuration variables
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
  
  /*
    * Read and parse the configdata provided int the default config file path. 
    * Right now custom config file paths are not acceptable from the command line arguments.
    */
  AudioMappings *config_map=init_audio_mapping(total_track_number);
  if (config_map==NULL){
    return 1;
  }
  FILE *config_file=fopen("config.txt", "r");
  if (config_file==NULL){
    fprintf(stderr, "Opening config failed. Error: %s\n", strerror(errno));
    return 1;
  }
  size_t len=1024;
  char *buffer=calloc(len, sizeof(char));
  if (buffer==NULL){
    fprintf(stderr, "Error allocating string buffer for reading context file. Error: %s\n", strerror(errno));
    return 1;
  }
  while (getline(&buffer, &len, config_file)>0){
    char *newline_idx=strchr(buffer, '\n');
    if (newline_idx!=NULL){
      *newline_idx='\0';
    }
    add_new_mapping(config_map, buffer);
  }
  fclose(config_file);
  free(buffer);

  /*
  * Initialize the Unix pipes for inter thread communication between the main thread decoding and resampling audio streams
  * and the pipewire stream that is consuming the data coming for playback.
  */
  if (pipe2(pipeline, O_NONBLOCK)!=0){ // Initiate the pipe file descriptor
    fprintf(stderr, "Error while creating pipe for sending the pipewire server data");
    return 1;
  }
  
  /*
   * Initialize the pipewire config and connnect the stream with the help of the context object 
   * on a seperate thread so as to not block the main function execution.
    */
  //pthread_t pw_thread;
  //if (pthread_create(&pw_thread, 0, init_pipewire, &pipeline[0])!=0){
    //fprintf(stderr, "Launching pipewire failed.\n");
    //return 1;
  //}
  init_pipewire(&(pipeline[0])); 
  /*
  * Initialize the ffmpeg audio processing header.
  */
  if (init_av_objects(total_track_number)!=0){
    fprintf(stderr, "Error initializing all the av objects\n");
    return 1;
  }
  
  /*
  * Configure the serial port for io with the given path to the serial device.
  */
  if (serial_port==NULL){
    fprintf(stderr, "Please provide a address for the serial input device.\n");
    return 1;
  }
  int serial_port_fd=init_serial_port(serial_port);
  if (serial_port_fd<=0){
    fprintf(stderr, "Error configuring serial port device.\n");
  }

  //PlayInput audio_input={.track_number=1, .pipe_write_head=pipeline[1], .is_running=false, .config=config_map}; 
  //uint8_t input;
  //while(true){
    //if (read(serial_port_fd, &input, 1)>0){
      //audio_input.track_number=input-(int)'0';
      //fprintf(stderr, "Serial input data received: %d\n", audio_input.track_number);
      //play(&audio_input);
    //}
  //}

  fprintf(stderr, "\nClosing the programme\n");
  deinit_pipewire();
  deinit_av_objects(total_track_number);
  return 0;
}
