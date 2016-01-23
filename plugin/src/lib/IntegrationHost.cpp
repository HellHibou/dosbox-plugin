/**
 * \brief Virtual machine's integration tool host.
 * \author Jeremy Decker
 * \version 0.3
 * \date 09/01/2016
 */

#include "IntegrationHost.hpp"

#ifdef WIN32
	#include <Windows.h>
#endif

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

#define READ_TYPE_DEFAULT           0
#define READ_TYPE_CLIPBOARD         1
#define READ_TYPE_CLIPBOARD_CONTENT 2

namespace vm {
	void IntegrationToolHost::clear() {
		shutdownRequest = false;
		memset(&guestFct, 0, sizeof(guestFct));
		argsSetCursorPos.x = 32765;
		argsSetCursorPos.y = 32765;
		mouseMoved = true;
		readType = READ_TYPE_DEFAULT;
		dataReaded = 0;


		if (hClipboardBuffer) {
			GlobalFree(hClipboardBuffer);
			hClipboardBuffer = NULL;
		}

		writeBlock.function = INTEGRATION_TOOL_FCT_INIT;
		memcpy(&writeBlock.data.initHost.magic, INTEGRATION_TOOL_MAGIC, sizeof(writeBlock.data.initHost.magic));
		writeBlock.data.initHost.majorVersion = INTEGRATION_TOOL_MAJOR_VERSION;
		writeBlock.data.initHost.minorVersion = INTEGRATION_TOOL_MINOR_VERSION;

		setBufferRead(&readBlock);
		setBufferWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
	};

	IntegrationToolHost::IntegrationToolHost(vm::type::VirtualMachine * myVM) {
		virtualMachine = myVM;
		hClipboardBuffer = NULL;
		clipboardBuffer = NULL;
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
			// Transfert clipboard from guest to host.
			//////////////////////////////////////////////////////////////
			case INTEGRATION_TOOL_FCT_SET_CLIPBOARD_CONTENT:
				#ifdef _DEBUG
					virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Start clipboard transfert from guest.");
				#endif

				readType = READ_TYPE_CLIPBOARD;
				dataReaded = 0;
				OpenClipboard(NULL);
				EmptyClipboard();
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

	void IntegrationToolHost::read (unsigned int val, unsigned short iolen) {
		switch (readType) {
			case READ_TYPE_DEFAULT:
				vm::PipeIoHost::read(val, iolen);
				break;

			//////////////////////////////////////////////////////////////
			// Transfert clipboard from guest to host.
			//////////////////////////////////////////////////////////////
			case READ_TYPE_CLIPBOARD:
				strncpy(((char*) &clipboardBloc)+dataReaded, (char*)&val, iolen);
				dataReaded += iolen;

				if (dataReaded >= sizeof(clipboardBloc)) {
					if (clipboardBloc.dataSize == 0L) {
						if (clipboardBloc.contentType == 0) {
							CloseClipboard();
							readType = READ_TYPE_DEFAULT; 

						#ifdef _DEBUG
							virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Clipboard transfered complete.");
						#endif
						} else {
							SetClipboardData(clipboardBloc.contentType, NULL);
						#ifdef _DEBUG
							char * msgBuffer = (char*) malloc (128);
							sprintf(msgBuffer, "Set clipboard data type=%i size=%i", (int)clipboardBloc.contentType, clipboardBloc.dataSize);
							virtualMachine->logMessage(VMHOST_LOG_DEBUG, msgBuffer);
							free(msgBuffer);
						#endif
						}
						dataReaded = 0;	
						return;
					}

				#ifdef WIN32
					hClipboardBuffer = GlobalAlloc(GMEM_MOVEABLE, clipboardBloc.dataSize);
					if (!hClipboardBuffer) {
						clipboardBuffer = NULL;
						return;
					}		

					clipboardBuffer = (char*)GlobalLock(hClipboardBuffer);
				#endif
					dataReaded = 0;
					readType = READ_TYPE_CLIPBOARD_CONTENT; 
					return;
				}
				break;

			case READ_TYPE_CLIPBOARD_CONTENT: // Read clipboard item
				#ifdef WIN32
					strncpy(clipboardBuffer, (char*) &val, iolen);
					clipboardBuffer+=iolen;
				#endif

				dataReaded += iolen;
				if (dataReaded >= clipboardBloc.dataSize) { 
					#ifdef WIN32
							GlobalUnlock(hClipboardBuffer);
							SetClipboardData(clipboardBloc.contentType, hClipboardBuffer);
							GlobalFree(hClipboardBuffer); 
							hClipboardBuffer = NULL;
					
						#ifdef _DEBUG
							char * msgBuffer = (char*) malloc (128);
							sprintf(msgBuffer, "Set clipboard data : content-id=%i, content-size=%i", (int)clipboardBloc.contentType, clipboardBloc.dataSize);
							virtualMachine->logMessage(VMHOST_LOG_DEBUG, msgBuffer);
							free(msgBuffer);
						#endif
					#endif
					readType = READ_TYPE_CLIPBOARD; 
					dataReaded = 0;
				}
				break;
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
