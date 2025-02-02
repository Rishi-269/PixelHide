#include <fstream>
#include "file.hpp"

void File::save(){
    std::ofstream fout(filepath_, std::ios::binary);
    if(!fout)
        throw std::runtime_error("Failed to create file: " + filepath_.filename().string());

    fout.write(reinterpret_cast<char*>(data_), size_);
    fout.close();
}

//constructors and destructor
File::File(char *filepath){
    filepath_ = std::filesystem::absolute(filepath);
    if(!std::filesystem::exists(filepath_))
        throw std::runtime_error("File does not exist: \"" + filepath_.string() + '\"');

    std::ifstream fin(filepath, std::ios::binary);
    if (!fin)
        throw std::runtime_error("Could not open file: \"" + filepath_.string() + '\"');

    size_ = std::filesystem::file_size(filepath_);
    if (size_ <= 0)
        throw std::runtime_error("Please enter a valid file.\nFile is empty: \"" + filepath_.string() + '\"');
    
    data_ = new uint8_t[size_];

    fin.read(reinterpret_cast<char*>(data_), size_);

    fin.close();
}

File::File(std::string filename, uint64_t size) : size_(size){
    if (size_ <= 0)
        throw std::runtime_error("Can't create an empty file : " + filename);

    filepath_ = std::filesystem::absolute(filename);

    data_ = new uint8_t[size_];
}

File::~File(){
    delete[] data_;
}

//getters
std::string File::extension(){
    return filepath_.extension().string();
}

uint64_t File::size(){
    return size_;
}

uint8_t* File::data(){
    return data_;
}
