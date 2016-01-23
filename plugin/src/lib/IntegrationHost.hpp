/**
 * \brief Virtual machine's integration tool host.
 * \author Jeremy Decker
 * \version 0.3
 * \date 09/01/2016
 */
#pragma once
#ifndef __JREKCED_INTEGRATION_TOOL_HPP__
#define __JREKCED_INTEGRATION_TOOL_HPP__

#include <string.h>
#include "vm_host.h"
#include "vm_pipeiohost.hpp"

#ifdef WIN32
	#include <Windows.h>
#endif

namespace vm {
	namespace type {
		#include "vmitt.h"
	};

	/** \brief Virtual machine's integration tool host. */
	class IntegrationToolHost : public vm::PipeIoHost  {
		private: 			
			vm::type::DataTransfertBlock     readBlock;
			vm::type::DataTransfertBlock     writeBlock;
			unsigned char     readType;
			unsigned long int dataReaded;

		#ifdef WIN32
			HGLOBAL hClipboardBuffer;
			char *  clipboardBuffer;
		#endif

			void clear();

		protected:
			#pragma pack(push, 2)
			/** \brief SetMousePos call arguments and mouse pointer position. */
			struct SeMousePosArgs {
				short x; /**< \brief Mouse cursor X position. */
				short y; /**< \brief Mouse cursor Y position. */
			} argsSetCursorPos;
			#pragma pack(pop)

			bool mouseMoved; /**< \brief true if host mouse cursor moved. */
			bool shutdownRequest;
			vm::type::VirtualMachine *        virtualMachine; /**< \brief Host virtual machine. */
			vm::type::StdGuestFunctionHandles guestFct;       /**< \brief Guest standard functions. */ 
			vm::type::ClipboardBlocHeader     clipboardBloc; 

			void onDataBlockReaded(void * data, unsigned short dataSize);
			void onDataBlockWrited();

			void readClipboard (unsigned int val, unsigned short iolen);

		public:
			/**
			 * \param virtualMachine Virtual machine to use.
			 */
			IntegrationToolHost(vm::type::VirtualMachine * virtualMachine);

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
			 * \brief Send Shutdown guest system request. 
			 * \return true If integration tool can send Shutdown signal to guest. 
			 */
			bool ShutdownRequest ();

			/**
			 * \brief Set guest mouse position.
			 * \param x New X mouse position.
			 * \param y New Y mouse position.
			 */
			inline void SetMousePos(int x, int y) {
				if (argsSetCursorPos.x != x || argsSetCursorPos.y != y) {
					argsSetCursorPos.x = x;
					argsSetCursorPos.y = y;
					mouseMoved = true;
				}
			}
	};
};

#endif
