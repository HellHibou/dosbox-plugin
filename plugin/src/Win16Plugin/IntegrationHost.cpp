/**
 * \file IntegrationHost.cpp
 * \brief Virtual machine's integration tool host.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */

#include "IntegrationHost.hpp"
#include <time.h>
#include <Windows.h>

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

#define INTEGRATION_TOOL_RESET() \
	setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2); \
	initialized = false; \
	callable = false;

namespace vm {
	IntegrationTool::IntegrationTool(vm::type::VirtualMachine * myVM) {
		initialized = false;
		callable = false;
		virtualMachine = myVM;

		memset(&guest, sizeof(guest), 0);
		argsSetCursorPos.x = 32765;
		argsSetCursorPos.y = 32765;
		mouseMoved = true;

		writeBlock.function = INTEGRATION_TOOL_FCT_INIT;
		memcpy(&writeBlock.data.initHost.magic, INTEGRATION_TOOL_MAGIC, sizeof(writeBlock.data.initHost.magic));
		writeBlock.data.initHost.majorVersion = INTEGRATION_TOOL_MAJOR_VERSION;
		writeBlock.data.initHost.minorVersion = INTEGRATION_TOOL_MINOR_VERSION;
		
		int mouseInfo [3];

		SystemParametersInfo(SPI_GETMOUSE, 0, &mouseInfo, 0);    
		writeBlock.data.initHost.systemMouseInfo[0] = mouseInfo [0];
		writeBlock.data.initHost.systemMouseInfo[1] = mouseInfo [1];
		writeBlock.data.initHost.systemMouseInfo[2] = mouseInfo [2];

		setBufferRead(&readBlock);
		setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
	}

	void IntegrationTool::onDataBlockReaded(void * data, unsigned short dataSize) {
		switch (((vm::type::DataTransfertBlock *)data)->function) {

			// Initialize integration tool
			case INTEGRATION_TOOL_FCT_INIT: 
				if (dataSize < (sizeof(vm::type::DataTransfertBlock::Data::InitGuest) + 2)) {
					#ifdef _DEBUG
						virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Integration tool : Communication error - Invalid data size.");
					#endif

					INTEGRATION_TOOL_RESET();
				}
			
				if (strncmp (((vm::type::DataTransfertBlock *)data)->data.initGuest.magic, INTEGRATION_TOOL_MAGIC, sizeof(writeBlock.data.initGuest.magic) != 0)) { 
					#ifdef _DEBUG
						char buffer [128];

						sprintf(buffer, "Integration tool : Communication error - Invalid protocol.",
							((vm::type::DataTransfertBlock *)data)->data.initGuest.majorVersion,
							((vm::type::DataTransfertBlock *)data)->data.initGuest.minorVersion);

						virtualMachine->logMessage(VMHOST_LOG_DEBUG, buffer);
					#endif

					INTEGRATION_TOOL_RESET();
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
				
					INTEGRATION_TOOL_RESET();
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

					INTEGRATION_TOOL_RESET();
					return;
				}

				guest = ((vm::type::DataTransfertBlock *)data)->data.initGuest;

				setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);






				initialized = true;
				callable = true;

			#ifdef _DEBUG
				virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Integration tool initialized.");
			#endif
				break;
 
			// Un-initialize integration tool
			case INTEGRATION_TOOL_FCT_UNINIT:
				INTEGRATION_TOOL_RESET();
			#ifdef _DEBUG
				virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Integration tool un-initialized.");
			#endif
				break;
			
			case INTEGRATION_TOOL_FCT_TIMER:
				if (mouseMoved) {
					virtualMachine->callGuestFct(guest.SetMousePos.pointer.word[1], guest.SetMousePos.pointer.word[0], guest.SetMousePos.flags, 2, (unsigned short*) &argsSetCursorPos);
					mouseMoved = false;
				}
				break;

		#ifdef _DEBUG
			// Unnkow function
			default: {
				char buffer [128];
				sprintf (buffer,  "Integration tool : Invalid function code (0x%X)", ((vm::type::DataTransfertBlock *)data)->function);
				virtualMachine->logMessage(VMHOST_LOG_DEBUG, buffer);
			}
		#endif
		}
	}

	void IntegrationTool::onDataBlockWrited() {	
		setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2); 
	}
}
