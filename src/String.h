//
// Created by souranil on 16/1/2026
//
#ifndef STRING_H
#define STRING_H

#include <stdlib.h>

/*
* An incomplete implementation of string objects to use in replacement of c style char strings.
* This implementation is only to be used for this project so there maybe some missing functionalities.
*/
typedef struct{
  size_t len;
  char *str;
}String;

/*
  * Allocate an empty string object. 
  * The internal char array remains uinitialized.
  */
String * init_string();

/*
  * Allocate a new string object from an existring char array. 
  * This src char array must be Null terminated.
  * Incase of failure to initialize the String object return NULL
  */
String * init_string_from_src(const char *src);

/*
  * Deinitialize a string object.
  */
void deinit_string(String *str);

#endif
