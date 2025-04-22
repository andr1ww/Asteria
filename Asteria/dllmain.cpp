#include "Framework.h"
#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/External/Header/External.h"
#include "Fortnite/Engine/FCurlHTTPRequest/Header/FCurlHTTPRequest.h"
#include "Fortnite/Engine/UObject/Header/UObject.h"
#include <Asteria/Windows/Header/Windows.h>

#include "Asteria/Finder/Header/Finder.h"

DWORD WINAPI Main(LPVOID dllBase)
{
    MODULEINFO modInfo;
    Asteria::Windows::ThisBuf = dllBase;
    LI_FN_CACHED(K32GetModuleInformation)(LI_FN_CACHED(GetCurrentProcess)(), (HMODULE)dllBase, &modInfo, sizeof(MODULEINFO));
    Asteria::Windows::ThisSize = modInfo.SizeOfImage;

    modInfo = {};

    LI_FN_CACHED(K32GetModuleInformation)(LI_FN_CACHED(GetCurrentProcess)(), (HMODULE)ImageBase, &modInfo, sizeof(MODULEINFO));
    ImageSize = modInfo.SizeOfImage;

    if (Asteria::Globals::bConsole)
    {
        LI_FN(AllocConsole)()();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "r", stdin);
    }
    
    LI_FN(MH_Initialize)()();
    LI_FN(Sleep)(1000);
    Engine::FCurlHTTPRequest::Setup();
    Asteria::Finder::GetExit();
    Asteria::Socket::Client::Setup(skCrypt("ws://asteria.solarisfn.org:2082").decrypt());
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        LI_FN_CACHED(CreateThread)(NULL, 0, (LPTHREAD_START_ROUTINE)Main, hModule, 0, NULL); 
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
	
    return TRUE;
}

