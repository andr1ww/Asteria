#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <d3d11.h>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <map>
#include <intrin.h>
#include <sstream>
#include <array>
#include <tlhelp32.h>
#include <future>
#include <set>
#include <Psapi.h>
#include <immintrin.h>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <string>
#include <iostream>
#include <filesystem>
#include <string> 
#include <wintrust.h>
#include <softpub.h>
#include <tlhelp32.h>
#include <WinDNS.h>
#include <winsock.h>
#include <fstream>
#include <AclAPI.h>
#include <sstream>
#include <iomanip>
#include <map>
#include <array>
#include <Psapi.h>
#include <detours.h>
#include <mscat.h>
//#include "SDK.hpp"
#include "SDK/Basic.hpp"
#include "SDK/BlueprintContext_classes.hpp"
#include "SDK/CoreUObject_classes.hpp"
#include "SDK/CoreUObject_structs.hpp"
#include "SDK/Engine_classes.hpp"
#include "SDK/Engine_structs.hpp"
#include "SDK/FortniteGame_classes.hpp"
#include "SDK/FortniteGame_structs.hpp"
#include "SDK/FortniteUI_classes.hpp"
#include "SDK/FortniteUI_structs.hpp"
#include <shellapi.h>
using namespace SDK;
// asteria include
#include "Libraries/Include.h"

namespace Asteria {}

using namespace std;

#define Log(...) do { \
std::ostringstream ss; \
ss << __VA_ARGS__; \
printf("[Asteria] %s (%s:%d) --> %s\n", __func__, __FILE__, __LINE__, ss.str().c_str()); \
} while(0)
inline uint64_t ImageBase = *(uint64_t*)(__readgsqword(0x60) + 0x10);
inline uint64_t ImageSize;
    static std::vector<std::wstring> args = []() {
        std::vector<std::wstring> result;
        LPWSTR* argv;
        int argc;

        argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv) {
            for (int i = 0; i < argc; ++i) {
                result.emplace_back(argv[i]);
            }
            LocalFree(argv);
        }
        return result;
    }();

static inline void *_NpFH = nullptr;

__forceinline static void _HookVT(uintptr_t *_Vt, uint32_t _Ind, void *_Detour)
{
    DWORD _Vo;
    VirtualProtect(_Vt + _Ind, 8, PAGE_EXECUTE_READWRITE, &_Vo);
    _Vt[_Ind] = reinterpret_cast<uintptr_t>(_Detour);
    VirtualProtect(_Vt + _Ind, 8, _Vo, &_Vo);
}

template <typename _Ct, typename _Ot = void *>
__forceinline static void Hook(uint32_t _Ind, void *_Detour, _Ot &_Orig = _NpFH)
{
    auto _Vt = _Ct::GetDefaultObj()->VTable;
    if (!is_same_v<_Ot, void *>)
        _Orig = (_Ot)_Vt[_Ind];

    _HookVT(reinterpret_cast<uintptr_t *>(_Vt), _Ind, _Detour);
}