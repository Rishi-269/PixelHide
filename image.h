#include<stdint.h>
#include<string>

enum ImageType {
	PNG, JPG, BMP, TGA
};

class image{

private:
    uint8_t *data_ = nullptr;
	int channels_ = 0;
	int width_ = 0;
	int height_ = 0;

public:

	bool save(std::string &filepath, ImageType extension);

//constructors and destructor
	image(std::string &filepath);
	~image();

//getters
	int height();
	int width();
	int channels();
	uint64_t size();
	uint8_t* data();
};