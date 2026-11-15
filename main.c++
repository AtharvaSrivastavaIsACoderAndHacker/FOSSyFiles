#include"oneInclude.h"

using namespace std;

 
int main(int argc, char const *argv[]){

    windowsWSAInitNClean(0); // init

    // call broadcaster and receiver
    // call gui


    listenFinal(KEYS.publicKey, 12000);


    std::string path = "sample/minecraft.png";
    fragmentEncryptAndSendAFile(path, CLIENT.clientSocket, CLIENTFULL, calculate_file_sha256(path));    


    windowsWSAInitNClean(0); // clear

    return 0;
}



