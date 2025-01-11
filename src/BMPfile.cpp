#include <iostream>
#include <fstream>
#include <stdexcept>
#include "BMPfile.h"

const unsigned char templateBMP[]={
        0x42,
        0x4d,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x3e,
        0x0,
        0x0,
        0x0,
        0x28,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x1,
        0x0,
        0x1,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0xff,
        0xff,
        0xff,
        0x0
};

BMPfile::BMPfile(unsigned int width, unsigned int height)
{
	if ( width == 0 )
		throw std::invalid_argument("Bad width");
	if ( height == 0 )
		throw std::invalid_argument("Bad height");

	bmpPtr_ = nullptr;
    std::uint32_t rowSize = ((width + 31) / 32) * 4;//формула из википедии, правильная
    std::uint32_t bodySize = rowSize * height;
    std::uint32_t fileSize = bodySize + HEADER_SIZE_;
	bmpPtr_ = new std::uint8_t[fileSize];
    for(size_t i=0;i<HEADER_SIZE_;++i)
    {
        bmpPtr_[i]=templateBMP[i];
    }

	for (auto i = static_cast<std::uint32_t>(HEADER_SIZE_);
            i < fileSize; ++i)
    {
        bmpPtr_[i] = 0b11111111;//белый
    }

	//записываем параметры
	
	*(reinterpret_cast<std::uint32_t*>(bmpPtr_ + FS_OFFSET_)) = fileSize;
	width_ = reinterpret_cast<std::uint32_t*>(bmpPtr_ + WIDTH_OFFSET_);
	*width_ = width;
	height_ = reinterpret_cast<std::uint32_t*>(bmpPtr_ + HEIGHT_OFFSET_);
 	*height_ = height;
	*(reinterpret_cast<std::uint32_t*>(bmpPtr_ + BS_OFFSET_)) = bodySize;
	size_=fileSize;
}

BMPfile::BMPfile(const char* namefile)
{
	std::fstream file;
	file.open(namefile, std::ios_base::in | std::ios_base::binary);
    if (!file.is_open())
    {
        throw std::invalid_argument("Can't open file to read");
    }
	file.seekg(FS_OFFSET_, std::ios_base::beg);
	std::uint8_t fileSizeChar[4];
	file.read((char*)fileSizeChar, 4);
    size_ = (fileSizeChar[3] << 3 * 8) | (fileSizeChar[2] << 2 * 8) | (fileSizeChar[1] << 8) | (fileSizeChar[0]);
	bmpPtr_ = new std::uint8_t[size_];
	file.seekg(0, std::ios_base::beg);
	file.read((char*)bmpPtr_, size_);
    file.close();

    width_ = reinterpret_cast<std::uint32_t*>(bmpPtr_ + WIDTH_OFFSET_);
    height_ = reinterpret_cast<std::uint32_t*>(bmpPtr_ + HEIGHT_OFFSET_);
}

BMPfile::~BMPfile()
{
	delete[] bmpPtr_;
	bmpPtr_ = nullptr;
    height_  = nullptr;
    width_  = nullptr;
}

