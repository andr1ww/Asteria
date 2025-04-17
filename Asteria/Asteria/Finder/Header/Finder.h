#pragma once
#include "Framework.h"

namespace Asteria::Finder {
    uint64_t FindRequestExit();
    uint64_t FindUnsafeEnvironment();

    inline void RequestExitHook(unsigned __int8 a1) {}

    inline __int64 __fastcall UnsafeEnvironmentPopupHook(__int64* a1, char a2, __int64 a3, char a4) { return 0; };

    inline uint64_t UnsafeEnvironmentPopup = 0;
    inline uint64_t RequestExit = 0;
    void GetExit();
    void InitializeExitHook();
}