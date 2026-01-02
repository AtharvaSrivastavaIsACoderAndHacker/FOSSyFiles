

#pragma once
#include <openssl/evp.h>
#include <stdexcept>

#ifndef sharedStructsIncluded
#include "sharedStructs.h"
#endif

#define keyGenHeaderIncluded ;

inline DHKeyPair generateDHKeyPair() {
    DHKeyPair pair{nullptr, nullptr};

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr);
    if (!ctx) throw std::runtime_error("CTX fail");

    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_keygen(ctx, &pair.privateKey);

    // extract public key
    size_t len = 0;
    EVP_PKEY_get_raw_public_key(pair.privateKey, nullptr, &len);

    std::string pub(len, '\0');
    EVP_PKEY_get_raw_public_key(pair.privateKey,
        reinterpret_cast<unsigned char*>(&pub[0]), &len);

    pair.publicKey = EVP_PKEY_new_raw_public_key(
        EVP_PKEY_X25519, nullptr,
        reinterpret_cast<unsigned char*>(&pub[0]), pub.size()
    );

    EVP_PKEY_CTX_free(ctx);
    return pair;
}


std::string deriveAESKey256(const std::string& sharedSecret) {
    unsigned char out[32]; // 256-bit AES key

    EVP_MD_CTX* md = EVP_MD_CTX_new();
    EVP_DigestInit_ex(md, EVP_sha256(), nullptr);
    EVP_DigestUpdate(md, sharedSecret.data(), sharedSecret.size());
    EVP_DigestFinal_ex(md, out, nullptr);
    EVP_MD_CTX_free(md);

    return std::string(reinterpret_cast<char*>(out), 32);
}

inline std::string deriveSharedSecret(
    EVP_PKEY* myPrivateKey,
    EVP_PKEY* peerPublicKey
) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(myPrivateKey, nullptr);
    if (!ctx) throw std::runtime_error("derive ctx fail");

    if (EVP_PKEY_derive_init(ctx) <= 0)
        throw std::runtime_error("derive init fail");

    if (EVP_PKEY_derive_set_peer(ctx, peerPublicKey) <= 0)
        throw std::runtime_error("set peer key fail");

    size_t secretLen = 0;
    EVP_PKEY_derive(ctx, nullptr, &secretLen);

    std::string secret(secretLen, '\0');

    if (EVP_PKEY_derive(
            ctx,
            reinterpret_cast<unsigned char*>(&secret[0]),
            &secretLen
        ) <= 0)
        throw std::runtime_error("derive failed");

    secret.resize(secretLen);
    EVP_PKEY_CTX_free(ctx);
    return secret;
}
