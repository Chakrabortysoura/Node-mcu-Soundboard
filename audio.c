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

typedef struct streamcontext{ // A struct to hold the various streamcodectx for each tracks 
            int nb_streams; // Number of streams for i th indexed file 
            AVCodecContext **streamctx; // array of codectx for this particular file's streams
}StreamContext;

AVPacket *datapacket; // Package level datapacket to use when demuxign a particular trackcontext_buffer[track_number-1].
AVFrame *dataframe; // Package level dataframe to use when decoding from previously obtained data packets.
AVFormatContext **trackcontext_buffer; // Package level buffer to hold the context data of all the audio files.
StreamContext *track_stream_ctx_buffer; // Package buffer for struct type streamcontext to hold all the AVCodecContext for all the track that are mapped.


int init_av_objects(const int total_track_number){
  /*
   * Initializes the objects needed for audio demuxing and playback. This function returns 0 if succesfull or 
   * exits with the proper error message. 
  */
  if ((trackcontext_buffer=calloc(total_track_number, sizeof(AVFormatContext *)))==NULL){
    fprintf(stderr, "Unable to allocate av context buffer for all the files\n");
    return -1;
  }
  if ((dataframe=av_frame_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    free(trackcontext_buffer);
    return -1;
  }
  if ((datapacket=av_packet_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    av_frame_free(&dataframe);
    free(trackcontext_buffer);
    return -1;
  }
  if ((track_stream_ctx_buffer=calloc(total_track_number, sizeof(StreamContext)))==NULL){
    fprintf(stderr, "Unable to allocate trackcontext buffer\n");
    av_frame_free(&dataframe);
    av_packet_free(&datapacket);
    free(trackcontext_buffer);
    return -1;
  }
  return 0;
}

void deinit_av_objects(const int total_track_number){
  //Cleanup all the allocated objects before exiting the programme
  av_frame_free(&dataframe);
  av_packet_free(&datapacket);
  for (int i=0;i<total_track_number;i++){
    avformat_close_input(&trackcontext_buffer[i]);
    for (int j=0;j<track_stream_ctx_buffer[i].nb_streams;j++){
      avcodec_free_context(track_stream_ctx_buffer[i].streamctx+j);
    }
    free(track_stream_ctx_buffer[i].streamctx);
  }
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

int check_sample_rate(const int track_number){
  /*
   * Reads the audio file uptil we get a datapacket from the audio file that belongs to one of the audio streams and then 
   * returns the sample rate of the audio stream. 
   * Returns -1 for any error or audio sample rate(greater than 0).
   */
  int err;
  if (trackcontext_buffer[track_number-1]==NULL){// If this particular track(given with the track_number) is yet to be read once 
    err=read_audio_file_header(track_number);
    if (err!=0){ // Failed to read the header from the audio file. Error val from the read_audio_file_header is returned
      return -1;
    }
  }
  if (track_stream_ctx_buffer[track_number-1].streamctx==NULL){
    err=get_avcodec_decoder(track_number);
    if (err!=0){// Failed to get all the decoder for all the stream in the audio file
      return -1;
    }
  }
  int audio_sample_rate=-1;
  while (true){
    err=av_read_frame(trackcontext_buffer[track_number-1], datapacket);
    if (err!=0){
      fprintf(stderr, "Error: Unable to read frame from track no:%d\n", track_number);
      break;
    }
    if (trackcontext_buffer[track_number-1]->streams[datapacket->stream_index]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){ 
      // Calculate the sample rate when we get the first frame from the stream containing the audio data 
      err=avcodec_send_packet(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], datapacket);
      if (err!=0){
        fprintf(stderr, "Error: Unable to feed packet to the decoder. Track no:%d\n", track_number);
        break;
      }
      err=avcodec_receive_frame(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], dataframe);
      if (err!=0){
        fprintf(stderr, "Error: Unable to decode packet to frame. Track no:%d\n", track_number);
        break;
      }
      audio_sample_rate=dataframe->sample_rate;
      break;
    }
  }
  av_seek_frame(trackcontext_buffer[track_number-1], -1, 0, AVSEEK_FLAG_BACKWARD); // Go back to the first to the use next time
  av_packet_unref(datapacket);  
  av_frame_unref(dataframe);
  
  return audio_sample_rate;
}

int check_number_of_channels(const int track_number){
  /*
   * This function returns the number of channels after reading a frame from the file 
   */
  int err;
  if (&trackcontext_buffer[track_number-1]==NULL){// If this particular track(given with the track_number) is yet to be read once 
    err=read_audio_file_header(track_number);
    if (err!=0){ // Failed to read the header from the audio file. Error val from the read_audio_file_header is returned
      return -1;
    }
  }
  if (track_stream_ctx_buffer[track_number-1].streamctx==NULL){
    err=get_avcodec_decoder(track_number);
    if (err!=0){// Failed to get all the decoder for all the stream in the audio file
      return -1;
    }
  }
  int channels_number=-1;
  while (true){
    err=av_read_frame(trackcontext_buffer[track_number-1], datapacket);
    if (err!=0){
      fprintf(stderr, "Error: Unable to read frame from track no:%d\n", track_number);
      break;
    }
    if (trackcontext_buffer[track_number-1]->streams[datapacket->stream_index]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){ 
      // Calculate the sample rate when we get the first frame from the stream containing the audio data 
      err=avcodec_send_packet(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], datapacket);
      if (err!=0){
        fprintf(stderr, "Error: Unable to feed packet to the decoder. Track no:%d\n", track_number);
        break;
      }
      err=avcodec_receive_frame(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], dataframe);
      if (err!=0){
        fprintf(stderr, "Error: Unable to decode packet to frame. Track no:%d\n", track_number);
        break;
      }
      channels_number=dataframe->ch_layout.nb_channels;
      break;
    }
  }
  av_seek_frame(trackcontext_buffer[track_number-1], -1, 0, AVSEEK_FLAG_BACKWARD); // Go back to the first to the use next time
  av_packet_unref(datapacket);  
  av_frame_unref(dataframe);
  
  return channels_number;
}

