/**
 * \brief Common source code for DosPlugin and Win16Plugin.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#ifdef WIN32
	#include <windows.h>
	#include <WinIoCtl.h>
#else
	#include <SDL.h>
#endif

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include "plugin_common.hpp"

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

#define PLUGIN_MAJOR_VERSION 0
#define VM_NAME "DOSBOX"

extern const char PLUGIN_INTRO [];

//////////////////////////////////////////////////////////////////////////////////////

static char * IS_ISO = "ISO";
/*
static const unsigned char MSDOS_ICON[32*32*4] = { 
	#include "msdos_icon.h"
};
*/

//////////////////////////////////////////////////////////////////////////////////////

bool ImgMount (vm::type::VirtualMachine * vm, Instance * instance, char drive, const char * isos)
{
	if (isos == NULL) { return false; }
	int size = strlen(isos);
	if (size < 1) { return false; }
	
	/* // Multi iso, bugged on DosBox 0.74
	int isoCount = 1;
	const char * ptrBegin = isos;
	
	while(*ptrBegin != NULL) 
	{
		if(*ptrBegin == ';') { isoCount++; }
		ptrBegin++; 
	}

	size += 26 + (3 * isoCount);
	char * cmd = (char*)malloc(size);
	char * cmdPtr = cmd; 

	//sprintf(cmd, "imgmount %c %s -t iso > NUL", drive, isos);
	sprintf(cmdPtr, "imgmount %c", drive);
	cmdPtr += 10;
	ptrBegin = isos;
	const char * ptrEnd = isos;
	size = 0;

	while(true) 
	{
		if (*ptrEnd == ';' || *ptrEnd == 0x00) 
		{
			cmdPtr[0] = ' ';
			cmdPtr[1] = '"';
			cmdPtr += 2;
			strncpy(cmdPtr, ptrBegin, size);
			cmdPtr += size;
			cmdPtr[0] = '"';
			cmdPtr++;
			ptrBegin = ptrEnd + 1;
			size = 0;
			if (*ptrEnd == 0x00) break;
		} 
		else { size++; }
		ptrEnd++;
	}

	sprintf(cmdPtr, " -t iso > NUL", drive);
	/*/ // Single iso
	struct stat buffer;

	if (stat (isos, &buffer) != 0) 
	{ 
		char * msg = (char *)malloc(strlen(isos)+36);
		sprintf(msg, "CD-ROM image file \"%s\" not found.", isos);
		vm->logMessage(VMHOST_LOG_ERROR, msg);
		free(msg);
		return false;
	}

	size += 28;
	char * cmd = (char*)malloc(size);
	sprintf(cmd, "imgmount %c \"%s\" -t iso > NUL", drive, isos);
	//*/

	int retVal = vm->sendCommand(cmd);
	free(cmd);
	instance->driveMap[drive - 'A'] = IS_ISO;
	return (retVal == VM_NO_ERROR);
}

//////////////////////////////////////////////////////////////////////////////////////

