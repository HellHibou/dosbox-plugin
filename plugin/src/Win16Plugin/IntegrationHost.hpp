/**
 * \file IntegrationHost.hpp
 * \brief Virtual machine's integration tool host.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */
#pragma once

#include <string.h>
#include "../lib/vm_host.h"
#include "../lib/vm_utils.hpp"

namespace vm {
	namespace type {
		#include "VMITPT.h"
	};

	/** \brief Virtual machine's integration tool host. */
	class IntegrationTool : public vm::PipeIoHost  {
		private:
			#pragma pack(push, 2)
			struct SeMousePosArgs {
				short x;
				short y;
			};
			#pragma pack(pop)

			bool initialized;
			bool callable;
			vm::type::VirtualMachine * virtualMachine;
			vm::type::DataTransfertBlock readBlock;
			vm::type::DataTransfertBlock writeBlock;
			
			bool mouseMoved;
			vm::type::DataTransfertBlock::Data::InitGuest guest;
			SeMousePosArgs argsSetCursorPos;

			vm::type::GuestHandle guestShutdownSystem;

		protected:
			void onDataBlockReaded(void * data, unsigned short dataSize);
			void onDataBlockWrited();

		public:
			/**
			 * \param virtualMachine Virtual machine to use.
			 */
			IntegrationTool(vm::type::VirtualMachine * virtualMachine);

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

			/**
			 * \brief Shutdown guest system.
			 */
			inline void ShutdownSystem () {
				if (guest.ShutdownSystem.pointer.guestPtr != NULL) {
					virtualMachine->callGuestFct(guest.ShutdownSystem.pointer.word[1], guest.ShutdownSystem.pointer.word[0], guest.ShutdownSystem.flags, 0, NULL);
				}
			}
	};
};
