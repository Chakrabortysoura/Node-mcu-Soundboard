/*
 * Serial Port communiction functionalities
*/
#define _GNU_SOURCE

#include "serial_com.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  // For using File control related macros and functions in glibc
#include <unistd.h> // For unix specific functionalities from glibc
#include <errno.h>  // For Error handling
#include <string.h> 
#include <sys/ioctl.h>
#include <termios.h>

int init_serial_port(const char *serial_port_path){
  /*
   * Function to initialize the Serial Port for communication with the ESP8266.
   * Given the path of the serial port(eg- /dev/ttyUSB0) this function obtains the serial port file descriptor.
   * Configure the termios structure for the serial port.
   * Assigns the new control flags for the serial port and return the serial port file descriptor to the calling function for reading the data 
   * from the serial port.
  */

  const int serial_port_fd=open(serial_port_path ,O_RDONLY|O_NONBLOCK|O_NOCTTY); // File descriptor obtained for the usb to com port used for communicating with the nodemcu
  if (serial_port_fd<=0) { // Exit the programme when the serial port file descriptor couldn't be obtained
    perror(strerror(errno));
    exit(1);
  }
  struct termios port_config;
  if (tcgetattr(serial_port_fd, &port_config)==-1){// Get the termios struct associated with serial port
    perror(strerror(errno));
    exit(1);
  }
  
  port_config.c_iflag&= ~(INPCK); // Configure the control flag values in for the serial port file descriptor
  port_config.c_cflag|= (CLOCAL|CREAD|CS8);
  port_config.c_lflag&= ~ICANON;

  cfsetspeed(&port_config, B115200);

  if (tcsetattr(serial_port_fd, TCSANOW, &port_config )==-1){// Set the new serial port config
    perror(strerror(errno));
    exit(1);
  }
  if (tcflush(serial_port_fd, TCIOFLUSH)!=0){ //Flush any previous data in the input buffer for the serial port
    perror("Serial port queue flush Error");
    exit(1);
  }
  
  return serial_port_fd;
}

int close_serial_port(const int serial_port_fd){
  /*
   * Closing the opened serial port file descriptor when no longer
   * needed for readign serial port input data
  */

  return close(serial_port_fd);
}
