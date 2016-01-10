/**
 * \brief Integration's tool windows 16 bits guest application.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */
#include <windows.h>
#include <stdio.h>
#include "vmitg.hpp"

#define INTEGRATION_TOOL_GUEST_ID "WIN16  "

const char AppTitle [] = "DosBox Integration Tool";
IntegrationToolGuest integrationTool = IntegrationToolGuest();
char timerLock = 0;
unsigned short oldMouseParam [3] = { 0, 0, 0 };

void beforeExit() {
	KillTimer(NULL, NULL);
	integrationTool.Disconnect();
	SystemParametersInfo(SPI_SETMOUSE, 0, &oldMouseParam, 0);
}

void CALLBACK timer(HWND /*hwnd*/, UINT /*uMsg*/, UINT /*timerId*/, DWORD /*dwTime*/ ) {
	if (timerLock) { return; }
	timerLock = 1;
	integrationTool.TimerRequest();
	timerLock = 0;
}

void ShutdownSystem() {
	beforeExit();
	ExitWindows(0,0);
}

int PASCAL WinMain (HINSTANCE /*hinst*/, HINSTANCE prev_inst, LPSTR /*cmdline*/, int /*cmdshow*/) {
	MSG msg;

	if (prev_inst != NULL) {
		MessageBox(NULL, "A instance is already running", AppTitle, MB_ICONHAND | MB_OK);
		return 1;
	}

	IntegrationToolGuest::Exception * e = integrationTool.Connect();
	if (e) {
		MessageBox(NULL, e->getMessage(), AppTitle, MB_ICONHAND | MB_OK);
	  	return e->getCode();
	}

	SystemParametersInfo(SPI_GETMOUSE, 0, &oldMouseParam, 0);
	unsigned short mouseParam [3] = { 0, 0, 0 };
	SystemParametersInfo(SPI_SETMOUSE, 0, &mouseParam, 0);

	integrationTool.defineSetMousePos(SetCursorPos, VM_CALL_FLAG_16BITS | VM_CALL_FLAG_PASCAL);
	integrationTool.defineShutdownSystem(ShutdownSystem, VM_CALL_FLAG_16BITS | VM_CALL_FLAG_C);
	integrationTool.InitHost(INTEGRATION_TOOL_GUEST_ID);

	SetTimer(NULL, 0, 10,(TIMERPROC) timer);

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	 }

	beforeExit ();
	return 0;
}

