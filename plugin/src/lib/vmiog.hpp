/**
 * \brief Virtual machine's guest pipe I/O library.
 * \author Jeremy Decker
 * \version 0.1
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
		PipeIoGuest(unsigned int port);

		/**
		 * \brief Send data block to virtual machine's plugin.
		 * \param data Data's pointer to send.
		 * \param size Data size. 
		 */
		void writeBlock (void * data, unsigned short size);

		/**
		 * \brief Read data block from virtual machine's plugin.
		 * \param data Data's pointer to recieve.
		 * \param size Maximum data size. 
		 * \return If communication error, return negative value; else return recieved data size, if returned value > size, recieved data is truncate.
		 */
		int readBlock (void * data, unsigned short size);
};

#endif
