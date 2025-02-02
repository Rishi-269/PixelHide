#include<stdint.h>
#include<filesystem>

class File{

private:
    uint8_t *data_ = nullptr;
    uint64_t size_ = 0;
	std::filesystem::path filepath_;

public:

	void save();

//constructors and destructor
	File(char *filepath);
    File(std::string filename, uint64_t size);
	~File();

//getters
	uint64_t size();
	uint8_t* data();
	std::string extension();
};