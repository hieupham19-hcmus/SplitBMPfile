#include "split_BMP.h"


bool isBMPfile(FILE* file) {
    if (!file) return false;
    char temp[2];
    fread(temp, sizeof(unsigned char), 2, file);
    if (temp[0] == 'B' && temp[1] == 'M') return true;
    return false;
}

void readBitmapHeader(FILE* file, Header& header) {
    fseek(file, 0, SEEK_SET);
    fread(&header, sizeof(Header), 1, file);
}

void readBitmapDIB(FILE* file, DIB& dib) {
    fseek(file, sizeof(Header), SEEK_SET);
    fread(&dib, sizeof(DIB), 1, file);
}

void writeHeaderBMP(FILE* file, Header header)
{
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(Header), 1, file);
}

void writeDIB(FILE* file, DIB dib)
{
    fseek(file, sizeof(Header), SEEK_SET);
    fwrite(&dib, sizeof(DIB), 1, file);
}

void remove_after_last_dot(char*& str) {
    char* last_dot = strrchr(str, '.');

    if (last_dot != NULL) {
        *last_dot = '\0';
    }
}

char* get_filename(const char* path) {
    char* path_temp = _strdup(path);
    remove_after_last_dot(path_temp);

    char* result = _strdup(path_temp);

    return result;
}

// functions use to process BMP files with a color table.

void readColorTable(FILE* file, BMP_CT& bmp) {
    if (bmp.dib.numColors != 0) {
        bmp.Pixel_arr.color_tab = new Color_t[bmp.dib.numColors];
        fseek(file, sizeof(Header) + sizeof(DIB), SEEK_SET);
        fread(bmp.Pixel_arr.color_tab, sizeof(Color_t), bmp.dib.numColors, file);
    }
}

void writeColorTable(FILE* file, BMP_CT& bmp)
{
    fseek(file, sizeof(Header) + sizeof(DIB), SEEK_SET);
    fwrite(bmp.Pixel_arr.color_tab, sizeof(Color_t), bmp.dib.numColors, file);
    fflush(file);
}

void CopyColorTable(BMP_CT& dest, BMP_CT bmp) {
    dest.Pixel_arr.color_tab = new Color_t[bmp.dib.numColors];
    for (int i = 0; i < bmp.dib.numColors; i++) {
        dest.Pixel_arr.color_tab[i] = bmp.Pixel_arr.color_tab[i];
    }
}

void freeBMP_CT(BMP_CT& bmp) {
    delete[] bmp.Pixel_arr.color_tab;

    for (int i = 0; i < bmp.dib.height; i++) {
        delete[] bmp.Pixel_arr.pixel[i];
    }

    delete[] bmp.Pixel_arr.pixel;
}

//////////////////////////////////////////////////////

///////////////// file BMP 1 bit ////////////////////

////////////////////////////////////////////////////

void readBitmapPixel_1(FILE* file, BMP_CT& bmp) {
    int padding = (4 - ((bmp.dib.width + 7) / 8) % 4) % 4;
    fseek(file, bmp.header.dataOffset, SEEK_SET);

    bmp.Pixel_arr.pixel = new uint8_t * [bmp.dib.height];
    for (int i = 0; i < bmp.dib.height; i++) {
        bmp.Pixel_arr.pixel[i] = new uint8_t[bmp.dib.width];
    }

    uint8_t* row_buffer = new uint8_t[(bmp.dib.width + 7) / 8];

    int row_byte_count = (bmp.dib.width + 7) / 8;
    for (int iRow = 0; iRow < bmp.dib.height; iRow++) {
        fread(row_buffer, sizeof(uint8_t), row_byte_count, file);
        int iCol = 0;
        int bit_count = 0;
        while (iCol < bmp.dib.width) {
            // extract the pixel value from the bitstream
            uint8_t pixel = (row_buffer[bit_count / 8] >> (7 - bit_count % 8)) & 0x01;
            bmp.Pixel_arr.pixel[(bmp.dib.height - iRow - 1)][iCol] = pixel;
            bit_count++;
            iCol++;
        }
        fseek(file, padding, SEEK_CUR);
    }
    delete[] row_buffer;
}

bool readBMP_1(const char* filename, BMP_CT& bmp) {

    FILE* file = fopen(filename, "rb");
    if (!isBMPfile(file)) {
        fclose(file);
        return false;
    }

    readBitmapHeader(file, bmp.header);
    readBitmapDIB(file, bmp.dib);
    readColorTable(file, bmp);
    readBitmapPixel_1(file, bmp);

    fclose(file);

    return true;
}

