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


    string ip = "127.0.0.1";
    int port = 12000;

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