/**
 * \brief DosPlugin DLL entries.
 * \author Jeremy Decker
 * \version 0.2
 * \date 14/11/2015
 */

#include <string.h>
#include <sys/stat.h>
#include "../CommonPlugin/plugin_common.hpp"

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

extern const char PLUGIN_INTRO [] = "Dos Plugin version 0.2\nCopyright 2015-2016 Hell Hibou";

LIBRARY_API int VMPLUGIN_PostInit(vm::type::VirtualMachine * vm, void * myInstance)
{
	if (vm == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	if (vm->structSize < sizeof(vm::type::VirtualMachine)) { return VM_ERROR_BAD_STRUCT_SIZE; }
	Instance * instance = (Instance*)myInstance;

	vm->sendCommand("echo on");
	const char * application = vm->getParameter("application");

	if (application == NULL)
	{
		const char * paramValue = vm->getParameter("short-intro");
		if (paramValue== NULL);
		else if (strcmpi("true", paramValue) == 0) 
		{ 
			vm->sendCommand("cls");
			vm->sendCommand("ver");
		}
		else if (strcmpi("false", paramValue) != 0) { vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'short-intro'"); }

		vm->sendCommand("echo.");

		//vm->setWindowIcon(MSDOS_ICON, 32, 32, 32);
		setAppIcon(vm, vm->getParameter("plugin"));
	}
	else
	{ 
		vm->sendCommand("cls");

		// Pause at end of application
		bool pauseAtEnd;
		const char * paramValue = vm->getParameter("pause-before-exit");
		if (paramValue== NULL) { pauseAtEnd = DEFAULT_pause_before_exit; }
		else if (strcmpi("true", paramValue) == 0) { pauseAtEnd = true; }
		else if (strcmpi("false", paramValue) == 0) { pauseAtEnd = false; }
		else 
		{ 
			pauseAtEnd = DEFAULT_pause_before_exit; 
			vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'pause-before-exit'"); 
		}
		
		if (!setAppIcon(vm, application))
		{ 
			//vm->setWindowIcon(MSDOS_ICON, 32, 32, 32); 
			setAppIcon(vm, vm->getParameter("plugin"));
		} 
		
		/////////////////////////////////////////////////////////////////////////////////////////////////
		// Launch application
		/////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
		bool reMount = false;
#endif
		struct stat buffer;

		if (stat (application, &buffer) != 0)
		{
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

		if (dosPath == NULL)
		{
			const char * appCmd = application;
			const char * ptr = application;

			while(*ptr != '\0')
			{
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
	#ifdef WIN32
			reMount = true;
		}
	#endif
		char * appCmd = dosPath;
		char * ptr = dosPath;

		while(*ptr != '\0')
		{
			if ((*ptr == '/' || *ptr == '\\')) { appCmd = ptr; }
			ptr++;
		}

		*appCmd = '\0';
		appCmd ++;

		char * path = (char *) malloc (strlen (dosPath) + 8);
		sprintf(path, "%c:", dosPath[0]);
		vm->sendCommand(path);
		sprintf(path, "cd %s", dosPath);
		vm->sendCommand(path);
		free(path);

		vm->sendCommand("cls"); 
		vm->sendCommand(appCmd);
		free (dosPath);
		/////////////////////////////////////////////////////////////////////////////////////////////////
	
		const char * exit = vm->getParameter("exit");
		if (exit != NULL && strcmp(exit, "true") == 0)
		{ 
			if (pauseAtEnd)
			{
				vm->sendCommand("echo."); 
				vm->sendCommand("pause"); 
			}
			vm->sendCommand("exit"); 
		}
		else
		{
			setAppIcon(vm, vm->getParameter("plugin"));
		#ifdef WIN32
			if (reMount)
			{
		#endif
				const char * systemRoot = vm->getParameter("system-root");
				if ((instance->tmpAppDrive == 'C') && (systemRoot != NULL))
				{
					MapDevice (vm, instance, "mount %c %s > NUL", 'C', systemRoot);
					vm->sendCommand("C:");
				}
				else
				{
					UnmapDevice(vm, instance, instance->tmpAppDrive);
					if (instance->driveMap[2] == NULL) { vm->sendCommand("Z:"); } else { vm->sendCommand("C:"); }
				}
		#ifdef WIN32
			}
		#endif
		
			vm->sendCommand("echo.");
		}
	}

	return VM_NO_ERROR;
}
