#pragma once

#include "../api.h"

// server relative E8 ? ? ? ? 4C 8B F8 48 8B 9C 24 r63
// server relative E8 ? ? ? ? 8D 4C 24 44 E8 ? ? ? ? 8B D8 r38
#define OFFSET_GETENTITYLIST		WIN32_64(0x5D2AB0, 0x140934B90)
// server relative 48 8B 05 ? ? ? ? 4C 8B C2 r63
// server 8B 0D ? ? ? ? 89 48 04 A3 ? ? ? ? B8 ? ? ? ? r38
#define OFFSET_FIRSTTYPEINFO		WIN32_64(0x2886EB0, 0x1426DAC60)
// server relative 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 44 0F B6 C6 r63
// server A1 ? ? ? ? 89 98 ? ? ? ? 8B 46 24 8B 4E 20 r38
#define RESOURCE_MANAGER			WIN32_64(0x02B17908, 0x142B72F30)
// server relative 48 8B 15 ? ? ? ? 49 8D 4A r63
// server 8B 15 ? ? ? ? 80 BA ? ? ? ? ? 75 15 83 C8 08 r38
#define OFFSET_GAMEWORLD			WIN32_64(0x2B3EE30, 0x142C71AD0)
// server 48 8B 15 ? ? ? ? 49 8B CE FF 90 ? ? ? ? 49 8B 3F 49 8B AE ? ? ? ? 48 3B FD 74 1B 0F 1F 00 r63
// server 8B 0D ? ? ? ? 8D 46 14 50 68 ? ? ? ? E8 ? ? ? ? 5E C2 04 00 r38
#define OFFSET_GAMECONTEXT			WIN32_64(0x2B3EE2C, 0x142C68148)