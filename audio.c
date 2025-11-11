//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct streamcontext{ // A struct to hold the various streamcodectx for each tracks 
            int nb_streams;
            AVCodecContext **streamctx;
}StreamContext;

AVPacket *datapacket; // Package level datapacket to use when demuxign a particular trackcontext[track_number-1].
AVFrame *dataframe; // Package level dataframe to use when decoding from previously obtained data packets.
AVFormatContext **trackcontext; // Package level buffer to hold the context data of all the audio files.
StreamContext *track_stream_ctx_buffer; // Package buffer for struct type streamcontext to hold all the AVCodecContext for all the track that are mapped.

int check_the_format(const int track_number, char *name_buffer){
  //This function tries to determine the entension of the file associated with the given track_number (eg- mp3, flac etc. Though right now only mp3 and flacs are acceptable)
  // Returns 0 when n ocombination of th etracknumber and any file extension could be located. 
  // -1 for any internal error usually something to do with sprintf() function. 1 when the file extension could be determined. 
  // If the file format was determined then the name_buffer array is filled with the path. 
  // This ensures that the file extension is not hard coded in the play function and user can easily switch to different file formats for a track if they 
  // adhere to the correct naming conventions for the purposes of proper mapping. Though in case there exists multiple tracks with the same but different extensions
  // the priority would be of mp3->flac (right now).
  if (sprintf(name_buffer, "%d.mp3", track_number)<0){
    fprintf(stderr, "Error with sprintf while guessing the file name\n");
    return -1;
  }
  FILE *track=fopen(name_buffer, "r");
  if (track!=NULL){
    fprintf(stderr, "Track with mp3 extension found\n");
    return 1;
  }
  if (sprintf(name_buffer, "%d.flac", track_number)<0){
    fprintf(stderr, "Error with sprintf while guessing the file name\n");
    return -1;
  }
  track=fopen(name_buffer, "r");
  if (track!=NULL){
    fprintf(stderr, "Track with flac extension found\n");
    return 1;
  }
  fprintf(stderr, "Couldn't find any file with the given name and any possible extension combination\n");
  return 0;
}

