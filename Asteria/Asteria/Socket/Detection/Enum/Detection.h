#pragma once

namespace Asteria::Socket::Detections
{
    enum Detection
    {
        ProcessEventCalledOutside,
        ProcessEventFlaggedFunction,
        D3D11CreateDeviceAndSwapChainFlagged,
        RuntimeMemoryTampered,
        LDRLoadDLLFlagged,
        ExternalDetection,
        UnhookedFunction
    };
}