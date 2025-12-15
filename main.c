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

void on_process(void *userdata){
  for (int i=0;i<1000;i++){
    printf("Node processing\n");
  }
}
const struct pw_stream_events stream_events={
    PW_VERSION_STREAM_EVENTS,
    .process=on_process,
};

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
    }else if (strcmp(argv[i], "--serial")==0){
      if (i+1<argc){
        serial_port=argv[i+1]; 
      }else{
        fprintf(stderr, "Please provide the address to the serial port.\n");
        return 1;
      }
    }else if (strcmp(argv[i], "--track")==0 && i+1<argc){
      if (i+1<argc){
        total_track_number=atoi(argv[i+1]);
      }else{
        fprintf(stderr, "Default total track mapping to 6 as no input was provided.\n");
        return 1;
      }
    }
  }
  pw_init(NULL, NULL);

  PW_Data data={0,};

  if ((data.loop=pw_main_loop_new(NULL))==NULL){
    fprintf(stderr, "Failed to acquire a pw main loop\n");
    return 1;
  }

  struct pw_context *context=pw_context_new(pw_main_loop_get_loop(data.loop),NULL, 0);
  if (context==NULL){
    fprintf(stderr, "Unable to acquire a pw context\n");
    return 1;
  }
  if ((data.core=pw_context_connect(context, NULL, 0))==NULL){
    fprintf(stderr, "Unable to connet to pw core deamon\n");
    return 1;
  }
  
  if (init_av_objects(total_track_number)<0){
    fprintf(stderr, "Error in initalizing all the internal audio buffers\n");
    return 1;
  }

  int track_number;
  printf("track number:");
  scanf("%d", &track_number);
  while (track_number>=total_track_number){
    printf("Target track_number should be within the total_track_number: %d\n", total_track_number);
    printf("track number:");
    scanf("%d", &track_number);
  }
  char audio_sample_rate[20];
  if (sprintf(&audio_sample_rate[0], "%d", check_sample_rate(track_number))<0){
    fprintf(stderr, "Sprintf error when converting sample rate to a str\n");
    return 1;
  }

  data.stream=pw_stream_new(
    data.core, "soundboard audio stream", 
    pw_properties_new(
      PW_KEY_MEDIA_TYPE, "Audio",
      PW_KEY_MEDIA_CATEGORY, "Playback",
      PW_KEY_MEDIA_ROLE, "Music",
      PW_KEY_NODE_NAME, "Audio source"
      PW_KEY_NODE_RATE, audio_sample_rate,
      NULL)
  );
  if (data.stream==NULL){
    fprintf(stderr, "Unable to create a new pw_stream\n");
    return 1;
  }

  uint8_t buffer[10024];
  struct spa_pod_builder b=SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  const struct spa_pod *param=spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                                      &SPA_AUDIO_INFO_RAW_INIT(
                                        .format=SPA_AUDIO_FORMAT_F32,
                                        .channels=check_number_of_channels(track_number),
                                        .rate=check_sample_rate(track_number))
                                      );
  
  pw_stream_connect(data.stream, 
                    PW_DIRECTION_OUTPUT,
                    PW_ID_ANY,
                    PW_STREAM_FLAG_AUTOCONNECT|
                    PW_STREAM_FLAG_MAP_BUFFERS|
                    PW_STREAM_FLAG_RT_PROCESS,
                    &param, 1);
  
  //pw_main_loop_run(data.loop);

  fprintf(stderr, "Closing the programme\n");
  return 0;
}
