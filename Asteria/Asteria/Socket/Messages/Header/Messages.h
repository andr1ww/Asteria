#pragma once
#include "Framework.h"

namespace Asteria::Socket::Messages
{
    inline wstring wpassword = wstring();
    inline uint64 LastSendTime = 0;
    string Init();
    string Heartbeat(const uint64 SendTime);
}