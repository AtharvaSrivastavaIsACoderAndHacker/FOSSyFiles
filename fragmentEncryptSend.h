#include <iostream>
#include "ENorDECRYPTstring.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>
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

namespace fs = std::filesystem;
using namespace std;


struct FileMetadata {
    std::string file_name;
    std::uintmax_t file_size;
    std::size_t chunk_size;
    std::size_t total_chunks;
};

std::string write_metadata_chunk(const FileMetadata& metadata, bool is_start) {
    // std::ofstream out(path, std::ios::binary); 
    // if (!out) {
    //     std::cerr << "Cannot open " << path << " for writing.\n";
    //     return;
    // }

    std::ostringstream oss; // Temporary string stream to build metadata string
    std::string meta;
    if (is_start) {
        oss << "_____metadata_____fossyfiles_____transmissionMetaPacket\n"
            << "START|" 
            << metadata.file_name << "|" 
            << metadata.file_size << "|" 
            << metadata.chunk_size << "|" 
            << metadata.total_chunks << "|||";
        meta = oss.str();

    } else {
        oss << "|||END|" << metadata.total_chunks;
        meta = oss.str();

    }

    return meta;
}

void fragmentEncryptAndSendAFile(const std::string& file_path, SOCKET receiverSOCKET, EVP_PKEY* key, std::size_t chunk_size = 214) {

    std::ifstream in(file_path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Cannot open file " << file_path << " for reading.\n";
        return;
    }

    std::uintmax_t file_size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::size_t total_chunks = (file_size + chunk_size - 1) / chunk_size; // number of chunks calc

    FileMetadata metadata{ fs::path(file_path).filename().string(), file_size, chunk_size, total_chunks };


    std::string meta = write_metadata_chunk(metadata ,true);
    // std::string meta = rsaCrypt(key,write_metadata_chunk(metadata ,true), 0);
    // cout<<rsaCrypt(key,meta, 0)<<endl;
    // send(receiverSOCKET,meta.c_str(), meta.size(),0);

    
    // Loop to fragment
    for (std::size_t chunk_index = 0; chunk_index < total_chunks; ++chunk_index) {
            std::vector<char> buffer(chunk_size);
            in.read(buffer.data(), chunk_size);
            std::streamsize bytes_read = in.gcount(); 
            buffer.resize(bytes_read);

            std::string finalTransmissionChunkString = std::string(buffer.begin(), buffer.end());
            // std::string enc = rsaCrypt(key,finalTransmissionChunkString, 0);

            
            // CURRENTLY THIS DOESNT ENCRYT AND IS SHITTY IN TERMS OF EXACT FILE RECONSTRUCTION OVER THERE !


            // cout<<finalTransmissionChunkString; // log
            // cout<<enc<<endl; // log


            send(receiverSOCKET,finalTransmissionChunkString.c_str(), finalTransmissionChunkString.size() ,0);



    }
    std::cout << "File fragmented into " << total_chunks << " chunks sent to  " << receiverSOCKET << "\n";

}