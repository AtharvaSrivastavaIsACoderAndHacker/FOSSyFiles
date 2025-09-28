#include<iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

struct ConnectionRequest {
    sockaddr_in clientAddr;
    int clientSocket;
};
struct ServerInfo {
    int serverSocket;
    sockaddr_in serverAddress;
};

extern atomic<bool> stopListening;

// Shared state
extern mutex mtx;
extern condition_variable cv;
extern bool newRequest;
extern bool decisionReady;
extern bool acceptConnection;
extern bool connected;
extern bool asking;
extern ConnectionRequest pendingRequest;
extern ConnectionRequest client;
extern ServerInfo server;

inline void listenAndAccept(string server_ip, int port){

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
    }

    int sockfd, newsock;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip.c_str(), &serverAddress.sin_addr) <= 0) {
        cerr << "Invalid server IP address: " << server_ip << "\n";
    }

    int bindingReturn = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bindingReturn == SOCKET_ERROR) perror("error in binding !");
    int eCodeOnListeningForConnection = listen(serverSocket, 1);
    if(eCodeOnListeningForConnection!=0) perror("error in listening !");
    cout<<bindingReturn<<" and "<<eCodeOnListeningForConnection<<endl;
    
    while (!stopListening.load()) {
        fd_set readfds;
        FD_ZERO(&readfds);                  
        FD_SET(serverSocket, &readfds);         // sets up incoming connections buffer
        timeval timeout{1,0}; 

        int activity = select(0, &readfds, nullptr, nullptr, &timeout);
        if (activity > 0 && FD_ISSET(serverSocket, &readfds)) { // 1 connection request pending
            cout<<"log from listener header, 1 request came from a client !"<<endl;
            sockaddr_in clientAddr{};
            int addrlen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrlen);
            // cout<<"from header listener --> clientAddr : "<<inet_ntoa(clientAddr.sin_addr)<<endl;
            client.clientAddr = clientAddr;
            client.clientSocket = clientSocket;
            connected = true;
            if (clientSocket != INVALID_SOCKET) {
                cout<<"valid socket --> clientSocket : "<<clientSocket<<endl;
                // ask main thread to respond to this
                {
                    unique_lock<mutex> lock(mtx);
                    pendingRequest.clientAddr = clientAddr;
                    pendingRequest.clientSocket = clientSocket;
                    newRequest = true;
                    asking = true;
                    cout<<"locked by listener"<<endl;
                    cout.flush();
                }
                cv.notify_one(); // notify main thread
                cout<<acceptConnection<<endl;
                // Wait for main thread's decision
                {   
                    unique_lock<mutex> lock(mtx);
                    cv.wait(lock, [](){ return decisionReady; });
                    if (acceptConnection) {
                        cout << "Connection accepted from " 
                             << inet_ntoa(clientAddr.sin_addr) << ":" 
                             << ntohs(clientAddr.sin_port) << "\n";
                             cout.flush();
                             connected = true;
                             client.clientAddr = clientAddr;
                             client.clientSocket = clientSocket;
                             acceptConnection = false;
                             newRequest = false;
                             asking = false;
                    } else {
                        cout << "Connection rejected.\n";
                        closesocket(clientSocket);
                        cout.flush();
                        connected = false;
                        acceptConnection = false;
                        newRequest = false;
                        asking = false;
                    }
                }
            }
        }
    server.serverAddress = serverAddress;
    server.serverSocket = serverSocket;
}
WSACleanup();
}