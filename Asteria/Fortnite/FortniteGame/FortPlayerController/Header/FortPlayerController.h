#pragma once
#include "Framework.h"

namespace FortniteGame::FortPlayerController
{
	inline bool bEditOnRelease = std::find(args.begin(), args.end(), skCrypt(L"-eor").decrypt()) != args.end();
	inline bool bDisablePreedits = std::find(args.begin(), args.end(), skCrypt(L"-nopreedits").decrypt()) != args.end();
    
    void PerformBuildingEditInteractionHook(AFortPlayerControllerAthena* PC);
    void EditSelectHook(AFortPlayerControllerAthena* PC);

    void Setup();
}