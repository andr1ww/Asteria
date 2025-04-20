#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <thread>
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
#include "Asteria/Globals.h"

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
