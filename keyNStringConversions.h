#pragma once
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include <string>
#include <iostream>

inline void logOpenSSLErrors(const std::string& context) {
    unsigned long e;
    while ((e = ERR_get_error()) != 0) {
        char buf[256];
        ERR_error_string_n(e, buf, sizeof(buf));
        std::cerr << "[" << context << "] OpenSSL error: " << buf << std::endl;
    }
}

inline std::string evpPkeyToPemString(EVP_PKEY* pkey) {
    if (!pkey) {
        std::cerr << "[evpPkeyToPemString] Null EVP_PKEY provided\n";
        return "";
    }

    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        std::cerr << "[evpPkeyToPemString] Failed to allocate BIO\n";
        return "";
    }

    std::string out;

    if (PEM_write_bio_PUBKEY(bio, pkey) == 1) {
        BUF_MEM* bptr = nullptr;
        BIO_get_mem_ptr(bio, &bptr);
        if (bptr && bptr->length > 0) {
            out.assign(bptr->data, bptr->length);
        }
        BIO_free_all(bio);
        return out;
    }

    BIO_free_all(bio);
    ERR_clear_error();

    if (EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA) {
        const RSA* rsa_const = EVP_PKEY_get0_RSA(pkey);
        if (rsa_const) {
            RSA* rsa_pub_dup = RSAPublicKey_dup(rsa_const);
            if (rsa_pub_dup) {
                BIO* bio2 = BIO_new(BIO_s_mem());
                if (bio2) {
                    if (PEM_write_bio_RSAPublicKey(bio2, rsa_pub_dup) == 1) {
                        BUF_MEM* bptr2 = nullptr;
                        BIO_get_mem_ptr(bio2, &bptr2);
                        if (bptr2 && bptr2->length > 0) {
                            out.assign(bptr2->data, bptr2->length);
                        }
                    } else {
                        logOpenSSLErrors("PEM_write_bio_RSAPublicKey");
                    }
                    BIO_free_all(bio2);
                } else {
                    std::cerr << "[evpPkeyToPemString] Failed to allocate BIO2\n";
                }
                RSA_free(rsa_pub_dup);
            } else {
                logOpenSSLErrors("RSAPublicKey_dup");
            }
        }
    }

    if (out.empty()) {
        logOpenSSLErrors("evpPkeyToPemString");
    }

    return out;
}

inline EVP_PKEY* StringToEvpPkey(const std::string& pem_string) {
    if (pem_string.empty()) {
        std::cerr << "[StringToEvpPkey] Empty PEM string\n";
        return nullptr;
    }

    EVP_PKEY* pkey = nullptr;
    BIO* bio = BIO_new_mem_buf(pem_string.data(), (int)pem_string.size());
    if (!bio) {
        std::cerr << "[StringToEvpPkey] Failed to allocate BIO\n";
        return nullptr;
    }

    pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free_all(bio);

    if (pkey) return pkey;

    ERR_clear_error();
    bio = BIO_new_mem_buf(pem_string.data(), (int)pem_string.size());
    if (!bio) {
        std::cerr << "[StringToEvpPkey] Failed to allocate BIO for fallback\n";
        return nullptr;
    }

    RSA* rsa = PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
    BIO_free_all(bio);

    if (!rsa) {
        logOpenSSLErrors("PEM_read_bio_RSAPublicKey");
        return nullptr;
    }

    EVP_PKEY* evp = EVP_PKEY_new();
    if (!evp) {
        std::cerr << "[StringToEvpPkey] Failed to allocate EVP_PKEY\n";
        RSA_free(rsa);
        return nullptr;
    }

    if (EVP_PKEY_assign_RSA(evp, rsa) <= 0) {
        std::cerr << "[StringToEvpPkey] EVP_PKEY_assign_RSA failed\n";
        logOpenSSLErrors("EVP_PKEY_assign_RSA");
        RSA_free(rsa);
        EVP_PKEY_free(evp);
        return nullptr;
    }

    return evp;
}