void writePixelArray_1(FILE* file, BMP_CT bmp, int padding) {
    int row_byte_count = (bmp.dib.width + 7) / 8;
    for (int i = bmp.dib.height - 1; i >= 0; i--) {
        fwrite(bmp.Pixel_arr.pixel[i], sizeof(uint8_t), row_byte_count, file);
        char zero = 0;
        for (int j = 0; j < padding; j++) {
            fwrite(&zero, sizeof(char), 1, file);
        }
    }
}

void writeBMPfile_1(const char* filename, BMP_CT b)
{
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    writeHeaderBMP(file, b.header);
    writeDIB(file, b.dib);

    int padding = (4 - ((b.dib.width + 7) / 8) % 4) % 4;
    writeColorTable(file, b);
    writePixelArray_1(file, b, padding);

    fclose(file);
}

void CopyHeader_1(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width)
{
    dest.header = src.header;
    int paddingCount = (4 - ((width + 7) / 8) % 4) % 4;
    dest.header.sizeBMP = (width * (src.dib.Color_depth / 8) + paddingCount) * height + src.header.dataOffset;
}

void CopyDIB_1(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width)
{
    dest.dib = src.dib;
    dest.dib.height = height;
    dest.dib.width = width;
    dest.dib.Color_depth = 1;
    int paddingCount = (4 - ((width + 7) / 8) % 4) % 4;
    dest.dib.PixelArray_size = (width * (src.dib.Color_depth / 8) + paddingCount) * height;

}

void CopyPixels_1(BMP_CT& dest, const BMP_CT& src, uint32_t height, uint32_t width) {
    dest.dib.width = width;
    dest.dib.height = height;
    dest.Pixel_arr.pixel = new uint8_t * [height];

    int paddingCount = (4 - ((width + 7) / 8) % 4) % 4;

    for (uint32_t i = 0; i < height; i++) {
        dest.Pixel_arr.pixel[i] = new uint8_t[(width + 7) / 8];
        // copy the pixel data of each row from the corresponding row of the source BMP_CT object
        memcpy(dest.Pixel_arr.pixel[i], (src.Pixel_arr.pixel[i]), ((width + 7) / 8) * sizeof(uint8_t));
    }
}

BMP_CT cutBMPfile_1(BMP_CT bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum) {
    BMP_CT b_new;
    CopyHeader_1(b_new, bmp, sizeRow, sizeColum);
    CopyDIB_1(b_new, bmp, sizeRow, sizeColum);

    PixelArray_CT data = bmp.Pixel_arr, data_new;

    data_new.pixel = new uint8_t * [sizeRow];
    for (int i = 0; i < sizeRow; i++) {
        data_new.pixel[i] = new uint8_t[sizeColum / 8];
        int byte_count = 0;
        uint8_t byte_value = 0;
        // iterate over the subset of the original pixel array, 
        // packing the pixels into bytes to populate the new array
        for (int j = 0; j < sizeColum; j++) {
            uint8_t pixel = data.pixel[i + startRow][j + startColum];
            byte_value = byte_value << 1 | pixel;

            if ((j + 1) % 8 == 0) {
                data_new.pixel[i][byte_count] = byte_value;
                byte_value = 0;
                byte_count++;
            }
        }
    }

    b_new.Pixel_arr = data_new;

    return b_new;
}

void splitBMPfile_1(BMP_CT bmp, const char* src_file_path, int parts_in_height, int parts_in_width) {
    BMP_CT** bmp_parts = new BMP_CT * [parts_in_height];
    for (int i = 0; i < parts_in_height; i++) {
        bmp_parts[i] = new BMP_CT[parts_in_width];
    }

    int part_height = bmp.dib.height / parts_in_height;
    int part_width = bmp.dib.width / parts_in_width;

    int x = 0;
    for (int iRow = 0; iRow < parts_in_height; iRow++) {
        int y = 0;
        for (int iCol = 0; iCol < parts_in_width; iCol++) {
            BMP_CT bmp_temp = cutBMPfile_1(bmp, x, y, part_height, part_width);
            CopyHeader_1(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyDIB_1(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyColorTable(bmp_parts[iRow][iCol], bmp);
            CopyPixels_1(bmp_parts[iRow][iCol], bmp_temp, part_height, part_width);
            y += part_width;
        }
        x += part_height;
    }


    char basename[1024];
    sprintf(basename, "%s_part", get_filename(src_file_path));
    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            char output_filename[1024];
            sprintf(output_filename, "%s%02d.bmp", basename, i * parts_in_width + j + 1);

            writeBMPfile_1(output_filename, bmp_parts[i][j]);
        }
    }

    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            delete[] bmp_parts[i][j].Pixel_arr.pixel;
        }
        delete[] bmp_parts[i];
    }
    delete[] bmp_parts;
}


