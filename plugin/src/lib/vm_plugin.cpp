/**
 * \brief Virtual machine's plugin library loader class.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#include <stdio.h>
#include "vm_plugin.hpp"

#if defined (WIN32)
	#define _CRT_SECURE_NO_WARNINGS
	#include <Windows.h>
#else
	#include <dlfcn.h>
	#define LoadLibrary(lib) dlopen(lib,RTLD_LAZY)
	#define GetProcAddress(lib,fct) dlsym(lib,fct)
	#define FreeLibrary(lib) dlclose(lib) 
#endif

#define SET_FCT(FCT)  fct_ ## FCT = (type::pluginLib:: ## FCT) GetProcAddress(lib, "VMPLUGIN_" #FCT );

namespace vm {
	Plugin::Plugin(const char * libname, type::VirtualMachine * myVm) {
		vm = myVm;
		instance = NULL;
		libPath = NULL;
		lib = LoadLibrary(libname);

		if (lib == NULL) { 
			initError = VM_ERROR_LIBRARY_NOT_FOUND; 
			instance = NULL;
			return;
		}

		SET_FCT(CreateInstance);

		if (fct_CreateInstance == NULL) { 
			FreeLibrary(lib); 
			lib = NULL;
			instance = NULL;
			initError = VM_ERROR_UNSUPPORTED_LIBRARY;
			return;
		}

		initError = fct_CreateInstance (vm, &instance);

		if (initError != VM_NO_ERROR) { 
			FreeLibrary(lib); 
			lib = NULL;
			instance = NULL;
			return;
		}

		SET_FCT(DestroyInstance);
		SET_FCT(PreInit);
		SET_FCT(PostInit);
		SET_FCT(ShutdownRequest);

#ifdef WIN32
		libPath = (char *)malloc(2048);

		int newSize = GetModuleFileName(lib, (LPSTR)libPath, 2048);

		if (newSize == 0) {
			free((void*)libPath);
			libPath = NULL;
		}
		else { 
			libPath = (char*) realloc((void*)libPath, newSize+1);
		}
#else
		Dl_info dl_info;
		dladdr((void *)lib, &dl_info);
		libPath = info.dli_fname;
#endif
	}

	Plugin::~Plugin() { 
		if (initError != VM_NO_ERROR) { return; }

#ifdef WIN32
		if (libPath != NULL) { free((void*)libPath); }
#endif
		if (lib != NULL) { 
			if (fct_DestroyInstance != NULL) { fct_DestroyInstance(vm, instance); }

			FreeLibrary(lib); 
			lib = NULL;
		}
	}

	int Plugin::getClassInitError() { return initError; }

	const char * Plugin::getLibraryPath() { return libPath; }

	int Plugin::preInit() {
		if (initError != VM_NO_ERROR) { return initError; }
		if (fct_PreInit == NULL) { return VM_ERROR_UNSUPPORTED_OPERATION; } 
		return fct_PreInit(vm, instance);
	}

	int Plugin::postInit() {
		if (initError != VM_NO_ERROR) { return initError; }
		if (fct_PostInit == NULL) { return VM_ERROR_UNSUPPORTED_OPERATION; } 
		return fct_PostInit(vm, instance);
	}

	int Plugin::shutdownRequest() {
		if (initError != VM_NO_ERROR) { return initError; }
		if (fct_ShutdownRequest == NULL) { return VM_ERROR_UNSUPPORTED_OPERATION; } 
		return fct_ShutdownRequest(vm, instance);
	}
}
