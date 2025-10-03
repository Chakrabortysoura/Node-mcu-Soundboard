#include "serial_communication.h"
#include <memory>
#include <string>

int main(int argc, char *argv[]){
	if (argc<0){
		std::perror("Please provide atleast one argument\n");
		return 1;
	}
	std::unique_ptr<Serial_port> comm_port=std::make_unique<Serial_port>(std::string(argv[1]));
	std::printf("Input coming from the serial port=>\n");
	for(auto i=0;i<10;i++){//Read the input from the serial device 10 times
		int8_t data=comm_port->Read();
		if (data<0){
			break;
		}
		std::printf("%d\n", data-int8_t('0'));
	}
	return 0;
}
