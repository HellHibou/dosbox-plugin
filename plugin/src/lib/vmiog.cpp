/**
 * \brief Virtual machine's guest pipe I/O library.
 * \author Jeremy Decker
 * \version 0.3
 * \date 04/01/2016
 */

#include <conio.h>
#include "vmiog.hpp"

#if defined(__WATCOMC__) || defined(_MSC_VER)
#  define inport(px)     inpw(px)
#  define inportb(px)    inp(px)
#  define outport(px,w)  outpw(px,w)
#  define outportb(px,b) outp(px,b)
#endif

static const char sign[] = "VMP\0";

PipeIoGuest::PipeIoGuest(unsigned short myPort) {
	port = myPort;
}

volatile void PipeIoGuest::WriteBlock (unsigned short port, void * data, unsigned short size) {
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

volatile void PipeIoGuest::Write (unsigned short port, void * data, unsigned int size) {
	char * ptr = (char *)data;
	while (size > 0) {
		outportb (port, *ptr);
		size --;
		ptr++;
	};
};

volatile int PipeIoGuest::ReadBlock(unsigned short port, void * data, unsigned short size) {
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


volatile void PipeIoGuest::Read(unsigned short port, void * data, unsigned short size) {
	char *ptr =(char *) data;
	while (size > 0) {
		*ptr = inportb (port);
		ptr++;
		size --;
	}
}
