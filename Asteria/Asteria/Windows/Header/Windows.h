#pragma once
#include "Framework.h"

typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;
inline NTSTATUS(NTAPI* LdrLoadDll)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);

namespace Asteria::Windows
{
    inline int IMGUILoadCTR = false;

    inline NTSTATUS(__stdcall *LdrLoadDllOG)(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);
    NTSTATUS __stdcall LdrLoadDllHook(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);

    BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);

    inline BOOL(*FlushInstructionCacheOG)(HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T dwSize);
    BOOL FlushInstructionCacheHook(HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T dwSize);

    void VEHChecker();

    void Setup();

    inline void* ThisBuf;
    inline size_t ThisSize;
}