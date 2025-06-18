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

typedef struct {
    char name[4];        // Identificador do arquivo
    uint32_t width;       // largura da imagem
    uint32_t height;      // altura da imagem
    uint32_t dataSize;    // tamanho dos dados comprimidos em bytes
} JLSHeader;


typedef struct {
    int zeros;      // número de zeros antes
    int categoria;  // categoria JPEG (de 1 a 10)
    int mantissa;   // valor codificado
} RLEToken;

#define BLOCK_SIZE 8
#define K_FACTOR 1.0



PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader);
BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos);
void DPCM(BlocoYCbCr* blocos, int num_blocos);
void comprimeBloco(BlocoYCbCr bloco, TabelaHuffman* tabela_Y, TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr, FILE* output);
long comprimirJPEGSemPerdas(PixelYCbCr* imagem_ycbcr, int largura, int altura, const char* output_jpeg);
Pixel* convertYCbCrToRgb(PixelYCbCr *input, BitmapInfoHeader infoHeader);
void aplicarZigZagBloco(int *entrada, int largura, int x_bloco, int y_bloco, int vetor_saida[64]);
#endif