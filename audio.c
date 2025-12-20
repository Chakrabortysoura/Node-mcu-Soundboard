//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

typedef struct streamcontext{ // A struct to hold the various streamcodectx for each tracks 
            int nb_streams; // Number of streams for i th indexed file 
            AVCodecContext **streamctx; // array of codectx for this particular file's streams
}StreamContext;

AVPacket *datapacket; // Package level datapacket to use when demuxign a particular trackcontext_buffer[track_number-1].
AVFrame *dataframein, *dataframeout; // Package level dataframe to use when decoding from previously obtained data packets.
AVFormatContext **trackcontext_buffer; // Package level buffer to hold the context data of all the audio files.
StreamContext *track_stream_ctx_buffer; // Package buffer for struct type streamcontext to hold all the AVCodecContext for all the track that are mapped.
SwrContext *resampler; // Context containing all the necessary AVOptions for resampling og audio frame data to our desired standard

int init_av_objects(const int total_track_number){
  /*
   * Initializes the objects needed for audio demuxing and playback. This function returns 0 if successfull or 
   * exits with the proper error message. 
  */
  if ((trackcontext_buffer=calloc(total_track_number, sizeof(AVFormatContext *)))==NULL){
    fprintf(stderr, "Unable to allocate av context buffer for all the files\n");
    return -1;
  }
  if ((dataframein=av_frame_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    free(trackcontext_buffer);
    return -1;
  }
  if ((dataframeout=av_frame_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    av_frame_free(&dataframein);
    free(trackcontext_buffer);
    return -1;
  }else{ 
    // Configure the output dataframe struct
    av_channel_layout_default(&(dataframein->ch_layout), 2);
    dataframeout->sample_rate=44100;
    dataframeout->format=AV_SAMPLE_FMT_FLT;
  }
  if ((datapacket=av_packet_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av packet\n");
    av_frame_free(&dataframein);
    av_frame_free(&dataframeout);
    free(trackcontext_buffer);
    return -1;
  }
  if ((resampler=swr_alloc())==NULL){
    fprintf(stderr, "Failed to initialized the ffmpeg resampler object\n");
    av_frame_free(&dataframein);
    av_frame_free(&dataframeout);
    av_packet_free(&datapacket);
    free(trackcontext_buffer);
    return -1;
  }
  if ((track_stream_ctx_buffer=calloc(total_track_number, sizeof(StreamContext)))==NULL){
    fprintf(stderr, "Unable to allocate trackcontext buffer\n");
    av_frame_free(&dataframein);
    av_frame_free(&dataframeout);
    av_packet_free(&datapacket);
    free(trackcontext_buffer);
    swr_free(&resampler);
    return -1;
  }
  return 0;
}

void deinit_av_objects(const int total_track_number){
  //Cleanup all the allocated objects before exiting the programme
  av_frame_free(&dataframein);
  av_frame_free(&dataframeout);
  av_packet_free(&datapacket);
  for (int i=0;i<total_track_number;i++){
    avformat_close_input(&trackcontext_buffer[i]);
    for (int j=0;j<track_stream_ctx_buffer[i].nb_streams;j++){
      avcodec_free_context(track_stream_ctx_buffer[i].streamctx+j);
    }
    free(track_stream_ctx_buffer[i].streamctx);
  }
  swr_free(&resampler);
  free(trackcontext_buffer);
  free(track_stream_ctx_buffer);
}

int read_audio_file_header(const int track_number){
  /*
   * Wrapper function to guess the audio format for the designated track(from the track number given) 
   * and read and allocate AVFormatContext for it and inspect the audio file stream info.
   * Returns 0 on success and -1 for any error. 
   * Errors are logged with stderr. 
   */
  if ((trackcontext_buffer[track_number-1]=avformat_alloc_context())==NULL){
    fprintf(stderr, "AVFormat context allocation failed for track number: %d\n", track_number);
    return -1;
  } 
  char target_track_path[10];
  if (sprintf(&target_track_path[0], "%d.flac", track_number)<=0){
    fprintf(stderr, "Sprintf error when constructing the name of the target file\n");
    return -1;
  }
  if ((avformat_open_input(&trackcontext_buffer[track_number-1], target_track_path, NULL, NULL))!=0){
    fprintf(stderr, "Error when trying to open the audio file: %s\n", target_track_path);
    return -1;
  }
  if (avformat_find_stream_info(trackcontext_buffer[track_number-1], NULL)<0){
    fprintf(stderr, "Error inspecting stream information for the audio trcak: %s\n", target_track_path);
    return -1;
  }
  av_dump_format(trackcontext_buffer[track_number-1], -1, NULL, 0);
  return 0;
}

int get_avcodec_decoder(const int track_number){
  /*
   * This function reads the AVFormatContext object for the correspoding file and tries to find the decoder for the streams.
   * If succesfull returns 0, otherwise -1. The resultant decoders for the streams are stored in the StreamContext buffer at package level along 
   * with the number of streams for each of the audio files.
   */
  int nb=trackcontext_buffer[track_number-1]->nb_streams; // number of streams in the file
  track_stream_ctx_buffer[track_number-1].nb_streams=nb;
  if ((track_stream_ctx_buffer[track_number-1].streamctx=calloc(nb, sizeof(AVCodecContext*)))==NULL){
    fprintf(stderr, "Error allocating AVCodecContext buffer array for the file: %d\n", track_number);
    return -1;
  }
  for(int i=0;i<nb;i++){
    if ((track_stream_ctx_buffer[track_number-1].streamctx[i]=avcodec_alloc_context3(NULL))==NULL){
      fprintf(stderr, "Internal error allocating AVCodecContext object for the %d stream for audio file: %d", i, track_number);
      return -1;
    }
    if (avcodec_open2(track_stream_ctx_buffer[track_number-1].streamctx[i], avcodec_find_decoder(trackcontext_buffer[track_number-1]->streams[i]->codecpar->codec_id),NULL)!=0){
      fprintf(stderr, "Error finding the decoder for stream no:%d in track no:%d\n", i, track_number);
      return -1;
    }
  }
  return 0;
}

int configure_resampler(const int track_number){
  /*
   * This function configures the SwrContext object with the properties of the input audio file.
   * The target format remains the same regardless to maintains compatibility with pipewire pw_buffer configurations 
   * at the time of initialization. This function assumes that a valid AVFrame has been decoded from the audio stream in datapacketin..
   */ 
  if (swr_config_frame(resampler, dataframeout, dataframein)!=0){
    fprintf(stderr, "Error in configuring resampler for the source and target audio data. Track number: %d\n", track_number);
    return -1;
  }
  return 0;
}

void * play(void *args){
  /*
    * This function expeects a path to a audio track to play and all the tracks are to labelled as numbers in the 
    * designated directory. Ex- 1.mp3, 2.mp3 etc or optionally this function can also take track names too. 
  */
  PlayInput *inputs=(PlayInput *)args; 
  
  pthread_mutex_lock(&inputs->track_input_mutex); //Reading the track input number from the shared playinput struct
  int8_t track_number=inputs->track_number;
  pthread_mutex_unlock(&inputs->track_input_mutex);
  
  pthread_mutex_lock(&inputs->state_var_mutex); //Setting the thread state to running by the shared variable
  inputs->is_running=true;
  pthread_mutex_unlock(&inputs->state_var_mutex);

  fprintf(stderr, "Input track number received: %d\n", track_number);

  char target_track_path[10];
  if (sprintf(&target_track_path[0], "%d.flac", track_number)<=0){
    fprintf(stderr, "Aborting the play function. Internal error with the sprintf() function\n");
    inputs->result=-1;
    pthread_mutex_lock(&inputs->state_var_mutex);
    inputs->is_running=false;
    pthread_mutex_unlock(&inputs->state_var_mutex);
    return inputs;
  }
  
  pthread_testcancel();  
  if (trackcontext_buffer[track_number-1]==NULL){ 
    if (read_audio_file_header(track_number)!=0){
      fprintf(stderr, "Aborting the play function. Error in reading audio file header data\n");
      inputs->result=-1;
      pthread_mutex_lock(&inputs->state_var_mutex);
      inputs->is_running=false;
      pthread_mutex_unlock(&inputs->state_var_mutex);
      return inputs;
    }
  }
  fprintf(stderr, "Context data obtained from the file: %s\n", target_track_path);

  pthread_testcancel();  
  if (track_stream_ctx_buffer[track_number-1].streamctx==NULL){
     if (get_avcodec_decoder(track_number)!=0){
      fprintf(stderr, "Aborting the play function. Error in getting the decoders for the audio file streams.\n");
      inputs->result=-1;
      pthread_mutex_lock(&inputs->state_var_mutex);
      inputs->is_running=false;
      pthread_mutex_unlock(&inputs->state_var_mutex);
      return inputs;
     }
  }
  
  pthread_testcancel();  // Another exit point from this function

  int err_ret=0;
  fprintf(stderr, "Starting to decode the streams\n");
  while(true){
    err_ret=av_read_frame(trackcontext_buffer[track_number-1], datapacket);
    if (err_ret==AVERROR_EOF){ // Handle the last demuxing error that happened at the time of while loop end
      fprintf(stderr, "End of File reached: %s. Completed decoding of the whole File.\n", target_track_path);
      goto closing;
    }else if (err_ret!=0){
      fprintf(stderr, "Some unknwon error while reading packets from the file: %s\n, Error code: %d\n",  target_track_path, err_ret);
      goto closing;
    }
    pthread_testcancel();  
    
    if (trackcontext_buffer[track_number-1]->streams[datapacket->stream_index]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){ 
      if (swr_is_initialized(resampler)==0 && configure_resampler(track_number)!=0){ // reconfigure the swr resampler for the current input audio file params
        fprintf(stderr, "Could not configure the resampler exiting play function.\n");
        av_packet_unref(datapacket);
        goto closing;
      }
      err_ret=avcodec_send_packet(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], datapacket); 
      if (err_ret==AVERROR(EINVAL) || err_ret!=0){
        fprintf(stderr, "Error occured while trying to feed the decoder datapackets\n");
        av_packet_unref(datapacket);// clean the packet after use
        goto closing;
      }
      while(true){
        err_ret=avcodec_receive_frame(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], dataframein);
        if (err_ret==AVERROR(EAGAIN) || err_ret==AVERROR_EOF){
          break;
        }else if (err_ret!=0){ // EINVAL cannot happen but still excluded just for debugging purposes.
          fprintf(stderr, "Error while receiving frames from the decoder. Error :%d\n", err_ret);
          avcodec_flush_buffers(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index]);
          av_packet_unref(datapacket);// clean the packet after use
          goto closing;
        }
      }
    }
    av_packet_unref(datapacket);// clean the packet after use
  }

  closing:
    av_seek_frame(trackcontext_buffer[track_number-1], -1, 0, AVSEEK_FLAG_BACKWARD); // Go back to the first to the use next time
    swr_close(resampler); // Closes the resampler so that it has to be reinitialized. Necessary for reconfiguring the swrcontext for use with the next audio file. 
    inputs->result=err_ret;
    pthread_mutex_lock(&inputs->state_var_mutex);
    inputs->is_running=false;
    pthread_mutex_unlock(&inputs->state_var_mutex);
    return inputs;
  
}
