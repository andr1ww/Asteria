#pragma once
#include "Framework.h"
#include <memory>

namespace Asteria::Socket::Client {
    extern bool bIsConnectionOpen;
    extern bool bReconnecting;
    extern int currentReconnectAttempts;
    extern std::string ServerURL;
    
    void Setup(string URL);
    void Connect();
    void Reconnect();
    void OnMessageCallback(const ix::WebSocketMessagePtr& Message);
    void Send(const std::string& Message);
    void Cleanup();
    void HeartbeatLoop();
    uint64 GetUnixTime();
}