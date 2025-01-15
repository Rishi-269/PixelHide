#include<iostream>
#include<fstream>
#include<filesystem>
#include<limits>
#include "image.hpp"

#ifdef _WIN32
    #define clear_console() system("cls") //for windows
#else
    #define clear_console() system("clear") //for linux/mac
#endif

const std::string headerMarker = "MSGSTART";

bool file_exists(const std::string &path) {
    std::ifstream file(path);

    if (file.good())
        std::cout << "\n[SUCCESS] File found: " << path << std::endl;
    else
        std::cout << "\n[ERROR] File not found at the given path: " << path << std::endl;

    return file.good();
}

Image *load_image(){
    std::string filepath;
    Image *input_img = nullptr;

    do {
        do {
            std::cout << "\nEnter the file path or the file name (if in 'input' folder): ";
            std::getline(std::cin, filepath);

            // If only a file name is provided, assume it is in the input folder
            if (filepath.find('/') == std::string::npos && filepath.find('\\') == std::string::npos)
                filepath = "input/" + filepath;

        } while(!file_exists(filepath));

        input_img = new Image(filepath);

        if(input_img->data() == nullptr){
            std::cout << "\n[ERROR] Failed to load image from the provided path. Ensure the file is a valid image.\n" << std::endl;
            delete input_img;
            input_img = nullptr;
        }
        else
            std::cout << "\n[SUCCESS] Image loaded successfully!" << std::endl;

    } while (input_img == nullptr);

    return input_img;
}

void insert_message(Image *input_img){

    const int channels = input_img->channels();
    //available_bytes is the amount of pixel channel bytes that can be used to edit
    uint64_t available_bytes = input_img->size_no_alpha() - 1; // -1 for header mode bit
    uint8_t *data = input_img->data();

    /*
    Header Structure:
        - Mode (1 bit): 
            Indicates the LSB mode for reading and storing message:
            0 -> 1 LSB per pixel channel
            1 -> 2 LSBs per pixel channel
        - "MSGSTART" Marker (8 bytes / 64 bits): 
            A fixed ASCII marker ("MSGSTART") that confirms the presence of a message in the image.
        - Message Size (8 bytes / 64 bits): 
            Specifies the size of the message in bytes. 

        Total Header Size: 1 bit (Mode) + 64 bits (Marker) + 64 bits (Message Size) = 129 bits.

    Message:
        - Variable Length: The actual message data follows the header and is encoded based on the LSB mode.
    */

    uint8_t mode = 1;
    std::string message;

    std::string key;
    char encrypt;

    while (true) {
        std::cout << "\nYour message should be less than " << (2 * available_bytes / 8 - headerMarker.length() - sizeof(uint64_t)) << " characters.\n";
        std::cout << "Enter your message: ";
        getline(std::cin, message);

        if (message.empty())
            std::cout << "\n[ERROR] Message cannot be empty. Please enter a valid message.\n";
        else if (message.length() > 2 * available_bytes / 8 - headerMarker.length() - sizeof(uint64_t))
            std::cout << "\n[ERROR] Message is too large to fit. Please try a shorter message.\n";
        else
            break;
    }

    std::cout << "\nWould you like to encrypt?(y/n) ";
    std::cin>>encrypt;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (encrypt == 'y' || encrypt == 'Y'){
        while (true) {
            std::cout << "\nYour key should be of minumum 16 ASCII characters(more character will be better)\n";
            std::cout << "Enter your key: ";
            getline(std::cin, key);

            if (key.length() < 16)
                std::cout << "\n[ERROR] Key cannot be less than 16 characters. Please enter a valid key.\n";
            else
                break;
        }
    }
    else
        std::cout << "\n[INFO] Not Encrypting message\n";
    
    if(message.length() > available_bytes / 8 - headerMarker.length() - sizeof(uint64_t))
        mode = 2;

    uint64_t i = 0;

    //inserting mode
    data[i++] = (~1 & data[i]) | (mode - 1);

    //xor encrpytion
    uint64_t messageLen = message.length(), keyLen = key.length();
    std::string header = headerMarker;
    if(encrypt == 'y' || encrypt == 'Y'){
        for (int8_t i = 0; i < 8; i++)
            header[i] ^= key[i];

        for (uint64_t i = 0; i < messageLen; i++)
            message[i] ^= key[(i % (keyLen - 8)) + 8];

        uint64_t key_64 = 0;
        for (int8_t i = 0; i < 8; i++)
            key_64 += uint64_t(key[i + 8]) << (i*8);
        messageLen ^= key_64;
    }

    //inserting marker
    for (char c : header){
        for (uint8_t j = 0; j < 8; j += mode){
            if((channels == 2 || channels == 4) && (i % channels) == channels - 1)
                i++;

            data[i] = (~((1<<mode) - 1) & data[i]) | ((c>>j) & ((1<<mode) - 1));
            i++;
        }
    }

    //inserting length
    for (uint64_t len = messageLen, j = 0; j < sizeof(uint64_t) * 8; j += mode){
        if((channels == 2 || channels == 4) && (i % channels) == channels - 1)
            i++;

        data[i] = (~((1<<mode) - 1) & data[i]) | ((len>>j) & ((1<<mode) - 1));
        i++;
    }

    //inserting message
    for (char c : message){
        for (uint8_t j = 0; j < 8; j += mode){

            if((channels == 2 || channels == 4) && (i % channels) == channels - 1)
                i++;
            
            data[i] = (~((1<<mode) - 1) & data[i]) | ((c>>j) & ((1<<mode) - 1));
            i++;
        }
    }

    std::cout << "\n[SUCCESS] The message was successfully inserted into the image." << std::endl;
}

