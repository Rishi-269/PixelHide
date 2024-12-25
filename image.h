#include<stdint.h>
#include<string>

class Image{

private:
    uint8_t *data_ = nullptr;
	int channels_ = 0;
	int width_ = 0;
	int height_ = 0;

public:

	bool save(std::string &filepath, int extension);

//constructors and destructor
	Image(std::string &filepath);
	~Image();

//getters
	int height();
	int width();
	int channels();
	uint64_t size();
	uint8_t* data();
};