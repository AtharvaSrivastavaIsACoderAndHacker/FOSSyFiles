#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include"platformIndependent.h"
#define sharedStructsIncluded ;

#include <cstdint>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma once
    #pragma comment(lib, "Ws2_32.lib")
    #include <openssl/evp.h>
    #include <openssl/rsa.h>
    #include <openssl/err.h>
    #include <stdexcept>
#else
    #include <sys/types.h>
    #include <openssl/evp.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#pragma pack(push, 1)
struct KnockPacket {
    char magic[128];
    int tcpReturn;
    std::string publicKey;
    uint32_t publicKeyLen;
    int filePort;
    int latencyOfConnection;
};
#pragma pack(pop)

struct Connection {
    sockaddr_in peerAddr;
    int peerSocket;
    EVP_PKEY* publicKey;
    std::string sharedSecret;
};
struct ConnectionFinal {
    sockaddr_in peerAddr;
    int peerSocket;
    EVP_PKEY* publicKey;
    int filePort;
    int tcpReturn;
    std::string clientIPViaUdp;
    int clientPortViaUdp;
    std::string sharedSecret;
    int latencyOfConnection;
};

struct FileInfo {
    std::string fileName;
    long fileSizeInBytes;
    int chunkSizeInBytes;
    long totalChunks;
};
struct DHKeyPair {
    EVP_PKEY* privateKey;
    EVP_PKEY* publicKey;
};
#endif
