#include<iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include"fragmentation.h"
#include"sendOrReceiveToASocket.h"
#include "listenForConnections.h"
#include <thread>

atomic<bool> stopListening(false);
mutex mtx;
condition_variable cv;
bool newRequest = false;
bool acceptConnection = false;
bool decisionReady = false;
ConnectionRequest pendingRequest;
ConnectionRequest client{};
ServerInfo server{};
bool connected = false;
bool exitNow = false;
bool asking = true;

using namespace std;

int main(int argc, char const *argv[]){

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }

    string ip = "127.0.0.1";
    int port = 12000;

    std::thread listenerThread(listenAndAccept, ip, port);

    while (!exitNow){

        bool requestPending = false;
        ConnectionRequest requestCopy;
        {
        lock_guard<mutex> lock(mtx);
        if (newRequest) {
            requestPending = true;
            requestCopy = pendingRequest; // copy the data safely
        }
        }

        if(asking){
            cout<<asking<<"new connection request from : "<<inet_ntoa(client.clientAddr.sin_addr)<<endl;
            cout.flush();
            // acceptConnection = false;
            {
                cin>>acceptConnection;
                decisionReady = true;
                lock_guard<mutex> lock(mtx);
                newRequest = false;
                asking = false;
            }
            cv.notify_one();
            cout<<"asked main : "<<inet_ntoa(pendingRequest.clientAddr.sin_addr)<<endl;
        }
        if(connected){
            cout<<"Connected to : "<<inet_ntoa(client.clientAddr.sin_addr)<<endl;
            char buffer[1024];
            int bytesReceived = recv(client.clientSocket, buffer, sizeof(buffer) - 1, 0);
            if(bytesReceived > 0){
                buffer[bytesReceived] = '\0';
                cout << "Received from client: " << buffer << "\n";
                cout.flush();
            }
        }
        
    }
    
    WSACleanup();

    
    return 0;
}