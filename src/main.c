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

static int8_t total_track_number;
static int pipeline[2];
AudioMappings *config_map;

static inline void print_help_message(){
  fprintf(stdout, "Command format: soundboard -[required flag] --[optional flag] [argument]\n"
                  "Available flags and their uses:\n"
                  "-serial     : Path to the serial input device\n"
                  "--config    : Path to the config file\n"
                  "--help / -h : Print the helper message\n"
                  "--err       : Change the standard error of this programme to point to any specific file\n"
                  "--track     : Total number of inputs to map to target tracks\n"
          );
  
}

static inline bool help_flag_present(const int argc, char *argv[]){
  for(int i=0;i<argc;i++){
    if(strstr(argv[i], "-h")!=nullptr){
      return true;
    }
  }
  return false;
}

void cleanup_handler(){
  deinit_av_objects(total_track_number); // deinitialize the audio.h package level objects for easy cleanup at the time of exit. 
  deinit_pipewire();
  deinit_audio_mapping(config_map);
  close(pipeline[0]);
  close(pipeline[1]);
}

void termination_handler(int signal){
  cleanup_handler();
  fprintf(stderr, "Closing the programme\n");
  exit(signal);
}

int main(int argc, char  *argv[]){
  if (argc<2){
    fprintf(stdout, "No path to the serial device provided.\n");
    print_help_message();
    return 1;
  }else if (help_flag_present(argc, argv)){
    print_help_message();
    return 0;
  }
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

  // Parse the necessary command line arguments provided to the programme to initialize the neccessary inital values. 
  total_track_number=6;
  const char *serial_port=nullptr, *config_filename=nullptr;
  for(int i=1;i<argc;i++){ 
    if (strcmp(argv[i], "--err")==0){
      if (i+1>=argc){
        fprintf(stderr, "No filename provided to remap he stderr to.\n");
        fprintf(stdout, "Stdout couldn't be remaped.\n");
        continue;
      }
      FILE *result=fopen(argv[i+1], "a");
      if (result==NULL){
        fprintf(stdout, "Stdout couldn't be remaped to the target file: %s. Error: %s", argv[i+1], strerror(errno));
      }else{
        fclose(stderr);
        stderr=result;
      }
    }else if (strcmp(argv[i], "-serial")==0){
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
      }
    }else if (strcmp(argv[i], "--config")==0){
      if (i+1<argc){
        config_filename=argv[i+1];
      }else{
        fprintf(stderr, "Default config file: config.txt\n");
      }
    }
  }
  if (total_track_number<=0){
    fprintf(stderr, "Please provide a positive integer value corresponding to the number of inputs coming from the serial device. Usually this means all the numeric value that the serial device can send as an input. Ex- For NodeMcu this can be 6. As we can connect 6 of it's GPIO pins to send 1-6 numeric values when triggered with switches.\n");
    return 1;
  }
  if (serial_port==nullptr){
    fprintf(stderr, "Please provide the address to the serial port with the \"-serial\" flag. \n");
    print_help_message();
    return 1;
  }
  if (config_filename==nullptr){
    config_filename="config.txt";
  }
   
  /*
    * Read and parse the configdata provided int the default config file path. 
    * Right now custom config file paths are not acceptable from the command line arguments.
    */
  config_map=init_audio_mapping(config_filename, total_track_number);
  if (config_map==NULL){
    return 1;
  }
  FILE *config_file=fopen(config_filename, "r");
  if (config_file==NULL){
    fprintf(stderr, "Opening config failed. Error: %s\n", strerror(errno));
    return 1;
  }
  size_t len=1024; // Read each line from the text file and parse those line to store the audio mappings for each of the inputs from the serial device.
  char *buffer=calloc(len, sizeof(char));
  if (buffer==NULL){
    fprintf(stderr, "Error allocating string buffer for reading context file. Error: %s\n", strerror(errno));
    return 1;
  }
  ssize_t linesize=0;
  while ((linesize=getline(&buffer, &len, config_file))>0){
    buffer[linesize-1]='\0';
    add_new_mapping(config_map, buffer); // Ignoring any error occuring in add_new_mapping as any error related to non-existent audio mapping is handled in the audio module's play function. 
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
  
  // Configure the serial port for io with the given path to the serial device.
  int serial_port_fd=init_serial_port(serial_port);
  if (serial_port_fd<=0){
    fprintf(stderr, "Error configuring serial port device.\n");
    return 1;
  }

  /*
   * Initialize the pipewire config and connnect the stream with the help of the context object 
   * on a seperate thread so as to not block the main function execution.
  */
  pthread_t pw_thread;
  if (pthread_create(&pw_thread, nullptr, init_pipewire, &pipeline[0])!=0){
    fprintf(stderr, "Launching pipewire failed.\n");
    goto cleanup_exit;
    return 1;
  }
  
  // Initialize the ffmpeg audio processing header.
  if (init_av_objects(total_track_number)!=0){
    fprintf(stderr, "Error initializing all the av objects\n");
    goto cleanup_exit;
    return 1;
  }

  PlayInput audio_input={.track_number=1, .pipe_write_head=pipeline[1], .is_running=false, .config=config_map}; 
  uint8_t input;
  pthread_t audio_thread;
  while(true){
    if (is_modified(config_map)==1){ // Config file was changed while the programme was running reload the config
      fprintf(stderr, "Config file modified reloading the the config details.\n"); 
    }
    if (read(serial_port_fd, &input, 1)>0){
      pthread_mutex_lock(&audio_input.track_input_mutex);
      audio_input.track_number=input-(int)'0'; // read the serial input data. we have to do it with mutex locks around the operation as the  same shared varibale in the PlayInput struct may be at the same time be read by the already running audio thread.
      pthread_mutex_unlock(&audio_input.track_input_mutex);
      fprintf(stderr, "Serial input data received: %d\n", audio_input.track_number);

      while (true){ // Wait till the running thread exists itself
        pthread_mutex_lock(&audio_input.state_var_mutex);
        if (audio_input.is_running==false){
          pthread_mutex_unlock(&audio_input.state_var_mutex);
          break;
        }
        pthread_mutex_unlock(&audio_input.state_var_mutex);
        fprintf(stderr, "Waiting for the running audio thread to exit.\n");
      }

      if (pthread_create(&audio_thread, nullptr, play, &audio_input)!=0){
        fprintf(stderr, "Launching a new audio thread failed.\n");
        continue;
      }
    }
  }
  cleanup_exit:
    cleanup_handler();
    fprintf(stderr, "\nClosing the programme\n");
  return 0;
}
