/**
 * \brief Virtual machine's host pipe I/O library.
 * \author Jeremy Decker
 * \version 0.1
 * \date 04/01/2016
 */
#pragma once

#include <stdio.h>
#include "vm_host.h"

namespace vm {
	/**
     * \brief Pipe betwen host (Dosbox plugin) and guest OS (emulated DOS, Windows) for host.
     */
	class PipeIoHost {
		private:
			unsigned short  readed;
			unsigned short  writed;
			unsigned short  dataToRead;
			unsigned short  dataToWrite;
			unsigned char * dataRead;
			unsigned char * dataWrite;
			unsigned short  maxDataReadSize; 

		protected:
			PipeIoHost();
			
			/**
			 * \brief Called when data read buffer is fully readed by guest.
			 * \param data Pointer to recieved data.
			 * \param dataSize Recieved data size, if returned value > size, recieved data is truncate.
			 */
			virtual void onDataBlockReaded(void * data, unsigned short dataSize) = NULL;
			
			/**
			 * \brief Called when data write buffer is fully writted by guest.
			 */
			virtual void onDataBlockWrited() = NULL;
			
			/**
			 * \brief Set data buffer to read from guest.
			 * \param data Data's pointer to recieve.
			 * \param maxDataSize Maximum data size.
			 */
			void setBufferRead(void * data, unsigned short maxDataSize);
			
			/**
			 * \brief Set data buffer to read from guest.
			 * \param data Data's pointer to recieve.
			 */
			template <typename T> inline void setBufferRead(T * data) {
				setBufferRead((void*)data, sizeof(T));
			}

			/**
			 * \brief Set data buffer to send to guest.
			 * \param data Data's pointer to send.
			 * \param dataSize Size of data to send.
			 */
			void setBufferWrite(void * data, unsigned short dataSize);

			/**
			 * \brief Set data buffer to send to guest.
			 * \param data Data's pointer to send.
			 */
			template <typename T> inline void setBufferWrite(T * data) {
				setBufferWrite((void*)data, sizeof(T)); 
			}

		public:
			/**
			 * \brief Used into a vm::type::IoOutputHandle function to read data from guest :
	         *
             * vm::PipeIoHost pipeIoHost;
			 * void io_write(unsigned int port,unsigned int val, unsigned int iolen) { 
			 *		pipeIoHost.read(val, iolen);
			 * }
 	         *
     	     * \param val Value send by guest.
             * \param iolen Size of value in bytes (1, 2 or 4 bytes).
			 */
			void read (unsigned int val, unsigned short iolen);

			/**
             * \brief Used into a vm::type::IoInputHandle function to send data to guest :
 	         *
  	         * vm::PipeIoHost pipeIoHost;
 	         * unsigned int io_read(unsigned int port, unsigned int iolen) { 
 	         *		return pipeIoHost.write(iolen);
 	         * }
 	         *
		     * \param iolen Size of value in bytes (1, 2 or 4 bytes).
             * \return Value to send to guest.
			 */
			unsigned int write(unsigned short iolen);
	};
};
