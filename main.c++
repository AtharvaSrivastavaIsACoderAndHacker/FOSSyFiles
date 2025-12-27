#include"fragmentEncryptSend.h"
#include "connectionInitiator.h"
#include "listen.h"
#include "RSA_keygen.h"

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


    

    listenFinal(KEYS.publicKey);

    cout<<"out in main now"<<endl;
    if (connected){
        fragmentEncryptAndSendAFile("README.md", CLIENT.clientSocket, CLIENT.publicKey);
    }
    



    // a very simple chat sender
    if(connected){
        while(1){
            std::string buffer;
            getline(cin, buffer);
            int bytesSent = send(CLIENT.clientSocket, buffer.c_str(), sizeof(buffer) - 1, 0);
            if(bytesSent > 0){
                cout << "Sent to client: " << buffer << "\n";
                cout.flush();
            }
        }
    }
    
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    
    
    return 0;
}



