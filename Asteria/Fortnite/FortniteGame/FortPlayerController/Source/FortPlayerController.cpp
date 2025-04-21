#include "Framework.h"
#include "../Header/FortPlayerController.h"
#include "Asteria/Holder.h"

using AFortPlayerController_PerformBuildingEditInteractionParams = void(*)(AFortPlayerControllerAthena* PC);
inline AFortPlayerController_PerformBuildingEditInteractionParams PerformBuildingEditInteractionOG = nullptr;
using AFortPlayerController_EditSelectParams = void(*)(AFortPlayerControllerAthena* PC);
inline AFortPlayerController_EditSelectParams EditSelectOG = nullptr;

using AFortPlayerController_EditSelectParams = void(*)(AFortPlayerControllerAthena* PC);
inline AFortPlayerController_EditSelectParams EditResetOG = nullptr;

inline void* (*CompleteBuildingEditInteraction)(AFortPlayerController* PlayerController) = decltype(CompleteBuildingEditInteraction)(__int64(*(uint64_t*)(__readgsqword(0x60) + 0x10) + 0x18285A0));

void FortniteGame::FortPlayerController::PerformBuildingEditInteractionHook(AFortPlayerControllerAthena* PC)
{
    if (bDisablePreedits && PC->TargetedBuilding->IsA(ABuildingPlayerPrimitivePreview::StaticClass()))
    {
        return;
    } else {
        PerformBuildingEditInteractionOG(PC);
    }
}

void FortniteGame::FortPlayerController::EditSelectHook(AFortPlayerControllerAthena* PC)
{
    EditSelectOG(PC);
    
    if (bEditOnRelease && PC && PC->EditBuildingActor)
    {
        CompleteBuildingEditInteraction(PC);
    }
}


void FortniteGame::FortPlayerController::EditResetHook(AFortPlayerControllerAthena* PC)
{
    EditResetOG(PC);
    
    if (bEnableResetOnRelease && PC && PC->EditBuildingActor)
    {
        CompleteBuildingEditInteraction(PC);
    }
}

void FortniteGame::FortPlayerController::Setup()
{
    MH_CreateHook(
        (LPVOID)Asteria::Holder::AFortPlayerController_PerformBuildingEditInteraction,
        &PerformBuildingEditInteractionHook,
        reinterpret_cast<void**>(&PerformBuildingEditInteractionOG)
    );
    MH_CreateHook(
      (LPVOID)Asteria::Holder::AFortPlayerController_EditSelect,
      &EditSelectHook,
      reinterpret_cast<void**>(&EditSelectOG)
  );
    MH_CreateHook(
      (LPVOID)Asteria::Holder::AFortPlayerController_BuildingReset,
      &EditResetHook,
      reinterpret_cast<void**>(&EditResetOG)
  );
    MH_EnableHook(MH_ALL_HOOKS);
}