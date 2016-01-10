/**
 * \brief DosBox plugin patch.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#include <SDL.h>
#include "control.h"
#include "inout.h"
#include "callback.h"
#include "cpu.h"
#include "pic.h"
#include "../lib/vm_host.h"
#include "dosboxPluginPatch.hpp"

#define VM_VERSION_MAJOR 0
#define VM_NAME "DOSBOX\0\0\0\0\0\0\0\0\0"

extern void GFX_SetTitle(Bit32s cycles,Bits frameskip,bool paused);
extern void GFX_SetIcon() ;
extern      DOS_Block dos;
extern      IO_WriteHandler * io_writehandlers[3][IO_MAX];
extern      IO_ReadHandler  * io_readhandlers [3][IO_MAX];
extern void IO_WriteDefault(Bitu port,Bitu val,Bitu iolen);
extern Bitu IO_ReadDefault(Bitu port,Bitu iolen);

vm::type::VirtualMachine         DosBoxPluginManager::vm;
vm::type::VirtualMachineInfo     DosBoxPluginManager::vmInfo;
vm::Plugin *                     DosBoxPluginManager::plugin;
DOS_Shell  *                     DosBoxPluginManager::shell;
DosBoxPluginManager::Properties  DosBoxPluginManager::properties;
DosBoxPluginManager::Status      DosBoxPluginManager::status = NOT_LOADED;
const char *                     DosBoxPluginManager::windowTitle = NULL;
vm::type::MouseMoveEventHandle   DosBoxPluginManager::mouseMoveHnd = NULL; 
DosBoxPluginManager::CallRequestParams DosBoxPluginManager::callRequestParams[16];

DosBoxPluginManager::Properties::~Properties() { clear(); }

void DosBoxPluginManager::Properties::parse (std::string line, bool cmdLine) {
	if (cmdLine) {
		if (line.length() < 3) return;
		if (strcmpi (line.substr(0,3).data(), "-x-") != 0) return;
		line = line.substr(3,line.length()-1); 
	}

	std::string::size_type loc = line.find('=');
	if (loc == std::string::npos) { 
		if (cmdLine) { set(line.data(), "true", true); } 
		return;
	}

	std::string key = line.substr(0,loc);
	size_t first = key.find_first_not_of(' ');
	size_t last = key.find_last_not_of(' ');
	key = key.substr(first, (last-first+1));
	if (!cmdLine && key[0] == '#') { return; }

	std::string val = line.substr(loc + 1);
	first = val.find_first_not_of(' ');
	last = val.find_last_not_of(' ');
	val = val.substr(first, (last-first+1));
	if (val.length() > 1 && val[0] == '"' && val [val.length()-1] == '"') { val = val.substr(1, val.length()-2); }
	if (key.length() > 0) { set(key.data(), val.data(), cmdLine); }
}

void DosBoxPluginManager::Properties::set(const char * key, const char * value, bool final) {
	for(std::list<Property *>::iterator prop = properties.begin(); prop != properties.end(); prop++) {
		if (stricmp(prop._Ptr->_Myval->key, key) == 0) {
			if (prop._Ptr->_Myval->value[0] == 0x01 && ! final) { return; }
			if (prop._Ptr->_Myval->value != NULL) { free (prop._Ptr->_Myval->value); }
			
			if (value == NULL) { 
				properties.erase(prop);
				return;
			}
			
			prop._Ptr->_Myval->value = (char*) malloc (strlen(value)+2);
			if (final) { prop._Ptr->_Myval->value[0] = 0x01; }
			else if (final) { prop._Ptr->_Myval->value[0] = 0x02; }
			strcpy(prop._Ptr->_Myval->value+1, value);
			key = NULL;
			break;
		}
	}

	if(key != NULL && value != NULL) {
		Property * prop = (Property*) malloc(sizeof(Property));
		prop->key = (char*) malloc (strlen(key)+1);
		strcpy(prop->key, key);

		prop->value = (char*) malloc (strlen(value)+2);
		strcpy(prop->value+1, value);
		if (final) { prop->value[0] = 0x01; }
		else if (final) { prop->value[0] = 0x02; }
		properties.push_back(prop);
	}
}

const char * DosBoxPluginManager::Properties::get(const char * key)
{
	for(std::list<Property *>::iterator prop = properties.begin(); prop != properties.end(); prop++) {
		if (stricmp(prop._Ptr->_Myval->key, key) == 0) 
		{ return prop._Ptr->_Myval->value+1; }
	}

	return NULL;
}

void DosBoxPluginManager::Properties::clear()
{
	for(std::list<Property *>::iterator prop = properties.begin(); prop != properties.end(); prop++) {
		free(prop._Ptr->_Myval->key);
		if (prop._Ptr->_Myval->value != NULL) { free(prop._Ptr->_Myval); }
	}

	properties.clear();
}

///////////////////////////////////////////////////////////////////////////////

void DosBoxPluginManager::preInit(Config * config) {
	memset(&callRequestParams, sizeof(callRequestParams), 0);

	vmInfo.structSize = sizeof (vm::type::VirtualMachineInfo);
	vmInfo.vm_version_major = VM_VERSION_MAJOR;
	vmInfo.vm_version_minor = VM_VirtualMachine_FCT_COUNT;
	memcpy (&vmInfo.name, VM_NAME, VM_SIZEOF_VMNAME);

	vm.structSize = sizeof(vm::type::VirtualMachine);
	vm.getVmInfo = VM_getVmInfo;
	vm.setWindowTitle = VM_setWindowTitle;
	vm.setWindowIcon = VM_setWindowIcon;
	vm.sendCommand = VM_sendCommand;
	vm.logMessage = VM_logMessage;
	vm.getParameter = VM_getParameter;
	vm.setInterruptHandle = VM_setInterruptHandle;
	vm.setMouseMoveEventHandle = VM_setMouseMoveEventHandle;
	vm.setIoInputHandle = VM_setIoInputHandle;
	vm.setIoOutputHandle = VM_setIoOutputHandle;
	vm.getIoInputHandle = VM_getIoInputHandle;
	vm.getIoOutputHandle = VM_getIoOutputHandle;
	vm.callGuestFct = VM_callGuestFct;

	config->AddSection_line("plugin", &DosBox_initParameter);
	MSG_Add("PLUGIN_CONFIGFILE_HELP",
		"Add plugin configuration's parameters there.\n"
		"plugin: Plugin's path to load or leave blank; you can use -plugin 'PLUGIN' in command line to define this parameter.\n"
	);
	
	properties.clear();
	std::vector<std::string> vector;
	config->cmdline->FillVector(vector);

	for (unsigned int boucle = 0; boucle < vector.size(); boucle++) 
	{ 
		const char * arg;
		arg = vector[boucle].c_str();
		if (strcmp(arg, "-plugin") == 0) 
		{
			if (boucle+1 < vector.size()) 
			{
				properties.set("plugin", vector[boucle+1].c_str(), true);
				boucle++;
			}
		}
		else if (strcmp(arg, "-exit") == 0) { properties.set("exit", "true"); }
		else if (((strncmp(arg, "-X-", 3) == 0) || (strncmp(arg, "-x-", 3) == 0)) && arg[3] != 0x00) 
		{
			if (boucle+1 < vector.size())
			{ 
				const char * arg2 = vector[boucle+1].c_str();
				if (arg2[0] == '"') 
				{
					arg2++;
					if (arg2[0] != 0x00 && (arg2[1] != 0x00))
					{
						int idx = strlen(arg2)-1;
						if (arg2[idx] == '\"') { (char)arg2[idx] = 0x00; }
					}
				}
				if (arg2[0] != 0x00 && arg2[0] != '-')
				{ 
					properties.set(arg+3, arg2, true); 
					std::string str;
					config->cmdline->FindString(arg, str, true);				
				}
				else
				{ properties.set(arg+3, NULL, true); }
				boucle++;
			}
			else { properties.set(arg+3, NULL, true); }
		}
	}

	DosBoxPluginManager::status = PRE_INITIALIZED;
}

void DosBoxPluginManager::DosBox_initParameter(Section* mySec) {
	Section_line * sec = static_cast<Section_line *> (mySec);
	std::string data = sec->data;

	for (unsigned int i = 0; i < data.length();i++) { 
		if (data[i] == '\r') { data[i] = '\n'; } }

	size_t pos = 0;
	std::string line;

	while ((pos = data.find("\n")) != std::string::npos) {
		line = data.substr(0, pos);
		for (unsigned int i = 0; i < pos;i++) { if (line[i] == '\n') { line[i] = ' '; } }
		properties.parse(line, false);
		data.erase(0, pos + 1);
	}
}

void DosBoxPluginManager::postInit(DOS_Shell * myShell) {
	if (DosBoxPluginManager::status >= POST_INITIALIZED) return;
	const char *pluginPath = DosBoxPluginManager::properties.get("plugin");
	shell = myShell;
	if (pluginPath == NULL || pluginPath[0] == 0x00) { plugin = NULL; }
	else {
		plugin = new vm::Plugin(pluginPath, &vm);

		switch(plugin->getClassInitError()) {
			case VM_NO_ERROR:
				DosBoxPluginManager::properties.set("plugin", plugin->getLibraryPath(), true);
				LOG_MSG("Plugin '%s' loaded.", plugin->getLibraryPath());
				plugin->preInit();
				break;

			case VM_ERROR_LIBRARY_NOT_FOUND:
				LOG_MSG("Plugin initialisation error: Library '%s' not found.", pluginPath);
				break;

			case VM_ERROR_UNSUPPORTED_LIBRARY:
				printf("Plugin initialisation error: Library '%s' is not supported.", pluginPath);
				break;

			case VM_ERROR_NULL_POINTER_EXCEPTION: 
				LOG_MSG("Plugin initialisation error: Null pointer exception.");
				break;

			case VM_ERROR_BAD_STRUCT_SIZE:        
				LOG_MSG("Plugin initialisation error: Bad structure size.");
				break;

			case VM_ERROR_BAD_PARAMETER_VALUE:    
				LOG_MSG("Plugin initialisation error: Bad parameter value.");
				break;

			case VM_ERROR_UNSUPPORTED_VM_NAME:   
				LOG_MSG("Plugin initialisation error: Unsupported virtual machine.");
				break;

			case VM_ERROR_UNSUPPORTED_VM_VERSION:
				LOG_MSG("Plugin initialisation error: Unsupported virtual machine version.");
				break;

			case VM_ERROR_UNSUPPORTED_OPERATION:  
				LOG_MSG("Plugin initialisation error: Unsupported operation.");
				break;

			case VM_ERROR_UNSUPPORTED_COMMAND:   
				LOG_MSG("Plugin initialisation error: Unsupported command.");
				break;

			case VM_UNKNOWN_ERROR:   
			default:
				LOG_MSG ("Plugin initialisation error: Unknow error.");
		}
	}

	DosBoxPluginManager::status = PRE_INITIALIZED;
}

void DosBoxPluginManager::start() {
	if (plugin == NULL || DosBoxPluginManager::status >= STARTED) return;
	plugin->postInit();
	DosBoxPluginManager::status = STARTED;
}

void DosBoxPluginManager::unload() { 
	if (plugin != NULL)	{ 
		delete plugin;
		plugin = NULL;
		LOG_MSG("Plugin unloaded.");
	}

	properties.clear(); 
	shell = NULL;
	DosBoxPluginManager::status = NOT_LOADED;
}

const vm::type::VirtualMachineInfo * DosBoxPluginManager::VM_getVmInfo () { 
	return &DosBoxPluginManager::vmInfo; 
}

int DosBoxPluginManager::VM_sendCommand (const char * args, ...) {
	if (args == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	char cmd [CMD_MAXLINE];
	memset(cmd, '\0', CMD_MAXLINE-1);
	strncpy (cmd, args, CMD_MAXLINE);

#ifdef _DEBUG
	LOG_MSG("Plugin send command : %s", args);
#endif

	shell->call=true;
	shell->ParseLine(cmd);
	shell->RunInternal();
	return VM_NO_ERROR;
}

int DosBoxPluginManager::VM_setWindowTitle (const char * title) { 
	if (title == NULL) { windowTitle = NULL; }
	else if (strlen(title) == 0) { windowTitle = NULL; }
	else { windowTitle = title; }
	GFX_SetTitle(-1,-1, false);
	return VM_NO_ERROR;
}

int DosBoxPluginManager::VM_logMessage (int messageType, const char * message) {
	switch (messageType) {
#ifdef WIN32
		case VMHOST_LOG_ERROR:
			MessageBox(NULL, message, "DosBox Plugin" ,MB_ICONERROR|MB_OK);
			break;

		case VMHOST_LOG_WARNING:
			MessageBox(NULL, message, "DosBox Plugin", MB_ICONWARNING|MB_OK);
			break;

		case VMHOST_LOG_INFO:
			MessageBox(NULL, message, "DosBox Plugin", MB_ICONEXCLAMATION|MB_OK);
			break;
#else
		case VMHOST_LOG_ERROR:
			LOG_MSG("Plugin ERROR : %s", message);
			break;

		case VMHOST_LOG_WARNING:
			LOG_MSG("Plugin WARNING : %s", message);
			break;

		case VMHOST_LOG_INFO:
			LOG_MSG("Plugin INFO : %s", message);
			break;
#endif

		case VMHOST_LOG_DEBUG:
			LOG_MSG("Plugin DEBUG : %s", message);
			break;
	}

	return VM_NO_ERROR;
}

int DosBoxPluginManager::VM_setWindowIcon  (const unsigned char * icon, int width, int height, int bits) { 
	if (icon == NULL) { GFX_SetIcon(); }
	if (width != height) { return VM_ERROR_BAD_PARAMETER_VALUE; }
	SDL_Surface* logo;
	unsigned char * icon32 = NULL;
	
	/* SDL only support 32x32 icon, other size is bugged. */
	if (width != 32) {
		int bytes = bits/8;
		float ratio = width; ratio /= 32;
		icon32 = (unsigned char *) malloc(32 * 32 * bytes);

		if(width < 32) {
			for (int boucleHeight = 0; boucleHeight < 16; boucleHeight++) {
				for (int boucleWidth = 0; boucleWidth < 16; boucleWidth++) {
					int index1 = ((boucleHeight * 16) + boucleWidth)  * bytes;
					int index2 = ((boucleHeight * 32 * (1/ratio)) + boucleWidth * (1/ratio)) * bytes;
					memcpy(icon32+index2,       icon+index1, bytes);
					memcpy(icon32+index2+bytes, icon+index1, bytes);
				}

				int index = (boucleHeight * 32 * (1/ratio)) * bytes;
				memcpy(icon32+index+(32*bytes), icon32+index, (32*bytes));
			}
		}
		else {
			for (int boucleHeight = 0; boucleHeight < 32; boucleHeight++) {
				for (int boucleWidth = 0; boucleWidth < 32; boucleWidth++) {
					int index1 = ((ceil(boucleHeight*ratio) * width) + ceil(boucleWidth*ratio) ) * bytes + bytes;
					int index2 = ((boucleHeight * 32) + boucleWidth ) * bytes;
					memcpy(icon32+index2,       icon+index1, bytes);
				}
			}
		}

		icon = icon32;
		width = 32;
		height = 32;
	}

	switch (bits) {
#if WORDS_BIGENDIAN
		case 24:
			logo = SDL_CreateRGBSurfaceFrom((void*)icon,width,height,bits,(width * 3),0xff000000,0x00ff0000,0x0000ff00,0x00000000);
			break;

		case 32:
			logo = SDL_CreateRGBSurfaceFrom((void*)icon,width,height,bits,(width * 4),0xff000000,0x00ff0000,0x0000ff00,0x000000ff);
			break;

		default:
			return VM_ERROR_BAD_PARAMETER_VALUE;
#else
		case 24:
			logo = SDL_CreateRGBSurfaceFrom((void*)icon,width,height,bits,(width * 3),0x000000ff,0x0000ff00,0x00ff0000,0x00000000);
			break;
	
		case 32:
			logo = SDL_CreateRGBSurfaceFrom((void*)icon,width,height,bits,(width * 4),0x000000ff,0x0000ff00,0x00ff0000,0xff000000);
			break;
	
		default:
			return VM_ERROR_BAD_PARAMETER_VALUE;
#endif
	}

	SDL_WM_SetIcon(logo, NULL);
	SDL_FreeSurface(logo);
	if (icon32 != NULL) { free(icon32); }
	return VM_NO_ERROR; 
}