bool MapDevice (vm::type::VirtualMachine * vm, Instance * instance, char * cmdPattern, char drive, const char * path, bool setLabel )
{
	int size = strlen(cmdPattern) + strlen(path) + 1;

#ifdef WIN32
	char * labelParam;

	if (setLabel)
	{
		TCHAR volName[256];
		volName[0] = 0x00;
		DWORD volSerial;
		DWORD nameLen;
		DWORD volFlags;
		TCHAR volFS[256];
		char chemin[4];
		chemin[0] = drive;
		chemin[1] = ':';
		chemin[2] = '\\';
		chemin[3] = 0x00; 
		GetVolumeInformation(chemin, volName, ARRAYSIZE(volName), &volSerial, &nameLen, &volFlags, volFS, ARRAYSIZE(volFS));

		if (volName[0] != 0x00)
		{
			char *ptr = volName;
			while(ptr != 0x00) 
			{
				if (*ptr == 0x00) break;
				if((*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= '0' && *ptr <= '9') || *ptr == '-'|| *ptr == '_');
				else if (*ptr >= 'a' && *ptr <= 'z')
				{ *ptr = *ptr - 'a' + 'A'; }
				else 
				{ *ptr = '_'; }
					
				ptr++;
			}

			int volParamSize = strlen(volName) + 8;
			size += volParamSize;
			labelParam = (char*) malloc(volParamSize+1);
			sprintf(labelParam, "-label %s ", volName);
		}
		else { labelParam = NULL; }
	}
	else { labelParam = NULL; }
#endif

	char * cmd = (char*)malloc(size);

#ifdef WIN32
	if (labelParam == NULL) 
	{ sprintf(cmd, cmdPattern, drive, path, ""); }
	else
	{
		sprintf(cmd, cmdPattern, drive, path, labelParam); 
		free(labelParam);
	}
#else
	sprintf(cmd, cmdPattern, drive, path);
#endif

	if (vm->sendCommand(cmd) == VM_NO_ERROR)
	{
		free(cmd);
		unsigned char index = drive - 'A';
		if (instance->driveMap[index] != NULL) { free(instance->driveMap[index]); }
		instance->driveMap[index] = (char*)malloc (strlen(path)+1);
		strcpy(instance->driveMap[index], path);
		return true;
	}

	free(cmd);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

bool UnmapDevice (vm::type::VirtualMachine * vm, Instance * instance, char drive)
{
	int index = instance->tmpAppDrive - 'A';
	if (instance->driveMap[index] == NULL) { return false; }
	char * unmount = (char *)malloc(20);
	sprintf(unmount, "mount -u %c > NUL", drive);
	
	if (vm->sendCommand(unmount) == VM_NO_ERROR)
	{
		free(instance->driveMap[index]);
		instance->driveMap[index] = NULL;
		free(unmount);
		return true;
	}

	free(unmount);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

char * toMappedDosPath(Instance * instance, const char * winPath)
{
	for (int boucle = 0; boucle < 42; boucle++)
	{
		if (instance->driveMap[boucle] == NULL) continue;

		if (strncmp(instance->driveMap[boucle], winPath, strlen(instance->driveMap[boucle])) == 0)
		{  
#ifdef WIN32
			int shortMapSize = GetShortPathName(instance->driveMap[boucle], NULL, 0) - 1;
			if (shortMapSize == 0) { return NULL; }
			int shortPathSize = GetShortPathName(winPath, NULL, 0);
			if (shortPathSize == 0) { return NULL; }
			char * shortPath = (char*) malloc(shortPathSize);
			GetShortPathName(winPath, shortPath, shortPathSize);
			char * retVal = (char*) malloc(strlen(shortPath+shortMapSize) + 5);
			sprintf(retVal, "%c:", 'A' + boucle);
			if (*(shortPath+shortMapSize) != '\\') { strcat(retVal, "\\"); }
			strcat(retVal, shortPath + shortMapSize);
			free(shortPath);
#else
			int pathSize = strlen(instance->driveMap[boucle]);
			char * retVal = (char*) malloc(strlen (winPath) - pathSize + 5);
			sprintf(retVal, "%c:", 'A' + boucle);
			if (*(winPath+shortMapSize) != '\\') { strcat(retVal, "\\"); }
			strcat(retVal, shortPath + shortMapSize);
#endif
			return retVal;
		}
	}	
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
static bool isDriveAvailable (char drive)
{
	char volume[8];
	sprintf(volume, "\\\\.\\%c:", drive);
    HANDLE hDevice = CreateFile(volume, 0, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hDevice == INVALID_HANDLE_VALUE) { return false;}
	DWORD retcount = 0;
	DISK_GEOMETRY diskGeometry;
	bool retVal = (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(diskGeometry), &retcount, (LPOVERLAPPED)NULL) != 0);
	CloseHandle(hDevice);
	return retVal;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

bool setAppIcon( vm::type::VirtualMachine * vm, const char * application)
{
	if (application == NULL)
	{ 
		vm->setWindowIcon(NULL,0,0,0); 
		return true; 
	}

	bool iconChange = false;
	Icon * icon = NULL;
	int extIndex = strlen(application) - 1;

	while (extIndex >= 0 && application[extIndex] != '.' &&  application[extIndex] != '\'' && application[extIndex] != '/')
	{ extIndex--; }

	if (extIndex >= 0 && (application[extIndex] != '\'' && application[extIndex] != '/'))
	{
		char * iconPath = (char *) malloc(extIndex + 5);
		strncpy(iconPath, application, extIndex);
		strcpy(iconPath+extIndex, ".ico");

		try
		{ 
			icon = new Icon(iconPath);
			if (icon->getSize() > 0)
			{
				static const unsigned char iconSearch[][2] = {{32,32},{32,24},{32,8},{32,4},{32,2}, {48,32},{48,24},{48,8},{48,4},{48,2}, {16,32},{16,24},{16,8},{16,4},{16,2} };
				int indexIcon = -1;
				
				for(int boucle1 = 0; boucle1 < 15; boucle1++)
				{
					for(int boucle2 = 0; boucle2 < icon->getSize(); boucle2++)
					{
						Icon::IconDirEntry * entry = icon->getEntryInfo(boucle2);

						if (entry->bHeight == entry->bWidth && entry->bWidth == iconSearch[boucle1][0] && entry->wBitCount == iconSearch[boucle1][1])
						{
							indexIcon = boucle2;
							break;
						}
					}

					if (indexIcon > -1) { break; }
				}
				
				if(indexIcon < 0) { indexIcon = 0; }
				Image * img = icon->getIcon(indexIcon);

				if (img != NULL)
				{
					vm->setWindowIcon(img->getRawData(),img->getWidth(), img->getHeight(), 32);  
					delete img;
					iconChange = true;
				}
			}
		}
		catch(int) { iconChange = false; }

		if (icon != NULL) 
		{ 
			delete icon; 				
			icon = NULL;
		}

		free(iconPath);
	}

	return iconChange;
}

//////////////////////////////////////////////////////////////////////////////////////

LIBRARY_API int VMPLUGIN_CreateInstance(vm::type::VirtualMachine * vm, void * * myInstance)
{
	// Check VM.
	if (vm == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	if (vm->structSize < VM_VirtualMachine_MINSIZE) { return VM_ERROR_BAD_STRUCT_SIZE; }
	const vm::type::VirtualMachineInfo * info = vm->getVmInfo();
	if (info == NULL) return VM_ERROR_NULL_POINTER_EXCEPTION;

	// Check VM version
	if (info->vm_version_minor < VM_VirtualMachine_FCT_COUNT) { return VM_ERROR_UNSUPPORTED_VM_VERSION; }
	if (info->vm_version_major != PLUGIN_MAJOR_VERSION) { return VM_ERROR_UNSUPPORTED_VM_VERSION; }
	if (strncmp (info->name, VM_NAME, VM_SIZEOF_VMNAME) != 0) { return VM_ERROR_UNSUPPORTED_VM_NAME; }

	Instance * instance = (Instance*)malloc(sizeof(Instance));
	for (int boucle = 0; boucle < 42; boucle++) { instance->driveMap[boucle] = NULL; }

#ifndef WIN32
	instance->firstCdRomLetter = DEFAULT_mscdex_first_letter;
	instance->nextCdRomLetter  = NULL;
#endif

	*myInstance = instance;

	return VM_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////////////////

LIBRARY_API void VMPLUGIN_DestroyInstance(vm::type::VirtualMachine * vm, void * myInstance)
{ 
	Instance * instance = (Instance*)myInstance;

	for (int boucle = 0; boucle < 42; boucle++)
	{
		if ((instance->driveMap[boucle] != NULL) && (instance->driveMap[boucle] != IS_ISO))
		{ 
			free(instance->driveMap[boucle]);
			instance->driveMap[boucle] = NULL;
		}
	}

	free(instance);
}

//////////////////////////////////////////////////////////////////////////////////////

LIBRARY_API int VMPLUGIN_PreInit(vm::type::VirtualMachine * vm, void * myInstance)
{
	char cmd [1024];

	if (vm == NULL) { return VM_ERROR_NULL_POINTER_EXCEPTION; }
	if (vm->structSize < sizeof(vm::type::VirtualMachine)) { return VM_ERROR_BAD_STRUCT_SIZE; }
	Instance * instance = (Instance*)myInstance;

	vm->logMessage(VMHOST_LOG_DEBUG, "---");
	vm->logMessage(VMHOST_LOG_DEBUG, PLUGIN_INTRO);
	vm->logMessage(VMHOST_LOG_DEBUG, "---");
	vm->sendCommand("echo off");
	vm->setWindowTitle(vm->getParameter("title"));

	int isoCount = 0;
	const char * paramValue; 
	char tmpAppDrive = '\0';

#ifdef WIN32
	bool useNativeMount = DEFAULT_native_mount;
	bool mountIsoReplace = DEFAULT_mount_iso_replace;
	paramValue = vm->getParameter("mount-iso-replace");
	if (paramValue == NULL);
	else if (strcmpi(paramValue, "true") == 0)  { mountIsoReplace = true;  }
	else if (strcmpi(paramValue, "false") == 0) { mountIsoReplace = false; }
	else { vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'mount-iso-replace'"); }

	paramValue = vm->getParameter("native-mount");
	if (paramValue == NULL);
	else if (strcmpi(paramValue, "true") == 0)  { useNativeMount = true;  }
	else if (strcmpi(paramValue, "false") == 0) { useNativeMount = false; }
	else { vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'native-mount'"); }
#else
	paramValue = vm->getParameter("mscdex-first-letter");
	if (paramValue != NULL)
	{
		int valueSize = (strlen (paramValue));

		switch (valueSize) 
		{ 
			case 0 : 
				break;

			case 1:
				if ((paramValue[0] >= 'a' && paramValue[0] <= 'z') || (paramValue[0] >= 'A' && paramValue[0] <= 'Z'))
				{ instance->firstCdRomLetter = toupper(paramValue[0]); }
				else
				{ vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'mscdex-first-letter'"); }
				break;

			case 2:
				if ((paramValue[1] == ':') && ((paramValue[0] >= 'a' && paramValue[0] <= 'z') || (paramValue[0] >= 'A' && paramValue[0] <= 'Z')))
				{ instance->firstCdRomLetter = toupper(paramValue[0]); }
				else
				{ vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'mscdex-first-letter'"); }
				break;

			default:
				vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'mscdex-first-letter'");
				break;
		}
	}
#endif

	// Mount C:
	const char * systemRoot = vm->getParameter("system-root");
	if (systemRoot == NULL) { instance->tmpAppDrive = 'C'; }
	else
	{
		struct stat buffer;

		if (stat (systemRoot, &buffer) != 0) 
		{ 
			char * msg = (char *)malloc(strlen(systemRoot)+25);
			sprintf(msg, "Directory \"%s\" not found.", systemRoot);
			vm->logMessage(VMHOST_LOG_ERROR, msg);
			free(msg);
		} else if ((buffer.st_mode & _S_IFDIR) == 0) {
			char * msg = (char *)malloc(strlen(systemRoot)+25);
			sprintf(msg, "\"%s\" is not a directory.", systemRoot);
			vm->logMessage(VMHOST_LOG_ERROR, msg);
			free(msg);
		} else {
			MapDevice(vm, instance, "mount %c \"%s\" -label SYSTEM > NUL", 'C', systemRoot);
			vm->sendCommand("C:");

			if (instance->tmpAppDrive == '\0')
			{
				instance->tmpAppDrive = 'Y';
				while ((instance->driveMap['A' - instance->tmpAppDrive] == NULL) && ( instance->tmpAppDrive >= 'A'))
				{ instance->tmpAppDrive--; }

				if (instance->tmpAppDrive < 'A') { instance->tmpAppDrive = 'C'; }
			}
		}
	}

	// Auto-mount drives
	char autoMountFlags;
	paramValue = vm->getParameter("auto-mount");
	if (paramValue == NULL) { autoMountFlags = AUTOMOUNT_DEFAULT; }
	else 
	{
		const char * paramBegin = paramValue;
		const char * paramEnd;

		autoMountFlags = AUTOMOUNT_NONE;

		while(paramBegin != '\0')
		{
			while(*paramBegin == '\t' || *paramBegin == ' ')
			{
				if (*paramBegin == '\0') { break; }
				paramBegin++;
			}

			if (*paramBegin == '\0') { break; }

			paramEnd = paramBegin;

			while(*paramEnd != '\t' && *paramEnd != ' '  && *paramEnd != '\0')
			{
				if (*paramBegin == 0x00) { break; }
				paramEnd++;
			}

			if (paramEnd == paramBegin) { break;  }
			int size = (paramEnd - paramBegin);

			if ((strncmp(paramBegin,"CDROM", size) == 0)  || (strncmp(paramBegin,"cdrom", size) == 0))
			{ autoMountFlags |= AUTOMOUNT_CDROM; }
			
			else if ((strncmp(paramBegin,"FLOPPY", size) == 0)  || (strncmp(paramBegin,"floppy", size) == 0))
			{ autoMountFlags |= AUTOMOUNT_FLOPPY; }

			else if ((strncmp(paramBegin,"REMOVABLE", size) == 0)  || (strncmp(paramBegin,"removable", size) == 0))
			{ autoMountFlags |= AUTOMOUNT_REMOVABLE; }

			else if ((strncmp(paramBegin,"FIXED", size) == 0)  || (strncmp(paramBegin,"fixed", size) == 0))
			{ autoMountFlags |= AUTOMOUNT_FIXED; }

			else if ((strncmp(paramBegin,"NETWORK", size) == 0)  || (strncmp(paramBegin,"network", size) == 0))
			{ autoMountFlags |= AUTOMOUNT_NETWORK; }
			
			else if ((strncmp(paramBegin,"NONE", size) == 0) || (strncmp(paramBegin, "NONE", size) == 0))
			{ 
				autoMountFlags = AUTOMOUNT_NONE; 
				break;
			}

			else if ((strncmp(paramBegin,"ALL", size) == 0)|| (strncmp(paramBegin,"all", size) == 0))
			{ 
				autoMountFlags = AUTOMOUNT_ALL;
				break;
			}

			else { vm->logMessage(VMHOST_LOG_WARNING,  "Bad value of attribute 'auto-mount'"); }
			paramBegin = paramEnd;

		};
	}

	const char * isos = vm->getParameter("iso"); 

#if defined(WIN32)
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Auto-mount windows drives
	char drivePath[] = "?:\\";

	if (useNativeMount)
	{
		char lastDrive = 0x00;
		char lastMscdexDrive = 0x00;
		
		for (char drive = 'A'; drive <= 'Z'; drive++)
		{
			if (drive == 'C') continue; // Don't mount C:
			drivePath[0] = drive;

			switch (GetDriveType(drivePath))
			{
				case DRIVE_NO_ROOT_DIR: break;
				
				case DRIVE_REMOVABLE:
					if ((drive < 'C') && (!(autoMountFlags & AUTOMOUNT_FLOPPY)))    break;
					if ((drive > 'B') && (!(autoMountFlags & AUTOMOUNT_REMOVABLE))) break;
					if (!isDriveAvailable(drive)) break;
					MapDevice(vm, instance, "mount %c %s -floppy %s> NUL", drive, drivePath, true);
					break;
					
				case DRIVE_CDROM:
					if (!(autoMountFlags & AUTOMOUNT_CDROM))
					{	
						if (isos != NULL && instance->mountIsoReplace) 
						{
							if (ImgMount(vm, instance, drive, isos)) { lastMscdexDrive = drive; }
							else if (MapDevice(vm, instance, "mount %c %s -t cdrom -ioctrl > NUL", drive, drivePath)) { lastMscdexDrive = drive; } 
							isos = NULL;
						}
					}
					
					else if (lastMscdexDrive == 0x00)
					{
						// Replace first CD-ROM by ISO(s)
						if (isos != NULL && instance->mountIsoReplace) 
						{
							if (ImgMount(vm, instance, drive, isos)) { lastMscdexDrive = drive; }
							else if (MapDevice(vm, instance, "mount %c %s -t cdrom -ioctrl > NUL", drive, drivePath)) { lastMscdexDrive = drive; } 
							isos = NULL;
						}
						else
						{
							if (MapDevice(vm, instance, "mount %c %s -t cdrom -ioctrl > NUL", drive, drivePath)) { lastMscdexDrive = drive; } 
						}
					}
					else
					{
						if (drive-1 == lastMscdexDrive)
						{
							if (MapDevice(vm, instance, "mount %c %s -t cdrom -ioctrl > NUL", drive, drivePath))
							{ lastDrive = lastMscdexDrive = drive; }
						}
						else
						{
							if (MapDevice(vm, instance, "mount %c %s -t floppy > NUL", drive, drivePath)) { lastDrive = drive; }
						}
					}
					
					lastDrive = drive; 
					break;
				
				case DRIVE_REMOTE:
					if (!(autoMountFlags & AUTOMOUNT_NETWORK)) break;
					if (MapDevice(vm, instance,"mount %c %s %s> NUL", drive, drivePath,true)) { lastDrive = drive; }
					break;
					
				case DRIVE_RAMDISK: 
				case DRIVE_FIXED: 
				case DRIVE_UNKNOWN:
				default:
				    if (!(autoMountFlags & AUTOMOUNT_FIXED)) break;
					if (MapDevice(vm, instance,"mount %c %s %s> NUL", drive, drivePath,true)) { lastDrive = drive; }
					break;
				
			}
		}
		
		// Mount ISO(s)
		if (isos != NULL)
		{
			if (lastMscdexDrive != 0x00)
			{
				drivePath[0] = lastMscdexDrive+1;
				
				if (GetDriveType(drivePath) == DRIVE_NO_ROOT_DIR)
				{
					if (ImgMount(vm, instance, lastMscdexDrive+1, isos))
					{ 
						lastMscdexDrive++; 
						isos = NULL;
				    } 
				}
			}
		
			if (isos != NULL)
			{
				if (lastDrive < 'Y')
				{
					if (ImgMount(vm, instance, lastDrive, isos)) { isos = NULL; } 
				}
				
				if (isos != NULL)
				{
					for (int boucle = 0; boucle < 42; boucle++)
					{
						if (instance->driveMap == NULL)
						{ 
							if (ImgMount(vm, instance, 'A' + boucle, isos)) 
							{ 
								if (lastDrive < boucle) { lastDrive = 'A' + boucle; } 
							}
							break;
						}
					}
				}
			}
		}
	}
	else // Re-map driver letters
	{
		char lastWinDrive = 0x00;
		char nextDrive = 'D';
		
		// Mount hard Disks
		for (char drive = 'A'; drive <= 'Z'; drive++)
		{
			if (drive == 'C') continue; // Don't mount C:
			drivePath[0] = drive;

			switch (GetDriveType(drivePath))
			{
				case DRIVE_NO_ROOT_DIR: break;
				
				case DRIVE_FIXED: 
				case DRIVE_RAMDISK: 
					if (!(autoMountFlags & AUTOMOUNT_FIXED)) break;
					if (MapDevice(vm, instance, "mount %c %s %s> NUL", nextDrive, drivePath, true)) 
					{ 
						nextDrive++; 
						if (nextDrive == 'C') { break; }
					}

				default:
					lastWinDrive=drive;
					break;
			}
		}
		
		// Mount removable devices on A, B as floppy
		char nextFloppy = 'A';
		if (autoMountFlags & AUTOMOUNT_FLOPPY) 
		{
			for (char drive = 'A'; drive < 'C'; drive++)
			{
				if (drive == 'C') continue; // Don't mount C:
				if (!isDriveAvailable(drive)) continue;
				drivePath[0] = drive;

				if (GetDriveType(drivePath) == DRIVE_REMOVABLE)
				{
					if (MapDevice(vm, instance, "mount %c %s -t floppy %s> NUL", nextFloppy, drivePath,true)) 
					{ nextFloppy++; }

					if (nextFloppy > 'B') { break; }
				}
			}
		}

		// Mount removable devices
		char nextRemovableDrive;
		
		if (nextDrive > 'C') { nextRemovableDrive = nextDrive; } else { nextRemovableDrive = 'D'; }
		
		if (autoMountFlags & AUTOMOUNT_REMOVABLE) 
		{
			for (char drive = 'D'; drive <= lastWinDrive; drive++)
			{
				drivePath[0] = drive;

				if ((GetDriveType(drivePath) == DRIVE_REMOVABLE) && (isDriveAvailable(drive)))
				{
					if (MapDevice(vm, instance, "mount %c %s -t floppy %s> NUL", nextRemovableDrive, drivePath, true)) 
					{ 
						nextRemovableDrive++; 
						
						if (nextRemovableDrive > 'B')
						{ 
							if (nextRemovableDrive == 'C') { nextRemovableDrive = nextDrive; }
							else { nextDrive++; }
						}
					}
				}
			}
		}

		
		// Mount CD-ROM drives, ISO first
		if (isos != NULL)
		{
			if (ImgMount(vm, instance, nextDrive, isos)) { nextDrive++; }
			isos = NULL;
		}
	
		if (autoMountFlags & AUTOMOUNT_CDROM)
		{
			for (char drive = 'A'; drive <= lastWinDrive; drive++)
			{
				if (drive == 'C') continue; // Don't mount C:
				drivePath[0] = drive;

				if(GetDriveType(drivePath) == DRIVE_CDROM)
				{
					if (MapDevice(vm, instance, "mount %c %s -ioctrl > NUL", nextDrive, drivePath)) { nextDrive++; }
				}
			}
		}
		nextDrive++; // Reserve 1 empty drive

		
		// Mount network drives
		if (autoMountFlags & AUTOMOUNT_NETWORK)
		{
			for (char drive = 'A'; drive <= lastWinDrive; drive++)
			{
				if (drive == 'C') continue; // Don't mount C:
				drivePath[0] = drive;

				if(GetDriveType(drivePath) == DRIVE_REMOTE)
				{
					if (MapDevice(vm, instance, "mount %c %s %s> NUL", nextDrive, drivePath,true)) { nextDrive++; }
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////

	// Use Windows Keyboard 
	TCHAR  buff [9];
	
	if (GetKeyboardLayoutName(buff))
	{
		int number = (int)strtol(buff+4, NULL, 16);
		if (GetLocaleInfo(number, LOCALE_SISO3166CTRYNAME,buff, sizeof(buff)))
		{
			sprintf(cmd, "keyb %c%c > NUL", buff[0], buff[1]);
			vm->sendCommand(cmd);
		}
	}
#else

	// Mount C:
	if (instance->systemRoot != NULL)
	{
		sprintf(cmd,"mount c \"%s\" > NUL", instance->systemRoot);
		if (vm->sendCommand(cmd) == VM_NO_ERROR) { runAutoexec = true; }
	}

	// Auto-mount CD-ROM drives
	int cdromCount = SDL_CDNumDrives();
	char * cdromDrives = new char [cdromCount];
	instance->nextCdRomLetter = firstCdRomLetter;

	if ((cdromCount > 0) && (instance->autoLoadCdRom))
	{
		for (int boucle=0; boucle < cdromCount; boucle++)
		{
			sprintf(cmd,"mount %c %s -t cdrom -ioctl > NUL", (instance->nextCdRomLetter), SDL_CDName(boucle));
			if (vm->sendCommand(cmd) == VM_NO_ERROR) { instance->nextCdRomLetter++; }

		}
	}
#endif
	return VM_NO_ERROR;
}

