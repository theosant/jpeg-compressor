#ifndef LOSSY_H
#define LOSSY_H

#include "jpeg.h"
#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int zigzag_order[64] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

void downSampling(PixelYCbCr *input, BitmapInfoHeader infoHeader, double **CbDown, double **CrDown);
void upSampling(double *CbDown, double *CrDown, BitmapInfoHeader infoHeader, double **CbFull, double **CrFull);
void precalcC(double C[BLOCK_SIZE][BLOCK_SIZE]);
void applyDctToImage(double* image, int width, int height, double* dctCoeffs, double C[BLOCK_SIZE][BLOCK_SIZE]);
void applyIdctToImage(double* dctCoeffs, int width, int height, double* image, double C[BLOCK_SIZE][BLOCK_SIZE]);
void quantizeImage(double* dctCoeffs, int* quantized, int width, int height, const int Q[8][8], double k);
void dequantizeImage(int* quantized, double* dctCoeffs, int width, int height, const int Q[8][8], double k);
void processImageDCT(PixelYCbCr *converted,BitmapInfoHeader InfoHeader, int **quantized_Y_out,int **quantized_Cb_out,int **quantized_Cr_out);
void reconstructImageFromDCT(int *quantized_Y,int *quantized_Cb,int *quantized_Cr,
    BitmapInfoHeader InfoHeader,
    PixelYCbCr *converted
);
void processImageDCT(
    PixelYCbCr *converted, 
    BitmapInfoHeader InfoHeader, 
    int **quantized_Y_out, 
    int **quantized_Cb_out, 
    int **quantized_Cr_out
);
void codificarDC(int diff, FILE* out);
void codificarAC(int* ac, FILE* out);
void aplicarZigZag(int bloco[8][8], int vetor_saida[64]);
long entropy_encode(int* quantized_Y, int* quantized_Cb, int* quantized_Cr, int largura, int altura);
int calcularCategoria(int valor);

#endif 