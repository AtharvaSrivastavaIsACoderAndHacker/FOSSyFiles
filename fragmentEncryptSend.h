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

extern FileInfo receivedFile;


struct FileMetadata {
    std::string file_name;
    std::string file_checksum;
    std::uintmax_t file_size;
    std::size_t chunk_size;
    std::size_t total_chunks;
};

std::string write_metadata_chunk(const FileMetadata& metadata) {
    std::ostringstream oss;
    std::string meta;
    oss << "_____metadata_____fossyfiles_____transmissionMetaPacket|"
        << "STARTMETA|" 
        << metadata.file_name << "|" 
        << metadata.file_size << "|" 
        << metadata.chunk_size << "|" 
        << metadata.file_checksum << "|" 
        << metadata.total_chunks << "|ENDMETA|\n\0";
    meta = oss.str().substr(0, 1024);

    return meta;
}

int recvAll(socket_t sock, void *buf, size_t len){
    char *p = static_cast<char *>(buf);
    size_t total = 0;
    while (total < len) {
        int n = recv(sock, p + total, len - total, 0);
        if (n <= 0){
            return false;
        }
        total += n;
    }

    return total;
}

bool sendAll(socket_t sock, const void* data, size_t len){
    const char* p = static_cast<const char*>(data);
    while (len > 0){
        int n = send(sock, p, len, 0);
        if (n <= 0){
            return false;
        }
        p += n;
        len -= n;
    }

    return true;
}

void fragmentEncryptAndSendAFile(const std::string& file_path, socket_t receiverSOCKET, ConnectionFinal CLIENT, std::string fileSHA256Checksum) {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    #endif
 
    std::ifstream in(file_path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "[fragmentEncryptSend.h] Cannot open file " << file_path << " for reading.\n";
        return;
    }

    std::uintmax_t file_size = in.tellg();
    in.seekg(0, std::ios::beg);

    size_t chunk_size = calculateChunkSize(file_size,CLIENT.latencyOfConnection);
    cout<<"CHUNK SIZE --> "<<chunk_size<<endl;
    cout.flush();

    std::size_t total_chunks = (file_size + chunk_size - 1) / chunk_size; // number of chunks calc

    FileMetadata metadata{ fs::path(file_path).filename().string(),fileSHA256Checksum, file_size, chunk_size, total_chunks };

   
    std::string meta = write_metadata_chunk(metadata);
    sendAll(receiverSOCKET,meta.c_str(), 1024);
    std::cout<<"Metadata --> "<<meta<<endl;
    
    // Loop to fragment
    for (std::size_t chunk_index = 0; chunk_index < total_chunks; ++chunk_index) {

        std::vector<char> buffer(chunk_size);
        in.read(buffer.data(), chunk_size);
        std::streamsize bytes_read = in.gcount(); 
        buffer.resize(bytes_read);
        

        std::string finalTransmissionChunkString = std::string(buffer.begin(), buffer.end()); // string
        std::string enc = aesEncrypt(CLIENT.sharedSecret,finalTransmissionChunkString);
        

        int size = enc.size();
        int x = sendAll(receiverSOCKET, reinterpret_cast<char*>(&size), sizeof(size));
        cout<<"Chunk : "<<chunk_index<<"--->"<<size<<"| Status --> "<<x<<endl;
        
        sendAll(receiverSOCKET,enc.data(), enc.size()); // send data that is encrypted !

    }
    std::cout << "File fragmented into " << total_chunks << " chunks sent to  " << receiverSOCKET << "\n";

}



bool defragmentDecryptAndReceiveAFile(socket_t socketToReceiveFile,ConnectionFinal peerWhoReceived, int filePort, std::string destPath = "") {
    
    char buffer[1024];

    int bytesReceived = recvAll(socketToReceiveFile,&buffer, 1024);
    std::cout<<"Metadata --> "<<buffer<<endl;

    // int bytesReceived = recvAll(socketToReceiveFile, buffer, sizeof(buffer) - 1); // receiving the meta chunk, maybe more due to the stream ----->    why the f*** did i do that ? that got me bitching the code for an hour
    string buff(buffer, bytesReceived);
    string metaForVerification;

    string info[5];
    bool fileValid = false;
    

    // get metadata parsed
    if(bytesReceived > 0){
        std::stringstream ss(buff);
        getline(ss, metaForVerification, '|');
        if(metaForVerification == "_____metadata_____fossyfiles_____transmissionMetaPacket"){ // if its the valid meta chunk from a fellow FOSSyFiles instance
            fileValid = true;
            string segment;
            int count = 0;
            
            while (std::getline(ss, segment, '|')) {
                if(segment == "STARTMETA") continue;
                if(segment == "ENDMETA") break;
                info[count] = segment;
                count++;
            }

            receivedFile.fileName = info[0];
            receivedFile.fileSizeInBytes = std::stol(info[1]);
            receivedFile.chunkSizeInBytes = std::stoi(info[2]);
            receivedFile.checksum = info[3];
            receivedFile.totalChunks = std::stol(info[4]);
            

            // starting to receive file
            std::string filePath = destPath+(receivedFile.fileName);
            std::ofstream out(filePath, std::ios::binary);
            uint64_t sizes[receivedFile.totalChunks] = {0};
            memset(sizes, 0, sizeof(sizes));
            
            std::vector<unsigned char> fragBuffer;
            
            size_t chunksReceived = 0;
            while (chunksReceived < receivedFile.totalChunks) {
                if (sizes[chunksReceived] == 0) {
                    int size;
                    recvAll(socketToReceiveFile, reinterpret_cast<char*>(&size), sizeof(size));
                    sizes[chunksReceived] = size;
                }
                if(sizes[chunksReceived] == 0) continue;

                
                if (fragBuffer.size() < sizes[chunksReceived]){
                    // char tmp[RECVMOREVAL]; // this shit is hilarious,cosmic level stupidity ! i mean i sent all sizes and shit but have always used a static size since ?????
                    // char tmp[sizes[chunksReceived]];
                    char *tmp = (char *)malloc(sizes[chunksReceived] * sizeof(char));
                    int n = recvAll(socketToReceiveFile, tmp, sizes[chunksReceived] * sizeof(char));
                    if (n <= 0) return false;
                    fragBuffer.insert(fragBuffer.end(), tmp, tmp + n);
                    continue;
                    free(tmp);
                }

                if(sizes[chunksReceived] == 0) continue;
                std::string encryptedChunk(fragBuffer.begin(), fragBuffer.begin() + sizes[chunksReceived]);
                
                fragBuffer.erase(fragBuffer.begin(), fragBuffer.begin() + sizes[chunksReceived]);

                cout<<"[fragmentEncryptSend.h] Chunk : "<<chunksReceived<<"--->"<<sizes[chunksReceived]<<endl;
                
                std::string decrypted = aesDecrypt(peerWhoReceived.sharedSecret, encryptedChunk);
                
                out.write(decrypted.data(), decrypted.size());
                
                
                chunksReceived++;
            }

            out.close();
            std::cout << "[Receiver] File reconstructed successfully\n";

            
            if(calculate_file_sha256(filePath) == receivedFile.checksum){
                cout<<"Checksums Match ! Transmission Pure and Successful !"<<endl;
                return true;
            }
            else{
                cout<<"We're doomed ! Hey Bhagwan another damn bug ! Laao kala hit !!"<<endl;
                return false;

            }
        }
    }
    return false;
}