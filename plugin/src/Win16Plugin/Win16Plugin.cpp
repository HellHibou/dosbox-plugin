/**
 * \brief Win16Plugin DLL entries.
 * \author Jeremy Decker
 * \version 0.3
 * \date 14/11/2015
 */

#include <string.h>
#include <sys/stat.h>
#include "../CommonPlugin/plugin_common.hpp"
#include "IntegrationHost.hpp"

const char PLUGIN_INTRO [] = "Win16 Plugin version 0.3\nCopyright 2015-2016 Hell Hibou";

#ifdef WIN32
	const char APP_NAME [] = "DosBox Win16 Plugin";
#endif

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif


struct Win16Instance /**< Plugin's instance. */
{
	Instance    common;
	vm::IntegrationToolHost * integrationTool;
	vm::type::VirtualMachine * vm;

	#ifdef WIN32
		HWND hwndNextClpViewer;
		HWND hwnd;
	#endif
};

extern const int InstanceSize = sizeof(Win16Instance);

static void mouseHnd(Win16Instance * instance, int x, int y) {
	instance->integrationTool->SetMousePos(x, y);
}

static void io_write(Win16Instance * instance, unsigned int val, unsigned int iolen) { 
	instance->integrationTool->read(val, iolen);
}

static unsigned int io_read(Win16Instance * instance, unsigned int iolen) { 
	return instance->integrationTool->write(iolen);
}


