//
// Created by souranil on 7/22/25.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "audio.h"

int main(int argc, char  *argv[]){
  char *track_name;
  if (argc<2){
    fprintf(stderr, "Provide track name to test :");
    track_name=calloc(256, sizeof(char));
    if (track_name==NULL){
      fprintf(stderr, "Error in calloc\n");
      return 1;
    }
    scanf("%s", track_name); 
  }else{
    track_name=argv[1];
  }
  if(play(track_name)==0){
    fprintf(stderr, "Success in reading and decoding the whole file\n");
  }else{
    fprintf(stderr, "Failure in reading and decoding the whole file\n");
  }
  return 0;
}
