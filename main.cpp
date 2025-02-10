#include <iostream>
#include "tiny-aes/aes.hpp"
#include "image.hpp"
#include "file.hpp"

std::string headerMarker = "MSGSTART";

/*
    Header Structure:
        - Mode (1 bit): 
            Indicates the LSB mode for reading and storing message:
            0 -> 1 LSB per pixel channel
            1 -> 2 LSBs per pixel channel
        - "MSGSTART" Marker (8 bytes / 64 bits): 
            A fixed ASCII marker ("MSGSTART") that confirms the presence of a data in the image.
        - Data Size (8 bytes / 64 bits):
            Specifies the size of the data in bytes.

    // total header size = 1 + 64 + 64 = 129bits

    Hidden File Data:
        - Variable Length Data: The actual file data follows the header and is encoded based on the LSB mode.
        - Variable Length Extension: stored in reverse separating by null character. Example: [data][data]...[data]['\0'][t][x][t][.]
        - PKCS7 Padding (optional): For Block Cipher
*/

//without encryption insertData
void insertData(Image &inputImage, File &inputFile){
    
    uint8_t *imgData = inputImage.data();
    uint8_t *fileData = inputFile.data();

    uint64_t imgIterator = 0;

    const uint64_t fileSize = inputFile.size();
    const uint8_t channels = inputImage.channels();

    uint8_t mode = 1;

    if(fileSize > ((inputImage.size_no_alpha() - 1) / 8) - headerMarker.length() - sizeof(uint64_t))
        mode = 2;

    //inserting mode
    imgData[imgIterator++] = (~1 & imgData[imgIterator]) | (mode - 1);

    //inserting marker
    for (char c : headerMarker){
        for (uint8_t j = 0; j < 8; j += mode){
            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            imgData[imgIterator] = (~((1<<mode) - 1) & imgData[imgIterator]) | ((c>>j) & ((1<<mode) - 1));
            imgIterator++;
        }
    }
    
    //inserting size
    for (uint8_t j = 0; j < sizeof(uint64_t) * 8; j += mode){
        if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
            imgIterator++;

        imgData[imgIterator] = (~((1<<mode) - 1) & imgData[imgIterator]) | ((fileSize>>j) & ((1<<mode) - 1));
        imgIterator++;
    }

    //inserting file data
    for (uint64_t fileIterator = 0; fileIterator < fileSize; fileIterator++){
        for (uint8_t j = 0; j < 8; j += mode){
            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;
            
            imgData[imgIterator] = (~((1<<mode) - 1) & imgData[imgIterator]) | ((fileData[fileIterator]>>j) & ((1<<mode) - 1));
            imgIterator++;
        }
    }
}

//with encryption insertData
void insertData(Image &inputImage, File &inputFile, Key &inputKey){
    
    uint8_t *imgData = inputImage.data();
    uint8_t *fileData = inputFile.data();

    uint64_t imgIterator = 0;

    const uint64_t fileSize = inputFile.size();
    const uint8_t channels = inputImage.channels();

    AES_ctx ctx;
    uint8_t *iv = inputKey.IV();
    uint8_t *key = inputKey.key();


    uint8_t mode = 1;

    if(fileSize > ((inputImage.size_no_alpha() - 1) / 8) - headerMarker.length() - sizeof(uint64_t))
        mode = 2;

    //inserting mode
    imgData[imgIterator++] = (~1 & imgData[imgIterator]) | (mode - 1);

    //inserting encrypted (marker + filesize) 128 bits with IV 128 bits using ECB
    uint8_t headerData[AES_BLOCKLEN];

    for (int i = 0; i < 8; i++)
        headerData[i] = headerMarker[i];
    
    for (int i = 0; i < 8; i++)
        headerData[8 + i] = (fileSize >> (i * 8)) & UINT8_MAX;

    AES_init_ctx(&ctx, iv); //using iv as key to encrypt header in ECB
    AES_ECB_encrypt(&ctx, headerData);
    
    for (uint8_t i = 0; i < AES_BLOCKLEN; i++){
        for (uint8_t j = 0; j < 8; j += mode){
            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;
            
            imgData[imgIterator] = (~((1<<mode) - 1) & imgData[imgIterator]) | ((headerData[i]>>j) & ((1<<mode) - 1));
            imgIterator++;
        }
    }

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CTR_xcrypt_buffer(&ctx, fileData, fileSize);

    //inserting file data
    for (uint64_t fileIterator = 0; fileIterator < fileSize; fileIterator++){
        for (uint8_t j = 0; j < 8; j += mode){
            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;
            
            imgData[imgIterator] = (~((1<<mode) - 1) & imgData[imgIterator]) | ((fileData[fileIterator]>>j) & ((1<<mode) - 1));
            imgIterator++;
        }
    }
}

