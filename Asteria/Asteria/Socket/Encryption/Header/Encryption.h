#pragma once
#include "Framework.h"

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

namespace Asteria::Socket::Encryption {
    std::string Encrypt(const std::string& plainText);
    std::wstring Encrypt(const std::wstring& plainText);
}