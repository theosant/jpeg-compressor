#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"
#include <math.h>

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

void aplicarZigZagBloco(int *entrada, int largura, int x_bloco, int y_bloco, int vetor_saida[64]) {
    for (int i = 0; i < 64; i++) {
        int dx = zigzag_order[i] / 8;
        int dy = zigzag_order[i] % 8;
        int x = x_bloco + dx;
        int y = y_bloco + dy;
        vetor_saida[i] = entrada[x * largura + y];
    }
}

void aplicarUnZigZag(int vetor[64], int bloco[8][8]) {
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

    for (int i = 0; i < 64; i++) {
        int x = zigzag_order[i] / 8;
        int y = zigzag_order[i] % 8;
        bloco[x][y] = vetor[i];
    }
}

void aplicarDPCMnosDCs(int **zigzag, int total_blocos) {
    int anterior = zigzag[0][0];
    for (int i = 1; i < total_blocos; i++) {
        int atual = zigzag[i][0];
        zigzag[i][0] = atual - anterior;
        anterior = atual;
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

// Codifica apenas AC (posição 1 a 63), retorna número de tokens
int aplicarRLE_AC(int vetor[64], RLEToken *tokens) {
    int idx = 0;
    int zeroCount = 0;
    for (int i = 1; i < 64; i++) {
        if (vetor[i] == 0) {
            zeroCount++;
        } else {
            while (zeroCount > 15) { // JPEG usa código especial ZRL para 16 zeros
                tokens[idx++] = (RLEToken){15, 0, 0}; // ZRL
                zeroCount -= 16;
            }

            int cat = calcularCategoria(vetor[i]);
            tokens[idx++] = (RLEToken){zeroCount, cat, vetor[i]};
            zeroCount = 0;
        }
    }

    // EOB (end of block)
    if (zeroCount > 0) {
        tokens[idx++] = (RLEToken){0, 0, 0}; // EOB
    }
    return idx;
}

BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos) {
    // Calcula quantos blocos serão necessários na horizontal e vertical (arredonda para cima a divisão)
    int blocos_largura = (largura + 7) / 8;
    int blocos_altura = (altura + 7) / 8;
    *num_blocos = blocos_largura * blocos_altura;
    
    BlocoYCbCr* blocos = malloc(*num_blocos * sizeof(BlocoYCbCr));
    
    for (int by = 0; by < blocos_altura; by++) {
        for (int bx = 0; bx < blocos_largura; bx++) {
            BlocoYCbCr* bloco = &blocos[by * blocos_largura + bx];
            
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    // Calcula a posição do pixel na imagem original
                    int px = bx * 8 + x;
                    int py = by * 8 + y;
                    
                    if (px < largura && py < altura) {
                        // Se estiver dentro dos limites, copia o pixel
                        int idx = py * largura + px;
                        bloco->Y[y][x] = imagem[idx].Y;
                        bloco->Cb[y][x] = imagem[idx].Cb;
                        bloco->Cr[y][x] = imagem[idx].Cr;
                    } else {
                        // Padding com últimos valores válidos
                        bloco->Y[y][x] = bloco->Y[y][x-1];
                        bloco->Cb[y][x] = bloco->Cb[y][x-1];
                        bloco->Cr[y][x] = bloco->Cr[y][x-1];
                    }
                }
            }
        }
    }
    
    return blocos;
}

void DPCM(BlocoYCbCr* blocos, int num_blocos) {
    float dc_prev_Y = 0, dc_prev_Cb = 0, dc_prev_Cr = 0;

    for (int i = 0; i < num_blocos; i++) {
        float dc_Y = blocos[i].Y[0][0];
        float dc_Cb = blocos[i].Cb[0][0];
        float dc_Cr = blocos[i].Cr[0][0];

        blocos[i].Y[0][0] = dc_Y - dc_prev_Y;
        blocos[i].Cb[0][0] = dc_Cb - dc_prev_Cb;
        blocos[i].Cr[0][0] = dc_Cr - dc_prev_Cr;

        dc_prev_Y = dc_Y;
        dc_prev_Cb = dc_Cb;
        dc_prev_Cr = dc_Cr;
    }
}

// Função auxiliar para compirmir_bloco
const char* buscarCodigo(TabelaHuffman* tabela, int valor) {
    valor = valor < 0 ? 0 : (valor > 255 ? 255 : valor);
    return tabela->codigos[valor];
}

