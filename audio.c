//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

AVPacket *datapacket; // Package level datapacket to use when demuxign a particular fileformatctx.
AVFrame *dataframe; // Package level dataframe to use when decoding from previously obtained data packets.
AVFormatContext *fileformatctx; // Package level fileformatctx. This holds the info of the file that is being currently being played.
AVCodecContext **streamcodectx; // Package level streamcodectx object array. Allocation and deallocation of this happens inside the play function. This buffer is 
// allocated with streamcodectx->nb_streams properties. 

void free_av_objects(){
  //Cleanup all the allocated objects before exiting the programme
  av_frame_free(&dataframe);
  av_packet_free(&datapacket);
  avformat_close_input(&fileformatctx);
}

int init_av_objects(){
  /*
   * Initializes the objects needed for audio demuxing and playback. This function returns 0 if succesfull or 
   * exits with the proper error message. 
  */
  if ((dataframe=av_frame_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    exit(1);
  }
  if ((datapacket=av_packet_alloc())==NULL){
    fprintf(stderr, "Unable to allocate av frame\n");
    av_frame_free(&dataframe);
    free_av_objects();
    exit(1);
  }
  if ((fileformatctx=avformat_alloc_context())==NULL){
    fprintf(stderr, "Unable to allocate file format ctx object.\n");
    av_frame_free(&dataframe);
    av_packet_free(&datapacket);
    free_av_objects();
    exit(1);
  }
  return 0;
}

int play(char *target_track_path){
  /*
    * This function expeects a path to a audio track to play and all the tracks are to labelled as numbers in the 
    * designated directory. Ex- 1.mp3, 2.mp3 etc or optionally this function can also take track names too. 
  */

  fprintf(stderr, "Track path given to the play function: %s\n", target_track_path);
  if(avformat_open_input(&fileformatctx, target_track_path, NULL, NULL)!=0){
    fprintf(stderr, "Unable to open the requested file: %s\n", strerror(errno));
    avformat_free_context(fileformatctx);
    return -1;
  }
  if(avformat_find_stream_info(fileformatctx, NULL)<0){
    fprintf(stderr, "Unable to obtain stream info: %s\n", strerror(errno));
    avformat_free_context(fileformatctx);
    return -1;
  }
  fprintf(stderr, "Context data obtained from the file: %s\n", target_track_path);

  //Starting to inspect and attempt to decode each individual stream in the given file
  fprintf(stderr, "Trying to obtain stream information from the File=>\n");
  streamcodectx=calloc(fileformatctx->nb_streams ,sizeof(AVCodecContext));
  if (streamcodectx==NULL){
    fprintf(stderr, "Unable to allocate streamcodectx object for the required number of streams\n");
    exit(1);
  }
  
  for(int i=0;i<fileformatctx->nb_streams;i++){
    if((streamcodectx[i]=avcodec_alloc_context3(NULL))==NULL){ // Initialize individual each elements in the array 
      fprintf(stderr, "Failed to allocate for stream details.\nFile: %s Stream no: %d\n", target_track_path, i+1);
      continue;
    }
    // Obtain decoder and details about all streams in the file and store those context in the streamcodectx array
    if(avcodec_open2(streamcodectx[i], avcodec_find_decoder(fileformatctx->streams[i]->codecpar->codec_id), NULL)!=0){
      fprintf(stderr, "Unable to get the decoder for the stream type\n");
      continue;
    }
    
    fprintf(stderr, "Stream Info\n Stream number: %d\n", i+1);
    switch(streamcodectx[i]->codec->type){
      case AVMEDIA_TYPE_AUDIO: fprintf(stderr, " Type: Audio, Codec: %s\n", streamcodectx[i]->codec->long_name);
                               break;
      case AVMEDIA_TYPE_VIDEO: fprintf(stderr, " Type: Video, Codec: %s\n", streamcodectx[i]->codec->long_name);
                               break;
      case AVMEDIA_TYPE_SUBTITLE: fprintf(stderr, " Type: Subtitle, Codec: %s\n", streamcodectx[i]->codec->long_name);
                                  break;
      default: fprintf(stderr, "Unknown codec type\n");
    }
  }  
  
  //Try to read just one packet from the file and decode that packet to a valid frame
  int demuxerr, decoderr, i=0;
  fprintf(stderr, "Starting to decodec the streams\n");
  while((demuxerr=av_read_frame(fileformatctx, datapacket))==0){
    //Feed the decoder a packet 
    if((decoderr=avcodec_send_packet(streamcodectx[datapacket->stream_index], datapacket))==0){
      //Retrieving decoded frames from the decoder till the decoder buff is not empty
      while((decoderr=avcodec_receive_frame(streamcodectx[datapacket->stream_index], dataframe))==0){
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

  for(int i=0;i<fileformatctx->nb_streams;i++){
    avcodec_free_context(streamcodectx+i); //free inividual codectx from the array before cleaning the whole array
  }
  free(streamcodectx); // Free the whole buffer allocated for the all the streamcodectx objects

  return 0;
}
