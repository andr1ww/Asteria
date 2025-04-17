#pragma once
#include "Framework.h"

namespace FortniteGame::FortPlayerController
{
    static std::vector<std::wstring> args = []() {
        std::vector<std::wstring> result;
        LPWSTR* argv;
        int argc;

        argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv) {
            for (int i = 0; i < argc; ++i) {
                result.emplace_back(argv[i]);
            }
            LocalFree(argv);
        }
        return result;
    }();

	inline bool bEditOnRelease = std::find(args.begin(), args.end(), skCrypt(L"-eor").decrypt()) != args.end();
	inline bool bDisablePreedits = std::find(args.begin(), args.end(), skCrypt(L"-nopreedits").decrypt()) != args.end();
    
    void PerformBuildingEditInteractionHook(AFortPlayerControllerAthena* PC);
    void EditSelectHook(AFortPlayerControllerAthena* PC);

    void Setup();
}