// Função auxiliar para escrever um código Huffman no buffer
void escreveCodigo(const char* codigo, unsigned char buffer, int pos, FILE* output) {
    for (int i = 0; codigo[i] != '\0'; i++) {
        if (codigo[i] == '1') {
            buffer |= (1 << (7 - pos));
        }
        pos++;
        
        if (pos == 8) {
            fwrite(&buffer, 1, 1, output);
            buffer = 0;
            pos = 0;
        }
    }
}

// Função auxiliar para processar uma componente 8x8
void processarComponente(float componente[8][8], TabelaHuffman* tabela, unsigned char buffer, int pos, FILE* output) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // Garante que o valor está no intervalo [0, 255]
            int valor = (int)round(componente[y][x]);
            valor = valor < 0 ? 0 : (valor > 255 ? 255 : valor);
            // printf("[DEBUG] Processando valor %d na pos (%d,%d)\n", valor, y, x);
            
            const char* codigo = buscarCodigo(tabela, valor);
            if (!codigo) {
                printf("[ERRO CRITICO] Codigo Huffman nao encontrado para valor %d\n", valor);
                exit(EXIT_FAILURE);
            }
            // printf("[DEBUG] Codigo encontrado: %s\n", codigo);

            escreveCodigo(codigo, buffer, pos, output);
        }
    }
}

void comprimeBloco(BlocoYCbCr bloco, TabelaHuffman* tabela_Y, 
                    TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr, 
                    FILE* output) {
    unsigned char buffer = 0;
    int pos = 0;

    // 1. Comprime componente Y (luminância)
    processarComponente(bloco.Y, tabela_Y, buffer, pos, output);

    // 2. Comprime componente Cb (crominância azul)
    processarComponente(bloco.Cb, tabela_Cb, buffer, pos, output);

    // 3. Comprime componente Cr (crominância vermelha)
    processarComponente(bloco.Cr, tabela_Cr, buffer, pos, output);

    // 4. Escreve quaisquer bits restantes no buffer
    if (pos > 0) {
        fwrite(&buffer, 1, 1, output);
    }
}

// Função principal para comprimir JPEG sem perdas
void comprimirJPEGSemPerdas(const char* input_bmp, const char* output_jpeg) {
    printf("\n[DEBUG] Iniciando compressao JPEG sem perdas...\n");
    
    FILE* fp = fopen(input_bmp, "rb");
    if (!fp) {
        perror("Erro ao abrir arquivo BMP");
        return;
    }
    
    // Ler cabeçalhos e imagem
    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;
    loadBmpHeaders(fp, &fileHeader, &infoHeader);

    Pixel* imagem_rgb = loadBmpImage(fp, infoHeader);
    fclose(fp);
    
    // Converter para YCbCr
    PixelYCbCr* imagem_ycbcr = convertRgbToYCbCr(imagem_rgb, infoHeader);
    
    // Dividir em blocos 8x8
    int num_blocos;
    BlocoYCbCr* blocos = dividirBlocos(imagem_ycbcr, infoHeader.width, infoHeader.height, &num_blocos);
    
    // Aplicar DPCM
    DPCM(blocos, num_blocos);
    
    // Construir tabelas Huffman
    TabelaHuffman* tabela_Y = construirTabelaHuffman(blocos, num_blocos, COMP_Y);
    TabelaHuffman* tabela_Cb = construirTabelaHuffman(blocos, num_blocos, COMP_CB);
    TabelaHuffman* tabela_Cr = construirTabelaHuffman(blocos, num_blocos, COMP_CR);
    
    // Escrever arquivo JPEG
    FILE* out = fopen(output_jpeg, "wb");
    if (!out) {
        perror("Erro ao criar arquivo de saída");
        return;
    }
    
    // Criar cabeçalho
    JLSHeader header;
    memcpy(header.name, "JLS1", 4);
    header.width = infoHeader.width;
    header.height = infoHeader.height;
    header.dataSize = num_blocos;

    // Escrever cabeçalho
    fwrite(&header, sizeof(JLSHeader), 1, out);
    
    // Comprimir cada bloco
    printf("[DEBUG] Iniciando compressao dos blocos...\n");
    for (int i = 0; i < num_blocos; i++) {
        comprimeBloco(blocos[i], tabela_Y, tabela_Cb, tabela_Cr, out);
    }
    
    fclose(out);
    
    // Liberar memória
    free(imagem_rgb);
    free(imagem_ycbcr);
    free(blocos);
    destruirTabelaHuffman(tabela_Y);
    destruirTabelaHuffman(tabela_Cb);
    destruirTabelaHuffman(tabela_Cr);
}

