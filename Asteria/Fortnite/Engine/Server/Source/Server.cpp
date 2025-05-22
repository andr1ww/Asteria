#include "Framework.h"
#include "../Header/Server.h"
#include "Asteria/Holder.h"

/*void Engine::Server::ServerMove_PerformMovement(UCharacterMovementComponent* Comp,
    float TimeStamp,
    FVector_NetQuantize10 InAccel,
    FVector_NetQuantize100 ClientLoc,
    uint8 MoveFlags,
    uint8 ClientRoll,
    uint32 View,
    UPrimitiveComponent* ClientMovementBase,
    FName ClientBaseBoneName,
    uint8 ClientMovementMode)
{
    return ServerMove_PerformMovementOG(Comp, TimeStamp <= 1.0f ? TimeStamp + 45387534.634453f : TimeStamp, InAccel, ClientLoc, MoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

void Engine::Server::Setup()
{
    Hook<UFortMovementComp_CharacterAthena>(uintptr_t(0x134), ServerMove_PerformMovement, ServerMove_PerformMovementOG);
} */