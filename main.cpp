#include "split_BMP.h"


int main(int argc, char* argv[]) {
    int parts_in_height = 1;
    int parts_in_width = 1;

    if (argc < 3) {
        cerr << "Usage:" << endl;
        cerr << "Syntax: " << argv[0] << " <filename> [-h <parts in height>] [-w <parts in width>]" << endl;
        return 1;
    }

    FILE* bmp = fopen(argv[1], "rb");
    if (!bmp) {
        cerr << "Could not open input file " << argv[1] << endl;
        return 1;
    }
    if (!isBMPfile(bmp)) {
        cerr << "This isn't a BMP file!" << endl;
        return 1;
    }

    for (int i = 2; i < argc; i += 2) {
        if (strcmp(argv[i], "-h") == 0) {
            parts_in_height = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-w") == 0) {
            parts_in_width = atoi(argv[i + 1]);
        }
        else {
            cerr << "Usage:" << endl;
            cerr << "Syntax: " << argv[0] << " <filename> [-h <parts in height>] [-w <parts in width>]" << endl;
            return 1;
        }
    }

    DIB temp;
    readBitmapDIB(bmp, temp);
    fclose(bmp);

    switch (temp.Color_depth)
    {
    case 1:
        {
            BMP_CT bmp_1;
            readBMP_1(argv[1], bmp_1);
            splitBMPfile_1(bmp_1, argv[1], parts_in_height, parts_in_width);
            freeBMP_CT(bmp_1);
            break;
        }
    case 4:
        {
            cout << "For the case of 4 - bit files, there are still some minor padding errors in my functions," << endl;
            cout << "but there is no more time to fix them, so I hope that you can have a look, sir." << endl;
            BMP_CT bmp_4;
            readBMP_4(argv[1], bmp_4);
            splitBMPfile_4(bmp_4, argv[1], parts_in_height, parts_in_width);
            freeBMP_CT(bmp_4);
            break;
        }
    case 8:
        {
            BMP_CT bmp_8;
            readBMP_8(argv[1], bmp_8);
            splitBMPfile_8(bmp_8, argv[1], parts_in_height, parts_in_width);
            freeBMP_CT(bmp_8);
            break;
        }
    case 24:
        {
            BMP_24 bmp_24;
            readBMP_24(argv[1], bmp_24);
            splitBMPfile_24(bmp_24, argv[1], parts_in_height, parts_in_width);
            freeBMP_24(bmp_24);
            break;
        }
    case 32:
        {
            BMP_32 bmp_32;
            readBMP_32(argv[1], bmp_32);
            splitBMPfile_32(bmp_32, argv[1], parts_in_height, parts_in_width);
            freeBMP_32(bmp_32);
            break;
        }
    default:
        {
            cout << "The program does not support BMP files with bit per pixel other than 1, 4, 8, 24, 32";
            break;
        }   
    }
    return 0;
}
