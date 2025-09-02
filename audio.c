/*
* This file contains all the audio playback related functionalities
*/
#define _GNU_SOURCE
#include "audio.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

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

int8_t read_dir_contents(const char *dirname, DIRENTRY *contents /* This value should be passed as not initiated */){
  DIR *directory;
  if((directory=opendir(dirname))==NULL){
    switch(errno){
      case EACCES: fprintf(stderr, "Unable to open the audio directory :%s => Permission denied\n", dirname);
                   break;
      case EMFILE: fprintf(stderr, "Unable to open the audio directory :%s => Permission denied\n", dirname);
                   break;
      case ENOMEM: fprintf(stderr, "Unable to open the audio directory :%s => Permission denied\n", dirname);
                   break;
    }
    return -1;
  }
  
  contents->count=0;
  errno=0;
  fprintf(stderr, "The directory contents from the DIR stream=>\n");
  for(contents->count=0; errno==0;contents->count++){
    struct dirent *dircontents= readdir(directory);
    if (errno==0){
      fprintf(stderr, "%s\n", dircontents->d_name);
      
      if(contents->direntrynames==NULL){
        contents->direntrynames=calloc(256, sizeof(char));
      }else{
        contents->direntrynames=realloc(contents->direntrynames, (contents->count+1)*256*sizeof(char));
      }
      (contents->direntrynames)=strcpy(contents->direntrynames+(contents->count*256), dircontents->d_name);
      contents->count++;
    }else{
      fprintf(stderr, "Error code: %s\n", strerror(errno));
    }
  }

  //print out the names stored in the direntryname
  fprintf(stderr, "The directory contents=>\n");
  for(int i=0;i<sizeof(contents->direntrynames);i++){
    if (contents->direntrynames[i]=='\0'){
      fprintf(stderr, "\n");
    }else{
      fprintf(stderr, "%c\n", contents->direntrynames[i]);
    }
  }
  
  return contents->count;
}

void init_audio(char *dir){

}
