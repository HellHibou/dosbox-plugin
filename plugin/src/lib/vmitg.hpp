/**
 * \brief Virtual machine's integration tool guest.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */

#pragma once

#ifndef __JREKCED_VMITG_HPP__
#define __JREKCED_VMITG_HPP__

#include "vmiog.hpp"
#include "vmitt.h"

#define __JREKCED_VMITG_HPP_SET_FCT__(FCT)                                    \
inline void define ## FCT (void * function, unsigned short flags) {           \
	dataBlock.data.initGuest.stdFct. ## FCT ## .guestPtr.pointer = function;  \
	dataBlock.data.initGuest.stdFct. ## FCT ## .flags = flags;                \
}

/** \brief Virtual machine's integration tool guest. */
class IntegrationToolGuest {
	private:
		PipeIoGuest *      io;
		DataTransfertBlock dataBlock;

	public:
		/** Virtual machine's integration tool exception. */
		class Exception {
			private:
				int    code;
				char * msg;

			public:
				/**
				 * \param code Error code.
				 * \parm message Error message.
				 */
				Exception(int code, char * message, ...);

				~Exception();

				/**
				 * \brief Get error code.
				 * \return Error code.
				 */
				int getCode();

				/**
				 * \brief Get error message.
				 * \return Error message.
				 */
				const char * getMessage(); 
		};

		IntegrationToolGuest();

		/**
		 * \brief Connect to host integration tool.
		 * \param ioPort I/O port to use (default value = INTEGRATION_TOOL_DEFAULT_IO_PORT).
		 * \return exception if error or NULL.
		 */
		IntegrationToolGuest::Exception * Connect(unsigned short ioPort = INTEGRATION_TOOL_DEFAULT_IO_PORT);

		/**
		 * \brief Disconnect from host integration tool.
		 */
		void Disconnect();

		/**
		 * \brief Send initialisation request to host. Use all IntegrationToolGuest.define??? functions before call this.
		 */
		void InitHost(const char guestId[8]);

		/**
		 * \brief Send timer request to host.
		 */
		inline void TimerRequest() { io->writeBlock (&dataBlock, 2); }	

		/**
		 * \fn defineSetMousePos
		 * \brief Define 'SetMousePos' function called by host integration tool.
		 * \param function Pointer to function.
		 * \param flags Call flags.
		 */
		__JREKCED_VMITG_HPP_SET_FCT__(SetMousePos);

		/**
		 * \fn defineShutdownSystem
		 * \brief Define 'ShutdownSystem' function called by host integration tool.
		 * \param function Pointer to function.
		 * \param flags Call flags.
		 */
		__JREKCED_VMITG_HPP_SET_FCT__(ShutdownSystem);
};

#undef __JREKCED_VMITG_HPP_SET_FCT__

#endif
