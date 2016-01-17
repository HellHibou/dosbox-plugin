/**
 * \brief Integration's tool windows 16 bits guest application.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */

#define STRICT
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vmitg.hpp"
#include "cfg.hpp"

#define INTEGRATION_TOOL_GUEST_ID "WIN16  "

const char AppTitle [] = "DosBox Integration Tool";
IntegrationToolGuest integrationTool = IntegrationToolGuest();
char timerLock = 0;
unsigned short oldMouseParam [3] = { 0, 0, 0 };


void CALLBACK timer(HWND /*hwnd*/, UINT /*uMsg*/, UINT /*timerId*/, DWORD /*dwTime*/ ) {
	if (timerLock == 0) {
		timerLock = 1;
		integrationTool.TimerRequest();
		timerLock = 0;
	}
}

void beforeExit() {
	KillTimer(NULL, NULL);
	integrationTool.Disconnect();
	SystemParametersInfo(SPI_SETMOUSE, 0, &oldMouseParam, 0);
}

void ShutdownRequest() {
	ExitWindows(0,0);
}

void Initialize(HINSTANCE hinst)  {
	// Read configuration file
	char * filename = (char*) malloc(2048);
	GetModuleFileName (hinst, filename, 2048);
	strcpy (filename + strlen (filename) - 3, "cfg");

	Cfg * cfg = new Cfg();
	cfg->load(filename);
	free(filename);

	const char * strIoPort = cfg->getValue("port");
	int ioPort = 0;

	if (strIoPort == NULL) { ioPort = INTEGRATION_TOOL_DEFAULT_IO_PORT; }
	else if (strIoPort[0] == 0x00) { ioPort = INTEGRATION_TOOL_DEFAULT_IO_PORT; }
	else {
		ioPort = strtol(strIoPort, NULL, 0);
		if (ioPort == 0 || ioPort > 0xFFFF) { ioPort = INTEGRATION_TOOL_DEFAULT_IO_PORT; }
	}

	delete cfg;

	// Initialisation
	IntegrationToolGuest::Exception * e = integrationTool.Connect(ioPort);
	if (e) {
		MessageBox(NULL, e->getMessage(), AppTitle, MB_ICONHAND | MB_OK);
		exit (e->getCode());
	}

	integrationTool.defineSetMousePos(SetCursorPos, VM_CALL_FLAG_16BITS | VM_CALL_FLAG_PASCAL);
	integrationTool.defineShutdownRequest(ShutdownRequest, VM_CALL_FLAG_16BITS | VM_CALL_FLAG_C);
	integrationTool.InitHost(INTEGRATION_TOOL_GUEST_ID);

	// For fluid mouse movement, set mouse speed to low value...
	SystemParametersInfo(SPI_GETMOUSE, 0, &oldMouseParam, 0);
	unsigned short mouseParam [3] = { 6, 10, 1 };
	SystemParametersInfo(SPI_SETMOUSE, 0, &mouseParam, 0);
	////////////////////////////////////////////////////////////

	atexit(beforeExit);
 	SetTimer(NULL, 0, 50,(TIMERPROC) timer);
}

int PASCAL WinMain (HINSTANCE hinst, HINSTANCE prev_inst, LPSTR /*cmdline*/, int /*cmdshow*/) {
	MSG msg;

	if (prev_inst != NULL) {
		MessageBox(NULL, "A instance is already running", AppTitle, MB_ICONHAND | MB_OK);
		return 1;
	}

	Initialize(hinst);

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

