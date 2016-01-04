/**
 * \file vm_guest.cpp
 * \brief Guest virtual machine library.
 * \author Jeremy Decker
 * \version 0.1
 * \date 04/01/2016
 */

#include "../../lib/vm_guest.hpp"
#include <dos.h>

static const char sign[] = "VMP\0";

PipeIoGuest::PipeIoGuest(unsigned int myPort) {
	port = myPort;
}

void PipeIoGuest::write (void * data, unsigned short size) {
	outport (port, *(short *) sign);
	outport (port, ((short *) sign)[1]);
	outport (port, size);
	char * ptr = (char *)data;
	while (size > 0) {
		outportb (port, *ptr);
		size --;
		ptr++;
	};
};

int PipeIoGuest::read(void * data, unsigned short size) {
	int index = 0;
	while (index < 4)
	{
		if (inportb(port) != sign[index]) {  
			return -1;
		}

		index++;
	}
		
	unsigned short dataSize = inport(port);
	if (size > dataSize) { index = dataSize; } else { index = size; }
	char * ptr = (char *)data;

	while (index > 0) {
		*(char*)ptr = inportb (port);
		ptr ++;
		index --;
	};
	
	if (dataSize > size ) {
		index = dataSize - size;
		while (index > 0) {
			 inportb (port);
			 index--;
		}
	}

	return dataSize;
}
