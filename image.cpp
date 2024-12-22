
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "image.h"

#include<iostream>

bool image::save(std::string &filepath, ImageType extension){
    bool success = false;
    switch (extension) {
        case PNG:
            success = stbi_write_png(filepath.data(), width_, height_, channels_, data_, width_*channels_);
            break;
        case BMP:
            success = stbi_write_bmp(filepath.data(), width_, height_, channels_, data_);
            break;
        case JPG:
            success = stbi_write_jpg(filepath.data(), width_, height_, channels_, data_, 100);
            break;
        case TGA:
            success = stbi_write_tga(filepath.data(), width_, height_, channels_, data_);
            break;
    }

    if (success)
        std::cout<<"Successfully saved image"<<std::endl;
    else
        std::cout<<"[ERROR] Failed to save image"<<std::endl;
    
    return success;
}


//constructors and destructor

image::image(std::string &filepath){
    data_ = stbi_load(filepath.data(), &width_, &height_, &channels_, 0);
    if(data == nullptr)
        std::cout<<"[ERROR] Failed to load image"<<std::endl;
    else
        std::cout<<"Image loaded successfully"<<std::endl;
}

image::~image(){
    free(data_);
}

//getters

int image::height(){
    return height_;
}

int image::width(){
    return width_;
}

int image::channels(){
    return channels_;
}

uint64_t image::size(){
    return (uint64_t)height_ * width_ * channels_;
}

uint8_t* image::data(){
    return data_;
}
