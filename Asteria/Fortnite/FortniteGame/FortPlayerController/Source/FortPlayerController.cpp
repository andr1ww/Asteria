#include "Framework.h"
#include "../Header/FortPlayerController.h"
#include "Asteria/Holder.h"
#include "Framework.h"

using AFortPlayerController_PerformBuildingEditInteractionParams = void(*)(UObject* PC);
inline AFortPlayerController_PerformBuildingEditInteractionParams PerformBuildingEditInteractionOG = nullptr;
using AFortPlayerController_EditSelectParams = void(*)(UObject* PC);
inline AFortPlayerController_EditSelectParams EditSelectOG = nullptr;
inline AFortPlayerController_EditSelectParams EditResetOG = nullptr;

void FortniteGame::FortPlayerController::PerformBuildingEditInteractionHook(UObject* PC)
{
    if (FNVer <= 15.00)
    {
        if (bDisablePreedits && PC->Get<UObject*>(L"TargetedBuilding")->IsA(TUObjectArray::FindObject<UClass>("ABuildingPlayerPrimitivePreview")))
        {
            return;
        } else {
            PerformBuildingEditInteractionOG(PC);
        }
    }
}

void FortniteGame::FortPlayerController::EditSelectHook(UObject* PC)
{
    if (FNVer <= 10.40)
    {
        EditSelectOG(PC);

        if (bEditOnRelease && PC && PC->Get<UObject*>(L"EditBuildingActor"))
        {
          reinterpret_cast<void(*)(UObject*)>(Asteria::Holder::AFortPlayerController_CompleteBuildingEditInteraction)(PC);
        }
    }
}

void FortniteGame::FortPlayerController::EditResetHook(UObject* PC)
{
    if (FNVer <= 24.20)
    {
        EditResetOG(PC);
        if (bEnableResetOnRelease && PC && PC->Get<UObject*>(L"EditBuildingActor"))
        {
            reinterpret_cast<void(*)(UObject*)>(Asteria::Holder::AFortPlayerController_CompleteBuildingEditInteraction)(PC);
        } 
    }
} 

void FortniteGame::FortPlayerController::Setup()
{
    if (Asteria::Holder::AFortPlayerController_PerformBuildingEditInteraction && FNVer <= 15.00)
    {
        MH_CreateHook(
            (LPVOID)Asteria::Holder::AFortPlayerController_PerformBuildingEditInteraction,
            &PerformBuildingEditInteractionHook,
            reinterpret_cast<void**>(&PerformBuildingEditInteractionOG)
        ); 
    } 
    if (Asteria::Holder::AFortPlayerController_EditSelect && FNVer <= 10.40)
    {
        MH_CreateHook(
          (LPVOID)Asteria::Holder::AFortPlayerController_EditSelect,
          &EditSelectHook,
          reinterpret_cast<void**>(&EditSelectOG)
      );
    }
    if (Asteria::Holder::AFortPlayerController_BuildingReset && FNVer <= 24.20)
    {
        MH_CreateHook(
          (LPVOID)Asteria::Holder::AFortPlayerController_BuildingReset,
          &EditResetHook,
          reinterpret_cast<void**>(&EditResetOG)
      );
    } 
    MH_EnableHook(MH_ALL_HOOKS);
}