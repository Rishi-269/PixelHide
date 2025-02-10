#ifndef FILE_HPP
#define FILE_HPP

#include<filesystem>
#include <fstream>
#include<random>

class File{

	private:
		uint8_t *data_ = nullptr;

		uint64_t original_size_ = 0;
		uint64_t size_ = 0;

		std::filesystem::path filepath_;

	public:

		void save();

	//constructors and destructor
		File(const char *filepath);
		File(const std::string filename, uint8_t* &data, const uint64_t size);
		~File();

	//getters
		uint64_t size();
		uint8_t* data();
};

class Key {

	private:
		std::filesystem::path filepath_;

		uint8_t* key_ = nullptr;
		uint8_t* iv_ = nullptr;

		uint8_t key_size_ = 0;
		uint8_t iv_size_ = 0;

	public:

		static void generateKey(const char* filename, const uint8_t key_size = 16);

		//constructors and destructor
		Key(const char *filepath, const uint8_t key_size = 16);
		~Key();

		//getters
		uint8_t keySize();
		uint8_t* key();
		uint8_t IVSize();
		uint8_t* IV();

};

#endif