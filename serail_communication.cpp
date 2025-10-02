/*
 * Implementation of the serial communication class defined int the header
*/
#include "serial_communication.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>

Serial_port::Serial_port(const std::string port_name){
	// Initializes the serial_port with the given serial port address and initialized the termios structure 
	port=port_name;	
	port_fd=open(port.c_str(), O_RDONLY|O_NONBLOCK|O_NOCTTY);
	if (port_fd<=0) { // Exit the programme when the serial port file descriptor couldn't be obtained
		std::perror(std::strerror(errno));
		exit(1);
	}

	if (tcgetattr(port_fd, &serial_port_config)==-1){// Get the termios struct associated with serial port
	   perror(strerror(errno));
	    exit(1);
	}
  
	serial_port_config.c_iflag&= ~(INPCK); // Configure the control flag values in for the serial port file descriptor
	serial_port_config.c_cflag|= (CLOCAL|CREAD|CS8);
	serial_port_config.c_lflag&= ~ICANON;

	cfsetspeed(&serial_port_config, B115200);

	if (tcsetattr(port_fd, TCSANOW, &serial_port_config)==-1){// Set the new serial port config
		std::perror(std::strerror(errno));
		exit(1);
	}

	if (tcflush(port_fd, TCIOFLUSH)!=0){ //Flush any previous data in the input buffer for the serial port
		std::perror("Serial port queue flush Error");
		exit(1);
	}
}

int8_t Serial_port::Read(const int8_t n_bytes){
	return n_bytes;
}

