// this code isn't obviously written by me, because I currently am learning encryption and all, and am a beginnner, so don't expect me to know cryptography deeply !

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string>
#include <stdexcept>
#include <openssl/x509.h>
#include <string>
#include <stdexcept>
#include <openssl/rand.h>
#include <cstring>


#ifndef sharedStructsIncluded
#include "sharedStructs.h"
#endif

#define ENDEincluded ;

std::string serializePublicKeyToString(EVP_PKEY* pubkey) {
    int len = i2d_PUBKEY(pubkey, nullptr);
    if (len <= 0) {
        throw std::runtime_error("i2d_PUBKEY failed");
    }

    std::string serialized(len, '\0');  // allocate string with len bytes
    unsigned char* p = reinterpret_cast<unsigned char*>(serialized.data());
    i2d_PUBKEY(pubkey, &p);  // serialize into string buffer

    return serialized;
}

EVP_PKEY* deserializePublicKeyFromString(const std::string& serialized) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(serialized.data());
    EVP_PKEY* pubkey = d2i_PUBKEY(nullptr, &p, serialized.size());
    if (!pubkey) {
        throw std::runtime_error("d2i_PUBKEY failed");
    }
    return pubkey;
}



// AES-GCM encrypt: returns IV + ciphertext + tag
std::string aesEncrypt(const std::string& aesKey, const std::string& data) {
    if (aesKey.size() != 32) throw std::runtime_error("AES-256 key must be 32 bytes");

    std::string iv(12, '\0'); // 12-byte IV
    if (1 != RAND_bytes(reinterpret_cast<unsigned char*>(&iv[0]), iv.size()))
        throw std::runtime_error("RAND_bytes failed");

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (1 != EVP_CipherInit_ex(ctx, EVP_aes_256_gcm(), nullptr,
                               reinterpret_cast<const unsigned char*>(aesKey.data()),
                               reinterpret_cast<const unsigned char*>(iv.data()),
                               1)) { // 1 = encrypt
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CipherInit_ex failed");
    }

    std::string out(data.size() + 16, '\0'); // +16 for safety
    int len = 0;

    if (1 != EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char*>(&out[0]), &len,
                              reinterpret_cast<const unsigned char*>(data.data()),
                              data.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CipherUpdate failed");
    }
    int total = len;

    if (1 != EVP_CipherFinal_ex(ctx, reinterpret_cast<unsigned char*>(&out[0]) + total, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CipherFinal_ex failed");
    }
    total += len;
    out.resize(total);

    // Get GCM tag (16 bytes)
    std::string tag(16, '\0');
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, reinterpret_cast<unsigned char*>(&tag[0]))) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl GET_TAG failed");
    }

    EVP_CIPHER_CTX_free(ctx);

    return iv + out + tag; // prepend IV, append tag
}

// AES-GCM decrypt: expects IV + ciphertext + tag as input
std::string aesDecrypt(const std::string& aesKey, const std::string& encrypted) {
    if (aesKey.size() != 32) throw std::runtime_error("AES-256 key must be 32 bytes");
    if (encrypted.size() < 12 + 16) throw std::runtime_error("Encrypted data too short");

    std::string iv = encrypted.substr(0, 12);
    std::string tag = encrypted.substr(encrypted.size() - 16, 16);
    std::string ciphertext = encrypted.substr(12, encrypted.size() - 12 - 16);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (1 != EVP_CipherInit_ex(ctx, EVP_aes_256_gcm(), nullptr,
                               reinterpret_cast<const unsigned char*>(aesKey.data()),
                               reinterpret_cast<const unsigned char*>(iv.data()),
                               0)) { // 0 = decrypt
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CipherInit_ex failed");
    }

    std::string out(ciphertext.size() + 16, '\0'); // +16 for safety
    int len = 0;

    if (1 != EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char*>(&out[0]), &len,
                              reinterpret_cast<const unsigned char*>(ciphertext.data()),
                              ciphertext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CipherUpdate failed");
    }
    int total = len;

    // Set tag **before finalizing**
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, reinterpret_cast<unsigned char*>(&tag[0]))) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl SET_TAG failed");
    }

    if (1 != EVP_CipherFinal_ex(ctx, reinterpret_cast<unsigned char*>(&out[0]) + total, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("AES GCM decrypt failed: tag mismatch");
    }
    total += len;
    out.resize(total);

    EVP_CIPHER_CTX_free(ctx);
    return out;
}