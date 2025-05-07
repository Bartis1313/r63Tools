// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID  lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		
	}
	return TRUE;
}

