#include <iostream>
#include "ENorDECRYPTstring.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>

struct ChunkHeader {
    uint64_t ci;
    uint64_t size;
};

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

#ifndef sharedStructsIncluded
#include "sharedStructs.h"
#endif
#ifndef metricsHeaderIncluded
#include "metrics.h"
#endif

namespace fs = std::filesystem;
using namespace std;

#define RECVMOREVAL 2048


struct FileMetadata {
    std::string file_name;
    std::uintmax_t file_size;
    std::size_t chunk_size;
    std::size_t total_chunks;
};

std::string write_metadata_chunk(const FileMetadata& metadata) {
    std::ostringstream oss; // Temporary string stream to build metadata string
    std::string meta;
    oss << "_____metadata_____fossyfiles_____transmissionMetaPacket\n"
        << "STARTMETA|" 
        << metadata.file_name << "|" 
        << metadata.file_size << "|" 
        << metadata.chunk_size << "|" 
        << metadata.total_chunks << "|ENDMETA|\n\0";
    meta = oss.str();

    return meta;
}

void fragmentEncryptAndSendAFile(const std::string& file_path, socket_t receiverSOCKET, ConnectionFinal CLIENT) {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    #endif
 
    std::ifstream in(file_path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Cannot open file " << file_path << " for reading.\n";
        return;
    }

    std::uintmax_t file_size = in.tellg();
    in.seekg(0, std::ios::beg);

    // std::size_t chunk_size = calculateChunkSize(file_size, CLIENT.latencyOfConnection);
    // cin>>chunk_size;
    std::size_t chunk_size = 100000;


    std::size_t total_chunks = (file_size + chunk_size - 1) / chunk_size; // number of chunks calc

    FileMetadata metadata{ fs::path(file_path).filename().string(), file_size, chunk_size, total_chunks };

  
    std::string meta = write_metadata_chunk(metadata);
    send(receiverSOCKET,meta.c_str(), meta.size(),0);

    socket_t sizeSock = socket(AF_INET, SOCK_STREAM, 0);
    if(sizeSock == INVALID_SOCKET) {
        cerr << "Socket creation failed\n";
    }
    sockaddr_in clientChunkSizeSender{};
    clientChunkSizeSender.sin_family = AF_INET;
    clientChunkSizeSender.sin_port = htons(CLIENT.filePort);
    #ifdef _WIN32
        inet_pton(AF_INET, CLIENT.clientIPViaUdp.c_str(), &clientChunkSizeSender.sin_addr);
    #else
        clientChunkSizeSender.sin_addr.s_addr = inet_addr(CLIENT.clientIPViaUdp.c_str());
    #endif

        if(connect(sizeSock, (sockaddr*)&clientChunkSizeSender, sizeof(clientChunkSizeSender)) == SOCKET_ERROR) {
            cerr << "Connection failed\n";
            closesocket(sizeSock);
        }


    
    // Loop to fragment
    for (std::size_t chunk_index = 0; chunk_index < total_chunks; ++chunk_index) {

        
        
        
        std::vector<char> buffer(chunk_size);
        in.read(buffer.data(), chunk_size);
        std::streamsize bytes_read = in.gcount(); 
        buffer.resize(bytes_read);
        

        std::string finalTransmissionChunkString = std::string(buffer.begin(), buffer.end()); // string
        std::string enc = aesEncrypt(CLIENT.sharedSecret,finalTransmissionChunkString);
        

        int size = enc.size();
        int x = send(sizeSock, reinterpret_cast<char*>(&size), sizeof(size), 0);
        cout<<"Chunk : "<<chunk_index<<"--->"<<size<<"| Status --> "<<x<<endl;
        
        
        send(receiverSOCKET,enc.data(), enc.size() ,0); // send data that is encrypted !
        




    }
    std::cout << "File fragmented into " << total_chunks << " chunks sent to  " << receiverSOCKET << "\n";

}



