#ifndef IMAGE_HPP
#define IMAGE_HPP

#include<filesystem>

class Image{

private:
    uint8_t *data_ = nullptr;
	std::filesystem::path filepath_;
	int channels_ = 0;
	int width_ = 0;
	int height_ = 0;

public:

	void save(const bool bmp = false);
	uint64_t size();
	uint64_t size_no_alpha();

//constructors and destructor
	Image(const char *filepath);
	~Image();

//getters
	int height();
	int width();
	int channels();
	uint8_t* data();
	std::string filename();
};

#endif