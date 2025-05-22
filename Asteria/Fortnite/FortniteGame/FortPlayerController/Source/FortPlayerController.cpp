#include "Framework.h"
#include "../Header/FortPlayerController.h"
#include "Asteria/Holder.h"
#include "Framework.h"

using AFortPlayerController_PerformBuildingEditInteractionParams = void(*)(UObject* PC);
inline AFortPlayerController_PerformBuildingEditInteractionParams PerformBuildingEditInteractionOG = nullptr;
using AFortPlayerController_EditSelectParams = void(*)(UObject* PC);
inline AFortPlayerController_EditSelectParams EditSelectOG = nullptr;
inline AFortPlayerController_EditSelectParams EditResetOG = nullptr;

inline void* (*CompleteBuildingEditInteraction)(UObject* PlayerController) = decltype(CompleteBuildingEditInteraction)(__int64(*(uint64_t*)(__readgsqword(0x60) + 0x10) + 0x18285A0));

void FortniteGame::FortPlayerController::PerformBuildingEditInteractionHook(UObject* PC)
{
    if (bDisablePreedits && PC) {
        static auto offset = PC->GetOffset("TargetedBuilding");
        if (offset != -1) {
            auto* targetedBuilding = GetFromOffset<UObject*>(PC, offset);
            if (targetedBuilding) {
                static auto* previewClass = TUObjectArray::FindObject<UClass*>("ABuildingPlayerPrimitivePreview");
                if (previewClass && targetedBuilding->IsA(*previewClass)) {
                    return;
                }
            }
        }
    }
    
    PerformBuildingEditInteractionOG(PC);
}

void FortniteGame::FortPlayerController::EditSelectHook(UObject* PC)
{
    EditSelectOG(PC);

    if (bEditOnRelease && PC) {
        static auto offset = PC->GetOffset("EditBuildingActor");
        if (offset != -1) {
            CompleteBuildingEditInteraction(PC);
        } 
    }
}

void FortniteGame::FortPlayerController::EditResetHook(UObject* PC) {
    EditResetOG(PC);
    
    if (bEnableResetOnRelease && PC) {
        static auto offset = PC->GetOffset("EditBuildingActor");
        if (offset != -1) {
            CompleteBuildingEditInteraction(PC);
        } 
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