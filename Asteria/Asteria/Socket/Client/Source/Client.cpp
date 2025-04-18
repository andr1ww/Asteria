#include "../Header/Client.h"
#include "Asteria/Socket/Messages/Header/Messages.h"
#include <memory>

namespace Asteria::Socket::Client {
    static std::unique_ptr<ix::WebSocket> WebSocket = nullptr;
    bool bIsConnectionOpen = false;
    std::unique_ptr<std::thread> HeartbeatThread = nullptr;
    std::atomic<bool> bShouldRunHeartbeat = false;
    std::string ServerURL;
    int currentReconnectAttempts = 0;
    bool bReconnecting = false;
}

void Asteria::Socket::Client::Setup(string URL)
{
    Cleanup();
    ServerURL = URL;
    currentReconnectAttempts = 0;
    
    if (!ix::initNetSystem())
    {
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Connect();
}

void Asteria::Socket::Client::Connect()
{
    try {
        WebSocket = std::make_unique<ix::WebSocket>();
        WebSocket->setUrl(ServerURL);
        WebSocket->setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
            OnMessageCallback(msg);
        });
        
        WebSocket->start();
    } catch (const std::exception& e) {
        Log(e.what());
        Reconnect();
        return;
    }
    
    const int maxWaitTimeMs = 5000;
    int elapsedTime = 0;
    while (!bIsConnectionOpen && elapsedTime < maxWaitTimeMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsedTime += 100;
    }
    
    if (!bIsConnectionOpen) {
        Reconnect();
    }
}

void Asteria::Socket::Client::Reconnect()
{
    if (bReconnecting) {
        return;
    }
    
    bReconnecting = true;
    
    if (currentReconnectAttempts < 5) {
        currentReconnectAttempts++;
        if (WebSocket) {
            WebSocket->stop();
            WebSocket.reset();
        }
        
        bIsConnectionOpen = false;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        
        Connect();
    } else {
        Cleanup();
        LI_FN_CACHED(TerminateProcess)(LI_FN_CACHED(GetCurrentProcess)(), 0);
    }
    
    bReconnecting = false;
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
            currentReconnectAttempts = 0;  
            string Message = Messages::Init();
            Send(Message);
            
            if (!bShouldRunHeartbeat || !HeartbeatThread || !HeartbeatThread->joinable()) {
                bShouldRunHeartbeat = true;
                HeartbeatThread = std::make_unique<std::thread>(HeartbeatLoop);
            }
            
            break;
        }
    case ix::WebSocketMessageType::Close:
        {
            bIsConnectionOpen = false;
            Reconnect();
            break;
        }
    case ix::WebSocketMessageType::Error:
        {
            bIsConnectionOpen = false;
            Reconnect();
            break;
        }
    case ix::WebSocketMessageType::Message:
        {
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
    bShouldRunHeartbeat = false;
    
    if (HeartbeatThread && HeartbeatThread->joinable()) {
        HeartbeatThread->join();
        HeartbeatThread.reset();
    }
    
    if (WebSocket)
    {
        if (bIsConnectionOpen)
        {
            WebSocket->stop();
            bIsConnectionOpen = false;
        }
        WebSocket.reset();
    }
    
    ix::uninitNetSystem();
}