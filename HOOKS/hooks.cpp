#include "hooks.h"

#include "../SDK/sdk.h"
#include <MinHook.h>

#define CREATE_SAFE_HOOK(addr, func, original) \
auto hook__##func = MH_CreateHook((LPVOID)addr, func, (LPVOID*)&original); \
if(hook__##func != MH_OK) { printf("%s failed, %s\n", #func, MH_StatusToString(hook__##func)); return FALSE; }

bool hooks::init()
{
	MH_Initialize();

#ifdef _WIN64
	CREATE_SAFE_HOOK(0x1403C3DB0, hkFF_ServerScreenshotReceivedMessage, oFF_ServerScreenshotReceivedMessage);
#endif
	CREATE_SAFE_HOOK(WIN32_64(0x01117A50, 0x1401A8800), hkPBsdk_DropClient, oPBsdk_DropClient);

	if (MH_STATUS all = MH_EnableHook(MH_ALL_HOOKS); all != MH_OK)
		printf("Couldn't enable hooks %s\n", MH_StatusToString(all));
	else
		printf("Hooks status, %s\n", MH_StatusToString(all));

	return true;
}

bool hooks::shutdown()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();

	return true;
}

#include "../MENU/ff.h"

char __fastcall hooks::hkFF_ServerScreenshotReceivedMessage(__int64 a1, fb::ServerPlayer* player, char* buf, unsigned int size)
{
	ff::onScreenshotReceived(player, buf, size);

	return oFF_ServerScreenshotReceivedMessage(a1, player, buf, size);
}

void WIN32_64(__cdecl, __fastcall) hooks::hkPBsdk_DropClient(unsigned int index, char* reason)
{
	return;
}