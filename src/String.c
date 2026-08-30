//
// created by souranil on 1/10/2026
//
#include "String.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "logging_str.h"

String * init_string(){
  return (String *)calloc(1, sizeof(String));
}

String * init_string_from_src(const char *src){
  String *result=(String *)calloc(1, sizeof(String));
  if (result==NULL){
    fprintf(stderr, "%sError allocating new string object.Error: %s\n", ERROR_LOG_STR, strerror(errno));
    return NULL;
  }
  result->str=strdup(src);
  if (result->str==NULL){
    fprintf(stderr, "%sError duplicating source string. Error: %s\n", ERROR_LOG_STR, strerror(errno));
    free(result);
    return NULL;
  }
  result->len=strlen(src);
  return result; 
}

void deinit_string(String **obj){
  free((*obj)->str);
  free(*obj);
  *obj=nullptr;
}
