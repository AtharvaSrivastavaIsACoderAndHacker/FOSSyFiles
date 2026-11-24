// Shared Preprocessor Definitions / Macros --------------------------------------------------------------------------------------


#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H


#include <cstdint>
#include <iostream>
#include <ctime>
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
    char publicKey[256];
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
    char name[64];
    char ip[46]; 
    uint16_t port;
    char magic[128];
};
#pragma pack(pop)
static_assert(sizeof(MulticastedInfo) == 46 + 128 + 2 + 64, "Struct size mismatch!");


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

inline std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    
    char buffer[80];
    // Format: 2026-11-24 08:24:00
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    
    return std::string(buffer);
}



// Shared Values/Constants ---------------------------------------------------------------------------------------------------


const std::string MULTICAST_IP = "239.255.0.1";
const int MULTICAST_PORT = 30001;