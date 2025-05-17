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


NTSTATUS NTAPI Asteria::Windows::RtlCreateUserThreadHook(
    HANDLE ProcessHandle,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN CreateSuspended,
    ULONG StackZeroBits,
    SIZE_T StackReserve,
    SIZE_T StackCommit,
    PVOID StartAddress,
    PVOID Parameter,
    PHANDLE ThreadHandle,
    PCLIENT_ID ClientId
) {
    fuckIda;
    DWORD currentProcessId = LI_FN_CACHED(GetCurrentProcessId)();
    DWORD targetProcessId = LI_FN_CACHED(GetProcessId)(ProcessHandle);
    
    if (targetProcessId == currentProcessId) {
        void* callerAddr = _ReturnAddress();
        
        if (!IsAddrInLoadedModule(callerAddr)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("RtlCreateUserThread: Caller not in loaded module").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (!IsAddrInLoadedModule(StartAddress)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("RtlCreateUserThread: Start address not in loaded module").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (HasSomeWeirdShitProtection(StartAddress)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("RtlCreateUserThread: Thread targeting memory with suspicious protection").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (IsAddressInSystemModule(StartAddress)) {
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(StartAddress, &mbi, sizeof(mbi))) {
                if (mbi.Protect & PAGE_WRITECOPY || mbi.Protect & PAGE_EXECUTE_WRITECOPY ||
                    mbi.Protect & PAGE_READWRITE || mbi.Protect & PAGE_EXECUTE_READWRITE) {
                    Socket::Client::Send(Socket::Messages::Detection(
                        Socket::Detections::RuntimeMemoryTampered,
                        skCrypt("RtlCreateUserThread: Thread targeting modified system module").decrypt()
                    ));
                    return STATUS_ACCESS_DENIED;
                }
            }
        }
        
        if (callerAddr && StartAddress && 
            ((DWORD_PTR)callerAddr & 0xFFFF000000000000) != ((DWORD_PTR)StartAddress & 0xFFFF000000000000)) {
            
            std::wstring callerModule = GetModuleNameFromAddress(callerAddr);
            std::wstring targetModule = GetModuleNameFromAddress(StartAddress);
            
            std::string callerModuleA(callerModule.begin(), callerModule.end());
            std::string targetModuleA(targetModule.begin(), targetModule.end());
            
            std::string detectionMsg = _("RtlCreateUserThread: module thread detected (").decrypt();
            detectionMsg += callerModuleA + " -> " + targetModuleA + ")";
            
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                detectionMsg.c_str()
            ));
            return STATUS_ACCESS_DENIED;
        }
    }
    
    return RtlCreateUserThreadOG(
        ProcessHandle,
        SecurityDescriptor,
        CreateSuspended,
        StackZeroBits,
        StackReserve,
        StackCommit,
        StartAddress,
        Parameter,
        ThreadHandle,
        ClientId
    );
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
    fuckIda;
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

NTSTATUS NTAPI Asteria::Windows::NtCreateThreadExHook(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    PVOID ObjectAttributes,
    HANDLE ProcessHandle,
    PVOID StartAddress,
    PVOID Parameter,
    ULONG CreateFlags,
    SIZE_T ZeroBits,
    SIZE_T StackSize,
    SIZE_T MaximumStackSize,
    PVOID AttributeList
) {
    fuckIda;
    DWORD currentProcessId = LI_FN_CACHED(GetCurrentProcessId)();
    DWORD targetProcessId = LI_FN_CACHED(GetProcessId)(ProcessHandle);
    
    if (targetProcessId == currentProcessId) {
        void* callerAddr = _ReturnAddress();
        
        if (!IsAddrInLoadedModule(callerAddr)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("NtCreateThreadEx").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (!IsAddrInLoadedModule(StartAddress)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("NtCreateThreadEx").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (HasSomeWeirdShitProtection(StartAddress)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("NtCreateThreadEx").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (CreateFlags & 0x00000004) { 
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("NtCreateThreadEx").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (callerAddr && StartAddress && 
            ((DWORD_PTR)callerAddr & 0xFFFF000000000000) != ((DWORD_PTR)StartAddress & 0xFFFF000000000000)) {
            
            std::wstring callerModule = GetModuleNameFromAddress(callerAddr);
            std::wstring targetModule = GetModuleNameFromAddress(StartAddress);
            
            std::string callerModuleA(callerModule.begin(), callerModule.end());
            std::string targetModuleA(targetModule.begin(), targetModule.end());
            
            std::string detectionMsg = skCrypt("NtCreateThreadEx: Cross-module thread detected (").decrypt();
            detectionMsg += callerModuleA + " -> " + targetModuleA + ")";
            
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                detectionMsg.c_str()
            ));
            return STATUS_ACCESS_DENIED;
        }
    }
    
    return NtCreateThreadExOG(
        ThreadHandle,
        DesiredAccess,
        ObjectAttributes,
        ProcessHandle,
        StartAddress,
        Parameter,
        CreateFlags,
        ZeroBits,
        StackSize,
        MaximumStackSize,
        AttributeList
    );
}

NTSTATUS NTAPI Asteria::Windows::NtQueueApcThreadHook(
    HANDLE ThreadHandle,
    PVOID ApcRoutine,
    PVOID ApcArgument1,
    PVOID ApcArgument2,
    PVOID ApcArgument3
) {
    fuckIda;
    if (!IsAddrInLoadedModule(ApcRoutine)) {
        Socket::Client::Send(Socket::Messages::Detection(
            Socket::Detections::RuntimeMemoryTampered,
                    skCrypt("NtQueueApcThread").decrypt()
        ));
        return STATUS_ACCESS_DENIED;
    }
    
    if (HasSomeWeirdShitProtection(ApcRoutine)) {
        Socket::Client::Send(Socket::Messages::Detection(
            Socket::Detections::RuntimeMemoryTampered,
                    skCrypt("NtQueueApcThread").decrypt()
        ));
        return STATUS_ACCESS_DENIED;
    }
    
    DWORD threadId = LI_FN_CACHED(GetThreadId)(ThreadHandle);
    DWORD processId = LI_FN_CACHED(GetProcessIdOfThread)(ThreadHandle);
    
    HANDLE hProcess = LI_FN_CACHED(OpenProcess)(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess != NULL) {
        wchar_t processName[MAX_PATH];
        if (LI_FN_CACHED(GetProcessImageFileNameW)(hProcess, processName, MAX_PATH) > 0) {
            if (wcsstr(processName, L"\\System32\\") || wcsstr(processName, L"\\SysWOW64\\")) {
                LI_FN_CACHED(CloseHandle)(hProcess);
                Socket::Client::Send(Socket::Messages::Detection(
                    Socket::Detections::RuntimeMemoryTampered,
                    skCrypt("NtQueueApcThread").decrypt()
                ));
                return STATUS_ACCESS_DENIED;
            }
        }
        LI_FN_CACHED(CloseHandle)(hProcess);
    }
    
    return NtQueueApcThreadOG(
        ThreadHandle,
        ApcRoutine,
        ApcArgument1,
        ApcArgument2,
        ApcArgument3
    );
}

NTSTATUS NTAPI Asteria::Windows::NtMapViewOfSectionHook(
    HANDLE SectionHandle,
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    SIZE_T CommitSize,
    PLARGE_INTEGER SectionOffset,
    PSIZE_T ViewSize,
    DWORD InheritDisposition,
    ULONG AllocationType,
    ULONG Win32Protect
) {
    fuckIda;
    DWORD currentProcessId = LI_FN_CACHED(GetCurrentProcessId)();
    DWORD targetProcessId = LI_FN_CACHED(GetProcessId)(ProcessHandle);
    
    if (targetProcessId == currentProcessId) {
        void* callerAddr = _ReturnAddress();
        
        if (!IsAddrInLoadedModule(callerAddr)) {
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::RuntimeMemoryTampered,
                skCrypt("NtMapViewOfSection").decrypt()
            ));
            return STATUS_ACCESS_DENIED;
        }
        
        if (Win32Protect & PAGE_EXECUTE || 
            Win32Protect & PAGE_EXECUTE_READ || 
            Win32Protect & PAGE_EXECUTE_READWRITE || 
            Win32Protect & PAGE_EXECUTE_WRITECOPY) {
            
            std::wstring callerModule = GetModuleNameFromAddress(callerAddr);
            std::string callerModuleA(callerModule.begin(), callerModule.end());
            
            std::string detectionMsg = skCrypt("NtMapViewOfSection: EXE mapping requested from").decrypt();
            detectionMsg += callerModuleA;
            
            Socket::Client::Send(Socket::Messages::Detection(
                Socket::Detections::ExternalDetection,  
                detectionMsg.c_str()
            ));
            
            NTSTATUS status = NtMapViewOfSectionOG(
                SectionHandle,
                ProcessHandle,
                BaseAddress,
                ZeroBits,
                CommitSize,
                SectionOffset,
                ViewSize,
                InheritDisposition,
                AllocationType,
                Win32Protect
            );
            
            if (NT_SUCCESS(status) && *BaseAddress != NULL) {
                BYTE* buffer = (BYTE*)*BaseAddress;
                SIZE_T scanSize = min(*ViewSize, 1024); 
                
                for (SIZE_T i = 0; i < scanSize - 10; i++) {
                    if (buffer[i] == 0x48 && buffer[i+1] == 0x83 && buffer[i+2] == 0xEC) { 
                        Socket::Client::Send(Socket::Messages::Detection(
                            Socket::Detections::RuntimeMemoryTampered,
                skCrypt("NtMapViewOfSection").decrypt()
                        ));
                        break;
                    }
                }
            }
            
            return status;
        }
    }
    
    return NtMapViewOfSectionOG(
        SectionHandle,
        ProcessHandle,
        BaseAddress,
        ZeroBits,
        CommitSize,
        SectionOffset,
        ViewSize,
        InheritDisposition,
        AllocationType,
        Win32Protect
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

    std::cout << "Blocked WriteProcMemory" << std::endl;

    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}

void Asteria::Windows::Setup()
{
    fuckIda;
    MH_CreateHook(LI_FN_CACHED(LdrLoadDll), Asteria::Windows::LdrLoadDllHook, (LPVOID*)&Asteria::Windows::LdrLoadDllOG);
    MH_CreateHook(LI_FN_CACHED(FlushInstructionCache), Asteria::Windows::FlushInstructionCacheHook, (LPVOID*)&Asteria::Windows::FlushInstructionCacheOG); // hooks
    MH_CreateHook(LI_FN_CACHED(RtlCreateUserThread), Asteria::Windows::RtlCreateUserThreadHook, (LPVOID*)&Asteria::Windows::RtlCreateUserThreadOG);
    MH_CreateHook(LI_FN_CACHED(NtAllocateVirtualMemory), Asteria::Windows::NtAllocateVirtualMemoryHook, (LPVOID*)&Asteria::Windows::NtAllocateVirtualMemoryOG);
//    MH_CreateHook(LI_FN_CACHED(NtCreateThreadEx), Asteria::Windows::NtCreateThreadExHook, (LPVOID*)&Asteria::Windows::NtCreateThreadExOG);
    MH_CreateHook(LI_FN_CACHED(NtQueueApcThread), Asteria::Windows::NtQueueApcThreadHook, (LPVOID*)&Asteria::Windows::NtQueueApcThreadOG);
    //MH_CreateHook(LI_FN_CACHED(NtMapViewOfSection), Asteria::Windows::NtMapViewOfSectionHook, (LPVOID*)&Asteria::Windows::NtMapViewOfSectionOG);
	MH_CreateHook(LI_FN_CACHED(WriteProcessMemory), Asteria::Windows::WriteProcessMemoryHook, (LPVOID*)&Asteria::Windows::Original_WriteProcessMemory);
    // LI_FN_CACHED(CreateThread)(0, 0, (LPTHREAD_START_ROUTINE)VEHChecker, 0, 0, 0);
}