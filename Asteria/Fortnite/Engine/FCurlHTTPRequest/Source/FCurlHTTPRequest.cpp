#include "Framework.h"
#include "Asteria/Windows/Header/Windows.h"
#include "../Header/FCurlHTTPRequest.h"

#include "Fortnite/Engine/Server/Header/Server.h"
#include "Fortnite/Engine/UObject/Header/UObject.h"
#include "Fortnite/FortniteGame/FortPlayerController/Header/FortPlayerController.h"
#include "Framework.h"
#include "Asteria/DirectX/Header/DirectX.h"
#include "Asteria/Finder/Header/Finder.h"
#include "Fortnite/Engine/ConsoleClass/Header/ConsoleClass.h"

// credits to https://github.com/plooshi/Starfall for some of the code

namespace Engine::FCurlHTTPRequest
{
    bool (*ProcessRequestOG)(FCurlHttpRequest* Request) = nullptr;
}

static bool HookedExit = false;

bool Engine::FCurlHTTPRequest::ShouldRedirect(const wchar_t* url)
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
    std::wstring URL = Request->GetURL().ToWString();
    
    if (ShouldRedirect(URL.c_str()))
    {
        const auto domains = {
            std::wstring(L"game-social.epicgames.com"),
            std::wstring(L"ol.epicgames.com"),
            std::wstring(L"ol.epicgames.net"),
            std::wstring(L"on.epicgames.com"),
            std::wstring(L"ak.epicgames.com"),
            std::wstring(L"epicgames.dev")
        };

        size_t end = size_t();
        
        for (const auto& d : domains) {
            if (size_t pos = URL.find(d); pos != std::wstring::npos) {
                end = pos + d.length(); 
            }
        }
        
        auto Backend = skCrypt(L"https://api-v1-horizon-fortnite-api.solarisfn.org");
        std::wstring newPath = URL.substr(end);
        std::wstring newUrl = Backend.decrypt() + newPath;
        Request->SetURL(newUrl.c_str());

        if (!HookedExit && newPath.find(L"/fortnite/api/v2/versioncheck/Windows") != std::wstring::npos)
        {
            HookedExit = true;
            Server::Setup();
            Asteria::Finder::InitializeExitHook();
        //    FortniteGame::FortPlayerController::Setup();
            std::this_thread::sleep_for(std::chrono::milliseconds(750));
            UObject::Setup();
            Asteria::Windows::Setup();
            Asteria::DirectX::Setup();
            MH_EnableHook(MH_ALL_HOOKS);
        }

        if (newPath.find(L"/QueryProfile?profileId=athena") != std::wstring::npos)
        {
            static bool bConsole = false;
            APlayerController* LocalPlayer = UWorld::GetWorld()->OwningGameInstance->LocalPlayers[0]->PlayerController;
            if (!bConsole && LocalPlayer->PlayerState->GetPlayerName().ToString() == "itztiva")
            {
                //ConsoleClass::EnableConsole(LocalPlayer);
            }
        }
    }

    auto ret = ProcessRequestOG(Request);
    return ret;
}

void Engine::FCurlHTTPRequest::Setup()
{
    auto sRef = Memcury::Scanner::FindStringRef(L"Could not set libcurl options for easy handle, processing HTTP request failed. Increase verbosity for additional information.");
    auto procRequest = sRef.ScanFor({ 0x40 }, false);
    auto procRequestVft = (void **) Memcury::Scanner::FindVftPtr(procRequest.Get()).Get();
    DWORD dwProtection;
    VirtualProtect(procRequestVft, 8, PAGE_EXECUTE_READWRITE, &dwProtection);
    ProcessRequestOG = decltype(ProcessRequestOG)(*procRequestVft);
    *procRequestVft = ProcessRequest;
    DWORD dwTemp;
    VirtualProtect(procRequestVft, 8, dwProtection, &dwTemp);
}