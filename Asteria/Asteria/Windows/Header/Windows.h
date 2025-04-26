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

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
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

typedef NTSTATUS(NTAPI* pNtProtectVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    PSIZE_T RegionSize,
    ULONG NewProtect,
    PULONG OldProtect
);

typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
);

typedef NTSTATUS(NTAPI* pNtCreateThreadEx)(
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
);

typedef NTSTATUS(NTAPI* pNtQueueApcThread)(
    HANDLE ThreadHandle,
    PVOID ApcRoutine,
    PVOID ApcArgument1,
    PVOID ApcArgument2,
    PVOID ApcArgument3
);

typedef NTSTATUS(NTAPI* pNtMapViewOfSection)(
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
);

typedef NTSTATUS(NTAPI* pNtSetContextThread)(
    HANDLE ThreadHandle,
    PCONTEXT ThreadContext
);

inline NTSTATUS(NTAPI* LdrLoadDll)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);
inline NTSTATUS(NTAPI* RtlCreateUserThread)(HANDLE, PSECURITY_DESCRIPTOR, BOOLEAN, ULONG, SIZE_T, SIZE_T, PVOID, PVOID, PHANDLE, PCLIENT_ID);
inline NTSTATUS(NTAPI* NtProtectVirtualMemory)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
inline NTSTATUS(NTAPI* NtAllocateVirtualMemory)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
inline NTSTATUS(NTAPI* NtCreateThreadEx)(PHANDLE, ACCESS_MASK, PVOID, HANDLE, PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PVOID);
inline NTSTATUS(NTAPI* NtQueueApcThread)(HANDLE, PVOID, PVOID, PVOID, PVOID);
inline NTSTATUS(NTAPI* NtMapViewOfSection)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
inline NTSTATUS(NTAPI* NtSetContextThread)(HANDLE, PCONTEXT);
inline NTSTATUS(NTAPI* NtQuerySection)(HANDLE, ULONG, PVOID, ULONG, PULONG);
inline NTSTATUS(NTAPI* NtSetInformationThread)(HANDLE, ULONG, PVOID, ULONG);
inline NTSTATUS(NTAPI* NtWriteVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);

typedef struct _SECTION_BASIC_INFORMATION {
    PVOID BaseAddress;
    ULONG Attributes;
    LARGE_INTEGER Size;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

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

    inline pNtProtectVirtualMemory NtProtectVirtualMemoryOG = NULL;
    NTSTATUS NTAPI NtProtectVirtualMemoryHook(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        PSIZE_T RegionSize,
        ULONG NewProtect,
        PULONG OldProtect
    );

    inline pNtAllocateVirtualMemory NtAllocateVirtualMemoryOG = NULL;
    NTSTATUS NTAPI NtAllocateVirtualMemoryHook(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
        ULONG AllocationType,
        ULONG Protect
    );

    inline pNtCreateThreadEx NtCreateThreadExOG = NULL;
    NTSTATUS NTAPI NtCreateThreadExHook(
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
    );

    inline pNtQueueApcThread NtQueueApcThreadOG = NULL;
    NTSTATUS NTAPI NtQueueApcThreadHook(
        HANDLE ThreadHandle,
        PVOID ApcRoutine,
        PVOID ApcArgument1,
        PVOID ApcArgument2,
        PVOID ApcArgument3
    );

    inline pNtMapViewOfSection NtMapViewOfSectionOG = NULL;
    NTSTATUS NTAPI NtMapViewOfSectionHook(
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
    );

    inline pNtSetContextThread NtSetContextThreadOG = NULL;
    NTSTATUS NTAPI NtSetContextThreadHook(
        HANDLE ThreadHandle,
        PCONTEXT ThreadContext
    );

    void VEHChecker();
    void ScanForSuspiciousMemory();
    void Setup();

    inline void* ThisBuf;
    inline size_t ThisSize;

	typedef BOOL(WINAPI* WriteProcessMemory_t)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);

	inline WriteProcessMemory_t Original_WriteProcessMemory = NULL;

	BOOL WINAPI WriteProcessMemoryHook(
		HANDLE hProcess,
		LPVOID lpBaseAddress,
		LPCVOID lpBuffer,
		SIZE_T nSize,
		SIZE_T* lpNumberOfBytesWritten
	);
}