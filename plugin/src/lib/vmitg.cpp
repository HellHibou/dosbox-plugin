/**
 * \brief Virtual machine's integration tool guest.
 * \author Jeremy Decker
 * \version 0.3
 * \date 09/01/2016
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "vmitg.hpp"
#include "vmitt.h"

#if defined(_WIN32) || defined(_WIN16)
	#include <windows.h>
#endif

IntegrationToolGuest::Exception::Exception(int myCode, char * myMsg, ...) {
	code = myCode;
	if (myMsg == NULL) { 
		msg = NULL;
	} else {
		msg = (char *) malloc(strlen(myMsg) + 128);
		va_list args;
		va_start(args, myMsg);
		vsprintf(msg, myMsg, args);
		va_end(args);
	}
};

IntegrationToolGuest::Exception::~Exception() {
	if (msg != NULL) { free(msg); }
}

int IntegrationToolGuest::Exception::getCode() { return code; }

const char * IntegrationToolGuest::Exception::getMessage() { return msg; }

////////////////////////////////////////////////////////////////////////////////////////

unsigned char IntegrationToolGuest::clipboardLock = 0;

IntegrationToolGuest::IntegrationToolGuest() {
	memset (&dataBlock, sizeof(dataBlock), 0);
	port = 0;

	#if defined(_WIN32) || defined(_WIN16)
		dataBlock.data.initGuest.stdFct.ShutdownRequest.guestPtr.pointer = IntegrationToolGuest::onShutdownRequest; 
		dataBlock.data.initGuest.stdFct.ShutdownRequest.flags = VM_CALL_FLAG_16BITS | VM_CALL_FLAG_C;

		dataBlock.data.initGuest.stdFct.SetMousePos.guestPtr.pointer = SetCursorPos; 
		dataBlock.data.initGuest.stdFct.SetMousePos.flags = VM_CALL_FLAG_16BITS | VM_CALL_FLAG_PASCAL;
	#endif
}

 IntegrationToolGuest::Exception * IntegrationToolGuest::Connect(unsigned short ioPort) {

	if (ioPort == 0) { return new IntegrationToolGuest::Exception(-1, "Port number cannot be null."); }
	if (port != 0)   { return new IntegrationToolGuest::Exception(-2, "Connection already open."); }

	DataTransfertBlock initDataBlock;
	memset (&dataBlock, sizeof(dataBlock), 0);
	port = 0;

	int readedSize = PipeIoGuest::ReadBlock (ioPort, &initDataBlock, sizeof(DataTransfertBlock));

	if (readedSize < 7) {
		return new Exception(1, "Invalid port number : 0x%X", ioPort);
	}

	if (strncmp (initDataBlock.data.initHost.magic, INTEGRATION_TOOL_MAGIC, sizeof(INTEGRATION_TOOL_MAGIC)) != 0) {
		return new Exception(2, "Invalid protocol");
	}

	if (initDataBlock.data.initHost.majorVersion != INTEGRATION_TOOL_MAJOR_VERSION) {
		return new Exception(3, "Unsupported protocol version : %i.%i",  initDataBlock.data.initHost.majorVersion, initDataBlock.data.initHost.minorVersion);
	}

	if (initDataBlock.data.initHost.minorVersion < INTEGRATION_TOOL_MINOR_VERSION) {
		return new Exception(3, "Unsupported protocol version : %i.%i",  initDataBlock.data.initHost.majorVersion, initDataBlock.data.initHost.minorVersion);
	}

	if (readedSize < sizeof(DataTransfertBlock::Data::InitHost) + 2) {
		return new Exception(4, "Invalid data protocol");
	}

	port = ioPort;
	return NULL;
};

void IntegrationToolGuest::Disconnect() {
	if (port == 0) { return; }
	dataBlock.function = INTEGRATION_TOOL_FCT_UNINIT;
	PipeIoGuest::WriteBlock (port, &dataBlock, 2);
	port = 0;
}

void IntegrationToolGuest::InitHost(const char guestId[8]) {
	if (port == 0) { return; }

	dataBlock.function = INTEGRATION_TOOL_FCT_INIT;
	strcpy (dataBlock.data.initGuest.magic, INTEGRATION_TOOL_MAGIC);
	strcpy (dataBlock.data.initGuest.guestId, guestId);
	dataBlock.data.initGuest.majorVersion = INTEGRATION_TOOL_MAJOR_VERSION;
	dataBlock.data.initGuest.minorVersion = INTEGRATION_TOOL_MINOR_VERSION;
	dataBlock.data.initGuest.guestFunctionHandlesCount = INTEGRATION_TOOL_COUNT_STD_GUEST_FUNCTIONS;
	PipeIoGuest::WriteBlock(port, &dataBlock, sizeof(DataTransfertBlock::Data::InitGuest) + 2);

	dataBlock.function = INTEGRATION_TOOL_FCT_SYNC;
};

#if defined(_WIN32) || defined(_WIN16)

void IntegrationToolGuest::onShutdownRequest() {
	ExitWindows(0,0);
}

unsigned char IntegrationToolGuest::SendClipboardData(void * hwnd) {
	if (clipboardLock == 1) { return 0; }
	if (CountClipboardFormats() == 0) { return 1;}
	if (OpenClipboard ((HWND)hwnd) == FALSE) { return 1; }

	dataBlock.function = INTEGRATION_TOOL_FCT_SET_CLIPBOARD_CONTENT;
	PipeIoGuest::WriteBlock(port, &dataBlock, 2);
	ClipboardBlocHeader clpBloc;
	clpBloc.contentType = EnumClipboardFormats(0);

 	while (clpBloc.contentType) {
		if (clpBloc.contentType <= CF_GDIOBJLAST) { /**< Skip Non-standard format. */
			HGLOBAL hGlobal = GetClipboardData (clpBloc.contentType);

			if (hGlobal != NULL)  {
				char * pGlobal = (char *) GlobalLock (hGlobal);
				clpBloc.dataSize = GlobalSize(hGlobal);

				if (clpBloc.dataSize > 0) {
					PipeIoGuest::Write(port, &clpBloc, sizeof(ClipboardBlocHeader));
					PipeIoGuest::Write(port, pGlobal, clpBloc.dataSize);
				}

				GlobalUnlock(hGlobal);
			}
		}

		clpBloc.contentType = EnumClipboardFormats(clpBloc.contentType); 
	}

	clpBloc.contentType = 0;
	clpBloc.dataSize = 0;
	PipeIoGuest::Write(port, &clpBloc, sizeof(ClipboardBlocHeader));
	CloseClipboard ();
	dataBlock.function = INTEGRATION_TOOL_FCT_SYNC;
	return 1;
};


