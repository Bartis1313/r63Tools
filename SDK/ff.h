#pragma once

#include <cstdint>

namespace FF
{
	enum ScreenshotFormat
	{
		ScreenshotFormat_Targa, //0x0000
		ScreenshotFormat_Png, //0x0001
		ScreenshotFormat_Png16, //0x0002
		ScreenshotFormat_Jpeg, //0x0003
		ScreenshotFormat_JpegHighCompression, //0x0004
		ScreenshotFormat_OpenExr, //0x0005
		ScreenshotFormat_RawData //0x0006
	};

	struct GameBlocksScreenV1
	{
		uint32_t m_width;
		uint32_t m_height;
		bool m_adjustSizeWithAspectRatio;
		ScreenshotFormat m_screenshotFormat;
	};
	// 0xAD0 + 0x28 = player

	inline char requestScreen(fb::ServerPlayer* player, const FF::GameBlocksScreenV1& format, char unkType = 0)
	{
		typedef char(__fastcall* tRequestScreenshot1)(__int64 a1, fb::ServerPlayer* player, const FF::GameBlocksScreenV1& a3, char unk);
		tRequestScreenshot1 ffRequestScreenshot1 = (tRequestScreenshot1)0x140156C50;

		return ffRequestScreenshot1(*(__int64*)0x14265D3B0, player, format, unkType);
	}
}