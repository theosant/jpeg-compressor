#ifndef JPEG_H
#define JPEG_H

#include "bmp.h"
#include "huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    float Y, Cb, Cr;
} PixelYCbCr;

typedef struct {
    char name[4];        // Identificador do arquivo
    uint32_t width;       // largura da imagem
    uint32_t height;      // altura da imagem
    uint32_t dataSize;    // tamanho dos dados comprimidos em bytes
} JLSHeader;

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader);
Pixel* convertYCbCrToRgb(PixelYCbCr* imagem_ycbcr, int largura, int altura);
BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos);

#endif