const char * DosBoxPluginManager::VM_getParameter (const char * parameter) { 
	return properties.get(parameter); 
}

int DosBoxPluginManager::VM_setInterruptHandle(unsigned char intId, vm::type::InterruptHandle intHnd) {
#ifdef _DEBUG
	if (intHnd == NULL) {
		LOG_MSG("Plugin : Interrupt 0x%x unset.", intId);
	} else {
		LOG_MSG("Plugin : Interrupt 0x%x set to handle 0x%x.", intId, intHnd);
	}
#endif

	int call_int=CALLBACK_Allocate();
	CALLBACK_Setup(call_int,intHnd, CB_IRET,"Plugin interrupt");
	RealSetVec(intId,CALLBACK_RealPointer(call_int));
	return VM_NO_ERROR;
}

const vm::type::MouseMoveEventHandle DosBoxPluginManager::VM_setMouseMoveEventHandle (vm::type::MouseMoveEventHandle mHnd) {
#ifdef _DEBUG
	if (mHnd == NULL) {
		LOG_MSG("Plugin : Unset mouse event handle.");
	} else {
		LOG_MSG("Plugin : Set mouse event handle.");
	}
#endif
	vm::type::MouseMoveEventHandle retVal = DosBoxPluginManager::mouseMoveHnd;
	DosBoxPluginManager::mouseMoveHnd = mHnd;
	return retVal;
}

