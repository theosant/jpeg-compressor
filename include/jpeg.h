#ifndef JPEG_H
#define JPEG_H

#include "bmp.h"
#include "huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef struct {
    float Y, Cb, Cr;
} PixelYCbCr;

typedef struct {
    char name[4];                   // Assinatura do arquivo, ex: "JLS1"
    unsigned int width;             // Largura da imagem
    unsigned int height;            // Altura da imagem
    unsigned int dataSize;          // Número de blocos 8x8
    BitmapFileHeader bmp_file_header;
    BitmapInfoHeader bmp_info_header;
    
} JLSHeader;

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader);
Pixel* convertYCbCrToRgb(PixelYCbCr* imagem_ycbcr, int largura, int altura);
BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos);

#endif