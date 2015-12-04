#pragma

#if defined (WIN32)

#pragma comment(lib, "user32.lib")
#include <windows.h>

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{ 
	/*
	if (dwReason == DLL_PROCESS_ATTACH) {}
	else if (dwReason == DLL_PROCESS_DETACH) {}
	*/
	return TRUE;
}

#endif
