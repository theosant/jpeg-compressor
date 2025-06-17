#ifndef LOSSY_H
#define LOSSY_H

#include "jpeg.h"
#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#endif 