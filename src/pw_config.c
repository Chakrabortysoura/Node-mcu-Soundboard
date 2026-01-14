//
// Created by souranil on 1/3/2026.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/utils/defs.h>

typedef struct{
  struct pw_main_loop *loop;
  struct pw_core *core;
  struct pw_stream *stream;
  struct pw_context *context;
  struct spa_hook event_listener;
  const struct spa_pod *param;
  int pipe_read_head;
}PW_Data;

static PW_Data payload;

void on_process(void *data){
  /*
   * The main handler for handling the on_process events when triggered by the pipewire deamon.
   * Reads the data coming from the pipe_read_fd and que that for the pipewire server to process.
   */
  PW_Data *userdata=(PW_Data *)data;
  struct pw_buffer *buff;
  if ((buff=pw_stream_dequeue_buffer(userdata->stream))==NULL){
    fprintf(stderr, "Out of input buffers for pipewire stream\n");
    return;
  }
  
  uint8_t *data_buff=buff->buffer->datas[0].data;
  if (data_buff==NULL){
    fprintf(stderr, "There no data buffer allocated inside the pw_buffer\n");
    return;
  }
  
  uint32_t stride=sizeof(float)*2; // Because the pw stream in configure with stereo channel layout
  uint32_t n_frames=buff->buffer->datas[0].maxsize/stride;
  if (buff->requested){
    n_frames=SPA_MIN(n_frames, buff->requested);
  }
  uint32_t required_bytes=stride*n_frames;
   
  ssize_t received_bytes=read(userdata->pipe_read_head, data_buff, required_bytes);
  if (received_bytes>0){
    buff->buffer->datas[0].chunk->offset=0;
    buff->buffer->datas[0].chunk->stride=stride;
    buff->buffer->datas[0].chunk->size=(uint32_t) received_bytes;
    if (required_bytes>received_bytes){
      memset(data_buff+received_bytes, 0, required_bytes-received_bytes);
    }
    fprintf(stderr, "Received %d bytes of data from the pipe\n", (int) received_bytes);
  }else if (errno!=EAGAIN){
    fprintf(stderr, "Some unforseen error happened.Error: %s\n", strerror(errno));
    buff->buffer->datas[0].chunk->offset=0;
    buff->buffer->datas[0].chunk->stride=stride;
    buff->buffer->datas[0].chunk->size=0;
    memset(data_buff, 0, required_bytes);
  }

  pw_stream_queue_buffer(userdata->stream, buff); 
}

const struct pw_stream_events stream_events={
    PW_VERSION_STREAM_EVENTS,
    .process=on_process,
};

void init_pipewire(void *args){
  int8_t pipe_read_fd=*(int8_t *) args;
  fprintf(stderr, "unix pipe file descriptor: %d\n", pipe_read_fd);
  pw_init(NULL, NULL);
  
  if ((payload.loop=pw_main_loop_new(NULL))==NULL){
    fprintf(stderr, "Failed to acquire a pw main loop\n");
    return;
  }
  payload.pipe_read_head=pipe_read_fd;
  payload.context=pw_context_new(pw_main_loop_get_loop(payload.loop),NULL, 0);
  if (payload.context==NULL){
    fprintf(stderr, "Unable to acquire a pw context\n");
    return;
  }
  if ((payload.core=pw_context_connect(payload.context, NULL, 0))==NULL){
    fprintf(stderr, "Unable to connet to pw core deamon\n");
    return;
  }

  payload.stream=pw_stream_new(
    payload.core, "soundboard audio stream", 
    pw_properties_new(
      PW_KEY_MEDIA_TYPE, "Audio",
      PW_KEY_MEDIA_CATEGORY, "Playback",
      PW_KEY_MEDIA_ROLE, "Music",
      PW_KEY_NODE_NAME, "Audio source"
      PW_KEY_NODE_RATE, 44100,
      NULL)
  );
  if (payload.stream==NULL){
    fprintf(stderr, "Unable to create a new pw_stream\n");
    return;
  }
  pw_stream_add_listener(payload.stream, &payload.event_listener, &stream_events, &payload);
  
  uint8_t buffer[10024];
  struct spa_pod_builder b=SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  payload.param=spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                                      &SPA_AUDIO_INFO_RAW_INIT(
                                        .format=SPA_AUDIO_FORMAT_F32,
                                        .channels=2,
                                        .rate=44100)
                                      );
  pw_stream_connect(payload.stream, 
                    PW_DIRECTION_OUTPUT,
                    PW_ID_ANY,
                    PW_STREAM_FLAG_AUTOCONNECT|
                    PW_STREAM_FLAG_MAP_BUFFERS|
                    PW_STREAM_FLAG_RT_PROCESS,
                    &payload.param, 1);
  
  pw_main_loop_run(payload.loop);
}

void deinit_pipewire(){
  /*
  * Deinitializes all the pipewire allocated objects int payload variable and disconnects the programme
  * from the pipewire deamon. 
  */
  pw_main_loop_quit(payload.loop);
  pw_main_loop_destroy(payload.loop);
}