//////////////////////////////////////////////////////

///////////////// file BMP 4 bit ////////////////////

////////////////////////////////////////////////////


// For the case of 4 - bit files, there are still some minor padding errors in my functions, 
// but there is no more time to fix them, so I hope that you can have a look, sir.

void readBitmapPixel_4(FILE* file, BMP_CT& bmp) {
    int padding = (4 - ((bmp.dib.width + 1) / 2) % 4) % 4;
    fseek(file, bmp.header.dataOffset, SEEK_SET);

    bmp.Pixel_arr.pixel = new uint8_t * [bmp.dib.height];
    for (int i = 0; i < bmp.dib.height; i++) {
        bmp.Pixel_arr.pixel[i] = new uint8_t[bmp.dib.width];
    }

    uint8_t* row_buffer = new uint8_t[(bmp.dib.width + 1) / 2];

    int row_byte_count = (bmp.dib.width + 1) / 2;
    for (int iRow = 0; iRow < bmp.dib.height; iRow++) {
        fread(row_buffer, sizeof(uint8_t), row_byte_count, file);
        int iCol = 0;
        int bit_count = 0;
        while (iCol < bmp.dib.width) {
            uint8_t pixel = (row_buffer[bit_count / 2] >> ((1 - bit_count % 2) * 4)) & 0x0F;
            bmp.Pixel_arr.pixel[(bmp.dib.height - iRow - 1)][iCol] = pixel;
            bit_count++;
            iCol++;
        }
        fseek(file, padding, SEEK_CUR);
    }
    delete[] row_buffer;
}

bool readBMP_4(const char* filename, BMP_CT& bmp) {

    FILE* file = fopen(filename, "rb");
    if (!isBMPfile(file)) {
        fclose(file);
        return false;
    }

    readBitmapHeader(file, bmp.header);
    readBitmapDIB(file, bmp.dib);
    readColorTable(file, bmp);
    readBitmapPixel_4(file, bmp);

    fclose(file);

    return true;
}

void writePixelArray_4(FILE* file, BMP_CT bmp, int padding) {
    int row_byte_count = (bmp.dib.width + 1) / 2;
    for (int i = bmp.dib.height - 1; i >= 0; i--) {
        for (int j = 0; j < row_byte_count; j++) {
            uint8_t pixel = bmp.Pixel_arr.pixel[i][j];
            uint8_t pixel_high = (pixel & 0xF0) >> 4;
            uint8_t pixel_low = pixel & 0x0F;
            uint8_t new_pixel = (pixel_high << 4) | pixel_low;
            fwrite(&new_pixel, sizeof(uint8_t), 1, file);
        }
        char zero = 0; // padding
        for (int j = 0; j < padding; j++) {
            fwrite(&zero, sizeof(char), 1, file);
        }
    }
}

void writeBMPfile_4(const char* filename, BMP_CT b)
{
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    writeHeaderBMP(file, b.header);
    writeDIB(file, b.dib);

    int padding = (4 - ((b.dib.width + 1) / 2) % 4) % 4;
    writeColorTable(file, b);
    writePixelArray_4(file, b, padding);

    fclose(file);
}

void CopyHeader_4(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width)
{
    dest.header = src.header;
    int paddingCount = (4 - ((width + 1) / 2) % 4) % 4;
    dest.header.sizeBMP = (width * (src.dib.Color_depth / 8) + paddingCount) * height + src.header.dataOffset;
}

void CopyDIB_4(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width)
{
    dest.dib = src.dib;
    dest.dib.height = height;
    dest.dib.width = width;
    dest.dib.Color_depth = 4;
    int paddingCount = (4 - ((width * dest.dib.Color_depth / 8) % 4)) % 4;
    dest.dib.PixelArray_size = (width * dest.dib.Color_depth / 8 + paddingCount) * height;
}

void CopyPixels_4(BMP_CT& dest, const BMP_CT& src, uint32_t height, uint32_t width) {
    dest.dib.width = width;
    dest.dib.height = height;
    dest.Pixel_arr.pixel = new uint8_t * [height];

    int paddingCount = (4 - (width % 2)) % 4;

    for (uint32_t i = 0; i < height; i++) {
        dest.Pixel_arr.pixel[i] = new uint8_t[width / 2 + paddingCount];
        for (uint32_t j = 0; j < width; j += 2) {
            uint8_t pixel1 = src.Pixel_arr.pixel[i][(j + 1) / 2];
            uint8_t pixel2 = src.Pixel_arr.pixel[i][j / 2];
            uint8_t value = (pixel1 << 4) | pixel2;
            dest.Pixel_arr.pixel[i][j / 2] = value;
        }
        for (int j = 1; j <= paddingCount; j++) {
            dest.Pixel_arr.pixel[i][width / 2 + paddingCount - j] = 0;
        }
    }
}

