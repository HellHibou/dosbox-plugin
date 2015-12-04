/**
 * \file vm_plugin.hpp
 * \brief Virtual machine's plugin library loader class.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */
#pragma once

#include "vm_plugin.h"

#ifdef WIN32
	#include <Windows.h>
#else
	#include <dlfcn.h>
#endif

namespace vm 
{
	/**
	 * \class Plugin
	 * \brief Plugin's library manager's class. 
	 */
	class Plugin
	{
		private:
	#ifdef WIN32
			HINSTANCE lib;  /**< Pointer to dynamic library (Windows only). */
	#else
			void *    lib;  /**< Pointer to dynamic library (non-Windows only). */
	#endif

			type::VirtualMachine * vm; /**< Virtual machine pointer. */
			void * instance;           /**< Plugin's instance pointer.*/
			int    initError;          /**< Plugin's initialisation error. */
			const char * libPath;
			type::pluginLib::CreateInstance  fct_CreateInstance;	/**< Pointer to library function 'VMPLUGIN_CreateInstance'. */
			type::pluginLib::DestroyInstance fct_DestroyInstance;	/**< Pointer to library function 'VMPLUGIN_DestroyInstance'. */
			type::pluginLib::PreInit         fct_PreInit;			/**< Pointer to library function 'VMPLUGIN_PreInit'. */
			type::pluginLib::PostInit        fct_PostInit;			/**< Pointer to library function 'VMPLUGIN_PostInit'. */

		public:
			/**
			 * \param libname Plugin's library name.
			 * \param vm Virtual machine.
			 */
			Plugin(const char * libname, type::VirtualMachine * vm);
			
			~Plugin();

			/**
			 * \brief Get initialization error code.
			 * \return Error code or VM_NO_ERROR if no error.
			 */
			int getClassInitError();

			/**
			 * \brief Get full path of plugin library.
			 * \return Full path of plugin library or NULL if lilbrary not loaded. 
			 */
			const char * getLibraryPath();

			/**
			 * \brief Started by virtual machine before the VM.
			 * \return Error code or VM_NO_ERROR if no error.
			 */
			int preInit();

			/**
			 * \brief Started by virtual machine after the VM.
			 * \return Error code or VM_NO_ERROR if no error.
			 */
			int postInit();
	};
}
