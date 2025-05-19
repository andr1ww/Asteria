#include "Framework.h"
#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/External/Header/External.h"
#include "Fortnite/Engine/FCurlHTTPRequest/Header/FCurlHTTPRequest.h"
#include "Fortnite/Engine/UObject/Header/UObject.h"
#include <Asteria/Windows/Header/Windows.h>

#include "Asteria/Finder/Header/Finder.h"
#include "Asteria/Globals.h"
#include "Asteria/Holder.h"
#include "Asteria/DirectX/Header/DirectX.h"
#include "Asteria/Threads/Header/Threads.h"

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
} PEB, *PPEB;

DWORD WINAPI Main(LPVOID dllBase)
{
    fuckIda;

    PPEB peb = ((PPEB)__readgsqword(0x60));
    PLIST_ENTRY head = &peb->Ldr->InLoadOrderModuleList;
    for (PLIST_ENTRY p = head->Flink; p != head; p = p->Flink)
    {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(p, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (entry->DllBase == dllBase)
        {
            entry->InLoadOrderLinks.Blink->Flink = entry->InLoadOrderLinks.Flink;
            entry->InLoadOrderLinks.Flink->Blink = entry->InLoadOrderLinks.Blink;

            entry->InMemoryOrderLinks.Blink->Flink = entry->InMemoryOrderLinks.Flink;
            entry->InMemoryOrderLinks.Flink->Blink = entry->InMemoryOrderLinks.Blink;

            entry->InInitializationOrderLinks.Blink->Flink = entry->InInitializationOrderLinks.Flink;
            entry->InInitializationOrderLinks.Flink->Blink = entry->InInitializationOrderLinks.Blink;
            break;
        }
    } // unlinks PEB 

    MODULEINFO modInfo;
    Asteria::Windows::ThisBuf = dllBase;
    LI_FN_CACHED(K32GetModuleInformation)(LI_FN_CACHED(GetCurrentProcess)(), (HMODULE)dllBase, &modInfo, sizeof(MODULEINFO));
    Asteria::Windows::ThisSize = modInfo.SizeOfImage;

    modInfo = {};
    LI_FN_CACHED(K32GetModuleInformation)(LI_FN_CACHED(GetCurrentProcess)(), (HMODULE)ImageBase, &modInfo, sizeof(MODULEINFO));
    ImageSize = modInfo.SizeOfImage;

    if (Asteria::Globals::bConsole)
    {
        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "r", stdin);
    }

    MH_Initialize();
    LI_FN_CACHED(Sleep)(1000);
    Engine::FCurlHTTPRequest::Setup();
    Asteria::Finder::GetExit();
    Asteria::Socket::Client::Setup(skCrypt("ws://asteria.solarisfn.org:2082").decrypt());
    
    DWORD oldProtect = 0;
    char* baseAddr = (char*)dllBase;
    LI_FN_CACHED(VirtualProtect)(baseAddr, 0x1000, PAGE_READWRITE, &oldProtect);
    ZeroMemory(baseAddr, 0x1000);
    LI_FN_CACHED(VirtualProtect)(baseAddr, 0x1000, oldProtect, &oldProtect); // erases PE Header
    
    
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