BMP_CT cutBMPfile_4(BMP_CT bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum) {
    BMP_CT b_new;
    CopyHeader_4(b_new, bmp, sizeRow, sizeColum);
    CopyDIB_4(b_new, bmp, sizeRow, sizeColum);

    PixelArray_CT data = bmp.Pixel_arr, data_new;

    int paddingCount = (4 - (sizeColum % 2)) % 4;

    data_new.pixel = new uint8_t * [sizeRow];
    for (int i = 0; i < sizeRow; i++) {
        data_new.pixel[i] = new uint8_t[sizeColum / 2 + paddingCount]();
        int byte_count = 0;
        uint8_t byte_value = 0;

        for (int j = 0; j < sizeColum; j += 2) {
            uint8_t pixel1 = data.pixel[i + startRow][j + startColum];
            uint8_t pixel2 = data.pixel[i + startRow][j + startColum + 1];
            uint8_t value = (pixel2 << 4) | pixel1;
            data_new.pixel[i][byte_count] = value;
            byte_count++;
        }

        for (int j = 1; j <= paddingCount; j++) {
            data_new.pixel[i][sizeColum / 2 + paddingCount - j] = 0;
        }
    }

    b_new.Pixel_arr = data_new;
    return b_new;
}

void splitBMPfile_4(BMP_CT bmp, const char* src_file_path, int parts_in_height, int parts_in_width) {
    BMP_CT** bmp_parts = new BMP_CT * [parts_in_height];
    for (int i = 0; i < parts_in_height; i++) {
        bmp_parts[i] = new BMP_CT[parts_in_width];
    }

    int part_height = bmp.dib.height / parts_in_height;
    int part_width = bmp.dib.width / parts_in_width;

    int x = 0;
    for (int iRow = 0; iRow < parts_in_height; iRow++) {
        int y = 0;
        for (int iCol = 0; iCol < parts_in_width; iCol++) {
            BMP_CT bmp_temp = cutBMPfile_4(bmp, x, y, part_height, part_width);
            CopyHeader_4(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyDIB_4(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyColorTable(bmp_parts[iRow][iCol], bmp);
            CopyPixels_4(bmp_parts[iRow][iCol], bmp_temp, part_height, part_width);
            y += part_width;
        }
        x += part_height;
    }


    char basename[1024];
    sprintf(basename, "%s_part", get_filename(src_file_path));
    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            char output_filename[1024];
            sprintf(output_filename, "%s%02d.bmp", basename, i * parts_in_width + j + 1);

            writeBMPfile_4(output_filename, bmp_parts[i][j]);
        }
    }

    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            delete[] bmp_parts[i][j].Pixel_arr.pixel;
        }
        delete[] bmp_parts[i];
    }
    delete[] bmp_parts;
}

//////////////////////////////////////////////////////

///////////////// file BMP 8 bit ////////////////////

////////////////////////////////////////////////////

void readBitmapPixel_8(FILE* file, BMP_CT& bmp) {
    int padding = (4 - ((bmp.dib.width) % 4)) % 4;
    fseek(file, bmp.header.dataOffset, SEEK_SET);


    bmp.Pixel_arr.pixel = new uint8_t * [bmp.dib.height];
    for (int i = 0; i < bmp.dib.height; i++) {
        bmp.Pixel_arr.pixel[i] = new uint8_t[bmp.dib.width];
    }

    uint8_t* row_buffer = new uint8_t[bmp.dib.width];
    for (int iRow = 0; iRow < bmp.dib.height; iRow++) {
        fread(row_buffer, sizeof(uint8_t), bmp.dib.width, file);
        for (int iCol = 0; iCol < bmp.dib.width; iCol++) {
            bmp.Pixel_arr.pixel[(bmp.dib.height - iRow - 1)][iCol] = row_buffer[iCol];
        }
        fseek(file, padding, SEEK_CUR);
    }
    delete[] row_buffer;
}

bool readBMP_8(const char* filename, BMP_CT& bmp) {
    FILE* file = fopen(filename, "rb");
    if (!isBMPfile(file)) {
        fclose(file);
        return false;
    }

    readBitmapHeader(file, bmp.header);
    readBitmapDIB(file, bmp.dib);
    readColorTable(file, bmp);
    readBitmapPixel_8(file, bmp);

    fclose(file);

    return true;
}

