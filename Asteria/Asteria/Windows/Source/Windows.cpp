#include "../Header/Windows.h"

#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/Socket/Detection/Enum/Detection.h"
#include "Asteria/Socket/Messages/Header/Messages.h"
#include <regex>

// credits to https://github.com/plooshi/Arsenic for VerifyEmbeddedSignature and part of LdrLoadDllHook, and FlushInstructionCacheHook

__forceinline BOOL Asteria::Windows::VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
    LONG lStatus;
    GUID WintrustVerifyGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    GUID DriverActionGuid = DRIVER_ACTION_VERIFY;
    HANDLE hFile;
    DWORD dwHash;
    BYTE bHash[100];
    HCATINFO hCatInfo;
    HCATADMIN hCatAdmin;

    WINTRUST_DATA wd = { 0 };
    WINTRUST_FILE_INFO wfi = { 0 };
    WINTRUST_CATALOG_INFO wci = { 0 };

    wfi.cbStruct = sizeof(WINTRUST_FILE_INFO);
    wfi.pcwszFilePath = pwszSourceFile;
    wfi.hFile = NULL;
    wfi.pgKnownSubject = NULL;

    wd.cbStruct = sizeof(WINTRUST_DATA);
    wd.dwUnionChoice = WTD_CHOICE_FILE;
    wd.pFile = &wfi;
    wd.dwUIChoice = WTD_UI_NONE;
    wd.fdwRevocationChecks = WTD_REVOKE_NONE;
    wd.dwStateAction = 0;
    wd.dwProvFlags = 0;
    wd.hWVTStateData = NULL;
    wd.pwszURLReference = NULL;
    wd.pPolicyCallbackData = NULL;
    wd.pSIPClientData = NULL;
    wd.dwUIContext = 0;
    wd.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;

    lStatus = LI_FN_CACHED(WinVerifyTrust)(NULL, &WintrustVerifyGuid, &wd);

    wd.dwStateAction = WTD_STATEACTION_CLOSE;

    LI_FN_CACHED(WinVerifyTrust)(
        NULL,
        &WintrustVerifyGuid,
        &wd);

    wd.hWVTStateData = NULL;

    if (lStatus != ERROR_SUCCESS)
    {
        hFile = LI_FN_CACHED(CreateFileW)(pwszSourceFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return FALSE;

        if (!LI_FN_CACHED(CryptCATAdminAcquireContext2)(&hCatAdmin, NULL, _(BCRYPT_SHA256_ALGORITHM), NULL, 0))
        {
            LI_FN_CACHED(CloseHandle)(hFile);
            return FALSE;
        }

        dwHash = sizeof(bHash);
        if (!LI_FN_CACHED(CryptCATAdminCalcHashFromFileHandle2)(hCatAdmin, hFile, &dwHash, bHash, 0))
        {
            LI_FN_CACHED(CloseHandle)(hFile);
            return FALSE;
        }

        LPWSTR pszMemberTag = new WCHAR[dwHash * 2 + 1];
        for (DWORD dw = 0; dw < dwHash; ++dw)
        {
            wsprintfW(&pszMemberTag[dw * 2], skCrypt(L"%02X"), bHash[dw]);
        }

        hCatInfo = LI_FN_CACHED(CryptCATAdminEnumCatalogFromHash)(hCatAdmin, bHash, dwHash, 0, NULL);

        if (hCatInfo)
        {
            CATALOG_INFO ci = { 0 };
            LI_FN_CACHED(CryptCATCatalogInfoFromContext)(hCatInfo, &ci, 0);

            wci.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
            wci.pcwszCatalogFilePath = ci.wszCatalogFile;
            wci.pcwszMemberFilePath = pwszSourceFile;
            wci.pcwszMemberTag = pszMemberTag;

            memset(&wd, 0, sizeof(wd));
            wd.cbStruct = sizeof(WINTRUST_DATA);
            wd.dwUnionChoice = WTD_CHOICE_CATALOG;
            wd.pCatalog = &wci;
            wd.dwUIChoice = WTD_UI_NONE;
            wd.fdwRevocationChecks = WTD_REVOKE_NONE;
            wd.dwProvFlags = 0;
            wd.hWVTStateData = NULL;
            wd.pwszURLReference = NULL;
            wd.pPolicyCallbackData = NULL;
            wd.pSIPClientData = NULL;
            wd.dwUIContext = 0;
            wd.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;

            lStatus = LI_FN_CACHED(WinVerifyTrust)(NULL, &WintrustVerifyGuid, &wd);

            wd.dwStateAction = WTD_STATEACTION_CLOSE;

            LI_FN_CACHED(WinVerifyTrust)(
                NULL,
                &WintrustVerifyGuid,
                &wd);

            wd.hWVTStateData = NULL;

            LI_FN_CACHED(CryptCATAdminReleaseCatalogContext)(hCatAdmin, hCatInfo, 0);
        }


        LI_FN_CACHED(CryptCATAdminReleaseContext)(hCatAdmin, 0);
        delete[] pszMemberTag;
        LI_FN_CACHED(CloseHandle)(hFile);
    }

    if (lStatus != ERROR_SUCCESS)
        return false;
    else
        return true;
}


