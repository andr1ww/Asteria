#include "Framework.h"
#include "Fortnite/Engine/FCurlHTTPRequest/Header/FCurlHTTPRequest.h"

DWORD WINAPI Main(LPVOID)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
    MH_Initialize();
    MH_EnableHook(MH_ALL_HOOKS);

    Engine::FCurlHTTPRequest::Setup();
    Asteria::Finder::GetExit();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main, NULL, 0, NULL); 
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
	
    return TRUE;
}

