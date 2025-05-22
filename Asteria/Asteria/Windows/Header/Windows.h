#pragma once
#include "Framework.h"

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

inline NTSTATUS(NTAPI* LdrLoadDll)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);
namespace Asteria::Windows
{
    inline int IMGUILoadCTR = false;
    BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);
    inline NTSTATUS(__stdcall *LdrLoadDllOG)(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);
    NTSTATUS __stdcall LdrLoadDllHook(PWCHAR, PULONG, PUNICODE_STRING, PVOID*);

    void Setup();

    inline void* ThisBuf;
    inline size_t ThisSize;
}