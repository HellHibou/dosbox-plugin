/**
 * \brief DosBox plugin patch.
 * \author Jeremy Decker
 * \version 0.2
 * \date 14/11/2015
 */

#pragma once
#ifndef __JREKCED_DOSBOXPLUGINPATCH_HPP__
#define __JREKCED_DOSBOXPLUGINPATCH_HPP__

#include <SDL.h>
#include <shell.h>
#include <string>
#include "vm_plugin.hpp"
#include "vm_host.h"
#include "Control.h"
#include "regs.h"

#define DosBoxPluginManager_IO_READ  1
#define DosBoxPluginManager_IO_WRITE 2

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

	typedef struct CallRequestParams {
		CallRequestParams * next;
		unsigned int      segment;
		unsigned int      offset;
		unsigned short    flags;
		unsigned short    argc;
		unsigned short *  args;
	};

	static vm::type::VirtualMachine         vm;
	static vm::type::VirtualMachineInfo     vmInfo;
	static DOS_Shell *                      shell;
	static Status			                status;
	static DosBoxPluginManager::Properties  properties;
	static CallRequestParams *              callRequestParams;
	static vm::Plugin * plugin;

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
	static int                                  VM_callGuestFct        (unsigned int segment, unsigned int offset, unsigned int callTypeFlags, short stackCallArgc, unsigned short * stackCallArgs);

public:
	static const char * windowTitle;
	static vm::type::MouseMoveEventHandle mouseMoveHnd; 
	static Bit8u  ioType [64*1024];
	
	static void preInit(Config * config);
	static void postInit(DOS_Shell * shell);
	static void start();
	static void unload();

	static inline int shutdownRequest() {
		if (plugin) { return plugin->shutdownRequest(); }
		else  { return VM_ERROR_UNSUPPORTED_OPERATION; }
	}
	static inline void * getPluginInstance() { return plugin->getPluginInstance(); }
};

#endif
