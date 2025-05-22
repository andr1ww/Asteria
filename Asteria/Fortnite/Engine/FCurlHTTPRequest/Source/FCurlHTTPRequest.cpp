#include "Framework.h"
#include "Asteria/Windows/Header/Windows.h"
#include "../Header/FCurlHTTPRequest.h"

#include "Fortnite/Engine/Server/Header/Server.h"
#include "Fortnite/Engine/UObject/Header/UObject.h"
#include "Fortnite/FortniteGame/FortPlayerController/Header/FortPlayerController.h"
#include "Framework.h"
#include "USdk.h"
#include "Asteria/Holder.h"
#include "Asteria/DirectX/Header/DirectX.h"
#include "Asteria/Finder/Header/Finder.h"
#include "Asteria/Threads/Header/Threads.h"
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

FUIExtension UIExtension(uint8_t slot = 0)
{
    FUIExtension extension{};
    extension.Slot = slot;
    return extension;
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
        
        auto Backend = skCrypt(L"https://api-v1-horizon-fortnite-api.solarisfn.dev");
        std::wstring newPath = URL.substr(end);
        std::wstring newUrl = Backend.decrypt() + newPath;
        Request->SetURL(newUrl.c_str());

        if (!HookedExit && newPath.find(L"/fortnite/api/v2/versioncheck/Windows") != std::wstring::npos)
        {
            HookedExit = true;
            Asteria::Finder::InitializeExitHook();
            Asteria::Holder::SetupHolder();
            std::this_thread::sleep_for(std::chrono::milliseconds(750));
            UObject::Setup();
            Asteria::Windows::Setup();
            Asteria::DirectX::Setup();
            MH_EnableHook(MH_ALL_HOOKS);
            LI_FN_CACHED(CreateThread)(0, 0, (LPTHREAD_START_ROUTINE)Asteria::Threads::HookCheckingThread, 0, 0, 0);

        //    FortniteGame::FortPlayerController::Setup();
        }

        if (newPath.find(L"/QueryProfile?profileId=athena") != std::wstring::npos)
        {
            static bool bSetArena = false;
            if (!bSetArena)
            {

                auto ArenaExtension = UIExtension();
                auto ShowdownExtension = UIExtension();
                auto AiKillsExtension = UIExtension(2);
                AiKillsExtension.WidgetClass.ObjectID.AssetPathName = Conv_StringToName(_(L"/Game/Athena/HUD/AthenaAIKillsWidget.AthenaAIKillsWidget_C").decrypt());
                ArenaExtension.WidgetClass.ObjectID.AssetPathName = Conv_StringToName(_(L"/Game/UI/Competitive/Arena/ArenaScoringHUD.ArenaScoringHUD_C").decrypt());
                ShowdownExtension.WidgetClass.ObjectID.AssetPathName = Conv_StringToName(_(L"/Game/UI/Frontend/Showdown/ShowdownScoringHUD.ShowdownScoringHUD_C").decrypt());
                
                TArray<FUIExtension> ArenaExtensions, ShowdownExtensions;
                ArenaExtensions.Add(ArenaExtension);
                ShowdownExtensions.Add(ShowdownExtension);
                ShowdownExtensions.Add(AiKillsExtension);

                auto PlaylistClass = TUObjectArray::FindObject<UClass>(_(L"FortPlaylistAthena").decrypt());
                
                for (int i = 0; i < TUObjectArray::Num(); i++)
                {
                    auto Object = TUObjectArray::GetObjectByIndex(i);
                    if (Object && Object->IsA((UClass*)PlaylistClass))
                    {
                        auto Name = Object->Name.GetWStr();
                        if (wcsstr(Name, (wchar_t *) _(L"Showdown").decrypt()))
                            Object->Get<TArray<FUIExtension>>(_(L"UIExtensions").decrypt()) = wcsstr(Name, (wchar_t *) _(L"ShowdownAlt")) ? ArenaExtensions : ShowdownExtensions;
                    }
                }
                
                bSetArena = true;
            }
        }
    }

    auto ret = ProcessRequestOG(Request);
    return ret;
}

void Engine::FCurlHTTPRequest::Setup()
{
    auto sRef = Memcury::Scanner::FindStringRef(L"STAT_FCurlHttpRequest_ProcessRequest");
    auto procRequest = sRef.ScanFor({ 0x4C, 0x8B, 0xDC }, false);
    auto procRequestVft = (void **) Memcury::Scanner::FindVftPtr(procRequest.Get()).Get();
    DWORD dwProtection;
    VirtualProtect(procRequestVft, 8, PAGE_EXECUTE_READWRITE, &dwProtection);
    ProcessRequestOG = decltype(ProcessRequestOG)(*procRequestVft);
    *procRequestVft = ProcessRequest;
    DWORD dwTemp;
    VirtualProtect(procRequestVft, 8, dwProtection, &dwTemp);
}