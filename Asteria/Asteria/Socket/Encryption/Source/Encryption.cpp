#include "Framework.h"
#include "../Header/Encryption.h"

#include <openssl/sha.h>
#include <vector>
#include <cstdio>
#include <sstream>
#include <cstring>

#include "Asteria/Holder.h"

struct Result {
    std::string encryptedData;
    std::string encryptedHash;
    
    std::string serialize() const {
        fuckIda;
        std::string result;
        uint32_t dataLen = static_cast<uint32_t>(encryptedData.length());
        result.append(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
        result.append(encryptedData);
        result.append(encryptedHash);
        return result;
    }
    
    static Result deserialize(const std::string& serialized) {
        fuckIda;
        Result result;
        if (serialized.length() < sizeof(uint32_t)) return result;
        
        uint32_t dataLen;
        std::memcpy(&dataLen, serialized.data(), sizeof(dataLen));
        
        if (serialized.length() < sizeof(uint32_t) + dataLen) return result;
        
        result.encryptedData = serialized.substr(sizeof(uint32_t), dataLen);
        result.encryptedHash = serialized.substr(sizeof(uint32_t) + dataLen);
        return result;
    }
};

Result Main(const std::string& plainText) {
    fuckIda;
    static auto publicKeyEncrypted =
        skCrypt(R"(-----BEGIN PUBLIC KEY-----
MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAv4uZAFYdLMFtULQMpNop
WMxWzu3RgOk6fw96mvMorWiGjH/vzCWtTGralJ2rLZmPg2O/qSuOrFQ9EsVslR0w
hIZE2M5OheLWywB/TGqBkFjkqX4vWJDrWHsuaMt9SuOcVEB3GyNd973VfI4L8n1f
yu2yTTiSTD5XST/HZsFTYBPvYBoO+AXVlWalplpUU0KDycgqCVSmhoy0dp0uguDA
XYTpM7ZR/pcWbixdwEMeBNRx8c2XOObybgkB5KN1e1h2hV95dcCEsm0KZ52QG6XL
736c78ARxSWclS97PyQ2UqFhUcIG479SK5rUKRWtbvqVvLehmaNs+N92cF+FyVSV
yLZVCaUkX/qhiBySy19CaKTcBwUY/RbqeTIniAHCk6NyTDOtHS2lPkEktyLycj+S
Ego24QT8IvglvNgDP+pRcYGXZK09qWyx8PeFV6CcWiDPM0TJHXwsGV/NXVLSXdEW
DtUjGnCz50rK8sVwbXgPlU4c0XHfJ1eftVY7ll5tn2nS0DvRgFRbHaqu1l4HdF4D
gd0GdMuXPrfD4uv7SvVEr6EVcG93TMlpdg6P5Evm1ch5//NzJETbDyBrvVwduyMm
wTigIuTLq2K5rLiEm/LABipkzGD2ytcQ1QxzE7F/6Lh7QlrsMjYufijN1hoYDnx2
Mcleb0xxrVdTfNXvIXkpiA0CAwEAAQ==
-----END PUBLIC KEY-----)");

    const std::string publicKeyStr = publicKeyEncrypted.decrypt();
    publicKeyEncrypted.encrypt();

    BIO* bio = BIO_new_mem_buf(publicKeyStr.data(), publicKeyStr.size());
    if (!bio) {
        return {};
    }

    EVP_PKEY* publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (!publicKey) {
        BIO_free(bio);
        return {};
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
    if (!ctx) {
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0 ||
        EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) {

        unsigned long errorCode = ERR_get_error();
        char errorMsg[256] = {0};
        ERR_error_string_n(errorCode, errorMsg, sizeof(errorMsg));

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }

    size_t cipherLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &cipherLen,
                         reinterpret_cast<const unsigned char*>(plainText.data()),
                         plainText.size()) <= 0) {

        unsigned long errorCode = ERR_get_error();
        char errorMsg[256] = {0};
        ERR_error_string_n(errorCode, errorMsg, sizeof(errorMsg));

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }

    std::vector<uint8_t> firstEncryptedData(cipherLen);
    if (EVP_PKEY_encrypt(ctx, firstEncryptedData.data(), &cipherLen,
                         reinterpret_cast<const unsigned char*>(plainText.data()),
                         plainText.size()) <= 0) {

        unsigned long errorCode = ERR_get_error();
        char errorMsg[256] = {0};
        ERR_error_string_n(errorCode, errorMsg, sizeof(errorMsg));

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(firstEncryptedData.data(), firstEncryptedData.size(), hash);

    cipherLen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &cipherLen, hash, SHA256_DIGEST_LENGTH) <= 0) {
        unsigned long errorCode = ERR_get_error();
        char errorMsg[256] = {0};
        ERR_error_string_n(errorCode, errorMsg, sizeof(errorMsg));

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }

    std::vector<uint8_t> secondEncryptedData(cipherLen);
    if (EVP_PKEY_encrypt(ctx, secondEncryptedData.data(), &cipherLen, hash, SHA256_DIGEST_LENGTH) <= 0) {
        unsigned long errorCode = ERR_get_error();
        char errorMsg[256] = {0};
        ERR_error_string_n(errorCode, errorMsg, sizeof(errorMsg));

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(publicKey);
    BIO_free(bio);

    auto toHex = [](const std::vector<uint8_t>& data) -> std::string {
        std::string hexResult;
        hexResult.reserve(data.size() * 2);
        for (const uint8_t byte : data) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02X", byte);
            hexResult += hex;
        }
        return hexResult;
    };

    Result result;
    result.encryptedData = toHex(firstEncryptedData);
    result.encryptedHash = toHex(secondEncryptedData);
    
    return result;
}


std::string Asteria::Socket::Encryption::Encrypt(const std::string& plainText) {
    fuckIda;
    auto result = Main(plainText);
    std::string binaryData = result.serialize(); 
    
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, binaryData.data(), binaryData.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    std::string base64Result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);
    
    return base64Result;
}

std::wstring Asteria::Socket::Encryption::Encrypt(const std::wstring& plainText) {
    fuckIda;
    const std::string plainTextA(plainText.begin(), plainText.end());
    std::string base64Result = Encrypt(plainTextA);
    return { base64Result.begin(), base64Result.end() };
}