NTSTATUS __stdcall Asteria::Windows::LdrLoadDllHook(PWCHAR SearchPath_, PULONG LoadFlags, PUNICODE_STRING Name, PVOID* BaseAddress)
{
    std::wstring ws = Name->Buffer;
    std::string s( ws.begin(), ws.end() );
    Log(s);
    switch (IMGUILoadCTR)
    {
    case 0:
        if (wcscmp(Name->Buffer, _(L"xinput1_4.dll")) == 0)
        {
            IMGUILoadCTR++;
            return 0;
        }
        break;
    case 1:
        if (wcscmp(Name->Buffer, _(L"xinput1_3.dll")) == 0)
        {
            IMGUILoadCTR++;
            return 0;
        }
        break;
    case 2:
        if (wcscmp(Name->Buffer, _(L"xinput9_1_0.dll")) == 0)
        {
            IMGUILoadCTR++;
            return 0;
        }
        break;
    case 3:
        if (wcscmp(Name->Buffer, _(L"xinput1_2.dll")) == 0)
        {
            IMGUILoadCTR++;
            return 0;
        }
        break;
    case 4:
        if (wcscmp(Name->Buffer, _(L"xinput1_1.dll")) == 0)
        {
            Asteria::Socket::Client::Send(Asteria::Socket::Messages::Detection(
                Asteria::Socket::Detections::LDRLoadDLLFlagged,
                skCrypt("xinput1_1.dll was loaded").decrypt()
            ));
            return 0;
        }
        break;
    }
    
    wchar_t fileName[MAX_PATH];
    if (Name->Buffer[1] != L':')
    {
        ULONG NewFlags = 2;
        HMODULE lib = nullptr;
        LdrLoadDllOG(SearchPath_, &NewFlags, Name, (PVOID*)&lib);
        if (!lib)
        {
            return 0;
        }
        LI_FN_CACHED(GetModuleFileNameW)(lib, fileName, MAX_PATH);
        LI_FN_CACHED(FreeLibrary)(lib);
    }
    else
    {
        wcscpy_s(fileName, Name->Buffer);
    }

    wchar_t lowered[MAX_PATH];
    wcscpy_s(lowered, fileName);
    _wcslwr_s(lowered);
    if (wcscmp(lowered, _(L"c:\\windows\\system32\\rsaenh.dll")) == 0 || 
        wcscmp(lowered, _(L"c:\\windows\\system32\\win64help.dll")) == 0 || 
        wcscmp(lowered, _(L"c:\\windows\\system32\\wintrust.dll")) == 0 || 
        wcscmp(lowered, _(L"c:\\windows\\system32\\crypt32.dll")) == 0 || 
        wcscmp(lowered, _(L"c:\\windows\\system32\\nvspcap64.dll")) == 0 ||
        std::regex_match(lowered, std::wregex(_(L"c:\\\\windows\\\\[a-z0-9]{8}\\.[a-z0-9]{3}\\.dll"), std::regex_constants::icase)))
        return LdrLoadDllOG(SearchPath_, LoadFlags, Name, BaseAddress);

    bool verified = true;
    verified = VerifyEmbeddedSignature(fileName);
    if (!verified)
    {
        std::wstring capturedFileName = fileName;
        std::string capturedFileNameA = std::string(capturedFileName.begin(), capturedFileName.end());
        const char* Prefix = skCrypt("VerifyEmbeddedSignature fail! ").decrypt();
        std::string Full = std::string(Prefix) + capturedFileNameA;
    
        Socket::Client::Send(Asteria::Socket::Messages::Detection(
            Socket::Detections::LDRLoadDLLFlagged,
            Full.c_str()
        ));
    }

    return verified ? LdrLoadDllOG(SearchPath_, LoadFlags, Name, BaseAddress) : 0;
}


