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

#define BLOCK_SIZE 8

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader);
BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos);
void DPCM(BlocoYCbCr* blocos, int num_blocos);
void comprimeBloco(BlocoYCbCr bloco, TabelaHuffman* tabela_Y, TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr, FILE* output);
void comprimirJPEGSemPerdas(const char* input_bmp, const char* output_jpeg);
Pixel* convertYCbCrToRgb(PixelYCbCr *input, BitmapInfoHeader infoHeader);
void downSampling(PixelYCbCr *input, BitmapInfoHeader infoHeader, double **CbDown, double **CrDown);
void upSampling(double *CbDown, double *CrDown, BitmapInfoHeader infoHeader, double **CbFull, double **CrFull);
void precalcC(double C[BLOCK_SIZE][BLOCK_SIZE]);
void applyDctToImage(double* image, int width, int height, double* dctCoeffs, double C[BLOCK_SIZE][BLOCK_SIZE]);
void applyIdctToImage(double* dctCoeffs, int width, int height, double* image, double C[BLOCK_SIZE][BLOCK_SIZE]);
void quantizeImage(double* dctCoeffs, int* quantized, int width, int height, const int Q[8][8], double k);
void dequantizeImage(int* quantized, double* dctCoeffs, int width, int height, const int Q[8][8], double k);
#endif