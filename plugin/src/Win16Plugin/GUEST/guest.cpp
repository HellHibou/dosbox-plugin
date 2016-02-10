/**
 * \brief Integration's tool windows 16 bits guest application.
 * \author Jeremy Decker
 * \version 0.3
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

const char AppName[] = "GuestWin16";
const char AppTitle [] = "DosBox Integration Tool";
IntegrationToolGuest integrationTool = IntegrationToolGuest();
char timerLock = 0;
unsigned short oldMouseParam [3] = { 0, 0, 0 };
static HWND hwndNextClpViewer;
static HWND winHwnd;

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

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
	ChangeClipboardChain (winHwnd, hwndNextClpViewer);
}

void ShutdownRequest() {
	ExitWindows(0,0);
}
static BOOL clipboardLock = FALSE;
void SetCliboardContent () {
	clipboardLock = TRUE;
	integrationTool.ReceptClipboardData((void *)winHwnd);
	clipboardLock = FALSE;
}

void Initialize(HINSTANCE hinst) {
	//////////////////////////////////////////////////////////
	// Initialize application and unused window (required for Windows interactions)
	//////////////////////////////////////////////////////////
	WNDCLASS  wndclass;
	if (!GetClassInfo(hinst, AppName, &wndclass)) {
		wndclass.style         = 0;
		wndclass.lpfnWndProc   = WndProc ;
		wndclass.cbClsExtra    = 0 ;
		wndclass.cbWndExtra    = 0 ;
		wndclass.hInstance     = hinst ;
		wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
		wndclass.hbrBackground = 0;
		wndclass.lpszMenuName  = NULL ;
		wndclass.lpszClassName = AppName ;
	}

	if (!RegisterClass (&wndclass)) {
		MessageBox (NULL, "Can't register Window handle",  AppTitle, MB_ICONHAND | MB_OK) ;
		exit (2);
	}

	winHwnd = CreateWindow (AppName, AppTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hinst, NULL) ;

	//////////////////////////////////////////////////////////
	// Read configuration file
	//////////////////////////////////////////////////////////
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

	//////////////////////////////////////////////////////////
	// Initialize integration tools.
	//////////////////////////////////////////////////////////
	IntegrationToolGuest::Exception * e = integrationTool.Connect(ioPort);
	if (e) {
		MessageBox(NULL, e->getMessage(), AppTitle, MB_ICONHAND | MB_OK);
		exit (e->getCode());
	}

	integrationTool.defineSetMousePos(SetCursorPos, VM_CALL_FLAG_16BITS | VM_CALL_FLAG_PASCAL);
	integrationTool.defineShutdownRequest(ShutdownRequest, VM_CALL_FLAG_16BITS | VM_CALL_FLAG_C);
	integrationTool.defineSetCliboardContent(SetCliboardContent, VM_CALL_FLAG_16BITS);
	integrationTool.InitHost(INTEGRATION_TOOL_GUEST_ID);
	atexit(beforeExit);

	// Initialize clipboard Hook
	hwndNextClpViewer = SetClipboardViewer (winHwnd);

	// For fluid mouse movement, set mouse speed to low value...
	SystemParametersInfo(SPI_GETMOUSE, 0, &oldMouseParam, 0);
	unsigned short mouseParam [3] = { 6, 10, 1 };
	SystemParametersInfo(SPI_SETMOUSE, 0, &mouseParam, 0);


	// Enable host communication
	SetTimer(NULL, 0, 50,(TIMERPROC) timer);
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {

		////////////////////////////////////////////
		// Transfert clipboard from guest to host
		////////////////////////////////////////////
		case WM_CHANGECBCHAIN:
			if ((HWND) wParam == hwndNextClpViewer) { hwndNextClpViewer = (HWND) lParam; }
			else if (hwndNextClpViewer) { SendMessage (hwndNextClpViewer, message, wParam, lParam); }
			return 0;

		case WM_DRAWCLIPBOARD: if (clipboardLock == FALSE) {

			integrationTool.SendClipboardData ((void*)hwnd);
			if (hwndNextClpViewer) { SendMessage (hwndNextClpViewer, message, wParam, lParam) ; }
			return 0;
		}
		////////////////////////////////////////////

	 }
	 return DefWindowProc (hwnd, message, wParam, lParam) ;
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

