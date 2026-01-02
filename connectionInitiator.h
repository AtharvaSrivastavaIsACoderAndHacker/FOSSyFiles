#include<iostream>
#include <thread>
#include <atomic>
#include <cstring>


#ifndef sharedStructsIncluded
#include "sharedStructs.h"
#endif

#ifndef ENDEincluded
#include "fragmentEncryptSend.h" // quick fix sry, ik this aint tech debt , maybe xd
#endif

#ifndef keyGenHeaderIncluded
#include "RSA_keygen.h"
#endif

using namespace std;

extern bool connected;
ConnectionFinal peerWhoReceived;
extern atomic<bool> stopFlag;


#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

extern DHKeyPair KEYS;



void udpKnocker(const string& server_ip, int udpPort, int tcpReturnPort, EVP_PKEY* publicKey, int filePort){
    socket_t udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock == INVALID_SOCKET) {
        cerr << "UDP socket creation failed\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(udpPort);
    
    sockaddr_in selfAddr{};
    selfAddr.sin_family = AF_INET;
    selfAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    selfAddr.sin_port = htons(tcpReturnPort);


    KnockPacket pkt{};
    memset(&pkt, 0, sizeof(pkt));
    strncpy(pkt.magic, "_____connectionRequestDatagram_____fossyfiles_____", sizeof(pkt.magic)-1);
    pkt.magic[sizeof(pkt.magic)-1] = '\0'; // ensure null termination
    pkt.tcpReturn = tcpReturnPort;
    pkt.filePort = filePort;
    std::string publicKeyTem = serializePublicKeyToString(publicKey);
    pkt.publicKeyLen = htonl(publicKeyTem.size());

        sendto(udpSock, reinterpret_cast<char*>(&pkt), sizeof(pkt), 0,
                (sockaddr*)&serverAddr, sizeof(serverAddr));
        sendto(udpSock, publicKeyTem.data(), publicKeyTem.size(), 0,
                (sockaddr*)&serverAddr, sizeof(serverAddr));
                
    closesocket(udpSock);

    peerWhoReceived.filePort = filePort;
}



void connectTo(const string& server_ip, int udpPort, int tcpReturnPort, EVP_PKEY* publicKey, int filePort){
    thread knockThread(udpKnocker, server_ip, udpPort, tcpReturnPort, publicKey, filePort);

    socket_t listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == INVALID_SOCKET) {
        cerr << "[Initiator] -- TCP socket creation failed\n";
    }

    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddr.sin_port = htons(tcpReturnPort);

    if (bind(listenSock, (sockaddr*)&listenAddr, sizeof(listenAddr)) == SOCKET_ERROR) {
        #ifdef _WIN32
        cerr << "[Initiator] -- Bind failed: " << WSAGetLastError() << "\n";
        #else
        perror("[Initiator] -- Bind failed -- linux ");
        #endif

    }

    if (listen(listenSock, 1) == SOCKET_ERROR) {
        #ifdef _WIN32
        cerr << "[Initiator] -- Listen failed: " << WSAGetLastError() << "\n";
        #else
        perror("[Initiator] -- Listen failed -- linux ");
        #endif
    }

    cout << "[Initiator] -- Waiting for TCP connection on port " << tcpReturnPort << "...\n";

    
    sockaddr_in serverAddr;
    socklen_t addrlen = sizeof(serverAddr);
    int tcpSock = accept(listenSock, (sockaddr*)&serverAddr, &addrlen);
    if (tcpSock == INVALID_SOCKET) {
        #ifdef _WIN32
        cerr << "[Initiator] -- Return Accept failed: " << WSAGetLastError() << "\n";
        #else
        perror("[Initiator] -- Return Accept failed -- linux ");
        #endif
    }







    //////// RECEIVE KEY
    KnockPacket pkt{};
    size_t totalReceived = 0;
    char *ptr = reinterpret_cast<char*>(&pkt);

    while (totalReceived < sizeof(KnockPacket)) {
        int n = recv(tcpSock, ptr + totalReceived, sizeof(KnockPacket) - totalReceived, 0);
        if (n <= 0) {
            #ifdef _WIN32
            std::cerr << "TCP recv failed: " << WSAGetLastError() << "\n";
            #else
            perror("TCP recv failed");
            #endif
        }
        totalReceived += n;
    }

    if (strncmp(pkt.magic, "_____connectionRequestDatagram_____fossyfiles_____", sizeof(pkt.magic)) != 0) {
        std::cerr << "[Initiator] Invalid knock packet received!\n";
    }

    

    uint32_t keyLen = ntohl(pkt.publicKeyLen);
    std::string key(keyLen, '\0');
    recvfrom(tcpSock, key.data(), keyLen, 0,(struct sockaddr*)&serverAddr, &addrlen);
    peerWhoReceived.publicKey = deserializePublicKeyFromString(key);
    std::string sharedSecret = deriveSharedSecret(KEYS.privateKey, peerWhoReceived.publicKey);
    std::string sharedSecretFinal = deriveAESKey256(sharedSecret);
    peerWhoReceived.sharedSecret = sharedSecretFinal;
    peerWhoReceived.latencyOfConnection = pkt.latencyOfConnection;

    

    /////////// done






    cout << inet_ntoa(serverAddr.sin_addr) << ": Has Agreed to Connect To You Via tcp on port : "
         << ntohs(serverAddr.sin_port) << "\n"
         << "Connected !" << "\n";
    
    peerWhoReceived.peerAddr = serverAddr;
    peerWhoReceived.peerSocket = tcpSock;
    peerWhoReceived.clientIPViaUdp = inet_ntoa(serverAddr.sin_addr);
    peerWhoReceived.clientPortViaUdp = ntohs(serverAddr.sin_port);

    stopFlag = true; // stop sending UDP knocks
    knockThread.join();

}