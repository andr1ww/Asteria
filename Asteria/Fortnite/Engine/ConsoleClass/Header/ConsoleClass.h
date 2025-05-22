#pragma once
#include "Framework.h"

enum EInternalObjectFlags
{
    None = 0,
    ReachableInCluster = 1 << 23,
    ClusterRoot = 1 << 24,
    Native = 1 << 25,
    Async = 1 << 26,
    AsyncLoading = 1 << 27,
    Unreachable = 1 << 28,
    PendingKill = 1 << 29,
    RootSet = 1 << 30,
    GarbageCollectionKeepFlags = Native | Async | AsyncLoading,
    AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet,
};

typedef UObject* (__fastcall* f_StaticConstructObject_Internal)(
    UClass* Class,
    UObject* InOuter,
    void* Name,
    //EObjectFlags SetFlags,
    EInternalObjectFlags InternalSetFlags,
    UObject* Template,
    bool  bCopyTransientsFromClassDefaults,
    void* InstanceGraph,
    bool  bAssumeTemplateIsArchetype
);
static f_StaticConstructObject_Internal StaticConstructObject_Internal;

namespace Engine::ConsoleClass
{
  //  void EnableConsole(APlayerController* Player);
}