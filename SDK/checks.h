#pragma once

#include <Windows.h>

constexpr bool IsValidPtr(PVOID p)
{
	// 0x000F000000000000 _PTR_MAX_VALUE
	return (p >= (PVOID)0x10000) && (p < ((PVOID)0x000F000000000000)) && p != nullptr;
}

__forceinline bool IsValidPtrWithAVTable_BF4(PVOID p)
{
	if (IsValidPtr(p))
	{
		__try
		{
			auto vtable = *(void**)p;
			if (IsValidPtr(vtable) && (uintptr_t)vtable > 0x140000000 && (uintptr_t)vtable < 0x1430C6B50)
			{
				return true;
			}
		}
		__except (1)
		{
			return false;
		}
	}
	return false;
}