#pragma once

#ifdef _WIN64
#define WIN32_64(x86, x64) x64
#else
#define WIN32_64(x86, x64) x86
#endif

//#define USE_PB_DLL

#ifdef USE_PB_DLL
#include <Windows.h>
#include <filesystem>

inline HMODULE g_OriginalPbsv = NULL;
#define INSTANCE_PATH std::filesystem::path{ std::filesystem::current_path() / "Instance" }

extern "C" __declspec(dllexport) void __fastcall sb(char* a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6);
extern "C" __declspec(dllexport) int  __fastcall sa(__int64 a1, int a2);

#endif