const vm::type::IoOutputHandle DosBoxPluginManager::VM_getIoOutputHandle (unsigned short port) {
	return io_writehandlers[0][port];
}

int DosBoxPluginManager::VM_setIoOutputHandle (unsigned short port, vm::type::IoOutputHandle pHnd, unsigned char len) {
#ifdef _DEBUG
	if (pHnd == NULL) {
		LOG_MSG("Plugin : Unset output port handle %i.", port);
	} else {
		LOG_MSG("Plugin : Set output port handle %i.", port);
	}
#endif

	if (pHnd == NULL) { pHnd = IO_WriteDefault; }
	switch(len) {
		case 4:
			io_writehandlers[2][port] = pHnd;
		case 2:
			io_writehandlers[1][port] = pHnd;
		case 1:
			io_writehandlers[0][port] = pHnd;
			return VM_NO_ERROR;

		default:
			return VM_ERROR_BAD_PARAMETER_VALUE;
	}
}

const vm::type::IoInputHandle DosBoxPluginManager::VM_getIoInputHandle (unsigned short port) {
	return io_readhandlers[0][port];
}

int DosBoxPluginManager::VM_setIoInputHandle  (unsigned short port, vm::type::IoInputHandle  pHnd, unsigned char len) {
#ifdef _DEBUG
	if (pHnd == NULL) {
		LOG_MSG("Plugin : Unset input port handle 0x%X.", port);
	} else {
		LOG_MSG("Plugin : Set input port handle 0x%X.", port);
	}
#endif

	if (pHnd == NULL) { pHnd = IO_ReadDefault; }
	switch(len) {
		case 4:
			io_readhandlers[2][port] = pHnd;
		case 2:
			io_readhandlers[1][port] = pHnd;
		case 1:
			io_readhandlers[0][port] = pHnd;
			return VM_NO_ERROR;

		default:
			return VM_ERROR_BAD_PARAMETER_VALUE;
	}
}

