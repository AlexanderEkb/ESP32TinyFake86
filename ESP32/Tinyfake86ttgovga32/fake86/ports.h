#ifndef _PORTS_H
#define _PORTS_H

#include <stdint.h>

void set_port_write_redirector(unsigned short int startport, unsigned short int endport, void *callback);
void set_port_read_redirector(unsigned short int startport, unsigned short int endport, void *callback);

void portWriteTiny(uint32_t numPort, unsigned char aValue);
unsigned char portReadTiny(unsigned short int numPort);
#endif