#ifdef WIN32
LRESULT CALLBACK EventManager (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {

		////////////////////////////////////////////
		// Transfert clipboard from guest to host
		////////////////////////////////////////////
		case WM_CHANGECBCHAIN: {
			Win16Instance * instance = (Win16Instance *) GetWindowLongPtr (hwnd, GWLP_HINSTANCE);
			if ((HWND) wParam == instance->hwndNextClpViewer) { 
				instance->hwndNextClpViewer = (HWND) lParam; 
			} else if (instance->hwndNextClpViewer) { 
				SendMessage (instance->hwndNextClpViewer, message, wParam, lParam); 
			}
			return 0;
		}

		case WM_DRAWCLIPBOARD: {
			if (wParam != NULL) {
				Win16Instance * instance = (Win16Instance *) GetWindowLongPtr (hwnd, GWLP_HINSTANCE);
				instance->integrationTool->SendClipboardData();
				if (instance->hwndNextClpViewer) { SendMessage (instance->hwndNextClpViewer, message, wParam, lParam) ; }
			}
			return 0;
		}

		////////////////////////////////////////////
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}
#endif

LIBRARY_API int VMPLUGIN_PreInit(vm::type::VirtualMachine * vm, void * myInstance) {
	if (vm == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	if (vm->structSize < sizeof(vm::type::VirtualMachine)) { return VM_ERROR_BAD_STRUCT_SIZE; }
	Win16Instance * instance = (Win16Instance*)myInstance;

	vm->logMessage(VMHOST_LOG_CONSOLE, "---");
	vm->logMessage(VMHOST_LOG_CONSOLE, PLUGIN_INTRO);
	vm->logMessage(VMHOST_LOG_CONSOLE, "---");

	instance->vm = vm;

	#ifdef WIN32
		WNDCLASS wndclass;

		if (!GetClassInfo((HINSTANCE)instance, APP_NAME, &wndclass)) {
			wndclass.style         = 0;
			wndclass.lpfnWndProc   = EventManager ;
			wndclass.cbClsExtra    = 0 ;
			wndclass.cbWndExtra    = 0 ;
			wndclass.hInstance     =  (HINSTANCE)instance;
			wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
			wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
			wndclass.hbrBackground = 0;
			wndclass.lpszMenuName  = NULL ;
			wndclass.lpszClassName = APP_NAME;

			if (!RegisterClass (&wndclass)) {
				vm->logMessage(VMHOST_LOG_ERROR, "Can't register Window handle");
				return VM_INTERNAL_ERROR;
			}
		}

		instance->hwnd = CreateWindow (APP_NAME, APP_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, (HINSTANCE)instance, NULL);
		instance->hwndNextClpViewer = SetClipboardViewer (instance->hwnd);

		instance->integrationTool = new vm::IntegrationToolHost(vm, instance->hwnd);
	#else
		instance->integrationTool = new vm::IntegrationToolHost(vm);
	#endif

	const char * value = vm->getParameter("max-clipboard-transfert-size");
	if (value != NULL) {
		long lvalue = atoi(value);
		if (lvalue > 0) {
			instance->integrationTool->maxClipboardTransfertSize = lvalue;
		}
	}
	vm->setIoOutputHandle (INTEGRATION_TOOL_DEFAULT_IO_PORT, (vm::type::IoOutputHandle)io_write, 4);
	vm->setIoInputHandle  (INTEGRATION_TOOL_DEFAULT_IO_PORT, (vm::type::IoInputHandle) io_read,  4);
	vm->setMouseMoveEventHandle((vm::type::MouseMoveEventHandle)mouseHnd);

	return Common_PreInit(vm, (Instance *)instance);
}

LIBRARY_API int VMPLUGIN_PostInit(vm::type::VirtualMachine * vm, void * myInstance) {
	if (vm == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	if (vm->structSize < sizeof(vm::type::VirtualMachine)) { return VM_ERROR_BAD_STRUCT_SIZE; }
	Instance * instance = (Instance*)myInstance;

	vm->sendCommand("echo on");
	vm->sendCommand("cls");
	vm->sendCommand("ver set 6 22 > NUL");

	const char * application = vm->getParameter("application");
	setAppIcon(vm, vm->getParameter("plugin"));
	const char * winDir=vm->getParameter("windir");
	const char * exit = vm->getParameter("exit");

	if (winDir != NULL && winDir[0] != 0x00) { 
		char * cmd = (char*)malloc (strlen(winDir) + 17);
		sprintf(cmd, "SET PATH=%s;%%PATH%%", winDir);
		vm->sendCommand(cmd);
		sprintf(cmd, "SET WINDIR=%s", winDir);
		vm->sendCommand(cmd);
		free(cmd);
	}

	if (application == NULL) { vm->sendCommand("WIN"); }
	else { 
		/////////////////////////////////////////////////////////////////////////////////////////////////
		// Launch application
		/////////////////////////////////////////////////////////////////////////////////////////////////
		
		struct stat buffer;
		
		if (stat (application, &buffer) != 0) {
			char * msg = (char *)malloc(strlen(application) + 32);
			vm->sendCommand("echo.");
			sprintf(msg, "echo Application \"%s\" not found.", application);
			vm->sendCommand(msg);
			vm->sendCommand("echo.");
			vm->sendCommand("pause");
			vm->sendCommand("exit");
			return VM_NO_ERROR;
		}
		
		char * dosPath;
		dosPath = toMappedDosPath(instance, application);

		if (dosPath == NULL) {
			const char * appCmd = application;
			const char * ptr = application;

			while(*ptr != '\0') {
	#ifdef WIN32
				if ((*ptr == '/' || *ptr == '\\'))
	#else
				if (*ptr == '/')
	#endif
				{ appCmd = ptr; }

				ptr++;
			}

			int size = appCmd - application;
			char * path = (char*) malloc(size + 1);
			strncpy(path, application, size);
			path[size] = 0x00;
			MapDevice (vm, instance, "mount %c %s > NUL", instance->tmpAppDrive, path);
			free (path);
			dosPath = (char *) malloc (strlen (appCmd) + 4);
			sprintf(dosPath, "%c:%s", instance->tmpAppDrive, appCmd);
		}

		char * path = (char *) malloc (strlen (dosPath) + 30);
		const char *useWinRun=vm->getParameter("use-winrun");
		strcpy(path, "WIN ");

		if (useWinRun != NULL && strcmpi(useWinRun, "true") == 0) {
			strcat(path, "WINRUN "); 

			const char * param = vm->getParameter("exit-no-prompt");
			if (param != NULL && strcmpi(param, "true") == 0) {
				strcat(path, "/NOCONFIRM ");
			}

			param = vm->getParameter("application-window");
			if (param != NULL){
				if (strcmpi(param, "MINIMIZED") == 0) {
					strcat(path, "/MIN ");
				} else if (strcmpi(param, "MAXIMIZED") == 0) {
					strcat(path, "/MAX ");
				}
			}

			param = vm->getParameter("exit");
			if (param != NULL && strcmpi(param, "true") == 0) {
				strcat(path, "/EXIT ");
			}
		}

		strcat(path, dosPath);
		vm->sendCommand(path);
		free (dosPath);
	}

	vm->sendCommand("exit");
	return VM_NO_ERROR;
}

LIBRARY_API int VMPLUGIN_ShutdownRequest(vm::type::VirtualMachine * vm, void * instance) {
	if (vm == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	if (vm->structSize < sizeof(vm::type::VirtualMachine)) { return VM_ERROR_BAD_STRUCT_SIZE; }

	if ( ((Win16Instance*)instance)->integrationTool->ShutdownRequest()) {
		vm->logMessage(VMHOST_LOG_CONSOLE, "Send shutdown signal to guest system");
		return VM_NO_ERROR; 
	} else {
		return VM_ERROR_UNSUPPORTED_OPERATION;
	}
}
