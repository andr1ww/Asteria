#pragma once
#include "Framework.h"

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

#ifndef STATUS_ACCESS_DENIED
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xC0000022L)
#endif

typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
);
inline NTSTATUS(NTAPI* NtAllocateVirtualMemory)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
inline NTSTATUS(NTAPI* LdrLoadDll)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);

namespace Asteria::Windows
{
    inline pNtAllocateVirtualMemory NtAllocateVirtualMemoryOG = NULL;
    NTSTATUS NTAPI NtAllocateVirtualMemoryHook(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
        ULONG AllocationType,
        ULONG Protect
    );

    inline int IMGUILoadCTR = false;
    BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);
    inline NTSTATUS(__stdcall *LdrLoadDllOG)(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);
    NTSTATUS __stdcall LdrLoadDllHook(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);

    void Setup();

    inline void* ThisBuf;
    inline size_t ThisSize;
}