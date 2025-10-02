#include "serial_communication.h"
#include <memory>
#include <string>

int main(int argc, char *argv[]){
	if (argc<0){
		std::perror("Please provide atleast one argument\n");
		return 1;
	}
	std::unique_ptr<Serial_port> comm_port=std::make_unique<Serial_port>(std::string(argv[1]));
	return 0;
}
