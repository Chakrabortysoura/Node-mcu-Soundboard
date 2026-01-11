#ifndef STRING_H
#define STRING_H

#include <stdlib.h>

/*
* An incomplete implementation of string objects to use in replacement of c style char strings.
* This implementation is only to be used for this project so there maybe some missing functionalities.
*/
typedef struct{
  size_t len, size;
  char *str;
}String;

String * init_string();

String * init_string_from_src(const char *src);

#endif
