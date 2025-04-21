#pragma once
#include "Framework.h"

namespace Engine::Server
{
    inline void (*ServerMove_PerformMovementOG)(UCharacterMovementComponent*, float, FVector_NetQuantize10, FVector_NetQuantize100, uint8, uint8, uint32, UPrimitiveComponent*, FName, uint8) = nullptr;
    void ServerMove_PerformMovement(UCharacterMovementComponent*, float, FVector_NetQuantize10, FVector_NetQuantize100, uint8, uint8, uint32, UPrimitiveComponent*, FName, uint8);

    void Setup();
}