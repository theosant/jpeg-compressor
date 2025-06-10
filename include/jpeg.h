#ifndef JPEG_H
#define JPEG_H

#include "bmp.h"
#include "huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float Y, Cb, Cr;
} PixelYCbCr;

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader);
BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos);

#endif