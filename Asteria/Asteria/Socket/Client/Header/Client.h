#pragma once
#include "Framework.h"
#include <memory>

namespace Asteria::Socket::Client {
    extern std::unique_ptr<ix::WebSocket> WebSocket;
    extern bool bIsConnectionOpen;
    
    void Setup(string URL);
    void OnMessageCallback(const ix::WebSocketMessagePtr& Message);
    void Send(const std::string& Message);
    void Cleanup();
}