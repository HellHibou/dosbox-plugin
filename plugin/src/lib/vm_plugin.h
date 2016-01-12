/**
 * \brief Virtual machine's plugin library definition.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#pragma once
#ifndef __JREKCED_VM_PLUGIN_H__
#define __JREKCED_VM_PLUGIN_H__

#include "vm_host.h"

namespace vm { namespace type {

/** \brief Plugin's library functions. */
namespace pluginLib
{
	/**
	 * \brief Create a plugin's instance.
	 * \param vm Virtual machine's pointer.
	 * \param instance Pointer to store the pointer to instance of plugin.
	 * \return Error code or VM_NO_ERROR if no error.
	 */
	typedef int (*CreateInstance)(VirtualMachine * vm, void * * instance);

	/**
	 * \brief Destroy a plugin's instance.
	 * \param vm Virtual machine's pointer.
	 * \param instance Pointer to instance of plugin.
	 * \return Error code or VM_NO_ERROR if no error.
	 */
	typedef void (*DestroyInstance)(VirtualMachine * vm, void * instance);

	/**
	 * \brief Started by virtual machine before the VM.
	 * \param vm Virtual machine's pointer.
	 * \param instance Pointer to instance of plugin.
	 * \return Error code or VM_NO_ERROR if no error.
	 */
	typedef int (*PreInit)(VirtualMachine * vm, void * instance);

	/**
	 * \brief Started by virtual machine after the VM.
	 * \param vm Virtual machine's pointer.
	 * \param instance Pointer to instance of plugin.
	 * \return Error code or VM_NO_ERROR if no error.
	 */
	typedef int (*PostInit)(VirtualMachine * vm, void * instance);

	/**
	 * \brief Send shutdown request to guest O.S.
	 * \param vm Virtual machine's pointer.
	 * \param instance Pointer to instance of plugin.
	 * \return If < 0, return error code.
	 */
	typedef int (*ShutdownRequest)(VirtualMachine * vm, void * instance);

} } }

#endif
