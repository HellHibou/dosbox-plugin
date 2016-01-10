/**
 * \brief Virtual machine's integration tool host.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */
#pragma once
#ifndef __JREKCED_INTEGRATION_TOOL_HPP__
#define __JREKCED_INTEGRATION_TOOL_HPP__

#include <string.h>
#include "vm_host.h"
#include "vm_pipeiohost.hpp"

namespace vm {
	namespace type {
		#include "vmitt.h"
	};

	/** \brief Virtual machine's integration tool host. */
	class IntegrationToolHost : public vm::PipeIoHost  {
		private: 			
			vm::type::DataTransfertBlock     readBlock;
			vm::type::DataTransfertBlock     writeBlock;

		protected:
			#pragma pack(push, 2)
			/** \brief SetMousePos call arguments and mouse pointer position. */
			struct SeMousePosArgs {
				short x; /**< \brief Mouse cursor X position. */
				short y; /**< \brief Mouse cursor Y position. */
			} argsSetCursorPos;
			#pragma pack(pop)

			bool mouseMoved; /**< \brief true if host mouse cursor moved. */

			vm::type::VirtualMachine * virtualMachine; /**< \brief Host virtual machine. */
			vm::type::StdGuestFunctionHandles guestFct; /**< Guest standard functions. */ 

			void onDataBlockReaded(void * data, unsigned short dataSize);
			void onDataBlockWrited();

		public:
			/**
			 * \param virtualMachine Virtual machine to use.
			 */
			IntegrationToolHost(vm::type::VirtualMachine * virtualMachine);

			/** \brief Shutdown guest system. */
			void ShutdownSystem ();

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
