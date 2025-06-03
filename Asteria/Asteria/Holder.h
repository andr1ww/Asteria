#pragma once
#include "Framework.h"

namespace Asteria::Holder
{
    inline uint64_t AFortPlayerController_EditSelect = uint64_t();
    inline uint64_t AFortPlayerController_PerformBuildingEditInteraction = uint64_t();
    inline uint64_t AFortPlayerController_BuildingReset = uint64_t();
    inline uint64_t AFortPlayerController_CompleteBuildingEditInteraction = uint64_t();
    inline uint64_t SDK_ProcessEvent = uint64_t();
    inline wstring Backend = L"";

    inline void InitBackend()
    {
        try {
            for (const auto& arg : args) {
                if (arg.find(L"-a=") == 0 && arg.length() > 3) {
                    string s(arg.substr(3).begin(), arg.substr(3).end()), d, p;
                    const string c = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
                    int v = 0, b = -8;
                    for (char x : s) { 
                        if (c.find(x) == string::npos) break; 
                        v = (v << 6) + c.find(x); 
                        b += 6; 
                        if (b >= 0) { 
                            d.push_back(char((v >> b) & 0xFF)); 
                            b -= 8; 
                        } 
                    }
                    size_t f = d.find('.'), z = d.find('.', f + 1); 
                    if (f == string::npos || z == string::npos) continue;
                    string pb = d.substr(f + 1, z - f - 1); 
                    while (pb.length() % 4) pb += '=';
                
                    v = 0; b = -8; 
                    for (char x : pb) { 
                        if (c.find(x) == string::npos) break; 
                        v = (v << 6) + c.find(x); 
                        b += 6; 
                        if (b >= 0) { 
                            p.push_back(char((v >> b) & 0xFF)); 
                            b -= 8; 
                        } 
                    }
                    string safe_payload = p.length() > 500 ? p.substr(0, 500) + "..." : p;
                    string search_pattern = "\"issuer\":";
                    size_t issuer_pos = p.find(search_pattern);
                    if (issuer_pos != string::npos) {
                        size_t value_start = issuer_pos + search_pattern.length();
                        while (value_start < p.length() && (p[value_start] == ' ' || p[value_start] == '\t')) {
                            value_start++;
                        }
                        if (value_start < p.length() && p[value_start] == '"') {
                            value_start++; 
                            size_t value_end = value_start;
                            while (value_end < p.length() && p[value_end] != '"') {
                                value_end++;
                            }
                        
                            if (value_end < p.length() && value_end > value_start) {
                                string iss = p.substr(value_start, value_end - value_start);
                                if (!iss.empty() && iss.length() < 200) {
                                    constexpr int bufSize = 256;
                                    char buffer[bufSize] = {0};
                                
                                    size_t copy_len = min(iss.length(), static_cast<size_t>(bufSize - 1));
                                    strncpy_s(buffer, bufSize, iss.c_str(), copy_len);
                                    buffer[copy_len] = '\0'; 

                                    auto crypter = skc::skCrypter<bufSize, __TIME__[4], __TIME__[7], char>(buffer);
                                    char* decrypted = crypter.decrypt();
                                
                                    if (decrypted != nullptr) {
                                        size_t decrypted_len = strnlen(decrypted, bufSize);
                                        Backend = wstring(decrypted, decrypted + decrypted_len);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (...) {
            Log("stupid");
        }
    }
    
    inline void SetupHolder()
    {
        SDK_ProcessEvent = Memcury::Scanner::FindStringRef<const wchar_t*>(_(L"UMeshNetworkComponent::ProcessEvent: Invalid mesh network node type: %s").decrypt(), true, 0, FNVer >= 19.00)
            .ScanFor<1>({ 0xE8 }, true, FNVer < 19.00 ? 1 : 3, 0, 2000).RelativeOffset(1).Get();
        
        if (!SDK_ProcessEvent)
        {
            Log("Failed to find SDK_ProcessEvent");
        }

        auto editModeRef = Memcury::Scanner::FindStringRef<const wchar_t*>(_(L"EditModeInputComponent0").decrypt(), true, 0).Get();
        if (editModeRef)
        {
            auto editSelectAddr = Memcury::Scanner(editModeRef).ScanFor(std::array<uint8_t, 3>{ 0x48, 0x8D, 0x05 }, true, 1, 1, 2048).Get();
            if (editSelectAddr)
                AFortPlayerController_EditSelect = Memcury::Scanner(editSelectAddr).RelativeOffset(3).Get();

            auto resetSelectAddr = Memcury::Scanner(editModeRef).ScanFor(std::array<uint8_t, 3>{ 0x48, 0x8D, 0x05 }, true, 2, 1, 2048).Get();
            if (resetSelectAddr)
                AFortPlayerController_BuildingReset = Memcury::Scanner(resetSelectAddr).RelativeOffset(3).Get();

            auto completeEditRef = Memcury::Scanner::FindStringRef("CompleteBuildingEditInteraction", true, 0, editModeRef, 0x300).Get();
            if (completeEditRef)
            {
                auto completeEditAddr = Memcury::Scanner(completeEditRef).ScanFor(std::array<uint8_t, 2>{ 0x48, 0x8D }, false, 0, 1, 0x50).Get();
                if (completeEditAddr)
                {
                    AFortPlayerController_CompleteBuildingEditInteraction = Memcury::Scanner(completeEditAddr).RelativeOffset(3).Get();
                }
            }

            auto performEditRef = Memcury::Scanner::FindStringRef("PerformBuildingEditInteraction", true, 0, editModeRef, 0x300).Get();
            if (performEditRef)
            {
                auto performEditAddr = Memcury::Scanner(performEditRef).ScanFor(std::array<uint8_t, 2>{ 0x48, 0x8D }, true, 0, 1, 0x50).Get();
                if (performEditAddr)
                {
                    AFortPlayerController_PerformBuildingEditInteraction = Memcury::Scanner(performEditAddr).RelativeOffset(3).Get();
                }
            }
        }
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