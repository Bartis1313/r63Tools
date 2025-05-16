#include "api.h"
#ifdef USE_PB_DLL
#include <cstdio>

extern "C" __declspec(dllexport) void __fastcall sb(char* a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6)
{
	if (g_OriginalPbsv)
	{
		FARPROC fSB = GetProcAddress(g_OriginalPbsv, "sb");
		if (fSB)
		{
#ifdef __WIN64
			typedef void(__fastcall* tSB)(char*, __int64, __int64, __int64, __int64, __int64);
			tSB SB = (tSB)fSB;
			return SB(a1, a2, a3, a4, a5, a6);
#elif _WIN32
			typedef void(__cdecl* tSB)(char* a1, int a2, int a3, int a4, int a5);
			tSB SB = (tSB)fSB;
			return SB(a1, a2, a3, a4, a5);
#endif
		}
		else
		{
			printf("sb export not found\n");
		}
	}
	return;
}

extern "C" __declspec(dllexport) int __fastcall sa(__int64 a1, int a2)
{
	if (g_OriginalPbsv)
	{
		FARPROC fSA = GetProcAddress(g_OriginalPbsv, "sa");
		if (fSA)
		{
#ifdef _WIN64
			typedef int(__fastcall* tSA)(__int64, int);
			tSA SA = (tSA)fSA;
			return SA(a1, a2);
#elif _WIN32
			typedef int(__cdecl* tSA)(int a1, int a2);
			tSA SA = (tSA)fSA;
			return SA(a1, a2);
#endif
		}
		else
		{
			printf("sa export not found\n");
		}
	}
	return 0;
}
#endif