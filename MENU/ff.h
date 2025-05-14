#pragma once

namespace fb
{
	class ServerPlayer;
}

namespace ff
{
	void draw();
	void onScreenshotReceived(fb::ServerPlayer* player, char* buf, unsigned int size);
}