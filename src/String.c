//
// created by souranil on 1/10/2026
//

#include <stdlib.h>
#include <string.h>

#include "String.h"

String * init_string(){
  return (String *)calloc(1, sizeof(String));
}

String * init_string_from_src(const char *src){
  String *result=(String *)calloc(1, sizeof(String));
  if (result!=NULL){
    result->size=strlen(src)+10;
  }
  result->str=(char *)calloc(result->size,sizeof(char));
  if (result->str!=NULL){
    strcpy(result->str, src);
    result->len=result->size-10;
  }else{
    // Incase of actual string buffer allocation fails due to some internal error with calloc.
    // In that situation free the object and return null to indicate to the user that the allocation failed.
    free(result);
    result=NULL;
  }
  return result; 
}
