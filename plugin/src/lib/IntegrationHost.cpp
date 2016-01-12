/**
 * \brief Virtual machine's integration tool host.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */

#include "IntegrationHost.hpp"

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

namespace vm {
	void IntegrationToolHost::clear() {
		shutdownRequest = false;
		memset(&guestFct, 0, sizeof(guestFct));
		argsSetCursorPos.x = 32765;
		argsSetCursorPos.y = 32765;
		mouseMoved = true;

		writeBlock.function = INTEGRATION_TOOL_FCT_INIT;
		memcpy(&writeBlock.data.initHost.magic, INTEGRATION_TOOL_MAGIC, sizeof(writeBlock.data.initHost.magic));
		writeBlock.data.initHost.majorVersion = INTEGRATION_TOOL_MAJOR_VERSION;
		writeBlock.data.initHost.minorVersion = INTEGRATION_TOOL_MINOR_VERSION;

		setBufferRead(&readBlock);
		setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
	};

	IntegrationToolHost::IntegrationToolHost(vm::type::VirtualMachine * myVM) {
		virtualMachine = myVM;
		clear();
	}

	void IntegrationToolHost::onDataBlockReaded(void * data, unsigned short dataSize) {
		switch (((vm::type::DataTransfertBlock *)data)->function) {

			//////////////////////////////////////////////////////////////
			// Initialize integration tool
			//////////////////////////////////////////////////////////////
			case INTEGRATION_TOOL_FCT_INIT: { 
				clear();

				if (dataSize < (sizeof(vm::type::DataTransfertBlock::Data::InitGuest) + 2)) {
					#ifdef _DEBUG
						virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Integration tool : Communication error - Invalid data size.");
					#endif

					setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
				}
			
				if (strncmp (((vm::type::DataTransfertBlock *)data)->data.initGuest.magic, INTEGRATION_TOOL_MAGIC, sizeof(writeBlock.data.initGuest.magic) != 0)) { 
					#ifdef _DEBUG
						char buffer [128];

						sprintf(buffer, "Integration tool : Communication error - Invalid protocol.",
							((vm::type::DataTransfertBlock *)data)->data.initGuest.majorVersion,
							((vm::type::DataTransfertBlock *)data)->data.initGuest.minorVersion);

						virtualMachine->logMessage(VMHOST_LOG_DEBUG, buffer);
					#endif

					setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
					return;
				}

				if (((vm::type::DataTransfertBlock *)data)->data.initGuest.majorVersion != INTEGRATION_TOOL_MAJOR_VERSION) {
					#ifdef _DEBUG
						char buffer [128];

						sprintf(buffer, "Integration tool : Communication error - Unsupported protocol version : %i.%i",
							((vm::type::DataTransfertBlock *)data)->data.initGuest.majorVersion,
							((vm::type::DataTransfertBlock *)data)->data.initGuest.minorVersion);

						virtualMachine->logMessage(VMHOST_LOG_DEBUG, buffer);
					#endif
				
					setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
					return;
				}

				if (((vm::type::DataTransfertBlock *)data)->data.initGuest.minorVersion > INTEGRATION_TOOL_MINOR_VERSION) {
					#ifdef _DEBUG
						char buffer [128];

						sprintf(buffer, "Integration tool : Communication error - Unsupported protocol version : %i.%i",
							((vm::type::DataTransfertBlock *)data)->data.initGuest.majorVersion,
							((vm::type::DataTransfertBlock *)data)->data.initGuest.minorVersion);

						virtualMachine->logMessage(VMHOST_LOG_DEBUG, buffer);
					#endif

					setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
					return;
				}

				int copySize = ((vm::type::DataTransfertBlock *)data)->data.initGuest.guestFunctionHandlesCount * sizeof (vm::type::StdGuestFunctionHandles);
				
				if (copySize > sizeof (vm::type::StdGuestFunctionHandles)) {
					copySize = sizeof (vm::type::StdGuestFunctionHandles);
				} else if (copySize < sizeof (vm::type::StdGuestFunctionHandles)) {
					memset(&guestFct, 0, sizeof (vm::type::StdGuestFunctionHandles));
				}

				memcpy (&guestFct, &((vm::type::DataTransfertBlock *)data)->data.initGuest.stdFct, copySize);
				setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);

			#ifdef _DEBUG
				virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Integration tool initialized.");
			#endif
			} break;
 

			//////////////////////////////////////////////////////////////
			// Un-initialize integration tool
			//////////////////////////////////////////////////////////////
			case INTEGRATION_TOOL_FCT_UNINIT:
				clear();

			#ifdef _DEBUG
				virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Integration tool un-initialized.");
			#endif
				break;
			

			//////////////////////////////////////////////////////////////
			// Synchronize host ansd guest
			//////////////////////////////////////////////////////////////
			case INTEGRATION_TOOL_FCT_SYNC: 
				if (mouseMoved && guestFct.SetMousePos.guestPtr.dword != NULL) {
					virtualMachine->callGuestFct(guestFct.SetMousePos.guestPtr.word[1], guestFct.SetMousePos.guestPtr.word[0], guestFct.SetMousePos.flags, 2, (unsigned short*) &argsSetCursorPos);
					mouseMoved = false;
				}
				if (shutdownRequest) {
					shutdownRequest = false;

					if (guestFct.ShutdownRequest.guestPtr.dword != NULL) {
						virtualMachine->callGuestFct(guestFct.ShutdownRequest.guestPtr.word[1], guestFct.ShutdownRequest.guestPtr.word[0], guestFct.ShutdownRequest.flags, 0, NULL);
					}
				}
				break;


			//////////////////////////////////////////////////////////////
			// Unnkow function
			//////////////////////////////////////////////////////////////
		#ifdef _DEBUG
			default: {
				char buffer [128];
				sprintf (buffer,  "Integration tool : Invalid function code (0x%X)", ((vm::type::DataTransfertBlock *)data)->function);
				virtualMachine->logMessage(VMHOST_LOG_DEBUG, buffer);
			}
		#endif
		}
	}

	void IntegrationToolHost::onDataBlockWrited() {	
		setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2); 
	}

	bool IntegrationToolHost::ShutdownRequest () {
		if (guestFct.ShutdownRequest.guestPtr.dword != NULL) {
			shutdownRequest = true;
			return true;
		} else {
			return false;
		}
	}
}
