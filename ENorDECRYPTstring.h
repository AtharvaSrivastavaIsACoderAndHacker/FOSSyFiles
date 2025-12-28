#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <string>
#include <stdexcept>
#include <openssl/x509.h>
#include <string>
#include <stdexcept>

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


// mode = 0 → encrypt with public key
// mode = 1 → decrypt with private key
std::string rsaCrypt(EVP_PKEY* key, const std::string& input, int mode) {

    
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new failed");

    if (mode == 0) {
        if (EVP_PKEY_encrypt_init(ctx) <= 0) throw std::runtime_error("encrypt_init failed");
    } else {
        if (EVP_PKEY_decrypt_init(ctx) <= 0) throw std::runtime_error("decrypt_init failed");
    }

    size_t outlen = 0;
    int ok;
    if (mode == 0) {
        ok = EVP_PKEY_encrypt(ctx, nullptr, &outlen,
                              reinterpret_cast<const unsigned char*>(input.data()),
                              input.size());
    } else {
        ok = EVP_PKEY_decrypt(ctx, nullptr, &outlen,
                              reinterpret_cast<const unsigned char*>(input.data()),
                              input.size());
    }
    if (ok <= 0) throw std::runtime_error("size query failed");

    std::string output;
    output.resize(outlen);

    if (mode == 0) {
        ok = EVP_PKEY_encrypt(ctx,
                              reinterpret_cast<unsigned char*>(&output[0]), &outlen,
                              reinterpret_cast<const unsigned char*>(input.data()),
                              input.size());
    } else {
        ok = EVP_PKEY_decrypt(ctx,
                              reinterpret_cast<unsigned char*>(&output[0]), &outlen,
                              reinterpret_cast<const unsigned char*>(input.data()),
                              input.size());
    }
    if (ok <= 0) throw std::runtime_error("encrypt/decrypt failed");

    output.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return output;
}
