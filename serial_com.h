#ifndef SERIAL_COM_H
#define SERIAL_COM_H

int init_serial_port(const char *serial_port_path);

int close_serial_port(const int serial_port_fd);

#endif
