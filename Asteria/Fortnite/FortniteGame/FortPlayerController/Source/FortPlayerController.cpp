#include "Framework.h"
#include "../Header/FortPlayerController.h"
#include "Asteria/Holder.h"
#include "Framework.h"

using AFortPlayerController_PerformBuildingEditInteractionParams = void(*)(UObject* PC);
inline AFortPlayerController_PerformBuildingEditInteractionParams PerformBuildingEditInteractionOG = nullptr;
using AFortPlayerController_EditSelectParams = void(*)(UObject* PC);
inline AFortPlayerController_EditSelectParams EditSelectOG = nullptr;
inline AFortPlayerController_EditSelectParams EditResetOG = nullptr;

inline void* (*CompleteBuildingEditInteraction)(UObject* PlayerController) = decltype(CompleteBuildingEditInteraction)(Asteria::Holder::AFortPlayerController_CompleteBuildingEditInteraction);

void FortniteGame::FortPlayerController::PerformBuildingEditInteractionHook(UObject* PC)
{
    Log("ok!!?!");
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
            CompleteBuildingEditInteraction(PC);
        }
    }
}

void FortniteGame::FortPlayerController::EditResetHook(UObject* PC)
{
    if (FNVer <= 24.20)
    {
        Log("wow");
        EditResetOG(PC);
        if (bEnableResetOnRelease && PC && PC->Get<UObject*>(L"EditBuildingActor"))
        {
            Log("Wow");
            CompleteBuildingEditInteraction(PC);
        } else
        {
            Log("!??!");
        }
    }
} 

void FortniteGame::FortPlayerController::Setup()
{
    std::string message = "Hooking AFortPlayerController_CompleteBuildingEditInteraction, %llx";
    Log(message.c_str(), Asteria::Holder::AFortPlayerController_CompleteBuildingEditInteraction - ImageBase);
    if (Asteria::Holder::AFortPlayerController_PerformBuildingEditInteraction && FNVer <= 15.00)
    {
        std::string message = "Hooking AFortPlayerController_PerformBuildingEditInteraction, %llx";
        Log(message.c_str(), Asteria::Holder::AFortPlayerController_PerformBuildingEditInteraction - ImageBase);
        MH_CreateHook(
            (LPVOID)Asteria::Holder::AFortPlayerController_PerformBuildingEditInteraction,
            &PerformBuildingEditInteractionHook,
            reinterpret_cast<void**>(&PerformBuildingEditInteractionOG)
        ); 
    } else
    {
        Log("Failed to hook PerformBuildingEditInteraction");
    }
    if (Asteria::Holder::AFortPlayerController_EditSelect && FNVer <= 10.40)
    {
        MH_CreateHook(
          (LPVOID)Asteria::Holder::AFortPlayerController_EditSelect,
          &EditSelectHook,
          reinterpret_cast<void**>(&EditSelectOG)
      );
    } else
    {
        Log("Failed to hook EditSelect");
    }
    if (Asteria::Holder::AFortPlayerController_BuildingReset && FNVer <= 24.20)
    {
        std::string message = "Hooking AFortPlayerController_BuildingReset, %llx";
        Log(message.c_str(), Asteria::Holder::AFortPlayerController_BuildingReset - ImageBase);
        MH_CreateHook(
          (LPVOID)Asteria::Holder::AFortPlayerController_BuildingReset,
          &EditResetHook,
          reinterpret_cast<void**>(&EditResetOG)
      );
    } else
    {
        Log("Failed to hook EditReset");
    }
    MH_EnableHook(MH_ALL_HOOKS);
}