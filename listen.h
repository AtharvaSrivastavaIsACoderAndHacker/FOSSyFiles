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


void listenFinal(EVP_PKEY* publicKey){

    //// start
    int port = 12000;

    std::thread listenerThread(listenAndAccept, port);
    
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


            

            // SEND KEY
            KnockPacket keypayload{};
            memset(&keypayload, 0, sizeof(keypayload));
            strncpy(keypayload.magic, "_____connectionRequestDatagram_____fossyfiles_____", sizeof(keypayload.magic)-1);
            keypayload.magic[sizeof(keypayload.magic)-1] = '\0';
            std::string serialized = serializePublicKeyToString(publicKey);
            keypayload.publicKeyLen = htonl(serialized.size());

            sockaddr_in clientAddrForKeyShare{};
            clientAddrForKeyShare.sin_family = AF_INET;
            inet_pton(AF_INET, clientIPViaUdp.c_str(), &clientAddrForKeyShare.sin_addr);
            clientAddrForKeyShare.sin_port = htons(CLIENT.clientAddr.sin_port);

            int bytesSent = send(CLIENT.clientSocket, reinterpret_cast<char*>(&keypayload), sizeof(keypayload), 0);
            int bytesKeySent = sendto(CLIENT.clientSocket, serialized.data(), serialized.size(), 0,(sockaddr*)&CLIENT.clientAddr, sizeof(CLIENT.clientAddr));


            

            
            {
                lock_guard<mutex> lock(mtx);
                stopListening = true;
                decisionReady = true;  // sote hue kumbakaran ki 4th generation ko jagane ke liye
            }
            cv.notify_all();
            listenerThread.join();
            break;




        }
        
    }


    
}