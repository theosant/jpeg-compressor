#include "lossy.h"
#include "jpeg.h"

const int LUMINANCE_Q_TABLE[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

const int CHROMINANCE_Q_TABLE[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}
};
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

PixelYCbCr *reconstructImageFromDCT(
    int *quantized_Y,
    int *quantized_Cb,
    int *quantized_Cr,
    BitmapInfoHeader InfoHeader
) {
    int totalPixels = InfoHeader.width * InfoHeader.height;
    int width_chroma = InfoHeader.width / 2;
    int height_chroma = InfoHeader.height / 2;
    int totalPixels_chroma = width_chroma * height_chroma;

    PixelYCbCr *converted = malloc(totalPixels * sizeof(PixelYCbCr));
    if (converted == NULL) {
        fprintf(stderr, "Erro de alocação de memória para a imagem convertida.\n");
        return NULL;
    }

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

    return converted;
}



// Codificação binária (sem buffer bit a bit real aqui)
void escreverBits(FILE* out, int valor, int bits) {
    unsigned int code = valor >= 0 ? valor : ((1 << bits) - 1 + valor);
    for (int i = bits - 1; i >= 0; i--) {
        unsigned char bit = (code >> i) & 1;
        fwrite(&bit, 1, 1, out); // ou usar bitstream real
    }
}

void aplicarZigZag(int bloco[8][8], int vetor_saida[64]) {
    for (int i = 0; i < 64; i++) {
        int x = zigzag_order[i] / 8;
        int y = zigzag_order[i] % 8;
        vetor_saida[i] = bloco[x][y];
    }
}

void aplicarUnZigZag(int vetor[64], int bloco[8][8]) {
    for (int i = 0; i < 64; i++) {
        int x = zigzag_order[i] / 8;
        int y = zigzag_order[i] % 8;
        bloco[x][y] = vetor[i];
    }
}

// Codificação do coeficiente DC
void codificarDC(int diff, FILE* out) {
    int cat = calcularCategoria(diff);
    fwrite(&cat, 1, 1, out); // prefixo fictício

    if (cat > 0)
        escreverBits(out, diff, cat);
}

// Codificação RLE + Huffman simplificado para os 63 ACs
void codificarAC(int* ac, FILE* out) {
    int zeros = 0;
    for (int i = 0; i < 63; i++) {
        int val = ac[i];
        if (val == 0) {
            zeros++;
        } else {
            while (zeros > 15) {
                unsigned char zrl = 0xF0;
                fwrite(&zrl, 1, 1, out);
                zeros -= 16;
            }

            int cat = calcularCategoria(val);
            unsigned char prefixo = (zeros << 4) | cat;
            fwrite(&prefixo, 1, 1, out);
            escreverBits(out, val, cat);
            zeros = 0;
        }
    }

    // End of Block (EOB)
    unsigned char eob = 0x00;
    fwrite(&eob, 1, 1, out);
}

static unsigned char bit_buffer = 0;
static int bits_disponiveis = 0;

int lerUmBit(FILE* in) {
    if (bits_disponiveis == 0) {
        if (fread(&bit_buffer, 1, 1, in) != 1) {
            return -1; // Erro ou EOF
        }
        bits_disponiveis = 8;
    }

    int bit = (bit_buffer >> 7) & 1;
    bit_buffer <<= 1;
    bits_disponiveis--;

    return bit;
}

int lerBits(FILE* in, int n_bits) {
    if (n_bits > 32) return -1;

    int valor = 0;
    for (int i = 0; i < n_bits; i++) {
        int bit = lerUmBit(in);
        if (bit == -1) return -1;
        valor = (valor << 1) | bit;
    }
    return valor;
}

int decodificarMagnitude(int code, int categoria) {
    if (categoria == 0) return 0;
    int limite = 1 << (categoria - 1);
    return (code < limite) ? code - (1 << categoria) + 1 : code;
}

void decodificarDC(FILE* in, int* anterior, int* valor) {
    unsigned char cat;
    fread(&cat, 1, 1, in);
    if (cat == 0) {
        *valor = *anterior;
        return;
    }

    int mag = lerBits(in, cat);
    int diff = decodificarMagnitude(mag, cat);
    *valor = *anterior + diff;
    *anterior = *valor;
}

void decodificarAC(FILE* in, int* vetor_zigzag) {
    int pos = 1;
    while (pos < 64) {
        unsigned char byte;
        fread(&byte, 1, 1, in);
        if (byte == 0x00) {
            // EOB
            while (pos < 64) vetor_zigzag[pos++] = 0;
            break;
        }

        if (byte == 0xF0) {
            // ZRL
            for (int i = 0; i < 16 && pos < 64; i++) vetor_zigzag[pos++] = 0;
            continue;
        }

        int run = (byte >> 4) & 0xF;
        int cat = byte & 0xF;

        for (int i = 0; i < run && pos < 64; i++) vetor_zigzag[pos++] = 0;

        int mag = lerBits(in, cat);
        vetor_zigzag[pos++] = decodificarMagnitude(mag, cat);
    }
}

