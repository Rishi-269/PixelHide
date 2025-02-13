#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "image.hpp"

void Image::save(const bool bmp){

    std::filesystem::create_directory("output"); //creates folder if not exists

    std::string filename = "output/" + filepath_.stem().string() + "_i";
    bool success = false;
    if(bmp){
        filename += ".bmp";
        success = stbi_write_bmp(filename.c_str(), width_, height_, channels_, data_);
    }
    else{
        filename += ".png";
        success = stbi_write_png(filename.c_str(), width_, height_, channels_, data_, width_*channels_);
    }

    if(!success)
        throw std::runtime_error("Failed to create image: " + filename);
}

uint64_t Image::size(){
    return (uint64_t)height_ * width_ * channels_;
}

uint64_t Image::size_no_alpha(){
    return (uint64_t)height_ * width_ * (channels_ == 2 || channels_ == 4 ? channels_ - 1 : channels_);
}

//constructors and destructor

Image::Image(const char* filepath){
    filepath_ = std::filesystem::proximate(filepath);
    if(!std::filesystem::exists(filepath_))
        throw std::runtime_error("File does not exist: \"" + filepath_.string() + '\"');

    data_ = stbi_load(filepath, &width_, &height_, &channels_, 0);
    if(data_ == nullptr)
        throw std::runtime_error("Could not load the image.\nPlease check if it's a valid image format: \"" + filepath_.string() + '\"');
}

Image::~Image(){
    stbi_image_free(data_);
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

uint8_t* Image::data(){
    return data_;
}

std::string Image::filename(){
    return filepath_.stem().string();
}