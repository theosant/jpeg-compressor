#ifndef JPEG_H
#define JPEG_H

#include "bmp.h"

typedef struct {
    float Y, Cb, Cr;
} PixelYCbCr;

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader);

#endif