/**
 * \brief Virtual machine's guest pipe I/O library.
 * \author Jeremy Decker
 * \version 0.3
 * \date 04/01/2016
 */
#pragma once
#ifndef __JREKCED_VMIOG_HPP__
#define __JREKCED_VMIOG_HPP__

/**
 * \brief Pipe betwen host and guest OS for guest.
 */
class PipeIoGuest {
	private:
		unsigned short port; /**< Hardware port number. */

	public:
		/**
		 * \param port Hardware port number used to communicate with host. 
		 */
		PipeIoGuest(unsigned short port);

		/**
		 * \brief Send formated data block to virtual machine's plugin.
		 * \param port Hardware port number used to communicate with host. 
		 * \param data Data's pointer to send.
		 * \param size Data size. 
		 */
		static volatile void WriteBlock (unsigned short port, void * data, unsigned short size);

		/**
		 * \brief Read formated data block from virtual machine's plugin.
		 * \param port Hardware port number used to communicate with host. 
		 * \param data Data's pointer to recieve.
		 * \param size Maximum data size. 
		 * \return If communication error, return negative value; else return recieved data size, if returned value > size, recieved data is truncate.
		 */
		static volatile int ReadBlock (unsigned short port, void * data, unsigned short size);

		/**
		 * \brief Send un-formated data block to virtual machine's plugin.
		 * \param port Hardware port number used to communicate with host. 
		 * \param data Data's pointer to send.
		 * \param size Data size. 
		 */
		static volatile void Write (unsigned short port, void * data, unsigned int size);

		/**
		 * \brief Send data block to virtual machine's plugin.
		 * \param data Data's pointer to send.
		 * \param size Data size. 
		 */
		inline void writeBlock (void * data, unsigned short size) { WriteBlock(port, data, size); }


		/**
		 * \brief Read data block from virtual machine's plugin.
		 * \param data Data's pointer to recieve.
		 * \param size Maximum data size. 
		 * \return If communication error, return negative value; else return recieved data size, if returned value > size, recieved data is truncate.
		 */
		inline int readBlock (void * data, unsigned short size) { return ReadBlock(port, data, size); }
};

#endif
