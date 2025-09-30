// #include<iostream>
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #include "listenForConnections.h"
// #include <thread>
// #include <conio.h>
#include"fragmentEncryptSend.h"
#include "connectionInitiator.h"
#include "listen.h"
#include "RSA_keygen.h"


// atomic<bool> stopListening(false);
// mutex mtx;
// condition_variable cv;
// bool newRequest = false;
// bool acceptConnection = false;
// bool decisionReady = false;
// ConnectionRequest pendingRequest;
// ServerInfo server{};
// bool connected = false;
// bool exitNow = false;
// bool asking = true;
// bool listenMode = true;
// string clientIPViaUdp;
// int clientPortViaUdp;
// atomic<bool> stopFlag(false);

// // For when we have to initiate a Connection to a peer, and not listen
// Connection peerWhoReceived;

using namespace std;

int main(int argc, char const *argv[]){
    
    
    
    RSAKeyPair KEYS = generateRSAKeyPair();



    #ifdef _WIN32
    // Windows needs WSAStartup/WSACleanup
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
    #endif


    string ip = "127.0.0.1";
    int port = 12000;

    // std::thread listenerThread(listenAndAccept, ip, port);
    // while (!exitNow && listenMode){

    //     bool requestPending = false;
    //     ConnectionRequest requestCopy;
    //     {
        
    //     if (newRequest) {
    //         requestPending = true;
    //         requestCopy = pendingRequest; // copy the data safely
    //     }
    //     }

    //     if(asking && newRequest){
    //         cout<<asking<<"new connection request from : "<<clientIPViaUdp<<endl;
    //         cout.flush();
    //         // acceptConnection = false;
    //         {
    //             lock_guard<mutex> lock(mtx);
    //             cin>>acceptConnection;
    //             decisionReady = true;
    //             newRequest = false;
    //             asking = false;
    //         }
    //         cv.notify_one();
    //         cout<<"asked main : "<<inet_ntoa(pendingRequest.clientAddr.sin_addr)<<endl;
    //     }
    //     if(connected){
    //         // cout<<"Connected to : "<<inet_ntoa(client.clientAddr.sin_addr)<<endl;
    //         char buffer[1024];
    //         int bytesReceived = recv(CLIENT.clientSocket, buffer, sizeof(buffer) - 1, 0);
    //         if(bytesReceived > 0){
    //             buffer[bytesReceived] = '\0';
    //             cout << "Received from client: " << buffer << "\n";
    //             cout.flush();
    //         }
    //     }
        
    // }
   

    // std::thread listenFinalThread(listenFinal);

    listenFinal(KEYS.publicKey);

    // if (connected){
        // fragmentEncryptAndSendAFile("README.md", CLIENT.clientSocket, KEYS.privateKey);
    // }
    


    #ifdef _WIN32
    WSACleanup();
    #endif


    
    return 0;
}