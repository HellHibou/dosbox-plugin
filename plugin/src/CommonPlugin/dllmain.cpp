/**
 * \brief Windows main DLL file.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#pragma once

#if defined (WIN32)

#pragma comment(lib, "user32.lib")
#include <windows.h>

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{ return TRUE; }

#endif
