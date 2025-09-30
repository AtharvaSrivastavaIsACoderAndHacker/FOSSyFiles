// RSA_keygen.h
#pragma once
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <stdexcept>

struct RSAKeyPair {
    EVP_PKEY* privateKey; // owns private key
    EVP_PKEY* publicKey;  // owns public key
};

// Modern OpenSSL 3.x keygen, const-correct
RSAKeyPair generateRSAKeyPair(int bits = 2048) {
    RSAKeyPair pair{nullptr, nullptr};

    // Create context for RSA key generation
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) throw std::runtime_error("EVP_PKEY_CTX_new_id failed");

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_keygen_init failed");
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_CTX_set_rsa_keygen_bits failed");
    }

    // Generate the private key
    if (EVP_PKEY_keygen(ctx, &pair.privateKey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_keygen failed");
    }

    // Allocate EVP_PKEY for public key
    pair.publicKey = EVP_PKEY_new();
    if (!pair.publicKey) {
        EVP_PKEY_free(pair.privateKey);
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_new failed for public key");
    }

    // Extract the RSA from private key (read-only)
    const RSA* rsaPriv = EVP_PKEY_get0_RSA(pair.privateKey);
    if (!rsaPriv) {
        EVP_PKEY_free(pair.privateKey);
        EVP_PKEY_free(pair.publicKey);
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_get0_RSA failed");
    }

    // Duplicate only the public key components (n, e)
    RSA* rsaPubDup = RSAPublicKey_dup(rsaPriv);
    if (!rsaPubDup) {
        EVP_PKEY_free(pair.privateKey);
        EVP_PKEY_free(pair.publicKey);
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("RSAPublicKey_dup failed");
    }

    // Assign the duplicated public key to EVP_PKEY
    if (EVP_PKEY_assign_RSA(pair.publicKey, rsaPubDup) <= 0) {
        RSA_free(rsaPubDup);
        EVP_PKEY_free(pair.privateKey);
        EVP_PKEY_free(pair.publicKey);
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("EVP_PKEY_assign_RSA failed");
    }

    // Cleanup
    EVP_PKEY_CTX_free(ctx);

    return pair;
}
