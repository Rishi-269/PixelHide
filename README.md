# Pixel Hide

Pixel Hide is a C++ tool that implements Least Significant Bit (LSB) steganography to hide files within images. It supports optional AES encryption for secure insertion and retrieval of data. The tool is designed for both casual and secure data hiding, offering a command-line interface for embedding and extracting files from images.

## Features
- Supports multiple image formats for input: **PNG, JPG, BMP, etc**.
- Output is restricted to lossless formats: **PNG, BMP**.
- Command-line interface for seamless usage.
- Optional AES encryption (CTR mode) using **tiny-AES**.
- Uses **stb_image** for reading and writing image files.

## Compilation
To compile Pixel Hide, ensure you have **g++ with C++17 support** installed.

```sh
g++ -std=c++17 -o pixelhide main.cpp image.cpp file.cpp tiny-aes/aes.c 
```

## Installation & Usage
### Download & Compile
1. Clone the repository:
   ```sh
   git clone https://github.com/Rishi-269/PixelHide.git
   cd PixelHide
   ```
2. Compile the project:
   ```sh
   g++ -std=c++17 -o pixelhide main.cpp image.cpp file.cpp tiny-aes/aes.c 
   ```
3. Run the tool using command-line arguments.

### Usage
```
Usage:
  ./pixelhide <mode> [options]

Modes:
  -h, --help      Display this help message and exit.
  -k, --key       Generate a 16-byte encryption key and save it to a file.
                  Usage: ./pixelhide --key <filename>

  -i, --insert    Embed a file into an image using optional encryption.
                  Usage: ./pixelhide --insert <image> <file> [key]
                    <image> - Path to the image file.
                    <file>  - Path to the file to hide.
                    [key]   - Optional encryption key file path.

  -r, --retrieve  Extract hidden data from an image.
                  Usage: ./pixelhide --retrieve <image> [key]
                    <image> - Path to the steganographic image.
                    [key]   - Optional encryption key file path.

Examples:
  ./pixelhide --key mykey
  ./pixelhide --insert image.png secret.txt keys/mykey.key
  ./pixelhide --retrieve output/image_i.png keys/mykey.key
```

## Dependencies
- **tiny-AES** (CTR mode) for encryption: [tiny-AES](https://github.com/kokke/tiny-AES-c)
- **stb_image** for image handling: [stb_image](https://github.com/nothings/stb)

## Download Compiled Version
An executable for windows can be downloaded from:
[Download Pixel Hide](https://github.com/Rishi-269/PixelHide/releases/tag/v1.0)

---

This tool is built for secure and efficient steganography, making data hiding simple and reliable.