void writePixelArray_8(FILE* file, BMP_CT bmp, int padding) {
    for (int i = bmp.dib.height - 1; i >= 0; i--) {
        fwrite(bmp.Pixel_arr.pixel[i], sizeof(uint8_t), bmp.dib.width, file);
        char zero = 0;
        for (int j = 0; j < padding; j++) {
            fwrite(&zero, sizeof(char), 1, file);
        }
    }
}

void writeBMPfile_8(const char* filename, BMP_CT b)
{
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    writeHeaderBMP(file, b.header);
    writeDIB(file, b.dib);

    int padding = (4 - (b.dib.width * (b.dib.Color_depth / 8) % 4)) % 4;

    writeColorTable(file, b);
    writePixelArray_8(file, b, padding);

    fclose(file);
}

void CopyHeader_8(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width)
{
    dest.header = src.header;
    int paddingCount = (4 - (width * (src.dib.Color_depth / 8) % 4)) % 4;
    dest.header.sizeBMP = (width * src.dib.Color_depth / 8 + paddingCount) * height + src.header.dataOffset;
}

void CopyDIB_8(BMP_CT& dest, BMP_CT src, uint32_t height, uint32_t width)
{
    dest.dib = src.dib;
    dest.dib.height = height;
    dest.dib.width = width;
    int paddingCount = (4 - (width * (src.dib.Color_depth / 8) % 4)) % 4;
    dest.dib.PixelArray_size = (width * src.dib.Color_depth / 8 + paddingCount) * height;

}

void CopyPixels_8(BMP_CT& dest, const BMP_CT& src, uint32_t height, uint32_t width) {
    dest.dib.width = width;
    dest.dib.height = height;
    dest.Pixel_arr.pixel = new uint8_t * [height];

    int paddingCount = (4 - (width % 4)) % 4;

    for (uint32_t i = 0; i < height; i++) {
        dest.Pixel_arr.pixel[i] = new uint8_t[width];
        memcpy(dest.Pixel_arr.pixel[i], (src.Pixel_arr.pixel[i]), width * sizeof(uint8_t));
        if (paddingCount != 0) {
            memset((void*)(dest.Pixel_arr.pixel[i] + width), 0, paddingCount * sizeof(uint8_t));
        }
    }
}

BMP_CT cutBMPfile_8(BMP_CT bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum)
{
    BMP_CT b_new;
    CopyHeader_8(b_new, bmp, sizeRow, sizeColum);
    CopyDIB_8(b_new, bmp, sizeRow, sizeColum);
    //Copy Pixels Array

    PixelArray_CT data = bmp.Pixel_arr, data_new;
    //data_new.height = sizeRow;
    //data_new.width = sizeColum;

    data_new.pixel = new uint8_t * [sizeRow];
    for (int i = 0; i < sizeRow; i++) {
        data_new.pixel[i] = new uint8_t[sizeColum];
        for (int j = 0; j < sizeColum; j++)
            data_new.pixel[i][j] = data.pixel[i + startRow][j + startColum];
    }

    b_new.Pixel_arr = data_new;

    return b_new;
}

void splitBMPfile_8(BMP_CT bmp, const char* src_file_path, int parts_in_height, int parts_in_width) {
    BMP_CT** bmp_parts = new BMP_CT * [parts_in_height];
    for (int i = 0; i < parts_in_height; i++) {
        bmp_parts[i] = new BMP_CT[parts_in_width];
    }

    int part_height = bmp.dib.height / parts_in_height;
    int part_width = bmp.dib.width / parts_in_width;

    int x = 0;
    for (int iRow = 0; iRow < parts_in_height; iRow++) {
        int y = 0;
        for (int iCol = 0; iCol < parts_in_width; iCol++) {
            BMP_CT bmp_temp = cutBMPfile_8(bmp, x, y, part_height, part_width);
            CopyHeader_8(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyDIB_8(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyColorTable(bmp_parts[iRow][iCol], bmp);
            CopyPixels_8(bmp_parts[iRow][iCol], bmp_temp, part_height, part_width);
            y += part_width;
        }
        x += part_height;
    }


    char basename[1024];
    sprintf(basename, "%s_part", get_filename(src_file_path));
    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            char output_filename[1024];
            sprintf(output_filename, "%s%02d.bmp", basename, i * parts_in_width + j + 1);

            writeBMPfile_8(output_filename, bmp_parts[i][j]);
        }
    }

    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            delete[] bmp_parts[i][j].Pixel_arr.pixel;
        }
        delete[] bmp_parts[i];
    }
    delete[] bmp_parts;
}

