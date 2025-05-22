#pragma once
#include "Framework.h"

namespace Asteria::Holder
{
    inline uint64_t AFortPlayerController_EditSelect = ImageBase + 0x18249C0;
    inline uint64_t AFortPlayerController_PerformBuildingEditInteraction = ImageBase + 0x10E2358;
    inline uint64_t AFortPlayerController_BuildingReset = ImageBase + 0x18249E0;
    inline uint64_t AFortPlayerController_ServerMove = ImageBase + 0x2c38220;
    inline uint64_t SDK_ProcessEvent = uint64_t();

    inline void SetupHolder()
    {
        SDK_ProcessEvent = Memcury::Scanner::FindStringRef<const wchar_t*>(L"UMeshNetworkComponent::ProcessEvent: Invalid mesh network node type: %s", true, 0, FNVer >= 19.00).ScanFor<1>({ 0xE8 }, true, FNVer < 19.00 ? 1 : 3, 0, 2000).RelativeOffset(1).Get();
    }
}

#define __getCounter(_V) _V
#define _getCounter(_V) __getCounter(_V)
#define getCounter() _getCounter(__COUNTER__)

#define randomizeNum(n) (n ^ ((__DATE__[getCounter() % 10] * getCounter()) * (__TIME__[getCounter() % 7] * getCounter()) << (getCounter() % 3)))
#define fuckIda \
if (__int64(_ReturnAddress()) == randomizeNum(0x39d38f4a) && *(uint64*)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x442ba3f9)) > randomizeNum(0x131466bb)) \
{ \
auto _var = *(void**)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x5a81bc4b)); \
auto _var2 = *(void**)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x69ca48cb)); \
auto _var3 = *(void**)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x638a619d)); \
auto _var4 = *(void**)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x4f72e5ca)); \
constexpr unsigned __int8 method2[] = { 0x69 }; \
((void(*)())uintptr_t(method2))(); \
auto _var5 = *(void**)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x15da7681)); \
auto _var6 = *(void**)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x5fc93a05)); \
auto _troll = *(void**)(__int64(_AddressOfReturnAddress()) + randomizeNum(0x3a19cb8a)); \
((void(*)(void*, void*, void*, void*, void*, void*, void*)) _troll)(_troll, _var, _var2, _var3, _var4, _var5, _var6); \
((void(*)())uintptr_t(0x0))(); \
}