bool IsAddrInLoadedModule(void* addr)
{
    auto PEB = (li::detail::win::PEB_T*)__readgsqword(0x60);
    auto Entry = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)PEB->Ldr->InLoadOrderModuleList.Flink;
    auto Head = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)PEB->Ldr->InLoadOrderModuleList.Flink;

    do
    {
        if (addr >= Entry->DllBase && addr <= Entry->DllBase + Entry->SizeOfImage)
            return true;
    } while ((Entry = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)Entry->InLoadOrderLinks.Flink) && Entry != Head);

    return false;
}



BOOL Asteria::Windows::FlushInstructionCacheHook(HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T dwSize)
{
    auto Ret = _ReturnAddress();
    if (!IsAddrInLoadedModule(Ret))
    {
        Socket::Client::Send(Asteria::Socket::Messages::Detection(
            Socket::Detections::RuntimeMemoryTampered,
            skCrypt("FlushInstructionCache detected invalid access!").decrypt()
        ));
        return FALSE;
    }

    return FlushInstructionCacheOG(hProcess, lpBaseAddress, dwSize);
}

typedef struct _VECTOR_HANDLER_ENTRY {
    LIST_ENTRY ListEntry;
    PLONG64 pRefCount;
    DWORD unk_0;
    DWORD pad_0;
    PVOID EncodedHandler;
} VECTOR_HANDLER_ENTRY, * PVECTOR_HANDLER_ENTRY;

typedef struct _LDRP_VECTOR_HANDLER_LIST {
    PSRWLOCK LdrpVehLock;
    LIST_ENTRY LdrpVehList;
    PSRWLOCK LdrpVchLock;
    LIST_ENTRY LdrpVchList;
} LDRP_VECTOR_HANDLER_LIST, * PLDRP_VECTOR_HANDLER_LIST;

void Asteria::Windows::VEHChecker()
{
    auto RtlRemoveVectoredExceptionHandler_ = LI_FN_NO_DEF(RtlRemoveVectoredExceptionHandler).get();
    auto RtlpRemoveVectoredHandler_ = Memcury::Scanner(RtlRemoveVectoredExceptionHandler_).RelativeOffset(3);
    auto LdrpVectorHandlerList = (_LDRP_VECTOR_HANDLER_LIST*)RtlpRemoveVectoredHandler_.ScanFor({ 0x48, 0x8B, 0xE9 }).RelativeOffset(-4).Get();

    while (true)
    {
        LIST_ENTRY* pListHead = &LdrpVectorHandlerList->LdrpVehList;

        for (LIST_ENTRY* pListEntry = pListHead->Flink; pListEntry != pListHead; pListEntry = pListEntry->Flink)
        {
            PVECTOR_HANDLER_ENTRY pEntry = CONTAINING_RECORD(pListEntry, VECTOR_HANDLER_ENTRY, ListEntry);

            decltype(RemoveVectoredExceptionHandler) RtlRemoveVectoredExceptionHandler;
            LI_FN_CACHED(RtlRemoveVectoredExceptionHandler)(pEntry);
        }
    }

}

void Asteria::Windows::Setup()
{
    MH_CreateHook(LI_FN_CACHED(LdrLoadDll), Asteria::Windows::LdrLoadDllHook, (LPVOID*)&Asteria::Windows::LdrLoadDllOG);
    MH_CreateHook(LI_FN_CACHED(FlushInstructionCache), Asteria::Windows::FlushInstructionCacheHook, (LPVOID*)&Asteria::Windows::FlushInstructionCacheOG); // hooks
    LI_FN_CACHED(CreateThread)(0, 0, (LPTHREAD_START_ROUTINE)VEHChecker, 0, 0, 0);
}