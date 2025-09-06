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
#include <string.h>

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

DIRENT * read_dir_contents(const char *dirname){
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
    return nullptr;
  }
  
  DIRENT *result=calloc(1, sizeof(DIRENT));
  struct dirent *content;

  while((content=readdir(directory))){
    if (strcmp(content->d_name, ".")==0 || strcmp(content->d_name, "..")==0){ // Ignore . and .. entries in a unix directory
      continue;
    }
    
    if(result->entry_names==NULL){
      result->entry_names=calloc(result->count+1, sizeof(char *));
      if (!result->entry_names){
        fprintf(stderr, "Error allocating memory for the result buffer\n");
        break;
      }
    }else{
      result->entry_names=reallocarray(result->entry_names, result->count+1, sizeof(char *));
      if (!result->entry_names){
        fprintf(stderr, "Error allocating more memory for the result buffer\n");
        break;
      }
    }
    result->entry_names[result->count]=calloc(strlen(content->d_name), sizeof(char));
    if (result->entry_names[result->count]==NULL){
      break;
    }
    strcpy(result->entry_names[result->count], content->d_name);
    result->count+=1;
  }

  closedir(directory);
  fprintf(stderr, "Total length of the resulting array containing all the directory names: %d\n", result->count);
  return result;//Return the char buffer containing all the names of the directory entire seperated by newline
}

void free_direntry_obj(DIRENT **obj){
  for(int i=0;i<(*obj)->count;i++){
    free((*obj)->entry_names[i]);
  }
  free((*obj)->entry_names);
  free(*obj);
}
