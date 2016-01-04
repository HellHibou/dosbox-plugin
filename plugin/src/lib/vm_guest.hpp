/**
 * \file vm_guest.h
 * \brief Guest virtual machine library.
 * \author Jeremy Decker
 * \version 0.1
 * \date 04/01/2016
 */
#pragma once

/**
 * \brief Pipe betwen host (Dosbox plugin) and guest OS (emulated DOS, Windows) for guest.
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
		void write (void * data, unsigned short size);

		/**
		 * \brief Read data block from virtual machine's plugin.
		 * \param data Data's pointer to recieve.
		 * \param size Maximum data size. 
		 * \return If communication error, return negative value; else return recieved data size, if returned value > size, recieved data is truncate.
		 */
		int  read  (void * data, unsigned short size);
};
