#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;


struct FileMetadata {
    std::string file_name;
    std::uintmax_t file_size;
    std::size_t chunk_size;
    std::size_t total_chunks;
};

void write_metadata_chunk(const std::string& path, const std::vector<char>& data, const FileMetadata& metadata, bool is_start) {
    std::ofstream out(path, std::ios::binary); 
    if (!out) {
        std::cerr << "Cannot open " << path << " for writing.\n";
        return;
    }

    std::ostringstream oss; // Temporary string stream to build metadata string

    if (is_start) {
        oss << "START|" 
            << metadata.file_name << "|" 
            << metadata.file_size << "|" 
            << metadata.chunk_size << "|" 
            << metadata.total_chunks << "|||";
        out.write(oss.str().c_str(), oss.str().size()); 
        out.write(data.data(), data.size());
    } else {
        out.write(data.data(), data.size());
        oss << "|||END|" << metadata.total_chunks;
        out.write(oss.str().c_str(), oss.str().size());
    }

    out.close();
}

void fragment_file(const std::string& file_path, std::size_t chunk_size = 1024, const std::string& output_dir = "fragments") {
    if (!fs::exists(output_dir))
        fs::create_directory(output_dir);

    std::ifstream in(file_path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Cannot open file " << file_path << " for reading.\n";
        return;
    }

    std::uintmax_t file_size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::size_t total_chunks = (file_size + chunk_size - 1) / chunk_size; // number of chunks calc

    FileMetadata metadata{ fs::path(file_path).filename().string(), file_size, chunk_size, total_chunks };

    // Loop to fragment
    for (std::size_t chunk_index = 0; chunk_index < total_chunks; ++chunk_index) {
            std::vector<char> buffer(chunk_size); 
            in.read(buffer.data(), chunk_size);
            std::streamsize bytes_read = in.gcount(); 
            buffer.resize(bytes_read);

            // chunk file name
            std::string chunk_file = output_dir + "/" + metadata.file_name + ".part" + std::to_string(chunk_index + 1);

            if (chunk_index == 0) {
                // first file for metadata
                std::string metaFile = output_dir + "/" + metadata.file_name + "_____metadata_____fossyfiles_____transmissionMetaPacket.part0";
                std::ofstream out(metaFile, std::ios::binary);
                write_metadata_chunk(metaFile, buffer, metadata, true);
            }
            else {
                std::ofstream out(chunk_file, std::ios::binary);
                out.write(buffer.data(), buffer.size());
                out.close();
            }

            std::cout << "File fragmented into " << total_chunks << " chunks in '" << output_dir << "'.\n";
    }

}