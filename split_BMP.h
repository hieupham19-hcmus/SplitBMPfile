#ifndef SPLIT_BMP_H
#define SPLIT_BMP_H

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>

using namespace std;

#pragma pack(2)

struct signBMP {
    unsigned char s[2];
};

struct Header {
    signBMP sign;
    uint32_t sizeBMP;
    uint16_t reserved1, reserved2;
    uint32_t dataOffset;
};

struct DIB {
    uint32_t DIB_size;
    uint32_t width;
    uint32_t height;
    uint16_t numColorPlanes;
    uint16_t Color_depth;
    uint32_t compression;
    uint32_t PixelArray_size;
    int32_t Horizontal_res;
    int32_t Vertical_res;
    uint32_t numColors;
    uint32_t numMainColors;
};

struct Color {
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
};

struct Color_t {
    uint8_t Blue;
    uint8_t Green;
    uint8_t Red;
    uint8_t Alpha;
};

// struct file BMP with color table
struct PixelArray_CT {
    Color_t* color_tab;
    uint8_t** pixel;
};

struct BMP_CT {
    Header header;
    DIB dib;
    PixelArray_CT Pixel_arr;
};

// struct file BMP 24 bit
struct PixelArray_24 {
    Color** pixel;
    uint32_t height;
    uint32_t width;
};

struct BMP_24 {
    Header header;
    DIB dib;
    PixelArray_24 Pixel_arr;
};

//struct file BMP 32 bit
struct PixelArray_32 {
    Color_t** pixel;
    uint32_t height;
    uint32_t width;
};

struct BMP_32 {
    Header header;
    DIB dib;
    PixelArray_32 Pixel_arr;
};

#pragma pack (pop)

bool isBMPfile(FILE* file);
void readBitmapHeader(FILE* file, Header& header);
void readBitmapDIB(FILE* file, DIB& dib);
void writeHeaderBMP(FILE* file, Header header);
void writeDIB(FILE* file, DIB dib);
void remove_after_last_dot(char*& str);
char* get_filename(const char* path);

void readColorTable(FILE* file, BMP_CT& bmp);
void writeColorTable(FILE* file, BMP_CT& bmp);
void CopyColorTable(BMP_CT& dest, BMP_CT bmp);
void freeBMP_CT(BMP_CT& bmp);

// file BMP 1 bit

void readBitmapPixel_1(FILE* file, BMP_CT& bmp);
bool readBMP_1(const char* filename, BMP_CT& bmp);
void writePixelArray_1(FILE* file, BMP_CT bmp, int padding);
void writeBMPfile_1(const char* filename, BMP_CT b);
void CopyHeader_1(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width);
void CopyPixels_1(BMP_CT& dest, const BMP_CT& src, uint32_t height, uint32_t width);
BMP_CT cutBMPfile_1(BMP_CT bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum);
void splitBMPfile_1(BMP_CT bmp, const char* src_file_path, int parts_in_height, int parts_in_width);

// file BMP 4 bit

void readBitmapPixel_4(FILE* file, BMP_CT& bmp);
bool readBMP_4(const char* filename, BMP_CT& bmp);
void writePixelArray_4(FILE* file, BMP_CT bmp, int padding);
void writeBMPfile_4(const char* filename, BMP_CT b);
void CopyHeader_4(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width);
void CopyDIB_4(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width);
void CopyPixels_4(BMP_CT& dest, const BMP_CT& src, uint32_t height, uint32_t width);
BMP_CT cutBMPfile_4(BMP_CT bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum);
void splitBMPfile_4(BMP_CT bmp, const char* src_file_path, int parts_in_height, int parts_in_width);

// file BMP 8bit 

void readBitmapPixel_8(FILE* file, BMP_CT& bmp);
bool readBMP_8(const char* filename, BMP_CT& bmp);
void writePixelArray_8(FILE* file, BMP_CT bmp, int padding);
void writeBMPfile_8(const char* filename, BMP_CT b);
void CopyHeader_8(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width);
void CopyDIB_8(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width);
void CopyPixels_8(BMP_CT& dest, const BMP_CT& src, uint32_t height, uint32_t width);
BMP_CT cutBMPfile_8(BMP_CT bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum);
void splitBMPfile_8(BMP_CT bmp, const char* src_file_path, int parts_in_height, int parts_in_width);

// file BMP 24 bit

void readBitmapPixel_24(FILE* file, PixelArray_24* pixel_arr);
bool readBMP_24(const char* filename, BMP_24& bmp);
void freeBMP_24(BMP_24& bmp);
void writePixelArray_24(FILE* file, PixelArray_24 pixel_arr, int padding);
void writeBMPfile_24(const char* filename, BMP_24 b);
void CopyHeader_24(BMP_24& dest, BMP_24 src, uint32_t height, uint32_t width);
void CopyDIB_24(BMP_24& dest, BMP_24 src, uint32_t height, uint32_t width);
void CopyPixels_24(BMP_24& dest, const BMP_24& src, uint32_t height, uint32_t width);
BMP_24 cutBMPfile_24(BMP_24 bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum);
void splitBMPfile_24(BMP_24 bmp, const char* src_file_path, int parts_in_height, int parts_in_width);

// file BMP 32 bit

void readBitmapPixel_32(FILE* file, PixelArray_32* pixel_arr);
bool readBMP_32(const char* filename, BMP_32& bmp);
void freeBMP_32(BMP_32& bmp);
void writePixelArray_32(FILE* file, PixelArray_32 pixel_arr, int padding);
void writeBMPfile_32(const char* filename, BMP_32 b);
void CopyHeader_32(BMP_32& dest, BMP_32 src, uint32_t height, uint32_t width);
void CopyDIB_32(BMP_32& dest, BMP_32 src, uint32_t height, uint32_t width);
void CopyPixels_32(BMP_32& dest, const BMP_32& src, uint32_t height, uint32_t width);
BMP_32 cutBMPfile_32(BMP_32 bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum);
void splitBMPfile_32(BMP_32 bmp, const char* src_file_path, int parts_in_height, int parts_in_width);

#endif