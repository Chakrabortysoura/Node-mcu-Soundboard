//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
//#include "serial_com.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>

void free_av_objects(AVFormatContext **fileformatctx, AVPacket **datapacket, AVFrame **dataframe, AVCodecContext ***streamcodectx){
  //Cleanup all the allocated objects before exiting the programme
  for(int i=0;i<(*fileformatctx)->nb_streams;i++){
    avcodec_free_context(*(streamcodectx)+i); //free inividual codectx from the array before cleaning the whole array
  }
  free((*streamcodectx));
  av_frame_free(dataframe);
  av_packet_free(datapacket);
  avformat_close_input(fileformatctx);

  fprintf(stderr, "---------------\n");
}

int main(const int argc, char  *argv[]){
  if (argc<2){
    fprintf(stderr,"Not enough arguments\n");
    exit(1);
  }
  if (argc==3){ // If a third argument is provided to the programme
    FILE *new_stderr=fopen(argv[2], "a");// Change the stderr to the particular file provided
    if (new_stderr!=NULL)
      stderr=new_stderr;
    else
      fprintf(stderr, "Provided file path for the alternate stderr couldn't opened so switching back to default stderr\n");
  }

  AVFormatContext *fileformatctx=avformat_alloc_context();
  if(avformat_open_input(&fileformatctx, argv[1], NULL, NULL)!=0){
    fprintf(stderr, "Unable to open the requested file: %s\n", strerror(errno));
    avformat_free_context(fileformatctx);
    exit(2);
  }
  if(avformat_find_stream_info(fileformatctx, NULL)<0){
    fprintf(stderr, "Unable to obtain stream info: %s\n", strerror(errno));
    avformat_free_context(fileformatctx);
    exit(2);
  }
  fprintf(stderr, "Context data obtained from the file: %s\n", argv[1]);

  //Starting to inspect and attempt to decode each individual stream in the given file
  fprintf(stderr, "Trying to obtain stream information from the File=>\n");
  AVCodecContext **streamcodectx=calloc(fileformatctx->nb_streams ,sizeof(AVCodecContext));
  for(int i=0;i<fileformatctx->nb_streams;i++){
    if((streamcodectx[i]=avcodec_alloc_context3(NULL))==NULL){ // Initialize individual each elements in the array 
      fprintf(stderr, "Failed to allocate for stream details.\nFile: %s Stream no: %d\n", argv[1], i+1);
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
  AVPacket *datapacket=av_packet_alloc();
  AVFrame *dataframe=av_frame_alloc();
  int demuxerr, decoderr, i=0;
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
  }if (demuxerr==AVERROR_EOF){
    fprintf(stderr, "End of File reached: %s\n", argv[1]);
  }else{
    fprintf(stderr, "Some unknwon error while reading packets from the file: %s\n, Error code: %d\n",  argv[1], demuxerr);
  }
  fprintf(stderr, "Tota number of frames decoded: %d\n", i);

  free_av_objects(&fileformatctx, &datapacket, &dataframe, &streamcodectx);
  return 0;
}
