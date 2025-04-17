#pragma once
#include "Framework.h"
#include "Asteria/Finder/Header/Finder.h"

namespace Asteria::Globals
{
    inline auto Backend = skCrypt(L"https://api-v1-horizon-fortnite-api.solarisfn.org"); // someone fix this, i have to define in func rn
    inline bool bConsole = false;
}