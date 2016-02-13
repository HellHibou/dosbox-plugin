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

#define WRITE_TYPE_DEFAULT           0
#define WRITE_TYPE_CLIPBOARD         1
#define WRITE_TYPE_CLIPBOARD_CONTENT 2

namespace vm {

	IntegrationToolHost::IntegrationToolHost(vm::type::VirtualMachine * myVM
	#ifdef WIN32
		, HWND    myHwnd
	#endif
	) {
		virtualMachine = myVM;
		hClipboardReadBuffer = NULL;
		clipboardReadBuffer = NULL;

		#ifdef WIN32
			hwnd = myHwnd;
			maxClipboardTransfertSize = DEFAULT_MAX_CLIPBOARD_TRANSFERT_SIZE;
		#endif

		clear();
	}
		
	void IntegrationToolHost::clear() {
		memset(&guestFct, 0, sizeof(guestFct));
		memset(&eventFlags, 0 , sizeof(eventFlags));
		argsSetCursorPos.x = 32765;
		argsSetCursorPos.y = 32765;
		eventFlags.shutdownRequest = false;
		eventFlags.mouseMoved = true;
		eventFlags.sendClipboardToGuest = false; 
		readType = READ_TYPE_DEFAULT;
		dataReaded = 0;
		writeType = WRITE_TYPE_DEFAULT;

		if (hClipboardReadBuffer) {
			GlobalFree(hClipboardReadBuffer);
			hClipboardReadBuffer = NULL;
		}

		if (hClipboardWriteBuffer) {
			GlobalFree(hClipboardWriteBuffer);
			hClipboardWriteBuffer = NULL;
		}

		writeBlock.function = INTEGRATION_TOOL_FCT_INIT;
		memcpy(&writeBlock.data.initHost.magic, INTEGRATION_TOOL_MAGIC, sizeof(writeBlock.data.initHost.magic));
		writeBlock.data.initHost.majorVersion = INTEGRATION_TOOL_MAJOR_VERSION;
		writeBlock.data.initHost.minorVersion = INTEGRATION_TOOL_MINOR_VERSION;

		setBlocRead(&readBlock);
		setBlocWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
	};

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

					setBlocWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
				}
			
				if (strncmp (((vm::type::DataTransfertBlock *)data)->data.initGuest.magic, INTEGRATION_TOOL_MAGIC, sizeof(writeBlock.data.initGuest.magic) != 0)) { 
					#ifdef _DEBUG
						char buffer [128];

						sprintf(buffer, "Integration tool : Communication error - Invalid protocol.",
							((vm::type::DataTransfertBlock *)data)->data.initGuest.majorVersion,
							((vm::type::DataTransfertBlock *)data)->data.initGuest.minorVersion);

						virtualMachine->logMessage(VMHOST_LOG_DEBUG, buffer);
					#endif

					setBlocWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
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
				
					setBlocWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
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

					setBlocWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);
					return;
				}

				int copySize = ((vm::type::DataTransfertBlock *)data)->data.initGuest.guestFunctionHandlesCount * sizeof (vm::type::StdGuestFunctionHandles);
				
				if (copySize > sizeof (vm::type::StdGuestFunctionHandles)) {
					copySize = sizeof (vm::type::StdGuestFunctionHandles);
				} else if (copySize < sizeof (vm::type::StdGuestFunctionHandles)) {
					memset(&guestFct, 0, sizeof (vm::type::StdGuestFunctionHandles));
				}

				memcpy (&guestFct, &((vm::type::DataTransfertBlock *)data)->data.initGuest.stdFct, copySize);
				setBlocWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2);

			#ifdef _DEBUG
				virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Integration tool initialized.");
			#endif

				SendClipboardData();
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

				// Synchronize guest and host mouse position.
				if (eventFlags.mouseMoved && guestFct.SetMousePos.guestPtr.dword != NULL) {
					virtualMachine->callGuestFct(guestFct.SetMousePos.guestPtr.word[1], guestFct.SetMousePos.guestPtr.word[0], guestFct.SetMousePos.flags, 2, (unsigned short*) &argsSetCursorPos);
					eventFlags.mouseMoved = false;
				}

				// Send shutdown request.
				if (eventFlags.shutdownRequest) {
					eventFlags.shutdownRequest = false;

					if (guestFct.ShutdownRequest.guestPtr.dword != NULL) {
						virtualMachine->callGuestFct(guestFct.ShutdownRequest.guestPtr.word[1], guestFct.ShutdownRequest.guestPtr.word[0], guestFct.ShutdownRequest.flags, 0, NULL);
					}
				}

				// Send clipboard content to guest.
				if(eventFlags.sendClipboardToGuest) {
					#ifdef WIN32
						if (CountClipboardFormats()) { 
							if (OpenClipboard ((HWND)hwnd)) { 
								#ifdef _DEBUG
									virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Start clipboard transfert from host to guest.");
								#endif

								clipboardWriteBloc.contentType = 0;
								writeBlock.function = INTEGRATION_TOOL_FCT_SET_CLIPBOARD_CONTENT;
								setBlocWrite(&writeBlock, 2);
								writeType = WRITE_TYPE_CLIPBOARD_CONTENT;
								virtualMachine->callGuestFct(guestFct.SetCliboardContent.guestPtr.word[1], guestFct.SetCliboardContent.guestPtr.word[0], guestFct.SetCliboardContent.flags, 0, NULL);
							}
						}
					#endif

					eventFlags.sendClipboardToGuest = false;
				}
				break;


			//////////////////////////////////////////////////////////////
			// Transfert clipboard from guest to host.
			//////////////////////////////////////////////////////////////
			case INTEGRATION_TOOL_FCT_SET_CLIPBOARD_CONTENT:
				#ifdef _DEBUG
					virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Start clipboard transfert from guest to host.");
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
				strncpy(((char*) &clipboardReadBloc)+dataReaded, (char*)&val, iolen);
				dataReaded += iolen;

				if (dataReaded >= sizeof(clipboardReadBloc)) {
					if (clipboardReadBloc.dataSize == 0L) {
						if (clipboardReadBloc.contentType == 0) {
							CloseClipboard();
							readType = READ_TYPE_DEFAULT; 

						#ifdef _DEBUG
							virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Clipboard transfered complete.");
						#endif
						} else {
							SetClipboardData(clipboardReadBloc.contentType, NULL);
						#ifdef _DEBUG
							char * msgBuffer = (char*) malloc (128);
							sprintf(msgBuffer, "Recept clipboard data : content-id=%i, content-size=%i", (int)clipboardReadBloc.contentType, clipboardReadBloc.dataSize);
							virtualMachine->logMessage(VMHOST_LOG_DEBUG, msgBuffer);
							free(msgBuffer);
						#endif
						}
						dataReaded = 0;	
						return;
					}

				#ifdef WIN32
					hClipboardReadBuffer = GlobalAlloc(GMEM_MOVEABLE, clipboardReadBloc.dataSize);
					if (!hClipboardReadBuffer) {
						clipboardReadBuffer = NULL;
						return;
					}		

					clipboardReadBuffer = (char*)GlobalLock(hClipboardReadBuffer);
				#endif
					dataReaded = 0;
					readType = READ_TYPE_CLIPBOARD_CONTENT; 
					return;
				}
				break;

			case READ_TYPE_CLIPBOARD_CONTENT: // Read clipboard item
				#ifdef WIN32
					strncpy(clipboardReadBuffer, (char*) &val, iolen);
					clipboardReadBuffer += iolen;
				#endif

				dataReaded += iolen;
				if (dataReaded >= clipboardReadBloc.dataSize) { 
					#ifdef WIN32
							GlobalUnlock(hClipboardReadBuffer);
							SetClipboardData(clipboardReadBloc.contentType, hClipboardReadBuffer);
							GlobalFree(hClipboardReadBuffer); 
							hClipboardReadBuffer = NULL;
					
						#ifdef _DEBUG
							char * msgBuffer = (char*) malloc (128);
							sprintf(msgBuffer, "Recept clipboard data : content-id=%i, content-size=%i", (int)clipboardReadBloc.contentType, clipboardReadBloc.dataSize);
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
		switch(writeType) {
			case WRITE_TYPE_DEFAULT:
				setBlocWrite(&writeBlock, sizeof(vm::type::DataTransfertBlock::Data::InitHost) + 2); 
				break;
#ifdef WIN32
			case WRITE_TYPE_CLIPBOARD : {
				char * pGlobal = (char *) GlobalLock (hClipboardWriteBuffer);
				writeType = WRITE_TYPE_CLIPBOARD_CONTENT;
				setRawBlocWrite(pGlobal, clipboardWriteBloc.dataSize);
			}
			break;

			case WRITE_TYPE_CLIPBOARD_CONTENT :{
				bool looping = true;

				if (hClipboardWriteBuffer != NULL) {
					GlobalUnlock(hClipboardWriteBuffer);
					hClipboardWriteBuffer = NULL;
				}

				unsigned long dataTransfered = 0;

				while (looping) {
					clipboardWriteBloc.contentType = EnumClipboardFormats(clipboardWriteBloc.contentType); 

					if (clipboardWriteBloc.contentType) {

						if (clipboardWriteBloc.contentType <= CF_GDIOBJLAST) { /**< Skip Non-standard format. */
							hClipboardWriteBuffer = GetClipboardData (clipboardWriteBloc.contentType);

							if (hClipboardWriteBuffer != NULL)  {
								clipboardWriteBloc.dataSize = GlobalSize(hClipboardWriteBuffer);

								dataTransfered += clipboardWriteBloc.dataSize;
								if (clipboardWriteBloc.dataSize > 0 && dataTransfered < maxClipboardTransfertSize) {
									looping = false;
									setRawBlocWrite(&clipboardWriteBloc);
									writeType = WRITE_TYPE_CLIPBOARD;

									#ifdef _DEBUG
											char * msgBuffer = (char*) malloc (128);
											sprintf(msgBuffer, "Send clipboard data : content-id=%i, content-size=%i", clipboardWriteBloc.contentType, clipboardWriteBloc.dataSize);
											virtualMachine->logMessage(VMHOST_LOG_DEBUG, msgBuffer);
											free(msgBuffer);
									#endif
								}
							}
						}
			
					} else { // end of clipboard
						CloseClipboard ();
						writeType = WRITE_TYPE_DEFAULT;
						clipboardWriteBloc.contentType = 0;
						clipboardWriteBloc.dataSize = 0;
						setRawBlocWrite(&clipboardWriteBloc);

						#ifdef _DEBUG
							virtualMachine->logMessage(VMHOST_LOG_DEBUG, "Clipboard transfert completed.");
						#endif

						return;
					}
				}	
			}
		}
#endif
	}

	bool IntegrationToolHost::ShutdownRequest () {
		if (guestFct.ShutdownRequest.guestPtr.dword != NULL) {
			eventFlags.shutdownRequest = true;
			return true;
		} else {
			return false;
		}
	}


	void IntegrationToolHost::SendClipboardData() {
		if (maxClipboardTransfertSize > 0) {
			eventFlags.sendClipboardToGuest = true; 
		}
	}
}