///////////////////////////////////////////////////////

///////////////// file BMP 24 bit ////////////////////

////////////////////////////////////////////////////

void readBitmapPixel_24(FILE* file, PixelArray_24* pixel_arr) {
    int padding = (4 - ((pixel_arr->width * (sizeof(Color))) % 4)) % 4;
    fseek(file, sizeof(Header) + sizeof(DIB), SEEK_SET);

    // Allocate memory for the pixel array row by row
    pixel_arr->pixel = (Color**)calloc(pixel_arr->height, sizeof(Color*));
    for (int i = 0; i < pixel_arr->height; i++) {
        pixel_arr->pixel[i] = (Color*)calloc(pixel_arr->width, sizeof(Color));
    }

    Color* row_buffer = new Color[pixel_arr->width];
    for (int iRow = 0; iRow < pixel_arr->height; iRow++) {
        fread(row_buffer, sizeof(Color), pixel_arr->width, file);
        for (int iCol = 0; iCol < pixel_arr->width; iCol++) {
            pixel_arr->pixel[(pixel_arr->height - iRow - 1)][iCol] = row_buffer[iCol];
        }
        fseek(file, padding, SEEK_CUR);
    }
    delete[] row_buffer;

}

bool readBMP_24(const char* filename, BMP_24& bmp) {

    FILE* file = fopen(filename, "rb");
    if (!isBMPfile(file)) {
        fclose(file);
        return false;
    }

    readBitmapHeader(file, bmp.header);
    readBitmapDIB(file, bmp.dib);

    bmp.Pixel_arr.height = bmp.dib.height;
    bmp.Pixel_arr.width = bmp.dib.width;
    readBitmapPixel_24(file, &bmp.Pixel_arr);

    fclose(file);

    return true;
}

void freeBMP_24(BMP_24& bmp) {
    // Free the pixel array row by row
    for (int i = 0; i < bmp.Pixel_arr.height; i++) {
        free(bmp.Pixel_arr.pixel[i]);
        bmp.Pixel_arr.pixel[i] = nullptr;
    }
    // Free the pixel array
    free(bmp.Pixel_arr.pixel);
    bmp.Pixel_arr.pixel = nullptr;

}

void writePixelArray_24(FILE* file, PixelArray_24 pixel_arr, int padding) {

    for (int i = pixel_arr.height - 1; i >= 0; i--) {
        fwrite(pixel_arr.pixel[i], sizeof(Color), pixel_arr.width, file);
        char zero = 0;
        for (int j = 0; j < padding; j++) {
            fwrite(&zero, sizeof(char), 1, file);
        }
    }

}

void writeBMPfile_24(const char* filename, BMP_24 b)
{
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    writeHeaderBMP(file, b.header);
    writeDIB(file, b.dib);

    int padding = (4 - (b.dib.width * (b.dib.Color_depth / 8) % 4)) % 4;

    writePixelArray_24(file, b.Pixel_arr, padding);

    fclose(file);
}

void CopyHeader_24(BMP_24& dest, BMP_24 src, uint32_t height, uint32_t width)
{
    dest.header = src.header;
    int paddingCount = (4 - (width * (src.dib.Color_depth / 8) % 4)) % 4;
    dest.header.sizeBMP = (width * src.dib.Color_depth / 8 + paddingCount) * height + src.header.dataOffset;
}

void CopyDIB_24(BMP_24& dest, BMP_24 src, uint32_t height, uint32_t width)
{
    dest.dib = src.dib;
    dest.dib.height = height;
    dest.dib.width = width;
    int paddingCount = (4 - (width * (src.dib.Color_depth / 8) % 4)) % 4;
    dest.dib.PixelArray_size = (width * src.dib.Color_depth / 8 + paddingCount) * height;

}

void CopyPixels_24(BMP_24& dest, const BMP_24& src, uint32_t height, uint32_t width) {
    dest.Pixel_arr.width = width;
    dest.Pixel_arr.height = height;
    dest.Pixel_arr.pixel = new Color * [height];

    int paddingCount = (4 - (width * 3 % 4)) % 4;

    for (uint32_t i = 0; i < height; i++) {
        dest.Pixel_arr.pixel[i] = new Color[width];
        memcpy(dest.Pixel_arr.pixel[i], (src.Pixel_arr.pixel[i]), width * sizeof(Color));
        if (paddingCount != 0) {
            memset((void*)(dest.Pixel_arr.pixel[i] + width), 0, paddingCount * sizeof(Color));
        }
    }
}