int init_av_objects(const int total_track_number){
  /*
   * Initializes the objects needed for audio demuxing and playback. This function returns 0 if succesfull or 
   * exits with the proper error message. 
  */
  if ((trackcontext=calloc(total_track_number, sizeof(AVFormatContext *)))==NULL){
    fprintf(stderr, "Unable to allocate av context buffer for all the files\n");
    exit(1);
  }
  if ((dataframe=av_frame_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    free(trackcontext);
    exit(1);
  }
  if ((datapacket=av_packet_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    av_frame_free(&dataframe);
    free(trackcontext);
    exit(1);
  }
  if ((track_stream_ctx_buffer=calloc(total_track_number, sizeof(StreamContext)))==NULL){
    fprintf(stderr, "Unable to allocate trackcontext buffer\n");
    av_frame_free(&dataframe);
    av_packet_free(&datapacket);
    free(trackcontext);
    exit(1);
  }
  return 0;
}

void free_av_objects(const int total_track_number){
  //Cleanup all the allocated objects before exiting the programme
  av_frame_free(&dataframe);
  av_packet_free(&datapacket);
  for (int i=0;i<total_track_number;i++){
    avformat_close_input(&trackcontext[i]);
    for (int j=0;j<track_stream_ctx_buffer[i].nb_streams;j++){
      avcodec_free_context(track_stream_ctx_buffer[i].streamctx+j);
    }
    free(track_stream_ctx_buffer[i].streamctx);
  }
  free(trackcontext);
  free(track_stream_ctx_buffer);
}

void * play(void *args){
  /*
    * This function expeects a path to a audio track to play and all the tracks are to labelled as numbers in the 
    * designated directory. Ex- 1.mp3, 2.mp3 etc or optionally this function can also take track names too. 
  */
  PlayInput *inputs=(PlayInput *)args; 
  inputs->is_running=true;
  fprintf(stderr, "Input track number received: %d\n", inputs->track_number);
  char target_track_path[256];
  if (check_the_format(inputs->track_number, target_track_path)!=1){
    fprintf(stderr, "Aborting the play function\n");
    inputs->result=-1;
    inputs->is_running=false;
    return inputs;
  }
  if ((sprintf(target_track_path, "%d.flac", inputs->track_number))<0){
    fprintf(stderr, "There was an error in sprintf\n");
    inputs->result=-2;
    inputs->is_running=false;
    return inputs;
  }
  
  if (datapacket==NULL || dataframe==NULL){
    fprintf(stderr, "Some internal package level objects are not initialized. You may want to call init_av_objects() function before calling this play function.\n");
    inputs->result=-3;
    inputs->is_running=false;
    return inputs;
  }
  
  if (trackcontext[inputs->track_number-1]==NULL){ // if this media track is being read for the first time allocate and obtain the context for it. 
    // This context data is stored in the trackcontext buffer. 
    if ((trackcontext[inputs->track_number-1]=avformat_alloc_context())==NULL){
      fprintf(stderr, "Unable to allocate file format ctx object for the file %d no track. Aborting the play function.\n", inputs->track_number);
      inputs->result=-1;
      inputs->is_running=false;
      return inputs;
    }
    if(avformat_open_input(&(trackcontext[inputs->track_number-1]), target_track_path, NULL, NULL)!=0){
      fprintf(stderr, "Unable to open the requested file: %s| Error: %s\n", target_track_path, strerror(errno));
      avformat_free_context(trackcontext[inputs->track_number-1]);
      inputs->result=-1;
      inputs->is_running=false;
      return inputs;
    }
    if(avformat_find_stream_info(trackcontext[inputs->track_number-1], NULL)<0){
      fprintf(stderr, "Unable to obtain stream info: %s for the requested file\n", strerror(errno));
      avformat_free_context(trackcontext[inputs->track_number-1]);
      inputs->result=-1;
      inputs->is_running=false;
      return inputs;
    }
  }
  fprintf(stderr, "Context data obtained from the file: %s\n", target_track_path);

  if (track_stream_ctx_buffer[inputs->track_number-1].streamctx==NULL){
    //Starting to inspect and attempt to decode each individual stream in the given file
    track_stream_ctx_buffer[inputs->track_number-1].streamctx=calloc(trackcontext[inputs->track_number-1]->nb_streams ,sizeof(AVCodecContext *));
    if (track_stream_ctx_buffer[inputs->track_number-1].streamctx==NULL){
      fprintf(stderr, "Unable to allocate streamcodectx object for the required number of streams\n");
      inputs->result=-1;
      return inputs;
    }
    for(int i=0;i<trackcontext[inputs->track_number-1]->nb_streams;i++){
      if((track_stream_ctx_buffer[inputs->track_number-1].streamctx[i]=avcodec_alloc_context3(NULL))==NULL){ // Initialize individual each elements in the array 
        fprintf(stderr, "Failed to allocate for stream details.\nFile: %s Stream no: %d\n", target_track_path, i+1);
        inputs->result=-1;
        return inputs;
      }
      // Obtain decoder and details about all streams in the file and store those context in the streamcodectx array
      if(avcodec_open2(track_stream_ctx_buffer[inputs->track_number-1].streamctx[i], avcodec_find_decoder(trackcontext[inputs->track_number-1]->streams[i]->codecpar->codec_id), NULL)!=0){
        fprintf(stderr, "Unable to get the decoder for the stream type\n");
        inputs->result=-1;
        return inputs;
      }
      fprintf(stderr, "Stream Info\n Stream number: %d\n", i+1);
      switch(track_stream_ctx_buffer[inputs->track_number-1].streamctx[i]->codec->type){
        case AVMEDIA_TYPE_AUDIO: fprintf(stderr, " Type: Audio, Codec: %s\n", track_stream_ctx_buffer[inputs->track_number-1].streamctx[i]->codec->long_name);
                                break;
        case AVMEDIA_TYPE_VIDEO: fprintf(stderr, " Type: Video, Codec: %s\n", track_stream_ctx_buffer[inputs->track_number-1].streamctx[i]->codec->long_name);
                                break;
        case AVMEDIA_TYPE_SUBTITLE: fprintf(stderr, " Type: Subtitle, Codec: %s\n", track_stream_ctx_buffer[inputs->track_number-1].streamctx[i]->codec->long_name);
                                    break;
        default: fprintf(stderr, "Unknown codec type\n");
      }
    }  
  }
  
  
  //Try to read just one packet from the file and decode that packet to a valid frame
  int demuxerr, decoderr, i=0;
  fprintf(stderr, "Starting to decode the streams=>\n");
  while((demuxerr=av_read_frame(trackcontext[inputs->track_number-1], datapacket))==0){
    //Feed the decoder a packet 
    if((decoderr=avcodec_send_packet(track_stream_ctx_buffer[inputs->track_number-1].streamctx[datapacket->stream_index], datapacket))==0){
      //Retrieving decoded frames from the decoder till the decoder buff is not empty
      while((decoderr=avcodec_receive_frame(track_stream_ctx_buffer[inputs->track_number-1].streamctx[datapacket->stream_index], dataframe))==0){
        i++;
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
  fprintf(stderr, "Total number of frames decoded: %d\n", i);
  av_seek_frame(trackcontext[inputs->track_number-1], -1, 9, AVSEEK_FLAG_BACKWARD); // Go back to the first to the use next time
  
  inputs->result=0;
  inputs->is_running=false;
  return inputs;
}
