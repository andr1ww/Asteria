#include "Framework.h"
#include "../Header/Windows.h"
#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/Socket/Detection/Enum/Detection.h"
#include "Asteria/Socket/Messages/Header/Messages.h"
#include <regex>

#include "Asteria/Holder.h"

// credits to https://github.com/plooshi/Arsenic for VerifyEmbeddedSignature and part of LdrLoadDllHook, and FlushInstructionCacheHook

__forceinline BOOL Asteria::Windows::VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
    fuckIda;
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
    fuckIda;
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
    fuckIda;
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
    fuckIda;
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

bool HasSomeWeirdShitProtection(void* addr) {
    fuckIda;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(addr, &mbi, sizeof(mbi))) {
        if (mbi.Protect == PAGE_EXECUTE_READWRITE || 
            mbi.Protect == PAGE_EXECUTE_WRITECOPY) {
            return true;
            }
        
        if (mbi.Type == MEM_PRIVATE && mbi.State == MEM_COMMIT && 
            (mbi.Protect & PAGE_EXECUTE || mbi.Protect & PAGE_EXECUTE_READ)) {
            return true;
            }
    }
    return false;
}

bool IsAddressInSystemModule(void* addr) {
    fuckIda;
    auto PEB = (li::detail::win::PEB_T*)__readgsqword(0x60);
    auto Entry = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)PEB->Ldr->InLoadOrderModuleList.Flink;
    auto Head = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)PEB->Ldr->InLoadOrderModuleList.Flink;

    do {
        if (addr >= Entry->DllBase && addr <= (BYTE*)Entry->DllBase + Entry->SizeOfImage) {
            if (Entry->FullDllName.Buffer) {
                if (wcsstr(Entry->FullDllName.Buffer, L"\\System32\\") || 
                    wcsstr(Entry->FullDllName.Buffer, L"\\SysWOW64\\")) {
                    return true;
                    }
            }
        }
    } while ((Entry = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)Entry->InLoadOrderLinks.Flink) && Entry != Head);

    return false;
}

std::wstring GetModuleNameFromAddress(void* addr) {
    fuckIda;
    auto PEB = (li::detail::win::PEB_T*)__readgsqword(0x60);
    auto Entry = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)PEB->Ldr->InLoadOrderModuleList.Flink;
    auto Head = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)PEB->Ldr->InLoadOrderModuleList.Flink;

    do {
        if (addr >= Entry->DllBase && addr <= (BYTE*)Entry->DllBase + Entry->SizeOfImage) {
            if (Entry->BaseDllName.Buffer) {
                return std::wstring(Entry->BaseDllName.Buffer, Entry->BaseDllName.Length / sizeof(wchar_t));
            }
            return L"<unknown>";
        }
    } while ((Entry = (li::detail::win::LDR_DATA_TABLE_ENTRY_T*)Entry->InLoadOrderLinks.Flink) && Entry != Head);

    return L"<unmapped>";
}