BMP_24 cutBMPfile_24(BMP_24 bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum)
{
    BMP_24 b_new;
    CopyHeader_24(b_new, bmp, sizeRow, sizeColum);
    CopyDIB_24(b_new, bmp, sizeRow, sizeColum);
    //Copy Pixels Array

    PixelArray_24 data = bmp.Pixel_arr, data_new;
    data_new.height = sizeRow;
    data_new.width = sizeColum;

    data_new.pixel = new Color * [sizeRow];
    for (int i = 0; i < sizeRow; i++) {
        data_new.pixel[i] = new Color[sizeColum];
        for (int j = 0; j < sizeColum; j++)
            data_new.pixel[i][j] = data.pixel[i + startRow][j + startColum];
    }

    b_new.Pixel_arr = data_new;

    return b_new;
}

void splitBMPfile_24(BMP_24 bmp, const char* src_file_path, int parts_in_height, int parts_in_width) {
    BMP_24** bmp_parts = new BMP_24 * [parts_in_height];
    for (int i = 0; i < parts_in_height; i++) {
        bmp_parts[i] = new BMP_24[parts_in_width];
    }

    int part_height = bmp.dib.height / parts_in_height;
    int part_width = bmp.dib.width / parts_in_width;

    int x = 0;
    for (int iRow = 0; iRow < parts_in_height; iRow++) {
        int y = 0;
        for (int iCol = 0; iCol < parts_in_width; iCol++) {
            BMP_24 bmp_temp = cutBMPfile_24(bmp, x, y, part_height, part_width);
            CopyHeader_24(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyDIB_24(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyPixels_24(bmp_parts[iRow][iCol], bmp_temp, part_height, part_width);
            y += part_width;
        }
        x += part_height;
    }


    char basename[1024];
    sprintf(basename, "%s_part", get_filename(src_file_path));
    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            char output_filename[1024];
            sprintf(output_filename, "%s%02d.bmp", basename, i * parts_in_width + j + 1);

            writeBMPfile_24(output_filename, bmp_parts[i][j]);
        }
    }

    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            delete[] bmp_parts[i][j].Pixel_arr.pixel;
        }
        delete[] bmp_parts[i];
    }
    delete[] bmp_parts;
}

///////////////////////////////////////////////////////

///////////////// file BMP 32 bit ////////////////////

////////////////////////////////////////////////////

void readBitmapPixel_32(FILE* file, PixelArray_32* pixel_arr) {
    int padding = (4 - ((pixel_arr->width * (sizeof(Color_t))) % 4)) % 4;

    // Allocate memory for the pixel array row by row
    pixel_arr->pixel = (Color_t**)calloc(pixel_arr->height, sizeof(Color_t*));
    for (int i = 0; i < pixel_arr->height; i++) {
        pixel_arr->pixel[i] = (Color_t*)calloc(pixel_arr->width, sizeof(Color_t));
    }

    Color_t* row_buffer = new Color_t[pixel_arr->width];
    for (int iRow = 0; iRow < pixel_arr->height; iRow++) {
        fread(row_buffer, sizeof(Color_t), pixel_arr->width, file);
        for (int iCol = 0; iCol < pixel_arr->width; iCol++) {
            pixel_arr->pixel[(pixel_arr->height - iRow - 1)][iCol] = row_buffer[iCol];
        }
        fseek(file, padding, SEEK_CUR);
    }
    delete[] row_buffer;

}

bool readBMP_32(const char* filename, BMP_32& bmp) {

    FILE* file = fopen(filename, "rb");
    if (!isBMPfile(file)) {
        fclose(file);
        return false;
    }

    readBitmapHeader(file, bmp.header);
    readBitmapDIB(file, bmp.dib);

    bmp.Pixel_arr.height = bmp.dib.height;
    bmp.Pixel_arr.width = bmp.dib.width;
    readBitmapPixel_32(file, &bmp.Pixel_arr);

    fclose(file);

    return true;
}

void freeBMP_32(BMP_32& bmp) {
    // Free the pixel array row by row
    for (int i = 0; i < bmp.Pixel_arr.height; i++) {
        free(bmp.Pixel_arr.pixel[i]);
        bmp.Pixel_arr.pixel[i] = nullptr;
    }
    // Free the pixel array
    free(bmp.Pixel_arr.pixel);
    bmp.Pixel_arr.pixel = nullptr;

}

void writePixelArray_32(FILE* file, PixelArray_32 pixel_arr, int padding) {
    for (int i = pixel_arr.height - 1; i >= 0; i--) {
        fwrite(pixel_arr.pixel[i], sizeof(Color_t), pixel_arr.width, file);
        char zero = 0;
        for (int j = 0; j < padding; j++) {
            fwrite(&zero, sizeof(char), 1, file);
        }
    }

}