void retrieveData(Image &inputImage){

    const uint8_t *imgData = inputImage.data();
    const uint8_t channels = inputImage.channels();
    const uint64_t imgSize = inputImage.size();

    uint64_t imgIterator = 0, fileSize = 0;

    //retrieving mode
    uint8_t mode = (imgData[imgIterator++] & 1) + 1;

    //check if there is a message or not from marker
    for (char c : headerMarker) {
        char temp = 0;

        for (uint8_t j = 0; j < 8; j += mode){
            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            temp |= (imgData[imgIterator] & ((1<<mode) - 1)) << j;
            imgIterator++;
        }
        
        if (temp != c) {
            std::cout<<"No data found in this image.\n";
            return;
        }
    }

    //retrieving size of data
    for (uint8_t j = 0; j < sizeof(int64_t) * 8; j += mode) {
        if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
            imgIterator++;

        fileSize |= uint64_t((imgData[imgIterator] & ((1<<mode) - 1))) << j;
        imgIterator++;
    }
    
    if(fileSize < 1 || fileSize > (mode * (inputImage.size_no_alpha() - 1) / 8) - headerMarker.length() - sizeof(int64_t)){
        std::cout << "Corrupted header. The message length in the header is invalid. Cannot retrieve the message.\n";
        return;
    }

    //retrieving the data into the file
    uint8_t *fileData = new uint8_t[fileSize];

    for(uint64_t fileIterator = 0; fileIterator < fileSize; fileIterator++) {
        int8_t tempByte = 0;
        
        for (uint8_t j = 0; j < 8; j += mode){

            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            tempByte |= (imgData[imgIterator] & ((1<<mode) - 1)) << j;

            imgIterator++;
        }
        
        fileData[fileIterator] = tempByte;
    }

    File outputFile(inputImage.filename(), fileData, fileSize);

    outputFile.save();
}

void retrieveData(Image &inputImage, Key &inputKey){

    const uint8_t *imgData = inputImage.data();
    const uint8_t channels = inputImage.channels();
    const uint64_t imgSize = inputImage.size();

    uint64_t imgIterator = 0, fileSize = 0;

    AES_ctx ctx;
    uint8_t *iv = inputKey.IV();
    uint8_t *key = inputKey.key();

    //retrieving mode
    uint8_t mode = (imgData[imgIterator++] & 1) + 1;

    //check if there is a message or not from marker
    uint8_t headerData[AES_BLOCKLEN];

    for(uint64_t i = 0; i < AES_BLOCKLEN; i++) {
        uint8_t tempByte = 0;
        
        for (uint8_t j = 0; j < 8; j += mode){

            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            tempByte |= (imgData[imgIterator] & ((1<<mode) - 1)) << j;

            imgIterator++;
        }
        
        headerData[i] = tempByte;
    }

    AES_init_ctx(&ctx, iv);
    AES_ECB_decrypt(&ctx, headerData);

    for (int i = 0; i < 8; i++){
        if(headerData[i] != headerMarker[i]){
            std::cout<<"No data found in this image.\n";
            return;
        }
    }
        
    for (int i = 0; i < 8; i++)
        fileSize |= uint64_t(headerData[8 + i]) << (i * 8);
    
    if(fileSize < 1 || fileSize > (mode * (inputImage.size_no_alpha() - 1) / 8) - headerMarker.length() - sizeof(int64_t)){
        std::cout << "Corrupted header. The message length in the header is invalid. Cannot retrieve the message.\n";
        return;
    }

    //retrieving the data into the file
    uint8_t *fileData = new uint8_t[fileSize];

    for(uint64_t fileIterator = 0; fileIterator < fileSize; fileIterator++) {
        uint8_t tempByte = 0;
        
        for (uint8_t j = 0; j < 8; j += mode){

            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            tempByte |= (imgData[imgIterator] & ((1<<mode) - 1)) << j;

            imgIterator++;
        }
        
        fileData[fileIterator] = tempByte;
    }

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CTR_xcrypt_buffer(&ctx, fileData, fileSize);

    File outputFile(inputImage.filename(), fileData, fileSize);

    outputFile.save();
}

