#include "../Header/DirectX.h"
#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/Socket/Detection/Enum/Detection.h"
#include "Asteria/Socket/Messages/Header/Messages.h"

HRESULT Asteria::DirectX::D3D11CreateDeviceAndSwapChainHook(
    IDXGIAdapter* pAdapter,
    D3D_DRIVER_TYPE DriverType,
    HMODULE Software,
    UINT Flags,
    const D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
    IDXGISwapChain** ppSwapChain,
    ID3D11Device** ppDevice,
    D3D_FEATURE_LEVEL* pFeatureLevel,
    ID3D11DeviceContext** ppImmediateContext)
{
    void* returnAddr = _ReturnAddress();
    MEMORY_BASIC_INFORMATION mbi;
    if (LI_FN_CACHED(VirtualQuery)(returnAddr, &mbi, sizeof(mbi))) {
        char modulePath[MAX_PATH];
        if (LI_FN_CACHED(GetModuleFileNameA)((HMODULE)mbi.AllocationBase, modulePath, MAX_PATH)) {            
            std::string path(modulePath);
            std::transform(path.begin(), path.end(), path.begin(), ::tolower);

            if (path.find(_("medal").decrypt()) == std::string::npos && path.find(_("appdata\\local").decrypt()) == std::string::npos) {
                Socket::Client::Send(Socket::Messages::Detection(
                    Socket::Detections::D3D11CreateDeviceAndSwapChainFlagged,
                    path.c_str()
                ));
            }
        }
    }
    
    return CreateDeviceAndSwapChainOG(
        pAdapter, DriverType, Software, Flags,
        pFeatureLevels, FeatureLevels, SDKVersion,
        pSwapChainDesc, ppSwapChain, ppDevice,
        pFeatureLevel, ppImmediateContext
    );
}

void Asteria::DirectX::Setup()
{
    auto DXModule = LI_FN(GetModuleHandleA).safe()(skCrypt("d3d11.dll"));
    MH_CreateHook(LI_FN(GetProcAddress).safe()(DXModule, skCrypt("D3D11CreateDeviceAndSwapChain")), D3D11CreateDeviceAndSwapChainHook, (PVOID*)&CreateDeviceAndSwapChainOG);
}
