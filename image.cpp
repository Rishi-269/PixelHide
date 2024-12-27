
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "image.hpp"

bool Image::save(std::string &filename, int extension){
    switch (extension) {
        case 1:
            filename += ".png";
            return stbi_write_png(filename.c_str(), width_, height_, channels_, data_, width_*channels_);
        case 2:
            filename += ".bmp";
            return stbi_write_bmp(filename.c_str(), width_, height_, channels_, data_);
        default:
            return false;
    }
}


//constructors and destructor

Image::Image(std::string &filepath){
    data_ = stbi_load(filepath.c_str(), &width_, &height_, &channels_, 0);
}

Image::~Image(){
    free(data_);
}

//getters

int Image::height(){
    return height_;
}

int Image::width(){
    return width_;
}

int Image::channels(){
    return channels_;
}

uint64_t Image::size(){
    return (uint64_t)height_ * width_ * channels_;
}

uint8_t* Image::data(){
    return data_;
}
