#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"
#include <math.h>



PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader) {
    int totalPixels = infoHeader.width * infoHeader.height;
    PixelYCbCr *converted = malloc(totalPixels * sizeof(PixelYCbCr));
    if (!converted) return NULL;

    for (int i = 0; i < totalPixels; i++) {
        double r = input[i].R;
        double g = input[i].G;
        double b = input[i].B;

        converted[i].Y  = 0.299 * r + 0.587 * g + 0.114 * b;
        converted[i].Cb = -0.1687 * r - 0.3313 * g + 0.5 * b + 128;
        converted[i].Cr =  0.5 * r - 0.4187 * g - 0.0813 * b + 128;
    }

    return converted;
}

Pixel* convertYCbCrToRgb(PixelYCbCr *input, BitmapInfoHeader infoHeader) {
    int totalPixels = infoHeader.width * infoHeader.height;
    Pixel *rgbImage = malloc(totalPixels * sizeof(Pixel));
    if (!rgbImage) return NULL;

    for (int i = 0; i < totalPixels; i++) {
        double Y = input[i].Y;
        double Cb = input[i].Cb - 128;
        double Cr = input[i].Cr - 128;

        double r  = Y + 1.402 * Cr;
        double g = Y - 0.344136 * Cb  - 0.714136 * Cr;
        double b =  Y + 1.772 * Cb;

        if (r < 0) r = 0; 
        if (r > 255) r = 255;
        if (g < 0) g = 0; 
        if (g > 255) g = 255;
        if (b < 0) b = 0; 
        if (b > 255) b = 255;

        rgbImage[i].R = (unsigned char)(r + 0.5);
        rgbImage[i].G = (unsigned char)(g + 0.5);
        rgbImage[i].B = (unsigned char)(b + 0.5); 
    }

    return rgbImage;
}

void downSampling(PixelYCbCr *input, BitmapInfoHeader infoHeader, double **CbDown, double **CrDown){
    int width = infoHeader.width;
    int height =  infoHeader.height;

    *CbDown = malloc((width / 2) * (height / 2) * sizeof(double));
    *CrDown = malloc((width / 2) * (height / 2) * sizeof(double));
    if (!*CbDown || !*CrDown) {
        fprintf(stderr, "Erro ao alocar memória para downsampling.\n");
        exit(1);
    }
    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x += 2) {
            int i00 = y * width + x;
            int i01 = y * width + (x + 1);
            int i10 = (y + 1) * width + x;
            int i11 = (y + 1) * width + (x + 1);

            double avgCb = (input[i00].Cb + input[i01].Cb + input[i10].Cb + input[i11].Cb) / 4.0;
            double avgCr = (input[i00].Cr + input[i01].Cr + input[i10].Cr + input[i11].Cr) / 4.0;

            int downIndex = (y / 2) * (width / 2) + (x / 2);
            (*CbDown)[downIndex] = avgCb;
            (*CrDown)[downIndex] = avgCr;
        }
    }    
}  

void upSampling(double *CbDown, double *CrDown, BitmapInfoHeader infoHeader, double **CbFull, double **CrFull) {
    int width = infoHeader.width;
    int height = infoHeader.height;

    int adjustedWidth = width - (width % 2);
    int adjustedHeight = height - (height % 2);
    int downWidth = adjustedWidth / 2;
    int downHeight = adjustedHeight / 2;
    
    *CbFull = malloc(adjustedWidth * adjustedHeight * sizeof(double));
    *CrFull = malloc(adjustedWidth * adjustedHeight * sizeof(double));
    if (!*CbFull || !*CrFull) {
        fprintf(stderr, "Erro ao alocar memória para upsampling.\n");
        exit(1);
    }
    for (int y = 0; y < downHeight; y++) {
        for (int x = 0; x < downWidth; x++) {
            int downIndex = y * downWidth + x;
            double cbVal = CbDown[downIndex];
            double crVal = CrDown[downIndex];

            int baseY = y * 2;
            int baseX = x * 2;

            int i00 = baseY * adjustedWidth + baseX;
            int i01 = baseY * adjustedWidth + (baseX + 1);
            int i10 = (baseY + 1) * adjustedWidth + baseX;
            int i11 = (baseY + 1) * adjustedWidth + (baseX + 1);


            (*CbFull)[i00] = cbVal;
            (*CbFull)[i01] = cbVal;
            (*CbFull)[i10] = cbVal;
            (*CbFull)[i11] = cbVal;

            (*CrFull)[i00] = crVal;
            (*CrFull)[i01] = crVal;
            (*CrFull)[i10] = crVal;
            (*CrFull)[i11] = crVal;
        }
    }
}

