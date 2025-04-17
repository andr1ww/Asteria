#include "../Header/Client.h"
#include "Asteria/Socket/Messages/Header/Messages.h"
#include <memory>

namespace Asteria::Socket::Client {
    std::unique_ptr<ix::WebSocket> WebSocket = nullptr;
    bool bIsConnectionOpen = false;
}

void Asteria::Socket::Client::Setup(string URL)
{
    Cleanup();
    
    try {
        WebSocket = std::make_unique<ix::WebSocket>();
        WebSocket->setUrl(URL);
        WebSocket->setOnMessageCallback([](const ix::WebSocketMessagePtr& msg) {
            OnMessageCallback(msg);
        });
        
        WebSocket->start();
    } catch (const std::exception& e) {
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
            break;
        }
    case ix::WebSocketMessageType::Close:
        bIsConnectionOpen = false;
        LI_FN(TerminateProcess)(LI_FN(GetCurrentProcess)(), 0);
        break;
    case ix::WebSocketMessageType::Error:
        {
            bIsConnectionOpen = false;
            LI_FN(TerminateProcess)(LI_FN(GetCurrentProcess)(), 0);
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
        WebSocket.reset();
    }
}