void defragmentDecryptAndReceiveAFile(socket_t socketToReceiveFile,ConnectionFinal peerWhoReceived, int filePort, std::string destPath = "") {
    
    char buffer[256];
    int bytesReceived = recv(socketToReceiveFile, buffer, sizeof(buffer) - 1, 0); // receiving the meta chunk, maybe more due to the stream
    string buff(buffer, bytesReceived);
    string metaForVerification;

    std::string leftoverPayload;
    string info[4];
    bool fileValid = false;
    FileInfo receivedFile;

    // get metadata parsed
    if(bytesReceived > 0){
        std::stringstream ss(buff);
        getline(ss, metaForVerification);
        if(metaForVerification == "_____metadata_____fossyfiles_____transmissionMetaPacket"){ // if its the valid meta chunk from a fellow FOSSyFiles instance
            fileValid = true;
            getline(ss, metaForVerification);
            string segment;
            int count = 0;
            std::stringstream ssMeta(metaForVerification);
            
            while (std::getline(ssMeta, segment, '|')) {
                if(segment == "STARTMETA") continue;
                if(segment == "ENDMETA") break;
                info[count] = segment;
                count++;
            }
            receivedFile.fileName = info[0];
            receivedFile.fileSizeInBytes = std::stol(info[1]);
            receivedFile.chunkSizeInBytes = std::stoi(info[2]);
            receivedFile.totalChunks = std::stol(info[3]);
            std::getline(ss, leftoverPayload, '\0'); // read rest of stream



            // socket to receive chunk size 
                                    #ifdef _WIN32
                                        WSADATA wsaData;
                                        WSAStartup(MAKEWORD(2,2), &wsaData);
                                    #endif

                                        socket_t sizeSockDef = socket(AF_INET, SOCK_STREAM, 0);
                                        if(sizeSockDef == INVALID_SOCKET) { cerr << "Socket creation failed\n";  }

                                        sockaddr_in sizeSocketAddr{};
                                        sizeSocketAddr.sin_family = AF_INET;
                                        sizeSocketAddr.sin_port = htons(filePort);
                                        sizeSocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);

                                        if(bind(sizeSockDef, (sockaddr*)&sizeSocketAddr, sizeof(sizeSocketAddr)) == SOCKET_ERROR) {
                                            cerr << "Bind failed\n"; closesocket(sizeSockDef);
                                        }

                                        listen(sizeSockDef, 1);
                                        cout << "Waiting for incoming TCP connection on port " << filePort << "...\n";

                                        sockaddr_in client_addr{};
                                        socklen_t client_len = sizeof(client_addr);
                                        socket_t sizeSock = accept(sizeSockDef, (sockaddr*)&client_addr, &client_len);
                                        if(sizeSock == INVALID_SOCKET) { cerr << "Accept failed\n"; closesocket(sizeSock); }


                                    


            // starting to receive file

            std::ofstream out(destPath+(receivedFile.fileName), std::ios::binary);
            uint64_t sizes[receivedFile.totalChunks] = {0};
            memset(sizes, 0, sizeof(sizes));
            
            std::vector<unsigned char> fragBuffer;
            fragBuffer.insert(fragBuffer.end(), leftoverPayload.begin(), leftoverPayload.end());
            

            
            size_t chunksReceived = 0;
            while (chunksReceived < receivedFile.totalChunks) {
                if (sizes[chunksReceived] == 0) {
                    int size;
                    recv(sizeSock, reinterpret_cast<char*>(&size), sizeof(size), 0);
                    sizes[chunksReceived] = size;
                }
                if(sizes[chunksReceived] == 0) continue;

                
                if (fragBuffer.size() < sizes[chunksReceived]) {
                    char tmp[RECVMOREVAL];
                    int n = recv(socketToReceiveFile, tmp, sizeof(tmp), 0);
                    if (n <= 0) return;
                    fragBuffer.insert(fragBuffer.end(), tmp, tmp + n);
                    continue;
                }

                if(sizes[chunksReceived] == 0) continue;
                std::string encryptedChunk(fragBuffer.begin(), fragBuffer.begin() + sizes[chunksReceived]);
                
                fragBuffer.erase(fragBuffer.begin(), fragBuffer.begin() + sizes[chunksReceived]);

                cout<<"Chunk : "<<chunksReceived<<"--->"<<sizes[chunksReceived]<<endl;
                
                std::string decrypted = aesDecrypt(peerWhoReceived.sharedSecret, encryptedChunk);
                
                out.write(decrypted.data(), decrypted.size());
                
                
                chunksReceived++;
            }

            out.close();
            std::cout << "[Receiver] File reconstructed successfully\n";





        }
    }



}