//
// created by souranil on 1/10/2026
//

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "String.h"

String * init_string(){
  return (String *)calloc(1, sizeof(String));
}

String * init_string_from_src(const char *src){
  String *result=(String *)calloc(1, sizeof(String));
  if (result==NULL){
    fprintf(stderr, "Error allocating new string object.Error: %s\n", strerror(errno));
    return NULL;
  }
  result->str=strdup(src);
  if (result->str==NULL){
    fprintf(stderr, "Error duplicating source string. Error: %s\n", strerror(errno));
    free(result);
    return NULL;
  }
  result->size=strlen(src)+10;
  result->len=result->size-10;
  return result; 
}