void decompressEntropy(FILE* in, int** quantized_Y, int** quantized_Cb, int** quantized_Cr, BitmapInfoHeader* header) {
    char magic[5] = {0};
    fread(magic, 1, 4, in);
    if (strcmp(magic, "JPG1") != 0) {
        fprintf(stderr, "Formato inválido!\n");
        exit(1);
    }

    int width, height;
    fread(&width, sizeof(int), 1, in);
    fread(&height, sizeof(int), 1, in);
    
    header->width = width;
    header->height = height;
    header->imageSize  = width * height * 3;

    int blocos_Y = (width * height) / 64;
    int blocos_C = (width/2 * height/2) / 64;

    *quantized_Y = malloc(blocos_Y * 64 * sizeof(int));
    *quantized_Cb = malloc(blocos_C * 64 * sizeof(int));
    *quantized_Cr = malloc(blocos_C * 64 * sizeof(int));

    int vetor_zigzag[64];
    int bloco[8][8];
    int dc_ant = 0;

    // --- Y ---
    dc_ant = 0;
    for (int i = 0; i < blocos_Y; i++) {
        decodificarDC(in, &dc_ant, &vetor_zigzag[0]);
        decodificarAC(in, vetor_zigzag);

        aplicarUnZigZag(vetor_zigzag, bloco);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                (*quantized_Y)[i * 64 + y * 8 + x] = bloco[y][x];
    }

    // --- Cb ---
    dc_ant = 0;
    for (int i = 0; i < blocos_C; i++) {
        decodificarDC(in, &dc_ant, &vetor_zigzag[0]);
        decodificarAC(in, vetor_zigzag);

        aplicarUnZigZag(vetor_zigzag, bloco);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                (*quantized_Cb)[i * 64 + y * 8 + x] = bloco[y][x];
    }

    // --- Cr ---
    dc_ant = 0;
    for (int i = 0; i < blocos_C; i++) {
        decodificarDC(in, &dc_ant, &vetor_zigzag[0]);
        decodificarAC(in, vetor_zigzag);

        aplicarUnZigZag(vetor_zigzag, bloco);
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                (*quantized_Cr)[i * 64 + y * 8 + x] = bloco[y][x];
    }
}


int calcularCategoria(int valor) {
    valor = abs(valor);
    if (valor == 0) return 0;
    int categoria = 0;
    while (valor) {
        valor >>= 1;
        categoria++;
    }
    return categoria;
}


long entropy_encode(int* quantized_Y, int* quantized_Cb, int* quantized_Cr, int largura, int altura) {
    FILE* out = fopen("saida_entropy.jpegl", "wb");
    if (!out) {
        perror("Erro ao abrir arquivo de saída");
        return 0;
    }

    // Escrever cabeçalho simplificado
    fwrite("JPG1", 4, 1, out);
    fwrite(&largura, sizeof(int), 1, out);
    fwrite(&altura, sizeof(int), 1, out);

    int blocos_Y = (largura * altura) / 64;
    int blocos_C = (largura / 2 * altura / 2) / 64;

    int bloco_matriz[8][8];
    int vetor_zigzag[64];

    int dc_ant_Y = 0, dc_ant_Cb = 0, dc_ant_Cr = 0;

     for (int i = 0; i < blocos_Y; i++) {
        int* bloco = &quantized_Y[i * 64];

        // Copia para matriz
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                bloco_matriz[y][x] = bloco[y * 8 + x];

        aplicarZigZag(bloco_matriz, vetor_zigzag);

        int diff = vetor_zigzag[0] - dc_ant_Y;
        dc_ant_Y = vetor_zigzag[0];

        codificarDC(diff, out);
        codificarAC(&vetor_zigzag[1], out);
    }
    // --- Codificar Cb ---
    for (int i = 0; i < blocos_C; i++) {
        int* bloco = &quantized_Cb[i * 64];

        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                bloco_matriz[y][x] = bloco[y * 8 + x];

        aplicarZigZag(bloco_matriz, vetor_zigzag);

        int diff = vetor_zigzag[0] - dc_ant_Cb;
        dc_ant_Cb = vetor_zigzag[0];

        codificarDC(diff, out);
        codificarAC(&vetor_zigzag[1], out);
    }

    // --- Codificar Cr ---
    for (int i = 0; i < blocos_C; i++) {
        int* bloco = &quantized_Cr[i * 64];

        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
                bloco_matriz[y][x] = bloco[y * 8 + x];

        aplicarZigZag(bloco_matriz, vetor_zigzag);

        int diff = vetor_zigzag[0] - dc_ant_Cr;
        dc_ant_Cr = vetor_zigzag[0];

        codificarDC(diff, out);
        codificarAC(&vetor_zigzag[1], out);
    }

    long tamanho = ftell(out);
    fclose(out);
    return tamanho;

}