int DosBoxPluginManager::VM_callGuestFct(unsigned int segment, unsigned int offset, unsigned int callTypeFlags, short stackCallArgc, unsigned short * stackCallArgs) {
	for (int boucle = 0; boucle < 16; boucle++) {
		if (callRequestParams[boucle].segment == 0 && callRequestParams[boucle].offset == 0) {
			if (stackCallArgc == 0) {
				callRequestParams[boucle].args = NULL;
			} else {
				if (stackCallArgs == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
				callRequestParams[boucle].args = (unsigned short *) malloc (sizeof(unsigned short) * stackCallArgc);
				memcpy(callRequestParams[boucle].args, stackCallArgs, sizeof(unsigned short) * stackCallArgc);
			}
			
			callRequestParams[boucle].segment = segment;
			callRequestParams[boucle].offset = offset;
			callRequestParams[boucle].flags = callTypeFlags;
			callRequestParams[boucle].argc = stackCallArgc;
			PIC_AddEvent(DosBoxCallRequestHandle, 0, boucle);
			return VM_NO_ERROR;
		}
	}

#ifdef _DEBUG
	LOG_MSG("Plugin : Too many call request.");
#endif

	return VM_ERROR_OVERFLOW;
}

void DosBoxPluginManager::DosBoxCallRequestHandle (unsigned int hnd) {
	if ((callRequestParams[hnd].flags & VM_CALL_FLAG_PASCAL) == VM_CALL_FLAG_PASCAL) {
		for (int boucle = 0; boucle < callRequestParams[hnd].argc; boucle++) {
			CPU_Push16(callRequestParams[hnd].args[boucle]);
		} 
	} else {
		for (int boucle = callRequestParams[hnd].argc-1; boucle >= 0; boucle--) {
			CPU_Push16(callRequestParams[hnd].args[boucle]);
		} 
	}

	CPU_CALL((callRequestParams[hnd].flags & VM_CALL_FLAG_32BITS) == VM_CALL_FLAG_32BITS, callRequestParams[hnd].segment, callRequestParams[hnd].offset, reg_eip);

	if (callRequestParams[hnd].args != NULL) {
		free (callRequestParams[hnd].args);
		callRequestParams[hnd].args = NULL;
	}

	callRequestParams[hnd].segment = 0;
	callRequestParams[hnd].offset = 0;
}

