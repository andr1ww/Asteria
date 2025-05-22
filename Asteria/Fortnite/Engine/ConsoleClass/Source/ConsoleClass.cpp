#include "../Header/ConsoleClass.h"

void Engine::ConsoleClass::EnableConsole()
{
    struct GameplayStatics_SpawnObject final
    {
    public:
        class UObject* ObjectClass = SDK::Engine->Get<L"ConsoleClass">();                                      
        class UObject* Outer_0 = SDK::Engine->Get<L"GameViewport">();                                           
        class UObject* ReturnValue;                                       
    };
    GameplayStatics_SpawnObject OK;
    GetDefaultObj<L"GameplayStatics">()->Call<L"SpawnObject">(&OK);
    SDK::Engine->Get<L"GameViewport">()->Get<L"ViewportConsole">() = OK.ReturnValue;
}