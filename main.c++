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


    std::string path = "/home/Atharva/Desktop/archlinux-x86_64.iso";
    int chunkSize;
    // cout<<"Enter Path To File :";
    // cin>>path;
    // cout<<endl;
    cout<<"Enter ChunkSize:";
    cin>>chunkSize;
    cout<<endl;
    fragmentEncryptAndSendAFile(path, CLIENT.clientSocket, CLIENTFULL, chunkSize, calculate_file_sha256(path));

    // fragmentEncryptAndSendAFile("/home/Atharva/Desktop/archlinux-x86_64.iso", CLIENT.clientSocket, CLIENTFULL, 10000000, calculate_file_sha256("/home/Atharva/Desktop/archlinux-x86_64.iso"));
    // fragmentEncryptAndSendAFile("LICENSE", CLIENT.clientSocket, CLIENTFULL, 100000, calculate_file_sha256("sample/minecraft.png"));
    // fragmentEncryptAndSendAFile("../../Music/Kabhi Jo Badal Barse (Lyrics)-Arijit Singh [PKgJAV0SuC8].mp3", CLIENT.clientSocket, CLIENTFULL, 100000, calculate_file_sha256("../../Music/Kabhi Jo Badal Barse (Lyrics)-Arijit Singh [PKgJAV0SuC8].mp3"));
    // fragmentEncryptAndSendAFile("/home/Atharva/Videos/archMultitasking.mp4", CLIENT.clientSocket, CLIENTFULL, 100000, calculate_file_sha256("/home/Atharva/Videos/archMultitasking.mp4"));
    // fragmentEncryptAndSendAFile("sample/minecraft.png", CLIENT.clientSocket, CLIENTFULL, 100000, calculate_file_sha256("sample/minecraft.png"));
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



