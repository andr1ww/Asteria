#pragma once
#include "Framework.h"

typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

#ifndef STATUS_ACCESS_DENIED
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xC0000022L)
#endif

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef NTSTATUS(NTAPI* pRtlCreateUserThread)(
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
);

inline NTSTATUS(NTAPI* LdrLoadDll)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);

inline NTSTATUS(NTAPI* RtlCreateUserThread)(
    HANDLE,
    PSECURITY_DESCRIPTOR,
    BOOLEAN,
    ULONG,
    SIZE_T,
    SIZE_T,
    PVOID,
    PVOID,
    PHANDLE,
    PCLIENT_ID
);

namespace Asteria::Windows
{
    inline int IMGUILoadCTR = false;

    inline NTSTATUS(__stdcall *LdrLoadDllOG)(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);
    NTSTATUS __stdcall LdrLoadDllHook(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);

    BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);

    inline BOOL(*FlushInstructionCacheOG)(HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T dwSize);
    BOOL FlushInstructionCacheHook(HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T dwSize);

    inline pRtlCreateUserThread RtlCreateUserThreadOG = NULL;
    NTSTATUS NTAPI RtlCreateUserThreadHook(
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
    );

    void VEHChecker();

    void Setup();

    inline void* ThisBuf;
    inline size_t ThisSize;
}