#pragma once
#include "Framework.h"

// credits to https://github.com/plooshi/Starfall

namespace Engine::FCurlHTTPRequest
{
    class FCurlHttpRequest
    {
    public:
        void** VTable;
        FString GetURL() {
            return ((FString & (*)(FCurlHttpRequest*, FString)) * VTable)(this, FString());
        }
        void SetURL(FString url) {
            return ((void (*)(FCurlHttpRequest*, FString&)) VTable[10])(this, url);
        }
        FString GetHeader(FString Key) { 
            FString Result;
            return reinterpret_cast<FString & (__thiscall*)(FCurlHttpRequest*, FString&, FString&)>(VTable[2])(this, Result, Key);
        }
    };

    bool ShouldRedirect(const wchar_t* url);
    bool ProcessRequest(FCurlHttpRequest* Request);
    extern bool (*ProcessRequestOG)(FCurlHttpRequest* Request);
    void Setup();
}
