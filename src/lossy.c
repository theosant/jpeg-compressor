#include "lossy.h"

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
                F[i][j] += temp[i][k] * C[j][k]; 
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

void quantizeImage(double* dctCoeffs, int* quantized, int width, int height, const int Q[8][8], double k) {
    for (int i = 0; i < height; i += 8) {
        for (int j = 0; j < width; j += 8) {
            for (int u = 0; u < 8; u++) {
                for (int v = 0; v < 8; v++) {
                    int xi = i + u;
                    int yj = j + v;
                    if (xi < height && yj < width) {
                        int idx = xi * width + yj;
                        quantized[idx] = round(dctCoeffs[idx] / (k * Q[u][v]));
                    }
                }
            }
        }
    }
}

void dequantizeImage(int* quantized, double* dctCoeffs, int width, int height, const int Q[8][8], double k) {
    for (int i = 0; i < height; i += 8) {
        for (int j = 0; j < width; j += 8) {
            for (int u = 0; u < 8; u++) {
                for (int v = 0; v < 8; v++) {
                    int xi = i + u;
                    int yj = j + v;
                    if (xi < height && yj < width) {
                        int idx = xi * width + yj;
                        dctCoeffs[idx] = (double)quantized[idx] * (k * Q[u][v]);
                    }
                }
            }
        }
    }
}

void processImageDCT(
    PixelYCbCr *converted, 
    BitmapInfoHeader InfoHeader, 
    int **quantized_Y_out, 
    int **quantized_Cb_out, 
    int **quantized_Cr_out
) {
    // Downsampling dos canais Cb e Cr
    double *CbDown, *CrDown;
    downSampling(converted, InfoHeader, &CbDown, &CrDown);

    // Extrai canal Y para DCT
    int totalPixels = InfoHeader.width * InfoHeader.height;
    double *Y_component = malloc(totalPixels * sizeof(double));
    for (int i = 0; i < totalPixels; i++) {
        Y_component[i] = converted[i].Y;
    }

    // Prepara coeficientes da DCT
    double C[BLOCK_SIZE][BLOCK_SIZE];
    precalcC(C);

    // Aplica DCT
    int width_chroma = InfoHeader.width / 2;
    int height_chroma = InfoHeader.height / 2;
    int totalPixels_chroma = width_chroma * height_chroma;

    double *dctCoeffs_Y = malloc(totalPixels * sizeof(double));
    double *dctCoeffs_Cr = malloc(totalPixels_chroma * sizeof(double));
    double *dctCoeffs_Cb = malloc(totalPixels_chroma * sizeof(double));
    applyDctToImage(Y_component, InfoHeader.width, InfoHeader.height, dctCoeffs_Y, C);
    applyDctToImage(CrDown, width_chroma, height_chroma, dctCoeffs_Cr, C);
    applyDctToImage(CbDown, width_chroma, height_chroma, dctCoeffs_Cb, C);

    // Quantização
    int *quantized_Cb = malloc(totalPixels_chroma * sizeof(int));
    int *quantized_Cr = malloc(totalPixels_chroma * sizeof(int));
    int *quantized_Y = malloc(totalPixels * sizeof(int));
    quantizeImage(dctCoeffs_Y, quantized_Y, InfoHeader.width, InfoHeader.height, LUMINANCE_Q_TABLE, K_FACTOR);
    quantizeImage(dctCoeffs_Cb, quantized_Cb, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);
    quantizeImage(dctCoeffs_Cr, quantized_Cr, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);

    // Retorna ponteiros para os quantizados
    *quantized_Y_out = quantized_Y;
    *quantized_Cb_out = quantized_Cb;
    *quantized_Cr_out = quantized_Cr;

    // Liberação de memória temporária
    free(Y_component);
    free(dctCoeffs_Y);
    free(dctCoeffs_Cr);
    free(dctCoeffs_Cb);
    free(CbDown);
    free(CrDown);
}

void reconstructImageFromDCT(
    int *quantized_Y,
    int *quantized_Cb,
    int *quantized_Cr,
    BitmapInfoHeader InfoHeader,
    PixelYCbCr *converted
) {
    int totalPixels = InfoHeader.width * InfoHeader.height;
    int width_chroma = InfoHeader.width / 2;
    int height_chroma = InfoHeader.height / 2;
    int totalPixels_chroma = width_chroma * height_chroma;

    // Dequantização
    double *dequantized_dct_Y = malloc(totalPixels * sizeof(double));
    double *dequantized_dct_Cb = malloc(totalPixels_chroma * sizeof(double));
    double *dequantized_dct_Cr = malloc(totalPixels_chroma * sizeof(double));

    dequantizeImage(quantized_Y, dequantized_dct_Y, InfoHeader.width, InfoHeader.height, LUMINANCE_Q_TABLE, K_FACTOR);
    dequantizeImage(quantized_Cb, dequantized_dct_Cb, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);
    dequantizeImage(quantized_Cr, dequantized_dct_Cr, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);

    // IDCT
    double *reconstructedCbDown = malloc(totalPixels_chroma * sizeof(double));
    double *reconstructedCrDown = malloc(totalPixels_chroma * sizeof(double));
    double *reconstructedY = malloc(totalPixels * sizeof(double));

    double C[BLOCK_SIZE][BLOCK_SIZE];
    precalcC(C);
    applyIdctToImage(dequantized_dct_Cb, width_chroma, height_chroma, reconstructedCbDown, C);
    applyIdctToImage(dequantized_dct_Cr, width_chroma, height_chroma, reconstructedCrDown, C);
    applyIdctToImage(dequantized_dct_Y, InfoHeader.width, InfoHeader.height, reconstructedY, C);

    // Atualiza canal Y
    for (int i = 0; i < totalPixels; i++) {
        converted[i].Y = reconstructedY[i];
    }

    // Upsampling e atualização de Cb e Cr
    double *CbFull, *CrFull;
    upSampling(reconstructedCbDown , reconstructedCrDown , InfoHeader, &CbFull, &CrFull);

    for (int i = 0; i < totalPixels; i++) {
        converted[i].Cb = CbFull[i];
        converted[i].Cr = CrFull[i];
    }

    // Liberação de memória
    free(dequantized_dct_Y);
    free(dequantized_dct_Cb);
    free(dequantized_dct_Cr);
    free(reconstructedCbDown);
    free(reconstructedCrDown);
    free(reconstructedY);
    free(CbFull);
    free(CrFull);
}