void printHelp(char* program) {
    std::string progName = std::filesystem::path(program).stem().string();

    std::cout << "\nUsage:\n";
    std::cout << "  ./" << progName << " <mode> [options]\n\n";

    std::cout << "Modes:\n";
    std::cout << "  -h, --help      Display this help message and exit.\n";
    std::cout << "  -k, --key       Generate a 16-byte encryption key and save it to a file.\n";
    std::cout << "                  Usage: ./" << progName << " --key <filename>\n\n";

    std::cout << "  -i, --insert    Embed a file into an image using optional encryption.\n";
    std::cout << "                  Usage: ./" << progName << " --insert <image> <file> [key]\n";
    std::cout << "                    <image> - Path to the image file.\n";
    std::cout << "                    <file>  - Path to the file to hide.\n";
    std::cout << "                    [key]   - Optional encryption key file path.\n\n";

    std::cout << "  -r, --retrieve  Extract hidden data from an image.\n";
    std::cout << "                  Usage: ./" << progName << " --retrieve <image> [key]\n";
    std::cout << "                    <image> - Path to the steganographic image.\n";
    std::cout << "                    [key]   - Optional encryption key file path.\n\n";

    std::cout << "Examples:\n";
    std::cout << "  ./" << progName << " --key mykey.bin\n";
    std::cout << "  ./" << progName << " --insert image.png secret.txt mykey.bin\n";
    std::cout << "  ./" << progName << " --retrieve image.png mykey.bin\n\n";
}


int main(int argc, char* argv[]) {

    try {
        if (argc < 2)
            throw std::runtime_error("Invalid usage. Use \"./" + std::filesystem::path(argv[0]).stem().string() + " -h\" for help.");

        std::string mode(argv[1]);

        if (mode == "-h" || mode == "--help"){
            printHelp(argv[0]);
        }
        else if ((mode == "-k" || mode == "--key") && argc == 3){
            Key::generateKey(argv[2]);
        }
        else if ((mode == "-i" || mode == "--insert") && (argc == 4 || argc == 5)) {
            Image inputImage(argv[2]);

            uint64_t availableBytes = (2 * (inputImage.size_no_alpha() - 1) / 8) - headerMarker.length() - sizeof(uint64_t);

            if (availableBytes < std::filesystem::file_size(argv[3]) + std::filesystem::path(argv[3]).extension().string().length() + 1)
                throw std::runtime_error("File is too large to fit.\nThe Image can fit " + std::to_string(availableBytes) + " bytes.");

            File inputFile(argv[3]);

            if (argc == 5) {
                Key inputKey(argv[4]);
                insertData(inputImage, inputFile, inputKey);
            }
            else {
                insertData(inputImage, inputFile);
            }

            inputImage.save();
        }
        else if ((mode == "-r" || mode == "--retrieve") && (argc == 3 || argc == 4)) {
            Image inputImage(argv[2]);

            if (argc == 4) {
                Key inputKey(argv[3]);
                retrieveData(inputImage, inputKey);
            }
            else{
                retrieveData(inputImage);
            }
        }
        else {
            throw std::runtime_error("Invalid mode or incorrect number of arguments. Use -h for help.");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    
    return 0;
}