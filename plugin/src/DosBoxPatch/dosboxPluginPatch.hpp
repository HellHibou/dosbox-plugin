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

	
	static vm::type::VirtualMachine        vm;
	static vm::type::VirtualMachineInfo    vmInfo;
	static vm::Plugin *                    plugin;
	static DOS_Shell *                     shell;
	static Status			               status;
	static DosBoxPluginManager::Properties properties;

	static const vm::type::VirtualMachineInfo VM_getVmInfo      ();
	static int			VM_setWindowTitle (const char * title);
	static int			VM_setWindowIcon  (const unsigned char * icon, int width, int height, int bits);
	static int			VM_sendCommand    (const char * args, ...);
	static int			VM_logMessage     (int messageType, const char * message);
	static const char * VM_getParameter   (const char * parameter);
	static void         DosBox_initParameter  (Section * section);

public:
	static const char * windowTitle;
	static void preInit(Config * config);
	static void postInit(DOS_Shell * shell);
	static void start();
	static void unload();
};
