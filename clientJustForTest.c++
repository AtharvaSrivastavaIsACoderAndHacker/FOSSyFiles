#include"oneInclude.h"
extern atomic<bool> stopFlag;


using namespace std;


int main(int argc, char const *argv[]){


    
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


    
    connectTo(server_ip, port, 12001, KEYS.publicKey, 12002);
    cout<<"Server Info --> "<<inet_ntoa(peerWhoReceived.peerAddr.sin_addr)<<":"<<ntohs(peerWhoReceived.peerAddr.sin_port)<<endl;



    defragmentDecryptAndReceiveAFile(peerWhoReceived.peerSocket,peerWhoReceived,12002, "Received\\");





 
    // // a very simple chat receiver
    // cout<<"Receiving..."<<endl;
    // while (1){ 
    //     char buffer[256];
    //     int bytesReceived = recv(peerWhoReceived.peerSocket, buffer, sizeof(buffer) - 1, 0);
    //     if(bytesReceived > 0){
    //         cout << "Received from server: " << buffer << "\n";
    //         cout.flush();
    //     }
    // }




    
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    
    return 0;
}


