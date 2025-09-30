#include<iostream>
#include "listenForConnections.h"
#include <thread>
#include <conio.h>

atomic<bool> stopListening(false);
mutex mtx;
condition_variable cv;
bool newRequest = false;
bool acceptConnection = false;
bool decisionReady = false;
ConnectionRequest pendingRequest;
ServerInfo server{};
bool connected = false;
bool exitNow = false;
bool asking = true;
bool stopTHIS = false;
bool listenMode = true;
string clientIPViaUdp;
int clientPortViaUdp;
atomic<bool> stopFlag(false);

// For when we have to initiate a Connection to a peer, and not listen
Connection peerWhoReceived;

using namespace std;


void listenFinal(){

    //// start
    string ip = "127.0.0.1";
    int port = 12000;

    std::thread listenerThread(listenAndAccept, ip, port);
    
    while (!exitNow && listenMode){

        bool requestPending = false;
        ConnectionRequest requestCopy;
        {
        
        if (newRequest) {
            requestPending = true;
            requestCopy = pendingRequest; // copy the data safely
        }
        }

        if(asking && newRequest){
            cout<<asking<<"new connection request from : "<<clientIPViaUdp<<endl;
            cout.flush();
            // acceptConnection = false;
            {
                lock_guard<mutex> lock(mtx);
                cin>>acceptConnection;
                decisionReady = true;
                newRequest = false;
                asking = false;
            }
            cv.notify_one();
            cout<<"asked main : "<<inet_ntoa(pendingRequest.clientAddr.sin_addr)<<endl;
        }
        if(connected){
            cout<<"[listen.h] Connected to : "<<inet_ntoa(CLIENT.clientAddr.sin_addr)<<endl;
            {
                lock_guard<mutex> lock(mtx);
                stopListening = true;
                decisionReady = true;  // sote hue kumbakaran ki 4th generation ko jagane ke liye
            }
            cv.notify_all();
            cout<<1<<endl;
            listenerThread.join();
            cout<<2<<endl;
            break;
            // string buffer;
            // getline(cin, buffer);
            // int bytesSent = send(CLIENT.clientSocket, buffer.c_str(), sizeof(buffer) - 1, 0);
            // if(bytesSent > 0){
            //     cout << "Sent to client: " << buffer << "\n";
            //     cout.flush();
            // }
        }
        
    }


    
}