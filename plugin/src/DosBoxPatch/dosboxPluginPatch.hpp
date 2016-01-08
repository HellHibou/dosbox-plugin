/**
 * \file dosboxPluginPatch.cpp
 * \brief DosBox plugin patch.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#pragma once

#include <SDL.h>
#include <shell.h>
#include <string>
#include "../lib/vm_plugin.hpp"
#include "../lib/vm_host.h"
#include "Control.h"
#include "regs.h"

class DosBoxPluginManager
{
private:
	class Properties
	{
	private:
		typedef struct Property
		{
			char * key;
			char * value;
		};

		std::list<Property*> properties;

	public:
		~Properties();
		void set (const char * key, const char * value, bool final = false);
		void parse (std::string line, bool config);
		const char * get(const char * key);
		void clear();
	};

	typedef enum Status
	{
		NOT_LOADED = 0,
		PRE_INITIALIZED,
		POST_INITIALIZED,
		STARTED
	};

	static vm::type::VirtualMachine         vm;
	static vm::type::VirtualMachineInfo     vmInfo;
	static vm::Plugin *                     plugin;
	static DOS_Shell *                      shell;
	static Status			                status;
	static DosBoxPluginManager::Properties  properties;

	static void DosBox_initParameter  (Section * section);
	static void DosBoxCallRequestHandle (unsigned int id); 

	static const vm::type::VirtualMachineInfo * VM_getVmInfo ();
	static int									VM_setWindowTitle (const char * title);
	static int									VM_setWindowIcon  (const unsigned char * icon, int width, int height, int bits);
	static int									VM_sendCommand    (const char * args, ...);
	static int									VM_logMessage     (int messageType, const char * message);
	static const char *                         VM_getParameter   (const char * parameter);
	static int                                  VM_setInterruptHandle (unsigned char intId, vm::type::InterruptHandle intHnd);
	static const vm::type::MouseMoveEventHandle VM_setMouseMoveEventHandle (vm::type::MouseMoveEventHandle mHnd);
	static int									VM_setIoOutputHandle   (unsigned short port, vm::type::IoOutputHandle pHnd, unsigned char len);
	static int									VM_setIoInputHandle    (unsigned short port, vm::type::IoInputHandle  pHnd, unsigned char len);
	static const vm::type::IoOutputHandle       VM_getIoOutputHandle   (unsigned short port);
	static const vm::type::IoInputHandle        VM_getIoInputHandle    (unsigned short port);
	
public:
	static const char * windowTitle;
	static vm::type::MouseMoveEventHandle mouseMoveHnd; 

	static void preInit(Config * config);
	static void postInit(DOS_Shell * shell);
	static void start();
	static void unload();
};
