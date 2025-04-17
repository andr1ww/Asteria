#include "../Header/FCurlHTTPRequest.h"

// credits to https://github.com/plooshi/Starfall

namespace Engine::FCurlHTTPRequest
{
    bool (*ProcessRequestOG)(FCurlHttpRequest* Request) = nullptr;
}

bool Engine::FCurlHTTPRequest::ShouldRedirect(wchar_t* url)
{
    static constexpr const wchar_t* redirectedUrls[] = {
        L"ol.epicgames.com",
        L"ol.epicgames.net",
        L"on.epicgames.com",
        L"game-social.epicgames.com",
        L"ak.epicgames.com",
    };

    std::wstring urlWS = url + 8;
    auto origin = urlWS.substr(0, urlWS.find('/'));

    for (int i = 0; i < sizeof(redirectedUrls) / sizeof(wchar_t*); i++) {
        if (origin.ends_with(redirectedUrls[i])) return true;
    }
    return false;
}

bool Engine::FCurlHTTPRequest::ProcessRequest(FCurlHttpRequest* Request)
{
    auto URL = ((FString & (*)(Engine::FCurlHTTPRequest::FCurlHttpRequest*, FString))Request->VTable[0])(Request, FString()); 

    void *ihatecppstandardlib = nullptr;
    if (ShouldRedirect(URL.CStr()))
    {
        std::wstring URLWithoutHttps = URL.CStr() + 8;
        auto newURL = Asteria::Globals::Backend.decrypt() + URLWithoutHttps.substr(URLWithoutHttps.find(L'/'));
        ihatecppstandardlib = malloc(newURL.size() * 2 + 2);
        memcpy(ihatecppstandardlib, newURL.c_str(), newURL.size() * 2);
        *((wchar_t*)ihatecppstandardlib + newURL.size()) = 0;
        FString NewURL = FString((wchar_t*)ihatecppstandardlib);
        ((void (*)(FCurlHttpRequest*, FString))Request->VTable[10])(Request, NewURL);
    }

    auto ret = ProcessRequestOG(Request);
    if (ihatecppstandardlib) free(ihatecppstandardlib);
    return ret;
}

void Engine::FCurlHTTPRequest::Setup()
{
    auto sRef = Memcury::Scanner::FindStringRef(skCrypt(L"Could not set libcurl options for easy handle, processing HTTP request failed. Increase verbosity for additional information."));
    auto procRequest = sRef.ScanFor({ 0x40 }, false);
    auto procRequestVft = (void **) Memcury::Scanner::FindVftPtr(procRequest.Get()).Get();
    DWORD dwProtection;
    VirtualProtect(procRequestVft, 8, PAGE_EXECUTE_READWRITE, &dwProtection);
    ProcessRequestOG = decltype(ProcessRequestOG)(*procRequestVft);
    *procRequestVft = ProcessRequest;
    DWORD dwTemp;
    VirtualProtect(procRequestVft, 8, dwProtection, &dwTemp);
}
