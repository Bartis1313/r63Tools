#include "api.h"

#include "MENU/menu.h"
#include "HOOKS/hooks.h"
#include <filesystem>

DWORD WINAPI menuThread([[maybe_unused]] LPVOID)
{
	menu::drawInternal();
	return TRUE;
}

DWORD WINAPI init([[maybe_unused]] LPVOID)
{
#ifdef USE_PB_DLL
	g_OriginalPbsv = LoadLibraryA(std::filesystem::path{ INSTANCE_PATH / "pb" / "real_pbsv.d64" }.string().c_str());
#endif
	hooks::init();

	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID  lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CloseHandle(CreateThread(0, 0, init, 0, 0, 0));
		CloseHandle(CreateThread(0, 0, menuThread, 0, 0, 0));
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		hooks::shutdown();
	}
	return TRUE;
}


