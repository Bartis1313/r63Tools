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

	enum ScreenType
	{
		SUCCESS, // 0 seem always ok
		UNK_1,
		UNK_2,
		UNK_3
	};

	struct GameBlocksScreenV1
	{
		uint32_t m_width;
		uint32_t m_height;
		bool m_adjustSizeWithAspectRatio;
		ScreenshotFormat m_screenshotFormat;
	};
	// 0xAD0 + 0x28 = player
}