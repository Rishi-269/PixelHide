#include <iostream>
#include "image.cpp"
#include "file.cpp"

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
        - Extension (variable size):
            variable length ASCII string that tells the extension of hidden file
            Example: file.txt will have extension stored as "txt\0". '\0' tells the end of extension

    Hidden File Data:
        - Variable Length: The actual file data follows the header and is encoded based on the LSB mode.
*/

void insertData(Image &inputImage, File &inputFile){
    
    uint8_t *imgData = inputImage.data();
    uint8_t *fileData = inputFile.data();

    uint64_t imgIterator = 0;

    const uint64_t fileSize = inputFile.size();
    const uint8_t channels = inputImage.channels();
    const std::string extension = inputFile.extension() + '\0';

    uint8_t mode = 1;

    if(fileSize > ((inputImage.size_no_alpha() - 1) / sizeof(uint8_t)) - headerMarker.length() - sizeof(uint64_t) - inputFile.extension().length())
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

    //inserting extension
    for (uint8_t i = 1; i < extension.length(); i++){
        for (uint8_t j = 0; j < 8; j += mode){
            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            imgData[imgIterator] = (~((1<<mode) - 1) & imgData[imgIterator]) | ((extension[i]>>j) & ((1<<mode) - 1));
            imgIterator++;
        }
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

    inputImage.save();
}

void retrieveData(Image &inputImage){

    const uint8_t *imgData = inputImage.data();
    const uint8_t channels = inputImage.channels();
    const uint64_t imgSize = inputImage.size();

    uint64_t imgIterator = 0, fileSize = 0;

    std::string extension = ".";

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

    //retrieving extention
    for (char temp = 0; imgIterator < imgSize; temp = 0) {

        for (uint8_t j = 0; j < 8; j += mode){
            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            temp |= (imgData[imgIterator] & ((1<<mode) - 1)) << j;
            imgIterator++;
        }

        if(temp == '\0')
            break;

        extension += temp;
    }
    

    if(fileSize < 1 || fileSize > inputImage.size_no_alpha() - 1 - headerMarker.length() - sizeof(int64_t) - extension.length()){
        std::cout << "Corrupted header. The message length in the header is invalid. Cannot retrieve the message.\n";
        return;
    }

    //creating file
    File outputFile(inputImage.filename() + "_extract" + extension, fileSize);

    uint8_t *fileData = outputFile.data();

    //retrieving the data into the file
    for(uint64_t fileIterator = 0; fileIterator < fileSize; fileIterator++) {
        int8_t tempData = 0;
        
        for (uint8_t j = 0; j < 8; j += mode){

            if(channels % 2 == 0 && (imgIterator % channels) == channels - 1)
                imgIterator++;

            tempData |= (imgData[imgIterator] & ((1<<mode) - 1)) << j;

            imgIterator++;
        }
        
        fileData[fileIterator] = tempData;
    }

    outputFile.save();
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 5) {
        std::cerr << "Usage: ./program <mode> <imagepath> <text> <key>\n";
        return 1;
    }

    std::string mode(argv[1]);

    // Validate mode
    if (mode != "-i" && mode != "--insert" && mode != "-r" && mode != "--retrieve") {
        std::cerr << "Error: Invalid mode. Use '-i' or '--insert' for insertion, '-r' or '--retrieve' for extraction.\n";
        return 1;
    }

    try
    {
        // Load the image
        Image inputImage(argv[2]);

        if ((mode == "-i" || mode == "--insert") && (argc == 4 || argc == 5)) {
            
            File inputFile(argv[3]);

            if (inputFile.size() > (2 * (inputImage.size_no_alpha() - 1) / 8) - headerMarker.length() - sizeof(uint64_t) - inputFile.extension().length())
                throw std::runtime_error("File is too large to fit. Please try a smaller file.");

            if(argc == 5){
                // File keyFile(argv[4]);
                //encrypt the inputFile data
            }

            insertData(inputImage, inputFile);

        }
        else if((mode == "-r" || mode == "--retrieve") && (argc == 3 || argc == 4)){

            if(argc == 4){
                // File keyFile(argv[3]);

            }

            retrieveData(inputImage);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
    
    return 0;
}