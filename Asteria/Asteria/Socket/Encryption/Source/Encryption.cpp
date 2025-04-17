#include "../Header/Encryption.h"

std::string Asteria::Socket::Encryption::Encrypt(const std::string& plainText) {
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
        Log("RSA Encryption Error: BIO_new_mem_buf failed");
        return {};
    }
    
    EVP_PKEY* publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (!publicKey) {
        Log("RSA Encryption Error: PEM_read_bio_PUBKEY failed");
        BIO_free(bio);
        return {};
    }
    
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
    if (!ctx) {
        Log("RSA Encryption Error: EVP_PKEY_CTX_new failed");
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
        Log("RSA Encryption Error: Context setup failed: %s", errorMsg);
        
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
        Log("RSA Encryption Error: Buffer sizing failed: %s", errorMsg);
        
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }
    
    std::vector<uint8_t> encryptedData(cipherLen);
    if (EVP_PKEY_encrypt(ctx, encryptedData.data(), &cipherLen,
                         reinterpret_cast<const unsigned char*>(plainText.data()),
                         plainText.size()) <= 0) {
                         
        unsigned long errorCode = ERR_get_error();
        char errorMsg[256] = {0};
        ERR_error_string_n(errorCode, errorMsg, sizeof(errorMsg));
        Log("RSA Encryption Error: Encryption operation failed: %s", errorMsg);
        
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(publicKey);
        BIO_free(bio);
        return {};
    }
    
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(publicKey);
    BIO_free(bio);
    
    std::string hexResult;
    hexResult.reserve(encryptedData.size() * 2);
    
    for (const uint8_t byte : encryptedData) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02X", byte);
        hexResult += hex;
    }
    
    return hexResult;
}

std::wstring Asteria::Socket::Encryption::Encrypt(const std::wstring& plainText) {
    const std::string plainTextA(plainText.begin(), plainText.end());
    const std::string string = Encrypt(plainTextA);

    return { string.begin(), string.end() };
}