void BMPfile::saveBmp(const char* nameFile)
{
	std::fstream file;
	file.open(nameFile, std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
    {
		throw std::invalid_argument("Can't open file to write");
	}
	file.write((char*)bmpPtr_, size_);
	file.close();
}

void BMPfile::setPixel(unsigned int row, unsigned int col, bool isWhite) const
{
	if ( static_cast<std::uint32_t>(row) >= *height_ )
		throw std::invalid_argument("Bad row");
	if ( static_cast<std::uint32_t>(col) >= *width_ )
		throw std::invalid_argument("Bad col");

	const size_t BYTE_OFFSET = HEADER_SIZE_ + (((*width_ + 31) / 32) * 4 * (*height_ - row - 1))+ col/8;
	const size_t BIT_OFFSET = 7 - col % 8;
	if ( !isWhite )//черный
	{
		bmpPtr_[BYTE_OFFSET] &= ( ~ (1 << BIT_OFFSET));
	}
	else//белый
	{
		bmpPtr_[BYTE_OFFSET] |= (1 << BIT_OFFSET );
	}
}

bool BMPfile::getPixel(unsigned int row, unsigned int col) const//black - false, white - true
{
    if ( static_cast<std::uint32_t>(row) >= *height_ )
        throw std::invalid_argument("Bad row");
    if ( static_cast<std::uint32_t>(col) >= *width_ )
        throw std::invalid_argument("Bad col");

    const size_t BYTE_OFFSET = HEADER_SIZE_ + (((*width_ + 31) / 32) * 4 * (*height_ - row - 1))+ col/8;
    const size_t BIT_OFFSET = 7 - col % 8;
    unsigned char byte=bmpPtr_[BYTE_OFFSET] & (1 << BIT_OFFSET);
    if((byte>>BIT_OFFSET)==1)return true;
    return false;
}

void BMPfile::resize(unsigned int newWidth, unsigned int newHeight)
{
    if ( newWidth == 0 )
        throw std::invalid_argument("Bad width");
    if ( newHeight == 0 )
        throw std::invalid_argument("Bad height");
    BMPfile newBMP(newWidth, newHeight);
    std::uint32_t liminOfi=std::min(*newBMP.height_, *this->height_);
    std::uint32_t liminOfj=std::min(*newBMP.width_, *this->width_);
    for(std::uint32_t i=0;i< liminOfi;++i)
    {
        for(std::uint32_t j=0;j< liminOfj;++j)
        {
            newBMP.setPixel(i,j,this->getPixel(i,j));
        }
    }
    swap_(*this,newBMP);
}

void BMPfile::symResize(std::uint32_t newWidth, std::uint32_t newHeight){
    if ( newWidth == 0 )
        throw std::invalid_argument("Bad width");
    if ( newHeight == 0 )
        throw std::invalid_argument("Bad height");
    BMPfile newBMP(newWidth, newHeight);
    if ( (*newBMP.height_ > *this->height_) and
    (*newBMP.width_ > *this->width_) ) {
        for(std::uint32_t i=0;i<*this->height_;++i){
            for(std::uint32_t j=0;j<*this->width_;++j){
                newBMP.setPixel(
                        i + (*newBMP.height_ - *this->height_) / 2,
                        j + (*newBMP.width_ - *this->width_) / 2,
                        this->getPixel(i,j)
                        );
            }
        }
    }
    else {
        long long int x;
        long long int y;
        for(std::uint32_t i=0;i<*this->height_;++i){
            for(std::uint32_t j=0;j<*this->width_;++j){
                x = i + ( static_cast<long long int>(*newBMP.height_) - static_cast<long long int>(*this->height_) ) / 2;
                y = j + ( static_cast<long long int>(*newBMP.width_) - static_cast<long long int>(*this->width_) ) / 2;
                if(x<0 or y<0 or x>=(*newBMP.height_) or y>=(*newBMP.width_) ) {
                    continue;
                }
                newBMP.setPixel(x,y,this->getPixel(i,j));
            }
        }
    }
    swap_(*this,newBMP);
}

void BMPfile::swap_(BMPfile& first, BMPfile& second)
{
    std::swap(first.height_, second.height_);
    std::swap(first.width_, second.width_);
    std::swap(first.size_, second.size_);
    std::swap(first.bmpPtr_, second.bmpPtr_);
}

BMPfile::BMPfile(const BMPfile& other)
{
    size_=other.size_;
    bmpPtr_ = new std::uint8_t[size_];
    for(std::uint32_t i=0;i<size_;++i)
    {
        bmpPtr_[i]=other.bmpPtr_[i];
    }
    width_ = reinterpret_cast<std::uint32_t*>(bmpPtr_ + WIDTH_OFFSET_);
    height_ = reinterpret_cast<std::uint32_t*>(bmpPtr_ + HEIGHT_OFFSET_);
}

BMPfile& BMPfile::operator=(const BMPfile& other)
{
    if(this!= &other)
    {
        BMPfile tmp(other);
        swap_(tmp, *this);
    }
    return *this;
}

BMPfile::BMPfile(BMPfile&& other): BMPfile()
{
    swap_(other, *this);
}

BMPfile& BMPfile::operator=(BMPfile&& other)
{
    if(this!= &other)
    {
        swap_(other, *this);
    }
    return *this;
}