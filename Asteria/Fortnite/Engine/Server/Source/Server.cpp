#include "../Header/Server.h"

#include "Asteria/Holder.h"

void Engine::Server::CallServerMove(UObject* MovementComp, void* NewMove, void* OldMove)
{
    if (NewMove && *(float*)(int64(NewMove) + 0x1c) <= 1.f)
        *(float*)(int64(NewMove) + 0x1c) += 45387534.634453;
    
    CallServerMoveOG(MovementComp, NewMove, OldMove);
}

void Engine::Server::Setup()
{
    MH_CreateHook(
        (LPVOID)Asteria::Holder::AFortPlayerController_ServerMove,
        &CallServerMove,
        reinterpret_cast<void**>(&CallServerMoveOG)
    );
}