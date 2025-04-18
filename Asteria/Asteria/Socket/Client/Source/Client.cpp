#include "../Header/Client.h"
#include "Asteria/Socket/Messages/Header/Messages.h"
#include <memory>

namespace Asteria::Socket::Client {
    static std::unique_ptr<ix::WebSocket> WebSocket = nullptr;
    bool bIsConnectionOpen = false;
    std::unique_ptr<std::thread> HeartbeatThread = nullptr;
    std::atomic<bool> bShouldRunHeartbeat = false;
}

void Asteria::Socket::Client::Setup(string URL)
{
    Cleanup();
    if (!ix::initNetSystem())
    {
        Log("wow");
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try {
        WebSocket = std::make_unique<ix::WebSocket>();
        WebSocket->setUrl(URL);
        WebSocket->setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
            OnMessageCallback(msg);
        });
        
        WebSocket->start();
    } catch (const std::exception& e) {
        Log(e.what());
        Cleanup();
        return;
    }
    
    const int maxWaitTimeMs = 5000;
    int elapsedTime = 0;
    while (!bIsConnectionOpen && elapsedTime < maxWaitTimeMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsedTime += 100;
    }
    
    if (!bIsConnectionOpen) {
        Cleanup();
    }
}

uint64 Asteria::Socket::Client::GetUnixTime()
{ 
    const auto CurrentTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(CurrentTime.time_since_epoch()).count();
}

void Asteria::Socket::Client::HeartbeatLoop()
{
    while (bShouldRunHeartbeat)
    {
        if (bIsConnectionOpen)
        {
            uint64 CurrentTime = GetUnixTime();
            std::string HeartbeatMessage = Messages::Heartbeat(CurrentTime);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            Send(HeartbeatMessage);
        }
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }
}


void Asteria::Socket::Client::OnMessageCallback(const ix::WebSocketMessagePtr& Message)
{
    if (!Message) {
        return;
    }
    
    switch (Message->type)
    {
    case ix::WebSocketMessageType::Open:
        {
            bIsConnectionOpen = true;
            string Message = Messages::Init();
            Send(Message);
            bShouldRunHeartbeat = true;
            HeartbeatThread = std::make_unique<std::thread>(HeartbeatLoop);
            break;
        }
    case ix::WebSocketMessageType::Close:
        bIsConnectionOpen = false;
        LI_FN_CACHED(TerminateProcess)(LI_FN_CACHED(GetCurrentProcess)(), 0);
        break;
    case ix::WebSocketMessageType::Error:
        {
            bIsConnectionOpen = false;
            LI_FN_CACHED(TerminateProcess)(LI_FN_CACHED(GetCurrentProcess)(), 0);
            break;
        }
    default:
        break;
    }
}

void Asteria::Socket::Client::Send(const std::string& Message)
{
    if (WebSocket && bIsConnectionOpen)
    {
        WebSocket->send(Message);
    }
}

void Asteria::Socket::Client::Cleanup()
{
    if (WebSocket)
    {
        if (bIsConnectionOpen)
        {
            WebSocket->stop();
            bIsConnectionOpen = false;
        }
        ix::uninitNetSystem();
        WebSocket.reset();
    }
}