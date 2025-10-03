/*
* This header file provides functionalities for reading serial port data coming from the NodeMCU
*/
#pragma once
#include <string>
#include <termios.h>

class Serial_port{
private:
	std::string port;
	struct termios serial_port_config;
	int port_fd;
	int8_t buffer;
public: 
	Serial_port(const std::string port_name);
	int8_t Read();
};
