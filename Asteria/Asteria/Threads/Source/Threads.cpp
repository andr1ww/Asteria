#include "../Header/Threads.h"

#include "Asteria/Holder.h"
#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/Socket/Messages/Header/Messages.h"
#include "Asteria/Windows/Header/Windows.h"

void Asteria::Threads::HookCheckingThread()
{
    fuckIda;

    struct Funcs {
        void* address;
        const char* name;
    };
    
    auto D3D11CreateDeviceAndSwapChain = LI_FN(GetProcAddress).safe()(LI_FN(GetModuleHandleA).safe()(skCrypt("d3d11.dll")), skCrypt("D3D11CreateDeviceAndSwapChain"));
    
    Funcs checks[] = {
        { D3D11CreateDeviceAndSwapChain, "D3D11CreateDeviceAndSwapChain" },
        { LI_FN_CACHED(LdrLoadDll), "LdrLoadDll" },
        { LI_FN_CACHED(RtlCreateUserThread), "RtlCreateUserThread" },
        { LI_FN_CACHED(NtAllocateVirtualMemory), "NtAllocateVirtualMemory" },
        { LI_FN_CACHED(NtQueueApcThread), "NtQueueApcThread" },
        { LI_FN_CACHED(WriteProcessMemory), "WriteProcessMemory" }
    };
    
    while (true)
    {
        for (const auto& check : checks)
        {
            if (check.address && *(uint8_t*)check.address != 0xe9)
            {
                std::string detectionMsg = skCrypt("Attempted Unhook of: ").decrypt();
                detectionMsg += check.name;
                
                Socket::Client::Send(Socket::Messages::Detection(
                    Socket::Detections::UnhookedFunction,
                    detectionMsg.c_str()
                ));
                return; 
            }
        }
        
        LI_FN_CACHED(Sleep)(100);
    }
}