void precalcC(double C[8][8]) {
    double values[8][8] = {
        {0.354,  0.354,  0.354,  0.354,  0.354,  0.354,  0.354,  0.354},
        {0.490,  0.416,  0.278,  0.098, -0.098, -0.278, -0.416, -0.490},
        {0.462,  0.191, -0.191, -0.462, -0.462, -0.191,  0.191,  0.462},
        {0.416, -0.098, -0.490, -0.278,  0.278,  0.490,  0.098, -0.416},
        {0.354, -0.354, -0.354,  0.354,  0.354, -0.354, -0.354,  0.354},
        {0.278, -0.490,  0.098,  0.416, -0.416, -0.098,  0.490, -0.278},
        {0.191, -0.462,  0.462, -0.191, -0.191,  0.462, -0.462,  0.191},
        {0.098, -0.278,  0.416, -0.490,  0.490, -0.416,  0.278, -0.098}
    };

    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            C[i][j] = values[i][j];
}

void dctBlock8x8Matrix(double P[BLOCK_SIZE][BLOCK_SIZE], double F[BLOCK_SIZE][BLOCK_SIZE], double C[BLOCK_SIZE][BLOCK_SIZE]) {
    double temp[BLOCK_SIZE][BLOCK_SIZE] = {0};

    // Produto C * P
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            for (int k = 0; k < BLOCK_SIZE; k++) {
                temp[i][j] += C[i][k] * P[k][j];
            }
        }
    }

    // Produto temp * C^T
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            F[i][j] = 0.0;
            for (int k = 0; k < BLOCK_SIZE; k++) {
                F[i][j] += temp[i][k] * C[k][j]; 
            }
        }
    }
}


void applyDctToImage(double* image, int width, int height, double* dctCoeffs, double C[BLOCK_SIZE][BLOCK_SIZE]) {
    for (int i = 0; i < height; i += BLOCK_SIZE) {
        for (int j = 0; j < width; j += BLOCK_SIZE) {
            double P[BLOCK_SIZE][BLOCK_SIZE] = {0};
            double F[BLOCK_SIZE][BLOCK_SIZE] = {0};

            // Copia e centraliza
            for (int x = 0; x < BLOCK_SIZE; x++) {
                for (int y = 0; y < BLOCK_SIZE; y++) {
                    int xi = i + x;
                    int yj = j + y;
                    if (xi < height && yj < width) {
                        P[x][y] = image[xi * width + yj] - 128.0;
                    }
                }
            }

            dctBlock8x8Matrix(P, F, C);

            // Copia F de volta
            for (int x = 0; x < BLOCK_SIZE; x++) {
                for (int y = 0; y < BLOCK_SIZE; y++) {
                    int xi = i + x;
                    int yj = j + y;
                    if (xi < height && yj < width) {
                        dctCoeffs[xi * width + yj] = F[x][y];
                    }
                }
            }
        }
    }
}

void idctBlock8x8Matrix(double F[BLOCK_SIZE][BLOCK_SIZE], double P[BLOCK_SIZE][BLOCK_SIZE], double C[BLOCK_SIZE][BLOCK_SIZE]) {
    double temp[BLOCK_SIZE][BLOCK_SIZE] = {0};

    // Produto C^T * F
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            for (int k = 0; k < BLOCK_SIZE; k++) {
                temp[i][j] += C[k][i] * F[k][j];  // C^T: índice invertido
            }
        }
    }

    // Produto temp * C
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            P[i][j] = 0.0;
            for (int k = 0; k < BLOCK_SIZE; k++) {
                P[i][j] += temp[i][k] * C[k][j];
            }
            // Opcional: arredondar e recentralizar
            P[i][j] += 128.0;
            if (P[i][j] < 0) P[i][j] = 0;
            if (P[i][j] > 255) P[i][j] = 255;
        }
    }
}

void applyIdctToImage(double* dctCoeffs, int width, int height, double* image, double C[BLOCK_SIZE][BLOCK_SIZE]) {
    for (int i = 0; i < height; i += BLOCK_SIZE) {
        for (int j = 0; j < width; j += BLOCK_SIZE) {
            double F[BLOCK_SIZE][BLOCK_SIZE] = {0};
            double P[BLOCK_SIZE][BLOCK_SIZE] = {0};

            // Copia F
            for (int x = 0; x < BLOCK_SIZE; x++) {
                for (int y = 0; y < BLOCK_SIZE; y++) {
                    int xi = i + x;
                    int yj = j + y;
                    if (xi < height && yj < width) {
                        F[x][y] = dctCoeffs[xi * width + yj];
                    }
                }
            }

            idctBlock8x8Matrix(F, P, C);

            // Copia P de volta
            for (int x = 0; x < BLOCK_SIZE; x++) {
                for (int y = 0; y < BLOCK_SIZE; y++) {
                    int xi = i + x;
                    int yj = j + y;
                    if (xi < height && yj < width) {
                        image[xi * width + yj] = P[x][y];
                    }
                }
            }
        }
    }
}