void IntegrationToolGuest::ReceptClipboardData(void * hwnd) {
	ClipboardBlocHeader blocHeader;
	unsigned char clipOpened;
	clipboardLock = 1;

/////////////////////
	unsigned int fct;
	PipeIoGuest::ReadBlock(port, &fct, 2);
//*///////////////////

	if (OpenClipboard ((HWND)hwnd) == FALSE) {
		clipOpened = FALSE;
	} else {
		clipOpened = TRUE;
		EmptyClipboard();
	}

	PipeIoGuest::Read(port, &blocHeader, sizeof (ClipboardBlocHeader));

	while(blocHeader.contentType != 0 && blocHeader.dataSize != 0) {
		if (blocHeader.dataSize > 0) {
			HGLOBAL hClipboardReadBuffer = GlobalAlloc(GMEM_MOVEABLE, blocHeader.dataSize);
			char * clipboardBuffer = (char*) GlobalLock(hClipboardReadBuffer);
			PipeIoGuest::Read(port, clipboardBuffer, blocHeader.dataSize);
			GlobalUnlock(hClipboardReadBuffer);

			if (clipOpened == TRUE) {
				SetClipboardData(blocHeader.contentType, hClipboardReadBuffer);
			} else {
				GlobalFree(hClipboardReadBuffer);
			}
		}

		PipeIoGuest::Read(port, &blocHeader, sizeof (ClipboardBlocHeader));
	}

	CloseClipboard ();
	clipboardLock = 0;
}

#endif
