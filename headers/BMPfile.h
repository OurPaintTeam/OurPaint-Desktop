#pragma once
#ifndef BMP_FILES_PVM
#define BMP_FILES_PVM

#include <iostream>
#include <cstdint>

class BMPfile
{
	std::uint8_t * bmpPtr_;
	std::uint32_t * height_;
    std::uint32_t * width_;
    std::uint32_t size_;

	static const size_t HEADER_SIZE_ = 62;
    static const size_t FS_OFFSET_ = 2;
    static const size_t WIDTH_OFFSET_ = 18;
    static const size_t HEIGHT_OFFSET_ = 22;
    static const size_t BS_OFFSET_ = 34;

    static void swap_(BMPfile& first, BMPfile& second);
    BMPfile() noexcept: bmpPtr_(nullptr), height_(nullptr), width_(nullptr), size_(0) {}
public:
    BMPfile(const BMPfile&);
    BMPfile& operator=(const BMPfile&);
    BMPfile(BMPfile&&);
    BMPfile& operator=(BMPfile&&);
    ~BMPfile();

    BMPfile(unsigned int width, unsigned int height);
	BMPfile(const char* namefile);
	void saveBmp(const char* nameFile="print.bmp");
	void setPixel(unsigned int row, unsigned int col, bool isWhite) const;
    void resize(unsigned int newWidth, unsigned int newHeight);
    void symResize(std::uint32_t newWidth, std::uint32_t newHeight);
	unsigned int getWidth()const { return *width_; }
	unsigned int getHeight()const { return *height_; }
	bool getPixel(unsigned int, unsigned int) const;//black - false, white - true
};

#endif