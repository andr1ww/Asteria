#include "../Header/Messages.h"

#include "Asteria/Socket/Encryption/Header/Encryption.h"

string Asteria::Socket::Messages::Init()
{
    auto Argument = std::find_if(args.begin(), args.end(), [](const std::wstring& arg) {
        return arg.find(L"-AUTH_PASSWORD=") == 0;
    });
    
    if (Argument != args.end())
    {
        std::wstring wpassword = Argument->substr(Argument->find(L"=") + 1);
        if (!wpassword.empty())
        {
            std::string password(wpassword.begin(), wpassword.end());
            
            nlohmann::json JSON{};
            JSON[(skCrypt("message_type").decrypt())] = _str("register");
            JSON[(skCrypt("exchange_code").decrypt())] = password;

            string Message = Encryption::Encrypt(JSON.dump());
            Log(Message);
            
            return Message;
        }
    }
    
    return "";
}