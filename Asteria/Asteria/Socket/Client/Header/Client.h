#pragma once
#include "Framework.h"
#include <memory>

namespace Asteria::Socket::Client {
    extern bool bIsConnectionOpen;
    
    void Setup(string URL);
    void OnMessageCallback(const ix::WebSocketMessagePtr& Message);
    void Send(const std::string& Message);
    void Cleanup();
}