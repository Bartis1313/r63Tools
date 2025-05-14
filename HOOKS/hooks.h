#pragma once

namespace fb
{
	class ServerPlayer;
}

namespace hooks
{
	bool init();
	bool shutdown();

	typedef char(__fastcall* tFF_ServerScreenshotReceivedMessage)(__int64, fb::ServerPlayer*, char*, unsigned int);
	inline tFF_ServerScreenshotReceivedMessage oFF_ServerScreenshotReceivedMessage = 0;
	char __fastcall hkFF_ServerScreenshotReceivedMessage(__int64 a1, fb::ServerPlayer* player, char* buf, unsigned int size);

	typedef void(__fastcall* tPBsdk_DropClient)(unsigned int, char*);
	inline tPBsdk_DropClient oPBsdk_DropClient = 0;
	void __fastcall hkPBsdk_DropClient(unsigned int index, char* reason);
}