#pragma once

// use pb way of loadlib call, with replacement
// comment it if own dll load handdling
#define USE_PB_DLL

#ifdef USE_PB_DLL

#include <Windows.h>
#include <cstdio>
#include <filesystem>

HMODULE g_OriginalPbsv = NULL;
#define INSTANCE_PATH std::filesystem::path{ std::filesystem::current_path() / "Instance" }

extern "C" __declspec(dllexport) void __fastcall sb(char* a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6)
{
	if (g_OriginalPbsv)
	{
		FARPROC fSB = GetProcAddress(g_OriginalPbsv, "sb");
		if (fSB)
		{
			typedef void(__fastcall* tSB)(char* a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6);
			tSB SB = (tSB)fSB;
			return SB(a1, a2, a3, a4, a5, a6);
		}
		else
		{
			printf("sb export not found\n");
		}
	}
}
extern "C" __declspec(dllexport) int __fastcall sa(__int64 a1, int a2)
{
	if (g_OriginalPbsv)
	{
		FARPROC fSA = GetProcAddress(g_OriginalPbsv, "sa");
		if (fSA)
		{
			typedef int(__fastcall* tSA)(__int64 a1, int a2);
			tSA SA = (tSA)fSA;
			return SA(a1, a2);
		}
		else
		{
			printf("sa export not found\n");
		}
	}
}
#endif