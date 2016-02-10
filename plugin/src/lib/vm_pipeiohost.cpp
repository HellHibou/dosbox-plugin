/**
 * \brief Virtual machine's host pipe I/O library.
 * \author Jeremy Decker
 * \version 0.1
 * \date 04/01/2016
 */

#include <stdio.h>
#include <stdlib.h>
#include "vm_pipeiohost.hpp"

namespace vm {
	static char magic [] = "VMP\0";

	PipeIoHost::PipeIoHost() {
		writed = 0;
		readed = 0;
		dataToRead  = 0;
		dataToWrite = 0;
		maxDataReadSize = 0;
		dataRead  = NULL;
		dataWrite = NULL;
	}

	void PipeIoHost::setBlocRead(void * data, unsigned short dataSize) {
		dataRead = (unsigned char *)data;
		maxDataReadSize = dataSize;
		readed = 0;
	}

	void PipeIoHost::setBlocWrite(void * data, unsigned short dataSize) {
		dataWrite = (unsigned char *)data;
		dataToWrite = dataSize;
		writed = 0;
	}

	void PipeIoHost::setRawBlocWrite(void * data, unsigned short dataSize) {
		dataWrite = (unsigned char *)data;
		dataToWrite = dataSize;
		writed = 6;
	}

	void PipeIoHost::read(unsigned int val, unsigned short iolen) {
		while (iolen > 0) { 
			if (readed > 5) {
				if (readed - 6 > maxDataReadSize) { 
					readed++;
					if (readed - 6 > dataToRead) {
						readed = 0;
						dataToRead = 0;
					}
					return; 
				}

				dataRead[readed-6] = val & 0xFF;
				if (readed - 5 == dataToRead) {
					this->onDataBlockReaded((void*)dataRead, dataToRead);
					readed = 0;
					dataToRead = 0;
				}
			} else if (readed < 4) {
				if ((val & 0xFF) != magic [readed]) {
					readed = 0;
					dataToRead = 0;
				}
			} else if (readed == 4) {
				dataToRead = val & 0xFF;
			} else /*if (readed == 5)*/ {
				dataToRead |= val << 8;
				if (dataRead == NULL) { 
					dataRead = (unsigned char *)malloc(dataToRead); 
					maxDataReadSize = dataToRead;
				}
			}

			val >>= 8;
			iolen--;
			readed++;
		}
	}

	unsigned int PipeIoHost::write(unsigned short iolen) {
		unsigned int retVal = 0;
		if (dataWrite == NULL) { return 0; }
		while (iolen > 0) {
			retVal <<= 8;
			if (writed > 5) {
				retVal |= dataWrite[writed-6];
				if (writed - 5 == dataToWrite) { 
					dataToWrite = 0;
					dataWrite = NULL;
					writed = 0;
					this->onDataBlockWrited();
					return retVal;
				}
			} else if (writed < 4) {
				retVal |= magic [writed];
			} else if (writed == 4) {
				retVal |= dataToWrite >> 8;
			} else if (writed == 5) {
				retVal |= dataToWrite & 0xFF;
			}
			writed++;
			iolen--;
		}
		return retVal;
	}
};
