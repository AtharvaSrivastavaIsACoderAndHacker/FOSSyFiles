#include<iostream>
#include <thread>
#include <atomic>
#include <cstring>
#include "sharedStructs.h"
// #include <mutex>
// #include <condition_variable>

using namespace std;

extern bool connected;
extern Connection peerWhoReceived;
extern atomic<bool> stopFlag;

// mutex udpMutex;
// condition_variable udpCV;
// bool startUdp = false;

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



void udpKnocker(const string& server_ip, int udpPort, int tcpReturnPort, const string& self_ip, EVP_PKEY* publicKey){
    SOCKET udpSock = socket(AF_INET, SOCK_DGRAM, 0);
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
    inet_pton(AF_INET, self_ip.c_str(), &selfAddr.sin_addr);
    selfAddr.sin_port = htons(tcpReturnPort);

    if (bind(udpSock, (sockaddr*)&selfAddr, sizeof(selfAddr)) == SOCKET_ERROR) {
        cerr << "[Initiator] UDP bind failed: " << WSAGetLastError() << "\n";
        return;
    }
    

    KnockPacket pkt{};
    memset(&pkt, 0, sizeof(pkt));
    strncpy(pkt.magic, "_____connectionRequestDatagram_____fossyfiles_____", sizeof(pkt.magic)-1);
    pkt.magic[sizeof(pkt.magic)-1] = '\0'; // ensure null termination
    pkt.tcpReturn = tcpReturnPort;
    pkt.publicKey = publicKey;
    // cout<<pkt.tcpReturn;

    

    // while (!stopFlag) {
        sendto(udpSock, reinterpret_cast<char*>(&pkt), sizeof(pkt), 0,
               (sockaddr*)&serverAddr, sizeof(serverAddr));
        // {
        // unique_lock<mutex> lock(udpMutex);
        // udpCV.wait(lock, [](){ return startUdp; });
        // }
    // }
    
    closesocket(udpSock);
}



void connectTo(const string& server_ip,const string& self_ip, int udpPort, int tcpReturnPort, EVP_PKEY* publicKey){
    thread knockThread(udpKnocker, server_ip, udpPort, tcpReturnPort, self_ip, publicKey);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == INVALID_SOCKET) {
        cerr << "[Initiator] -- TCP socket creation failed\n";
    }

    sockaddr_in listenAddr{};
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = inet_addr(self_ip.c_str());   // accept on any interface
    listenAddr.sin_port = htons(tcpReturnPort);

    if (bind(listenSock, (sockaddr*)&listenAddr, sizeof(listenAddr)) == SOCKET_ERROR) {
        cerr << "[Initiator] -- Bind failed: " << WSAGetLastError() << "\n";
    }

    if (listen(listenSock, 1) == SOCKET_ERROR) {
        cerr << "[Initiator] -- Listen failed: " << WSAGetLastError() << "\n";
    }

    cout << "[Initiator] -- Waiting for TCP connection on port " << tcpReturnPort << "...\n";

    
    sockaddr_in serverAddr;
    int addrlen = sizeof(serverAddr);
    int tcpSock = accept(listenSock, (sockaddr*)&serverAddr, &addrlen);
    if (tcpSock == INVALID_SOCKET) {
        cerr << "[Initiator] -- Return Accept failed: " << WSAGetLastError() << "\n";
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
        std::cerr << "[Receiver] Invalid knock packet received!\n";
    }
    peerWhoReceived.publicKey = pkt.publicKey;
    /////////// done






    cout << inet_ntoa(serverAddr.sin_addr) << ": Has Agreed to Connect To You Via tcp on port : "
         << ntohs(serverAddr.sin_port) << "\n"
         << "Connected !" << "\n";
    
    peerWhoReceived.peerAddr = serverAddr;
    peerWhoReceived.peerSocket = tcpSock;

    stopFlag = true; // stop sending UDP knocks
    knockThread.join();

}