/*
* This file contains all the audio playback related functionalities
*/
#define _GNU_SOURCE
#include "dirutil.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


StringArray read_dir_contents(const char *dirname){
  /*
   * 
   */

  StringArray result={.array=nullptr, .count=0};
  DIR *directory;
  struct dirent *content;

  if((directory=opendir(dirname))==NULL){
    switch(errno){
      case EACCES: fprintf(stderr, "Unable to open the audio directory :%s => Permission denied\n", dirname);
                   break;
      case ENOMEM: fprintf(stderr, "Unable to open the audio directory :%s => No Memory\n", dirname);
                   break;
      case ENOTDIR: fprintf(stderr, "Unable to open the audio directory :%s => Given address doesn't point to a directory\n", dirname);
                   break;
    }
    return result;
  }

  while((content=readdir(directory))!=nullptr){
    if (strcmp(content->d_name, ".")==0 || strcmp(content->d_name, "..")==0){ // Ignore . and .. entries in a unix directory
      continue;
    }
    
    if(result.array==NULL){
      result.array=calloc(result.count+1, sizeof(char *));
      if (!result.array){
        fprintf(stderr, "Error allocating memory for the result buffer\n");
        break;
      }
    }else{
      result.array=reallocarray(result.array, result.count+1, sizeof(char *));
      if (!result.array){
        fprintf(stderr, "Error allocating more memory for the result buffer\n");
        break;
      }
    }
    result.array[result.count]=calloc(strlen(content->d_name), sizeof(char));
    if (result.array[result.count]==NULL){
      break;
    }
    strcpy(result.array[result.count], content->d_name);
    result.count+=1;
  }

  closedir(directory);
  fprintf(stderr, "Total length of the resulting array containing all the directory names: %d\n", result.count);
  return result;//Return the array of strings containing all the names of the directory entire seperated by newline
}

void free_direntry_obj(StringArray *obj){
  for(int i=0;i<obj->count;i++){
    free(obj->array[i]);
  }
  free(obj->array);
  free(obj);
}
