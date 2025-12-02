//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

#include "audio.h"

typedef struct data{
  struct pw_main_loop *loop;
  struct pw_core *core;
  struct pw_stream *stream;
}PW_Data;

int main(int argc, char  *argv[]){
  if (chdir("/home/souranil/Music")!=0){ //Always start at the users home directoryf;
    fprintf(stderr, "Error in changing the base directory: %s\n", strerror(errno));
    return 1;
  }

  int8_t total_track_number=6;
  char *serial_port;
  for(int i=1;i<argc;i++){ // Command line args parser to parse through the args
    if (strcmp(argv[i], "--err")==0 && i+1<argc){
      fclose(stderr);
      stderr=fopen(argv[i+1], "a");
    }else if (strcmp(argv[i], "--serial")==0 && i+1<argc){
     serial_port=argv[i+1]; 
    }else if (strcmp(argv[i], "--track")==0 && i+1<argc){
      total_track_number=atoi(argv[i+1]);
    }
  }
  
  //PW_Data data={0,};
//
  //pw_init(NULL, NULL);
  //fprintf(stderr, "Initialized pipewire library\n");
 // 
  //data.loop=pw_main_loop_new(NULL);
 // 
  //struct pw_context *context=pw_context_new(
    //pw_main_loop_get_loop(data.loop),
    //pw_properties_new(
      //PW_KEY_CONFIG_NAME, "client.conf",
      //NULL),
    //0);
  //if (context==NULL){
    //fprintf(stderr, "Pipewire context creation failed\n");
    //return 1;
  //}
  // 
  //data.stream=pw_stream_new(data.core, "Soundboard_stream", 
                            //pw_properties_new(
                              //PW_KEY_MEDIA_TYPE, "Audio",
                              //PW_KEY_MEDIA_CATEGORY, "Playback",
                              //PW_KEY_MEDIA_ROLE, "Music",
                              //PW_KEY_NODE_NAME, "Soundboard", 
                              //PW_KEY_NODE_RATE, rate,
                              //NULL));
  //if (data.stream==NULL){
    //fprintf(stderr, "Stream creation failed\n");
    //return 1;
  //}
  if (init_av_objects(total_track_number)!=0){
    fprintf(stderr, "Closing the programme\n");
    return -1;
  }
  printf("Give the track number: ");
  int track_number;
  scanf("%d", &track_number);
  int totalnchannel=check_number_of_channels(track_number);
  if (totalnchannel!=-1){
    fprintf(stderr, "Total number of channels in the audio file :%d\n", totalnchannel);
  }
  deinit_av_objects(total_track_number);
  fprintf(stderr, "Closing the programme\n");
  return 0;
}
