#include <stdio.h>
#include "bmp.h"

BITMAPFILEHEADER leituraHeader(FILE *F) {
    BITMAPFILEHEADER H;

    fread(&H.Type, sizeof(unsigned short int), 1, F);
    fread(&H.Size, sizeof(unsigned int), 1, F);
    fread(&H.Reserved1, sizeof(unsigned short int), 1, F);
    fread(&H.Reserved2, sizeof(unsigned short int), 1, F);
    fread(&H.OffBits, sizeof(unsigned int), 1, F);

    return H;
}

BITMAPINFOHEADER leituraInfoHeader(FILE *F) {
    BITMAPINFOHEADER INFO_H;

    fread(&INFO_H.Size, sizeof(unsigned int), 1, F);
    fread(&INFO_H.Width, sizeof(int), 1, F);
    fread(&INFO_H.Height, sizeof(int), 1, F);
    fread(&INFO_H.Planes, sizeof(unsigned short int), 1, F);
    fread(&INFO_H.BitCount, sizeof(unsigned short int), 1, F);
    fread(&INFO_H.Compression, sizeof(unsigned int), 1, F);
    fread(&INFO_H.SizeImage, sizeof(unsigned int), 1, F);
    fread(&INFO_H.XResolution, sizeof(int), 1, F);
    fread(&INFO_H.YResolution, sizeof(int), 1, F);
    fread(&INFO_H.NColours, sizeof(unsigned int), 1, F);
    fread(&INFO_H.ImportantColours, sizeof(unsigned int), 1, F);

    return INFO_H;
}
