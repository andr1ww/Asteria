#include "Framework.h"
#include "../Header/Messages.h"

#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/Socket/Detection/Enum/Detection.h"
#include "Asteria/Socket/Encryption/Header/Encryption.h"
#include "Framework.h"
#include "Asteria/Holder.h"

string Asteria::Socket::Messages::Init()
{
    fuckIda;
    static bool bSetBackend = false;
    if (!bSetBackend && Holder::Backend != L"" && !Holder::Backend.empty())
    {
        bSetBackend = true;
        nlohmann::json JSON{};
        JSON[(skCrypt("message_type").decrypt())] = _("backend").decrypt();
        std::string backend_str(Holder::Backend.begin(), Holder::Backend.end());
        JSON[(skCrypt("url").decrypt())] = backend_str;

        string Message = Encryption::Encrypt(JSON.dump());
        return Message;
    }

    Init();
}

string Asteria::Socket::Messages::Register()
{
    fuckIda;
    static bool bInit = false;
    if (!bInit)
    {
        auto Argument = std::find_if(args.begin(), args.end(), [](const std::wstring& arg) {
            return arg.find(L"-AUTH_PASSWORD=") == 0;
        });
    
        if (Argument != args.end())
        {
            wpassword = Argument->substr(Argument->find(L"=") + 1);
            if (!wpassword.empty())
            {
                std::string password(wpassword.begin(), wpassword.end());
            
                nlohmann::json JSON{};
                JSON[(skCrypt("message_type").decrypt())] = _str("register");
                JSON[(skCrypt("exchange_code").decrypt())] = password;
                JSON[(skCrypt("timestamp").decrypt())] = Client::GetUnixTime();

                string Message = Encryption::Encrypt(JSON.dump());

                bInit = true;
            
                return Message;
            }
        }
    }
    
    return "";
}

string Asteria::Socket::Messages::Heartbeat(const uint64 SendTime)
{
    fuckIda;
    const uint64 CurrentTime = Client::GetUnixTime();
    if (CurrentTime - LastSendTime >= 15)
    {
        LastSendTime = SendTime;
        nlohmann::json JSON{};
        JSON[(skCrypt("message_type").decrypt())] = _str("heartbeat");
        JSON[(skCrypt("exchange_code").decrypt())] = string(wpassword.begin(), wpassword.end());
        JSON[(skCrypt("timestamp").decrypt())] = SendTime;
        string Message = Encryption::Encrypt(JSON.dump());

        return Message;
    }
    return "";
}

string Asteria::Socket::Messages::Detection(Detections::Detection DetectionType, string msg)
{
    fuckIda;
    nlohmann::json JSON{};
    JSON[(skCrypt("message_type").decrypt())] = _("detection").decrypt();
    JSON[(skCrypt("exchange_code").decrypt())] = string(wpassword.begin(), wpassword.end());
    JSON[(skCrypt("detection").decrypt())] = static_cast<int32>(DetectionType);
    JSON[(skCrypt("message").decrypt())] = std::string(msg.begin(), msg.end());
    JSON[(skCrypt("timestamp").decrypt())] = Client::GetUnixTime();

    string Message = Encryption::Encrypt(JSON.dump());
    return Message;
}

string Asteria::Socket::Messages::HWID(std::vector<std::string> DiskDrives, string MotherboardUUID, string SID)
{
    fuckIda;
    nlohmann::json JSON{};
    JSON[(skCrypt("message_type").decrypt())] = _("hwid").decrypt();
    JSON[(skCrypt("exchange_code").decrypt())] = string(wpassword.begin(), wpassword.end());
    JSON[(skCrypt("disk_drives").decrypt())] = DiskDrives;
    JSON[(skCrypt("motherboard_uuid").decrypt())] = MotherboardUUID;
    JSON[(skCrypt("sid").decrypt())] = SID;
    JSON[(_("install_time").decrypt())] = 0;
    JSON[(skCrypt("timestamp").decrypt())] = Client::GetUnixTime();

    string Message = Encryption::Encrypt(JSON.dump());
    return Message;
}