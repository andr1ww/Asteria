#pragma once
#include "Framework.h"

namespace FortniteGame::FortPlayerController
{
	inline bool bEditOnRelease = std::find(args.begin(), args.end(), skCrypt(L"-eor").decrypt()) != args.end();
	inline bool bDisablePreedits = std::find(args.begin(), args.end(), skCrypt(L"-nopreedits").decrypt()) != args.end();
	inline bool bEnableResetOnRelease = std::find(args.begin(), args.end(), skCrypt(L"-ror").decrypt()) != args.end();

    void PerformBuildingEditInteractionHook(UObject* PC);
    void EditSelectHook(UObject* PC);
	void EditResetHook(UObject* PC);

    void Setup();
}