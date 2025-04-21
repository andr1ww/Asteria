#include "Framework.h"
#include "../Header/UObject.h"

#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/Socket/Messages/Header/Messages.h"

typedef void (*ProcessEventFn)(SDK::UObject*, UFunction*, void*);
ProcessEventFn OG_ProcessEvent = nullptr;
void Engine::UObject::ProcessEventHook(SDK::UObject* Object, UFunction* Function, void* Params)
{
    if (!Object || !Function)
    {
        return OG_ProcessEvent(Object, Function, Params);
    }

    static std::vector<uint64> Flagged = {};
    static std::vector<uint64> Backups = {};
    static size_t OGSize = 0;

    if (Flagged.empty())
    {
        Flagged = {
            Find(_str("Function Engine.Canvas.K2_DrawLine")),
            Find(_str("Function Engine.Canvas.K2_DrawBox")),
            Find(_str("Function Engine.Canvas.K2_DrawMaterial")),
            Find(_str("Function Engine.Canvas.K2_DrawMaterialTriangle")),
            Find(_str("Function Engine.Canvas.K2_DrawPolygon")),
            Find(_str("Function Engine.Canvas.K2_DrawText")),
            Find(_str("Function Engine.Canvas.K2_DrawTexture")),
            Find(_str("Function Engine.Canvas.K2_DrawProject")),
            Find(_str("Function Engine.Canvas.K2_DrawTextSize")),
            Find(_str("Function Engine.PlayerController.ProjectWorldLocationToScreen")),
            Find(_str("Function Engine.PlayerController.ServerChangeName")),
            Find(_str("Function Engine.GameplayStatics.ProjectWorldToScreen")),
            Find(_str("Function Engine.PrimitiveComponent.SetAllPhysicsLinearVelocity")),
            Find(_str("Function Engine.SkeletalMeshSocket.GetSocketLocation")),
            Find(_str("Function Engine.SceneComponent.GetSocketLocation"))
        };

        Backups = Flagged;
        OGSize = EncryptPTR(Flagged.size());
    }

    const size_t currentSize = Flagged.size();
    const size_t backupSize = Backups.size();
    const size_t expectedSize = EncryptPTR(OGSize);
    
    if (currentSize != backupSize || currentSize != expectedSize)
    {
        string tamperMessage = Asteria::Socket::Messages::Detection(
            Asteria::Socket::Detections::RuntimeMemoryTampered, 
            skCrypt("FlaggedFunctions size mismatch").decrypt()
        );
        Asteria::Socket::Client::Send(tamperMessage);
    }

    for (size_t i = 0; i < EncryptPTR(OGSize); ++i)
    {
        if (Flagged[i] != Flagged[i])
        {
            std::stringstream ss;
            ss << skCrypt("FlaggedFunction: 0x").decrypt() << std::hex << Flagged[i];
            
            string tamperMessage = Asteria::Socket::Messages::Detection(
                Asteria::Socket::Detections::RuntimeMemoryTampered, 
                ss.str()
            );
            Asteria::Socket::Client::Send(tamperMessage);
        }

        if (reinterpret_cast<uint64>(Function) == EncryptPTR(Flagged[i]))
        {
            std::stringstream ss;
            ss << skCrypt("FlaggedFunction: 0x").decrypt() << std::hex << Flagged[i];
            
            string accessMessage = Asteria::Socket::Messages::Detection(
                Asteria::Socket::Detections::RuntimeMemoryTampered, 
                ss.str()
            );
            Asteria::Socket::Client::Send(accessMessage);
            break;
        }
    }
    
    return OG_ProcessEvent(Object, Function, Params);
}

void Engine::UObject::Setup()
{
    MH_CreateHook(
        reinterpret_cast<void*>(ImageBase + SDK::Offsets::ProcessEvent),
        &ProcessEventHook,
        reinterpret_cast<void**>(&OG_ProcessEvent)
    );
}