void * play(void *args){
  /*
    * This function expeects a path to a audio track to play and all the tracks are to labelled as numbers in the 
    * designated directory. Ex- 1.mp3, 2.mp3 etc or optionally this function can also take track names too. 
  */
  PlayInput *inputs=(PlayInput *)args; 
  
  pthread_mutex_lock(&inputs->track_input_mutex);
  int8_t track_number=inputs->track_number;
  pthread_mutex_unlock(&inputs->track_input_mutex);
  
  pthread_mutex_lock(&inputs->state_var_mutex);
  inputs->is_running=true;
  pthread_mutex_unlock(&inputs->state_var_mutex);

  fprintf(stderr, "Input track number received: %d\n", track_number);

  char target_track_path[10];
  if (sprintf(&target_track_path[0], "%d.flac", track_number)<=0){
    fprintf(stderr, "Aborting the play function\n");
    inputs->result=-1;
    pthread_mutex_lock(&inputs->state_var_mutex);
    inputs->is_running=false;
    pthread_mutex_unlock(&inputs->state_var_mutex);
    return inputs;
  }
  
  pthread_testcancel();  
  if (trackcontext_buffer[track_number-1]==NULL){ 
    if (read_audio_header(track_number)!=0){
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
       inputs->result=-1;
       pthread_mutex_lock(&inputs->state_var_mutex);
       inputs->is_running=false;
       pthread_mutex_unlock(&inputs->state_var_mutex);
       return inputs;
     }
  }
  
  pthread_testcancel();  
  //Try to read just one packet from the file and decode that packet to a valid frame
  int demuxerr, decoderr;
  fprintf(stderr, "Starting to decode the streams\n");
  while((demuxerr=av_read_frame(trackcontext_buffer[track_number-1], datapacket))==0){
    //Feed the decoder a packet 
    pthread_testcancel();  
    if((decoderr=avcodec_send_packet(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], datapacket))==0){
      //Retrieving decoded frames from the decoder till the decoder buff is not empty
      while((decoderr=avcodec_receive_frame(track_stream_ctx_buffer[track_number-1].streamctx[datapacket->stream_index], dataframe))==0){
        pthread_testcancel();  
        av_frame_unref(dataframe);// clean the frame after use
      }
    }else if(AVERROR(EINVAL)){
      fprintf(stderr, "Codec not opened.\n");
    }else{
      fprintf(stderr, "Unknown error sending packets to the decoder.\n");
    }
    av_packet_unref(datapacket);// clean the packet after use
  }

  if (demuxerr==AVERROR_EOF){
    fprintf(stderr, "End of File reached: %s\n", target_track_path);
  }else{
    fprintf(stderr, "Some unknwon error while reading packets from the file: %s\n, Error code: %d\n",  target_track_path, demuxerr);
  }
  //fprintf(stderr, "Total number of frames decoded: %d\n", i);
  av_seek_frame(trackcontext_buffer[track_number-1], -1, 0, AVSEEK_FLAG_BACKWARD); // Go back to the first to the use next time
  
  inputs->result=0;
  pthread_mutex_lock(&inputs->state_var_mutex);
  inputs->is_running=false;
  pthread_mutex_unlock(&inputs->state_var_mutex);
  return inputs;
}
