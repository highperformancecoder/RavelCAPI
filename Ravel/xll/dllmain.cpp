// dllmain.cpp
// Copyright (c) 2006 KALX, LLC. All rights reserved. No warranty is made.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "auto.h"

HMODULE hModule;

#pragma warning(disable: 4100)
extern "C"
BOOL WINAPI
DllMain(HMODULE hDLL, ULONG reason, LPVOID lpReserved)
{
	switch (reason) {
		case DLL_PROCESS_ATTACH: 
			hModule = hDLL;
			DisableThreadLibraryCalls(hDLL);		
			break;		
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}
