/**
 * \file guest.h
 * \brief Integration's tool windows 16 bits guest application.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>
#include "../../lib/vm_guest.hpp"
#include "../VMITPT.h"

#define INTEGRATION_TOOL_GUEST_ID "WIN16  "

const char AppTitle [] = "DosBox Integration Tool";
PipeIoGuest * io = NULL;
DataTransfertBlock dataBlock;
char timerLock = 0;

void beforeExit() {
	KillTimer(NULL, NULL);
	dataBlock.function = INTEGRATION_TOOL_FCT_UNINIT;
	io->write (&dataBlock, 2);

}

void CALLBACK timer(HWND /*hwnd*/, UINT /*uMsg*/, UINT /*timerId*/, DWORD /*dwTime*/ ) {
	if (timerLock) { return; }
	timerLock = 1;
	io->write (&dataBlock, 2);
	timerLock = 0;
}

void ShutdownSystem() {
	beforeExit();
	ExitWindows(0,0);
}

int PASCAL WinMain (HINSTANCE /*hinst*/, HINSTANCE prev_inst, LPSTR /*cmdline*/, int /*cmdshow*/) {
	if (prev_inst != NULL) {
		MessageBox(NULL, "A instance is already running", AppTitle, MB_ICONHAND | MB_OK);
		return -1;
	}

	io = new PipeIoGuest (INTEGRATION_TOOL_IO_PORT);
	int readedSize = io->read (&dataBlock, sizeof(DataTransfertBlock));
	MSG msg;

	if (readedSize < 7) {
		char buffer [128];
		sprintf(buffer, "Invalid port number : 0x%X", INTEGRATION_TOOL_IO_PORT);
		MessageBox(NULL, buffer, AppTitle, MB_ICONHAND | MB_OK);
		return -2;
	}

	if (strncmp (dataBlock.data.initHost.magic, INTEGRATION_TOOL_MAGIC, sizeof(INTEGRATION_TOOL_MAGIC)) != 0) {
		MessageBox(NULL, "Invalid protocol", AppTitle, MB_ICONHAND | MB_OK);
		return -3;
	}

	if (dataBlock.data.initHost.majorVersion != INTEGRATION_TOOL_MAJOR_VERSION) {
		char buffer [128];
		sprintf(buffer, "Unsupported protocol version : %i.%i",  dataBlock.data.initHost.majorVersion, dataBlock.data.initHost.minorVersion);
		MessageBox(NULL, buffer, AppTitle, MB_ICONHAND | MB_OK);
		return -4;
	}

	if (dataBlock.data.initHost.minorVersion < INTEGRATION_TOOL_MINOR_VERSION) {
		char buffer [128];
		sprintf(buffer, "Unsupported protocol version : %i.%i",  dataBlock.data.initHost.majorVersion, dataBlock.data.initHost.minorVersion);
		MessageBox(NULL,buffer, AppTitle, MB_ICONHAND | MB_OK);
		return -4;
	}

	if (readedSize < sizeof(DataTransfertBlock::Data::InitHost) + 2) {
		MessageBox(NULL, "Invalid data protocol", AppTitle, MB_ICONHAND | MB_OK);
		return -5;
	}

	SystemParametersInfo(SPI_SETMOUSE, 0, dataBlock.data.initHost.systemMouseInfo, 0);

	dataBlock.function = 0;
	strcpy (dataBlock.data.initGuest.magic, INTEGRATION_TOOL_MAGIC);
	strcpy (dataBlock.data.initGuest.guestId, INTEGRATION_TOOL_GUEST_ID);
	dataBlock.data.initGuest.majorVersion = INTEGRATION_TOOL_MAJOR_VERSION;
	dataBlock.data.initGuest.minorVersion = INTEGRATION_TOOL_MINOR_VERSION;

	dataBlock.data.initGuest.SetMousePos.pointer.guestPtr = SetCursorPos;
	dataBlock.data.initGuest.SetMousePos.flags = VM_CALL_FLAG_PASCAL;

	dataBlock.data.initGuest.ShutdownSystem.pointer.guestPtr = ShutdownSystem;
	dataBlock.data.initGuest.ShutdownSystem.flags = VM_CALL_FLAG_C;

	io->write (&dataBlock, sizeof(DataTransfertBlock::Data::InitGuest) + 2);

	dataBlock.function = 2;
	SetTimer(NULL, 0, 10,(TIMERPROC) timer);

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	 }

	beforeExit ();
	return 0;
}

