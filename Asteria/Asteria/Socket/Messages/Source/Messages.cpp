#include "../Header/Messages.h"

void Asteria::Socket::Messages::Init()
{
    constexpr auto Argument = std::find(args.begin(), args.end(), skCrypt(L"-AUTH_PASSWORD=").decrypt());
    if (Argument != args.end())
    {
        std::wstring password = Argument->substr(Argument->find(L"=") + 1);
        if (!password.empty())
        {
            nlohmann::json JSON{};

            JSON[(skCrypt("message_type").decrypt())] = str("register");
            JSON[(skCrypt("exchange_code").decrypt())] = password;
        }
    }
}