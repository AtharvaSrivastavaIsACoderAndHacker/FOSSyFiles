#include<iostream>
#include <atomic>
#include "connectionInitiator.h"
#include "RSA_keygen.h"

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

struct Connection peerWhoReceived;
atomic<bool> stopFlag(false);


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

    string server_ip;
    cin>>server_ip;
    int port = 12000;


    connectTo(server_ip, "127.0.0.1", port, 12001, KEYS.publicKey);
    cout<<"Server Info --> "<<inet_ntoa(peerWhoReceived.peerAddr.sin_addr)<<":"<<ntohs(peerWhoReceived.peerAddr.sin_port)<<endl;



    

    // a very simple chat receiver
    cout<<"Receiving..."<<endl;
    while (1){
        char buffer[256];
                int bytesReceived = recv(peerWhoReceived.peerSocket, buffer, sizeof(buffer) - 1, 0);
                if(bytesReceived > 0){
                    cout << "Received from server: " << buffer << "\n";
                    cout.flush();
        }
    }







    
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    
    return 0;
}


