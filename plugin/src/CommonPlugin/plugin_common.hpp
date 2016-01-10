/**
 * \brief Common source code for DosPlugin and Win16Plugin.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */
#pragma once
#ifndef __JREKCED_PLUGIN_COMMON_HPP__
#define __JREKCED_PLUGIN_COMMON_HPP__

#include "vm_host.h"
#include "iconset.hpp"

#ifdef WIN32
	#define LIBRARY_API extern "C" __declspec(dllexport)
#else
	#define LIBRARY_API
#endif

#define DEFAULT_native_mount         true
#define DEFAULT_mount_iso_replace    true
#define DEFAULT_pause_before_exit    false
#define DEFAULT_mscdex_first_letter 'K'

#define AUTOMOUNT_NONE          (0)
#define AUTOMOUNT_CDROM         (1)
#define AUTOMOUNT_REMOVABLE     (2)
#define AUTOMOUNT_FLOPPY        (4)
#define AUTOMOUNT_FIXED   		(8)
#define AUTOMOUNT_NETWORK       (16)

#if defined WIN32
#	define AUTOMOUNT_ALL      (AUTOMOUNT_REMOVABLE | AUTOMOUNT_FLOPPY | AUTOMOUNT_FIXED | AUTOMOUNT_CDROM | AUTOMOUNT_NETWORK)
#	define AUTOMOUNT_DEFAULT  (AUTOMOUNT_FLOPPY | AUTOMOUNT_CDROM)
#else
#	define AUTOMOUNT_ALL      (AUTOMOUNT_CDROM)
#	define AUTOMOUNT_DEFAULT  (AUTOMOUNT_CDROM)
#endif

//////////////////////////////////////////////////////////////////////////////////////

struct Instance /**< Plugin's instance. */
{
#if defined (WIN32)
	bool          mountIsoReplace;
#else
	char          firstCdRomLetter;
	char          nextCdRomLetter;
#endif
	char     ( * ( driveMap[42] ));
	char           tmpAppDrive;
};

//////////////////////////////////////////////////////////////////////////////////////

bool setAppIcon( vm::type::VirtualMachine * vm, const char * application);
char * toMappedDosPath(Instance * instance, const char * winPath);
bool ImgMount (vm::type::VirtualMachine * vm, Instance * instance, char drive, const char * isos);
bool MapDevice (vm::type::VirtualMachine * vm, Instance * instance, char * cmdPattern, char drive, const char * path, bool setLabel = false);
bool UnmapDevice (vm::type::VirtualMachine * vm, Instance * instance, char drive);

#endif