NTSTATUS NTAPI Asteria::Windows::NtAllocateVirtualMemoryHook(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
) {
    fuckIda;
    DWORD currentProcessId = LI_FN_CACHED(GetCurrentProcessId)();
    DWORD targetProcessId = LI_FN_CACHED(GetProcessId)(ProcessHandle);
    
    if (targetProcessId == currentProcessId) {
        void* callerAddr = _ReturnAddress();
        
        if (!IsAddrInLoadedModule(callerAddr)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("NtAllocateVirtualMemory: Caller not in loaded module").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (Protect & PAGE_EXECUTE || 
            Protect & PAGE_EXECUTE_READ || 
            Protect & PAGE_EXECUTE_READWRITE || 
            Protect & PAGE_EXECUTE_WRITECOPY) {
            
            std::wstring callerModule = GetModuleNameFromAddress(callerAddr);
            std::string callerModuleA(callerModule.begin(), callerModule.end());
            
            if (*RegionSize > 1024 * 1024) {  
                std::string detectionMsg = skCrypt("NtAllocateVirtualMemory: Large executable allocation from ").decrypt();
                detectionMsg += callerModuleA + " (" + std::to_string(*RegionSize / 1024) + " KB)";
                
                Socket::Client::Send(Socket::Messages::Detection(
                    Socket::Detections::RuntimeMemoryTampered,
                    detectionMsg.c_str()
                ));
                return STATUS_ACCESS_DENIED;
            }
            
            return NtAllocateVirtualMemoryOG(
                ProcessHandle,
                BaseAddress,
                ZeroBits,
                RegionSize,
                AllocationType,
                Protect);
        }
    }
    
    return NtAllocateVirtualMemoryOG(
        ProcessHandle,
        BaseAddress,
        ZeroBits,
        RegionSize,
        AllocationType,
        Protect
    );
}

BOOL WINAPI Asteria::Windows::WriteProcessMemoryHook(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T* lpNumberOfBytesWritten
) {
    fuckIda;

    void* callerAddr = _ReturnAddress();
    DWORD currentProcessId = LI_FN_CACHED(GetCurrentProcessId)();
    DWORD targetProcessId = LI_FN_CACHED(GetProcessId)(hProcess);
    
    if (!IsAddrInLoadedModule(callerAddr)) {
        std::string detectionMsg = skCrypt("WriteProcessMemory: Caller not in loaded module - Address: 0x").decrypt();
        std::stringstream ss;
        ss << std::hex << (uintptr_t)callerAddr;
        detectionMsg += ss.str();
        
        Socket::Client::Send(Socket::Messages::Detection(
            Socket::Detections::RuntimeMemoryTampered,
            detectionMsg.c_str()
        ));
        
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    
    std::wstring callerModule = GetModuleNameFromAddress(callerAddr);
    std::string callerModuleA(callerModule.begin(), callerModule.end());
    
    if (targetProcessId == currentProcessId) {
        if (HasSomeWeirdShitProtection(lpBaseAddress)) {
            std::string detectionMsg = skCrypt("WriteProcessMemory: from ").decrypt();
            detectionMsg += callerModuleA;
            std::stringstream ss;
            ss << " - Target: 0x" << std::hex << (uintptr_t)lpBaseAddress << " Size: " << std::dec << nSize;
            detectionMsg += ss.str();
            
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                detectionMsg.c_str()
            ));
            
            SetLastError(ERROR_ACCESS_DENIED);
            return FALSE;
        }
        
        if (nSize > 4096) {
            std::string detectionMsg = skCrypt("WriteProcessMemory: Large write detected from ").decrypt();
            detectionMsg += callerModuleA;
            std::stringstream ss;
            ss << " - Size: " << nSize << " bytes to 0x" << std::hex << (uintptr_t)lpBaseAddress;
            detectionMsg += ss.str();
            
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                detectionMsg.c_str()
            ));
        }
        
        if (IsAddressInSystemModule(lpBaseAddress)) {
            std::string detectionMsg = skCrypt("WriteProcessMemory: Attempting to write to system module memory from ").decrypt();
            detectionMsg += callerModuleA;
            
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                detectionMsg.c_str()
            ));
            
            SetLastError(ERROR_ACCESS_DENIED);
            return FALSE;
        }
    } else {
        std::string detectionMsg = skCrypt("WriteProcessMemory: write attempt from ").decrypt();
        detectionMsg += callerModuleA;
        std::stringstream ss;
        ss << " - Target PID: " << targetProcessId << " Current PID: " << currentProcessId;
        detectionMsg += ss.str();
        
        Socket::Client::Send(Socket::Messages::Detection(
            Socket::Detections::RuntimeMemoryTampered,
            detectionMsg.c_str()
        ));
        
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    
    if (nSize >= 2 && lpBuffer) {
        const BYTE* buffer = static_cast<const BYTE*>(lpBuffer);
        
        bool suspiciousPattern = false;
        
        if (nSize >= 64 && buffer[0] == 'M' && buffer[1] == 'Z') {
            suspiciousPattern = true;
        }
        
        if (nSize >= 4) { // thx to uc
            if ((buffer[0] == 0x48 && buffer[1] == 0x89) || // mov instruction
                (buffer[0] == 0xFF && (buffer[1] == 0xD0 || buffer[1] == 0xE0)) || // call/jmp register
                (buffer[0] == 0xE8 || buffer[0] == 0xE9)) { // call/jmp relative
                suspiciousPattern = true;
            }
        }
        
        if (suspiciousPattern) {
            std::string detectionMsg = skCrypt("WriteProcessMemory: Suspicious code pattern detected from ").decrypt();
            detectionMsg += callerModuleA;
            
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                detectionMsg.c_str()
            ));
            
            SetLastError(ERROR_ACCESS_DENIED);
            return FALSE;
        }
    }
    
    return Original_WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

void Asteria::Windows::Setup()
{
    fuckIda;
    MH_CreateHook(LI_FN_CACHED(LdrLoadDll), Asteria::Windows::LdrLoadDllHook, (LPVOID*)&Asteria::Windows::LdrLoadDllOG);
    MH_CreateHook(LI_FN_CACHED(NtAllocateVirtualMemory), Asteria::Windows::NtAllocateVirtualMemoryHook, (LPVOID*)&Asteria::Windows::NtAllocateVirtualMemoryOG);
	MH_CreateHook(LI_FN_CACHED(WriteProcessMemory), Asteria::Windows::WriteProcessMemoryHook, (LPVOID*)&Asteria::Windows::Original_WriteProcessMemory);
}