void retrieve_message(Image *input_img){
    
    const int channels = input_img->channels();
    uint8_t *data = input_img->data();

    std::string message;
    uint64_t i = 0, message_length = 0;

    //retrieving mode
    uint8_t mode = (data[i++] & 1) + 1;

    //xor decryption
    std::string key;
    char decrypt = 'n';
    std::cout<<"\nDo you have a key ?(y/n)\n ";
    std::cin>>decrypt;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (decrypt == 'y' || decrypt == 'Y'){
        while (true) {
            std::cout << "\nEnter your key: ";
            getline(std::cin, key);

            if (key.length() < 16)
                std::cout << "\n[ERROR] Key cannot be less than 16 characters. Please enter a valid key.\n";
            else
                break;
        }
    }
    else
        std::cout << "\n[INFO] Retrieving message without key\n";
    uint64_t keyLen = key.length();

    //check if there is a message or not from marker
    for (int k = 0; k < 8; k++) {
        char temp = 0;

        for (uint8_t j = 0; j < 8; j += mode){
            if((channels == 2 || channels == 4) && (i % channels) == channels - 1)
                i++;

            temp |= (data[i] & ((1<<mode) - 1)) << j;
            i++;
        }
        
        if(decrypt == 'y' || decrypt == 'Y')
            temp ^= key[k];
        
        if (temp != headerMarker[k]) {
            std::cout<<"\nNo message found in this image."<<std::endl;
            return;
        }
    }

    //retrieving length of message
    for (uint8_t j = 0; j < sizeof(int64_t) * 8; j += mode) {
        if((channels == 2 || channels == 4) && (i % channels) == channels - 1)
            i++;

        message_length |= uint64_t((data[i] & ((1<<mode) - 1))) << j;
        i++;
    }

    if(decrypt == 'y' || decrypt == 'Y'){
        uint64_t key_64 = 0;
        for (int8_t i = 0; i < 8; i++)
            key_64 += uint64_t(key[i + 8]) << (i*8);
        message_length ^= key_64;
    }

    if( message_length < 1 || message_length > input_img->size_no_alpha() - 1 - headerMarker.length() - sizeof(int64_t)){
        std::cout << "\n[ERROR] Corrupted header. The message length in the header is invalid. Cannot retrieve the message." << std::endl;
        return;
    }


    //retrieving the message
    for(uint64_t k = 0; k < message_length; k++) {
        char c = 0;
        
        for (uint8_t j = 0; j < 8; j += mode){

            if((channels == 2 || channels == 4) && (i % channels) == channels - 1)
                i++;

            c |= (data[i] & ((1<<mode) - 1)) << j;

            i++;
        }
        if(decrypt == 'y' || decrypt == 'Y')
            c ^= key[(k % (keyLen - 8)) + 8];
        message += c;
    }

    std::cout << "\n[SUCCESS] Message retrieved successfully.\n";
    std::cout<<"\nMessage: "<<message<<std::endl;
}

bool isValidName(std::string filename){
    for (char c : filename)
        if(!isalnum(c) && c != ' ' && c != '_' && c != '-')
            return false;
    
    return true;
}

void save_image(Image *input_img){

    std::string filename;
    int fileType = 0;

    do {
        std::cout << "\nChoose file type to save by entering the corresponding number:\n";
        std::cout << "1. PNG\n";
        std::cout << "2. BMP\n";
        std::cout << "Enter your choice (1 or 2): ";
        std::cin >> fileType;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (fileType > 2 || fileType < 1)
            std::cout << "\n[ERROR] Invalid file type selected. Please choose 1, 2, or 3.\n";
        
    } while(fileType > 2 || fileType < 1);

    while (true) {
        std::cout << "\nEnter name to save (alphanumeric, spaces, underscores '_', and hyphens '-' allowed): ";
        std::getline(std::cin, filename);

        if (filename.empty())
            std::cout << "\n[ERROR] Filename cannot be empty. Please provide a valid name.\n";
        else if (!isValidName(filename))
            std::cout << "\n[ERROR] Filename contains invalid characters. Please use alphanumeric characters, spaces, '_', and '-' only.\n";
        else
            break;
    }

    if(input_img->save(filename, fileType))
        std::cout << "\n[SUCCESS] Image saved successfully as " << filename << std::endl;
    else
        std::cout << "\n[ERROR] Failed to save the image. Check if the output folder exists."<<std::endl;
}

int main() {

    clear_console();

    std::cout << "[LOAD IMAGE] Please load an image to proceed.\n";
    Image *input_img = load_image();

    bool running = true;

    int choice;

    do {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        clear_console();

        choice = 0;

        std::cout << "\nWhat would you like to do?\n";
        std::cout << "1. Insert message into the image\n";
        std::cout << "2. Retrieve message from the image\n";
        std::cout << "3. Save the image\n";
        std::cout << "4. Load another image\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice(1-5): ";
        std::cin >> choice;
        
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                insert_message(input_img);
                break;
            case 2:
                retrieve_message(input_img);
                break;
            case 3:
                save_image(input_img);
                break;
            case 4:
                delete input_img; // Free memory for the previous image
                input_img = load_image();
                break;
            case 5:
                std::cout << "\nExiting the program.\n";
                break;
            default:
                std::cout << "\n[ERROR] Invalid choice. Please choose an option between 1 and 5.\n";
                break;
        }

    } while(choice != 5);

    delete input_img;
    return 0;
}