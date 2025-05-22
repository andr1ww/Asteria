#include "Framework.h"
#include "../Header/Hardware.h"
#include <vector>
#include <format>
#include <bitset>
#include <Sddl.h>
#include <Lmcons.h>
#include <winioctl.h>
#include <sstream>

#include "Asteria/Socket/Client/Header/Client.h"
#include "Asteria/Socket/Messages/Header/Messages.h"
#include "Framework.h"
#include "Asteria/Holder.h"

string Asteria::Hardware::Init()
{
    fuckIda;
    std::vector<std::string> DiskDrives{};
    std::string MotherUUID{};
    std::string Sid{};

    for (int i = 0; i < 16; ++i)
    {
        const std::string Name = _("\\\\.\\PhysicalDrive").decrypt() + std::to_string(i);
        HANDLE Handle = LI_FN_CACHED(CreateFileA)(
            Name.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, 
            OPEN_EXISTING,
            0,
            nullptr
        );
        if (!Handle || Handle == INVALID_HANDLE_VALUE)
        {
            continue;
        }

        STORAGE_PROPERTY_QUERY Query{};
        Query.PropertyId = StorageDeviceProperty;
        Query.QueryType = PropertyStandardQuery;

        uint8_t Buffer[1024]{};
        DWORD BytesReturned{};
        if (!LI_FN_CACHED(DeviceIoControl)(
            Handle,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &Query,
            sizeof(STORAGE_PROPERTY_QUERY),
            Buffer,
            sizeof(Buffer),
            &BytesReturned,
            nullptr
        ))
        {
            continue;
        }

        STORAGE_DEVICE_DESCRIPTOR* DeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(Buffer);
        if (DWORD SerialNumberOffset = DeviceDescriptor->SerialNumberOffset)
        {
            const std::string SerialNumber = reinterpret_cast<const char*>(Buffer + SerialNumberOffset);
            
            std::string Bytes{};
            for (const char Character : SerialNumber)
            {
                const uint8 UnsignedCharacter = static_cast<uint8>(Character);
                Bytes += std::format("{:02X}", UnsignedCharacter);
            }

            if (!SerialNumber.empty())
            {
                DiskDrives.emplace_back(Bytes);
            }
        }
        else
        {
            continue;
        }
        
        CloseHandle(Handle);
    }

    constexpr size_t BufferSize = 0x200;
    uint8_t* Buffer = static_cast<uint8_t*>(malloc(BufferSize));
    const size_t TableSize = LI_FN_CACHED(GetSystemFirmwareTable)('RSMB', 0, Buffer, BufferSize);
    
    if (TableSize > BufferSize)
    {
        free(Buffer);
        Buffer = nullptr;
        Buffer = static_cast<uint8_t*>(malloc(TableSize));
        DWORD bytesRead = LI_FN_CACHED(GetSystemFirmwareTable)('RSMB', 0, Buffer, TableSize);
    }
    else
    {
        uint8_t* NewBuffer = static_cast<uint8_t*>(malloc(TableSize));
        memcpy(NewBuffer, Buffer, TableSize);
        free(Buffer);
        Buffer = NewBuffer;
    }

    typedef struct _SMBIOS_HEADER
    {
        BYTE Type;
        BYTE Length;
        WORD Handle;
    } SMBIOS_HEADER;

    typedef struct _SMBIOS_SYSTEM_INFO
    {
        SMBIOS_HEADER Header;
        BYTE Manufacturer;
        BYTE ProductName;
        BYTE Version;
        BYTE SerialNumber;
        BYTE UUID[16];
        BYTE WakeUpType;
        BYTE SKUNumber;
        BYTE Family;
    } SMBIOS_SYSTEM_INFO;

    uint8_t* Table = Buffer;
    while (Table + sizeof(SMBIOS_HEADER) <= Buffer + TableSize)
    {
        SMBIOS_HEADER* Header = reinterpret_cast<SMBIOS_HEADER*>(Table);
        
        if (Header->Type != 1 || Header->Length < sizeof(SMBIOS_SYSTEM_INFO) - 1)
        {
            Table += Header->Length;
            while (*(WORD*)Table != 0)
            {
                Table++;
            }

            Table += 2;
            continue;
        }

        SMBIOS_SYSTEM_INFO* SystemInfo = reinterpret_cast<SMBIOS_SYSTEM_INFO*>(Table);
        for (int i = 0; i < 16; i++)
        {
            MotherUUID += std::format(
                "{:02X}",
                SystemInfo->UUID[i]
            );
        }
        break;
    }

    free(Buffer);
    
    char Username[UNLEN + 1];
    DWORD UsernameLength = UNLEN + 1;
    LI_FN_CACHED(GetUserNameA)(Username, &UsernameLength);
    char DomainName[1024];
    DWORD DomainNameLength = 1024;

    SID_NAME_USE SIDType;
    char SidBuffer[1024];
    DWORD SidLength = 1024;
    SID* SidObj = reinterpret_cast<SID*>(SidBuffer);

    if (LI_FN_CACHED(LookupAccountNameA)(NULL, Username, SidBuffer, &SidLength, DomainName, &DomainNameLength, &SIDType)) {
        char* SidString{ 0 };
        if (LI_FN_CACHED(ConvertSidToStringSidA)(SidObj, &SidString)) {
            Sid = SidString;
            LI_FN_CACHED(LocalFree)(SidString);
        } 
    } 
    
   return std::string(Socket::Messages::HWID(DiskDrives, MotherUUID, Sid));
}
