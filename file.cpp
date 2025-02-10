#include "file.hpp"

//File
void File::save(){
    
    std::filesystem::create_directory("retrieved");

    std::ofstream fout(filepath_, std::ios::binary);
    if(!fout)
        throw std::runtime_error("Failed to create file: " + filepath_.filename().string());

    fout.write(reinterpret_cast<char*>(data_), original_size_);
    fout.close();
}

//constructors and destructor

File::File(const char *filepath) {
    filepath_ = std::filesystem::proximate(filepath);
    if(!std::filesystem::exists(filepath_))
        throw std::runtime_error("File does not exist: \"" + filepath_.string() + '\"');

    std::ifstream fin(filepath_, std::ios::binary);
    if (!fin)
        throw std::runtime_error("Could not open file: \"" + filepath_.string() + '\"');

    original_size_ = std::filesystem::file_size(filepath_);
    if (original_size_ <= 0)
        throw std::runtime_error("Please enter a valid file.\nFile is empty: \"" + filepath_.string() + '\"');

    std::string extension = filepath_.extension().string() + '\0';
    size_ = original_size_ + extension.length();
    
    data_ = new uint8_t[size_];

    fin.read(reinterpret_cast<char*>(data_), original_size_);

    fin.close();

    //put extension in data (in reverse)
    for (uint64_t i = size_ - 1; i >= original_size_; i--)
        data_[i] = extension[size_ - 1 - i];

}

File::File(const std::string filename, uint8_t* &data, const uint64_t size) : data_(data), size_(size){
    data = nullptr;

    if (size_ <= 0){
        delete[] data_;
        throw std::runtime_error("Can't create an empty file : " + filename);
    }

    uint64_t rev_iterator = size_ - 1;

    //get extension
    std::string extension;
    while (rev_iterator >= 0) {
        char c = data_[rev_iterator--];

        if(c == '\0')
            break;

        extension += c;
    }
    
    original_size_ = rev_iterator + 1;
    filepath_ = std::filesystem::proximate("retrieved/" + filename + "_r");
    filepath_.replace_extension(extension);
}

File::~File(){
    delete[] data_;
}

//getters

uint64_t File::size(){
    return size_;
}

uint8_t* File::data(){
    return data_;
}

//Key

void Key::generateKey(const char* filename, const uint8_t key_size) {
    // Validate key size
    if (key_size != 16 && key_size != 24 && key_size != 32)
        throw std::runtime_error("Invalid key size. Must be 16, 24, or 32 bytes.");

    std::filesystem::create_directory("keys");

    std::filesystem::path filepath("keys/" + std::string(filename));
    filepath.replace_extension(".key");
    
    std::ofstream fout(filepath , std::ios::binary);
    if(!fout)
        throw std::runtime_error("Failed to create file: " + filepath.stem().string());
    
    uint8_t data[key_size + 16];

    // Fill with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);

    for (int i = 0; i < key_size + 16; ++i)
        data[i] = dis(gen);

    // Save to file

    fout.write(reinterpret_cast<char*>(data), key_size + 16);

    fout.close();
}

Key::Key(const char *filepath, const uint8_t key_size) : key_size_(key_size), iv_size_(16) {
    filepath_ = std::filesystem::proximate(filepath);
    if(!std::filesystem::exists(filepath_))
        throw std::runtime_error("Key does not exist: \"" + filepath_.string() + '\"');

    std::ifstream fin(filepath_, std::ios::binary);
    if (!fin)
        throw std::runtime_error("Could not open key: \"" + filepath_.string() + '\"');

    if (std::filesystem::file_size(filepath_) < iv_size_ + key_size_)
        throw std::runtime_error("Key size is less than " + std::to_string(iv_size_ + key_size_) + "\nPlease enter a valid key: \"" + filepath_.string() + '\"');
    
    iv_ = new uint8_t[iv_size_];
    key_ = new uint8_t[key_size_];

    fin.read(reinterpret_cast<char*>(iv_), iv_size_);
    fin.read(reinterpret_cast<char*>(key_), key_size_);

    fin.close();
}

Key::~Key(){
    delete[] key_;
    delete[] iv_;
}

//getters
uint8_t Key::keySize(){
    return key_size_;
}

uint8_t* Key::key(){
    return key_;
}

uint8_t Key::IVSize(){
    return iv_size_;
}

uint8_t* Key::IV(){
    return iv_;
}
