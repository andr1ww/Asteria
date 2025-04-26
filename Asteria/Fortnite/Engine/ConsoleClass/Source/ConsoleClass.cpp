#include "../Header/ConsoleClass.h"

void Engine::ConsoleClass::EnableConsole(APlayerController* Player)
{
    UEngine::GetEngine()->GameViewport->ViewportConsole = (UConsole*)UGameplayStatics::SpawnObject(UConsole::StaticClass(), UEngine::GetEngine()->GameViewport);
    UGameplayStatics::SpawnObject(UFortCheatManager::StaticClass(), Player);
}