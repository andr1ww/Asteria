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

    void Setup();
}