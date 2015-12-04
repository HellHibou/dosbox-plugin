#include <string.h>
#include <sys/stat.h>
#include "../CommonPlugin/plugin_common.hpp"


#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

LIBRARY_API int VMPLUGIN_PostInit(vm::type::VirtualMachine * vm, void * myInstance)
{
	if (vm == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	if (vm->structSize < sizeof(vm::type::VirtualMachine)) { return VM_ERROR_BAD_STRUCT_SIZE; }
	Instance * instance = (Instance*)myInstance;
	
	vm->sendCommand("echo on");
	vm->sendCommand("cls");
	vm->sendCommand("ver set 6 22 > NUL");

	const char * application = vm->getParameter("application");
	setAppIcon(vm, vm->getParameter("plugin"));

	if (application == NULL) { vm->sendCommand("win"); }
	else
	{ 
		/////////////////////////////////////////////////////////////////////////////////////////////////
		// Launch application
		/////////////////////////////////////////////////////////////////////////////////////////////////
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
		}

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
		sprintf(path, "win %s", appCmd);
		vm->sendCommand(path);
		free(path);
		free (dosPath);
	}
	
	const char * exit = vm->getParameter("exit");
	if (exit != NULL && strcmp(exit, "true") == 0)
	{ vm->sendCommand("exit"); }
	else
	{
		vm->setWindowTitle(NULL);
		vm->setWindowIcon(NULL, 0,0,0);
		vm->sendCommand("ver");
		vm->sendCommand("echo.");
	}
	return VM_NO_ERROR;
}
