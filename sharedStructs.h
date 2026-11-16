// Shared Preprocessor Definitions / Macros --------------------------------------------------------------------------------------


#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H


#include <cstdint>
#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma once
#pragma comment(lib, "Ws2_32.lib")
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <stdexcept>


#define slash "\\"
#else
#include <sys/types.h>
#include <openssl/evp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define slash "/"
#endif


// Shared Inclusions ---------------------------------------------------------------------------------------------------------


#include"platformIndependent.h"
#define sharedStructsIncluded ;


// Shared Structures ---------------------------------------------------------------------------------------------------------


#pragma pack(push, 1)
struct KnockPacket {
    char magic[128];
    uint16_t tcpReturn;
    std::string publicKey;
    uint32_t publicKeyLen;
    uint16_t filePort;
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
    std::string checksum;
    long fileSizeInBytes;
    int chunkSizeInBytes;
    long totalChunks;
};
struct DHKeyPair {
    EVP_PKEY* privateKey;
    EVP_PKEY* publicKey;
};
#endif


struct DiscoveredDeviceInfo {
    std::string ip;
    std::string name;
    uint16_t port;
};

#pragma pack(push, 1)
struct MulticastedInfo {
    std::string ip;
    std::string name;
    uint16_t port;
    char magic[128];
};
#pragma pack(pop)


// Shared Functions ---------------------------------------------------------------------------------------------------------


inline void windowsWSAInitNClean(int mode){
    if (mode == 0){
        #ifdef _WIN32
        // Windows needs WSAStartup/WSACleanup
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
        }
        #endif
        return;
    }
    else if(mode == 1){
        #ifdef _WIN32
        WSACleanup();
        #endif
        
    }
}



// Shared Values/Constants ---------------------------------------------------------------------------------------------------


const std::string MULTICAST_IP = "239.255.0.1";
const int MULTICAST_PORT = 30001;