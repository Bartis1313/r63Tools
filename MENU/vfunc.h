#pragma once

#include <cstdint>
#include <cassert>

namespace vfunc
{
    template <typename T>
    inline void* getVFunc(void* thisptr, size_t index)
    {
        assert(thisptr != nullptr && "thisptr is NULL");
        return (*static_cast<void***>(thisptr))[index];
    }

    // use for any function that does not represent packed args and can be thiscall
    template <typename T, size_t index, typename ... Args_t>
    inline constexpr T callVFunc(void* thisptr, Args_t... args)
    {
#if defined(_WIN64)
        using virtualFunction = T(__fastcall***)(void*, Args_t...);
#else
        using virtualFunction = T(__thiscall***)(void*, Args_t...);
#endif
        auto function = reinterpret_cast<virtualFunction>(getVFunc(thisptr, index));
        return function(thisptr, args...);
    }

    // ONLY for packed arguments in base case !!!
    template<typename T, size_t index, typename... Args_t>
    inline constexpr T callVPack(void* thisptr, Args_t... args)
    {
        using packedVirtualFunction = T(__cdecl*)(void*, Args_t...);
        auto function = reinterpret_cast<packedVirtualFunction>(getVFunc(thisptr, index));
        return function(thisptr, args...);
    }
}
// type - type of function
// name - function's name
// index - vtable index for this method
// args - args to pass eg: (int value)
// variables - variables from args, the starting will always be a pointer to class, eg: (this, value)
#define VFUNC(type, name, index, args, variables) \
type name args { \
	return vfunc::callVFunc<type, index>variables; \
}
