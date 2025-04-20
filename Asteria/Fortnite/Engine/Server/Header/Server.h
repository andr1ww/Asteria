#pragma once
#include "Framework.h"

namespace Engine::Server
{
    inline void (*CallServerMoveOG)(UObject*, void*, void*) = nullptr;
    void CallServerMove(UObject* MovementComp, void* NewMove, void* OldMove);

    void Setup();
}