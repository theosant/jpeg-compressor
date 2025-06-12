#ifndef JPEG_H
#define JPEG_H

#include "bmp.h"

typedef struct {
    float Y, Cb, Cr;
} PixelYCbCr;

#define BLOCK_SIZE 8

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader);
Pixel* convertYCbCrToRgb(PixelYCbCr *input, BitmapInfoHeader infoHeader);
void downSampling(PixelYCbCr *input, BitmapInfoHeader infoHeader, double **CbDown, double **CrDown);
void upSampling(double *CbDown, double *CrDown, BitmapInfoHeader infoHeader, double **CbFull, double **CrFull);
void precalcC(double C[BLOCK_SIZE][BLOCK_SIZE]);
void applyDctToImage(double* image, int width, int height, double* dctCoeffs, double C[BLOCK_SIZE][BLOCK_SIZE]);
void applyIdctToImage(double* dctCoeffs, int width, int height, double* image, double C[BLOCK_SIZE][BLOCK_SIZE]);

#endif