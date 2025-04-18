#pragma once
#include "Framework.h"
#include "Asteria/Socket/Detection/Enum/Detection.h"

namespace Asteria::Socket::Messages
{
    inline wstring wpassword = wstring();
    inline uint64 LastSendTime = 0;
    string Init();
    string Heartbeat(const uint64 SendTime);
    string Detection(Detections::Detection DetectionType,  string msg);
    string HWID(std::vector<std::string> DiskDrives, string MotherboardUUID, string SID);
}
