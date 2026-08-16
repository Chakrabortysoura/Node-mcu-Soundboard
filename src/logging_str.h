// Created by souranil on 15-07-2026

#ifndef LOGGING_STR_H 
#define LOGGING_STR_H 

#include <stdint.h>
#include <stdio.h>

/*
 * Return the string "INFO" colored with ansi escape code to appear blue in 
 * error logs.
 */
static inline char * info_log_str(){
  return "\x1b[38;2;3;186;252m[INFO] \x1b[0m";
}

/*
 * Return the string "ERROR" colored with ansi escape code to appear red in 
 * error logs.
 */
static inline char * error_log_str(){
  return "\x1b[38;2;245;145;32m[ERROR] \x1b[0m";
}

/*
* Given the r,g,b values this function returns the ansi function wrapped str representation of the given 
* str string. buffer should be adequitely sized to hold the resulting str. This function internally uses snprintf
* to be safe from buffer overflows
*/
static inline void change_str_color(uint8_t r,uint8_t g,uint8_t b,char *buffer, size_t len, const char *str){
  snprintf(buffer, len, "\x1b[38;2;%d;%d;%dm%s\x1b[0m", r,g,b,str);
}

#endif
