#pragma once

#include <Windows.h>

__forceinline bool IsValidPtr(void* p)
{
	constexpr uintptr_t MIN_PTR = 0x10000;
#ifdef _WIN64
	constexpr uintptr_t MAX_PTR = 0x000F000000000000;
#else
	constexpr uintptr_t MAX_PTR = 0xFFF00000;
#endif

	uintptr_t addr = reinterpret_cast<uintptr_t>(p);
	return p != nullptr && addr >= MIN_PTR && addr < MAX_PTR;
}

__forceinline bool IsValidPtrWithAVTable(void* p)
{
    if (!IsValidPtr(p))
        return false;

    __try
    {
        void* vtable = *(void**)p;
        if (!IsValidPtr(vtable))
            return false;

#ifdef _WIN64
        constexpr uintptr_t VTABLE_MIN = 0x140000000;
        constexpr uintptr_t VTABLE_MAX = 0x1430C6B50;
#else
        constexpr uintptr_t VTABLE_MIN = 0x00401000;
        constexpr uintptr_t VTABLE_MAX = 0x02CC0FFF;
#endif

        uintptr_t vtAddr = reinterpret_cast<uintptr_t>(vtable);
        return vtAddr >= VTABLE_MIN && vtAddr < VTABLE_MAX;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}