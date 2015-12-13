#include <string.h>
#include <sys/stat.h>
#include "../CommonPlugin/plugin_common.hpp"

extern const char PLUGIN_INTRO [] = "Win16Plugin by Hell Hibou";

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

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
