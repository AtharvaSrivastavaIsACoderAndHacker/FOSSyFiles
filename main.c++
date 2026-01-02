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


    




    listenFinal(KEYS.publicKey);


    
    
    fragmentEncryptAndSendAFile("sample\\minecraft.png", CLIENT.clientSocket, CLIENTFULL);
    // fragmentEncryptAndSendAFile("sample\\montagem_tomada.mp3", CLIENT.clientSocket, CLIENTFULL);
    // fragmentEncryptAndSendAFile("sample\\15th August 2025.mp4", CLIENT.clientSocket, CLIENTFULL);

    











    
    
    
    
    // // a very simple chat sender
    // if(connected){ 
        //     while(1){
            //         std::string buffer;
            //         getline(cin, buffer);
    //         buffer = aesEncrypt(CLIENTFULL.sharedSecret, buffer);
    //         int bytesSent = send(CLIENT.clientSocket, buffer.data(), buffer.size() - 1, 0);
    //         if(bytesSent > 0){
    //             cout << "Sent to client: " << buffer << "\n";
    //             cout.flush();
    //         }
    //     }
    // }
    



    // int filesize;
    // cin>>filesize;
    // uint64_t freeRAM = getAvailableRAM();   // bytes
    // double cpuUsage = getCPUUsage();        // 0.0 - 100.0
    // double ramGB = freeRAM / 1024.0 / 1024.0 / 1024.0;

    // std::cout << "=== System & Network Metrics ===\n";
    // std::cout << "CPU Usage: " << cpuUsage << " %\n";
    // std::cout << "Free RAM: " << freeRAM << " bytes (" << ramGB << " GB)\n";
    // std::cout << "Measured RTT: " << CLIENTFULL.latencyOfConnection << " ms\n";

    // size_t suggestedChunk = calculateChunkSize(filesize, CLIENTFULL.latencyOfConnection); // example: 1GB file
    // std::cout << "Suggested chunk size for this RTT & system load: " << suggestedChunk << " bytes\n";
    // std::cout << "===============================\n";
    


    #ifdef _WIN32
    WSACleanup();
    #endif
    
    
    
    return 0;
}