void writeBMPfile_32(const char* filename, BMP_32 b)
{
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    writeHeaderBMP(file, b.header);
    writeDIB(file, b.dib);

    int padding = (4 - (b.dib.width * (b.dib.Color_depth / 8) % 4)) % 4;

    writePixelArray_32(file, b.Pixel_arr, padding);

    fclose(file);
}

void CopyHeader_32(BMP_32& dest, BMP_32 src, uint32_t height, uint32_t width)
{
    dest.header = src.header;
    int paddingCount = (4 - (width * (src.dib.Color_depth / 8) % 4)) % 4;
    dest.header.sizeBMP = (width * src.dib.Color_depth / 8 + paddingCount) * height + src.header.dataOffset;
}

void CopyDIB_32(BMP_32& dest, BMP_32 src, uint32_t height, uint32_t width)
{
    dest.dib = src.dib;
    dest.dib.height = height;
    dest.dib.width = width;
    int paddingCount = (4 - (width * (src.dib.Color_depth / 8) % 4)) % 4;
    dest.dib.PixelArray_size = (width * src.dib.Color_depth / 8 + paddingCount) * height;

}

void CopyPixels_32(BMP_32& dest, const BMP_32& src, uint32_t height, uint32_t width) {
    dest.Pixel_arr.width = width;
    dest.Pixel_arr.height = height;
    dest.Pixel_arr.pixel = new Color_t * [height];

    int paddingCount = (4 - (width * 4 % 4)) % 4;

    for (uint32_t i = 0; i < height; i++) {
        dest.Pixel_arr.pixel[i] = new Color_t[width];
        memcpy(dest.Pixel_arr.pixel[i], (src.Pixel_arr.pixel[i]), width * sizeof(Color_t));
        if (paddingCount != 0) {
            memset((void*)(dest.Pixel_arr.pixel[i] + width), 0, paddingCount * sizeof(Color_t));
        }
    }
}

BMP_32 cutBMPfile_32(BMP_32 bmp, uint32_t startRow, uint32_t startColum, uint32_t sizeRow, uint32_t sizeColum)
{
    BMP_32 b_new;
    CopyHeader_32(b_new, bmp, sizeRow, sizeColum);
    CopyDIB_32(b_new, bmp, sizeRow, sizeColum);
    //Copy Pixels Array

    PixelArray_32 data = bmp.Pixel_arr, data_new;
    data_new.height = sizeRow;
    data_new.width = sizeColum;

    data_new.pixel = new Color_t * [sizeRow];
    for (int i = 0; i < sizeRow; i++) {
        data_new.pixel[i] = new Color_t[sizeColum];
        for (int j = 0; j < sizeColum; j++)
            data_new.pixel[i][j] = data.pixel[i + startRow][j + startColum];
    }

    b_new.Pixel_arr = data_new;

    return b_new;
}

void splitBMPfile_32(BMP_32 bmp, const char* src_file_path, int parts_in_height, int parts_in_width) {
    BMP_32** bmp_parts = new BMP_32 * [parts_in_height];
    for (int i = 0; i < parts_in_height; i++) {
        bmp_parts[i] = new BMP_32[parts_in_width];
    }

    int part_height = bmp.dib.height / parts_in_height;
    int part_width = bmp.dib.width / parts_in_width;

    int x = 0;
    for (int iRow = 0; iRow < parts_in_height; iRow++) {
        int y = 0;
        for (int iCol = 0; iCol < parts_in_width; iCol++) {
            BMP_32 bmp_temp = cutBMPfile_32(bmp, x, y, part_height, part_width);
            CopyHeader_32(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyDIB_32(bmp_parts[iRow][iCol], bmp, part_height, part_width);
            CopyPixels_32(bmp_parts[iRow][iCol], bmp_temp, part_height, part_width);
            y += part_width;
        }
        x += part_height;
    }


    char basename[1024];
    sprintf(basename, "%s_part", get_filename(src_file_path));
    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            char output_filename[1024];
            sprintf(output_filename, "%s%02d.bmp", basename, i * parts_in_width + j + 1);

            writeBMPfile_32(output_filename, bmp_parts[i][j]);
        }
    }

    for (int i = 0; i < parts_in_height; i++) {
        for (int j = 0; j < parts_in_width; j++) {
            delete[] bmp_parts[i][j].Pixel_arr.pixel;
        }
        delete[] bmp_parts[i];
    }
    delete[] bmp_parts;
}

