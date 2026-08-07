#include"oneInclude.h"

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

    listenFinal(KEYS.publicKey, 12000);


    std::string path = "sample/minecraft.png";
    fragmentEncryptAndSendAFile(path, CLIENT.clientSocket, CLIENTFULL, calculate_file_sha256(path));    


    #ifdef _WIN32
    WSACleanup();
    #endif

    return 0;
}



