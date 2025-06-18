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

        converted[i].Y = round(0.299 * r + 0.587 * g + 0.114 * b);
        converted[i].Cb = round(-0.168736 * r - 0.331264 * g + 0.5 * b + 128);
        converted[i].Cr = round(0.5 * r - 0.418688 * g - 0.081312 * b + 128);
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


void aplicarDPCMnosDCs(int **zigzag, int total_blocos) {
    int anterior = zigzag[0][0];
    for (int i = 1; i < total_blocos; i++) {
        int atual = zigzag[i][0];
        zigzag[i][0] = atual - anterior;
        anterior = atual;
    }
}

BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos) {
    int blocos_largura = (largura + 7) / 8;
    int blocos_altura = (altura + 7) / 8;
    *num_blocos = blocos_largura * blocos_altura;
    
    BlocoYCbCr* blocos = malloc(*num_blocos * sizeof(BlocoYCbCr));
    if (!blocos) { return NULL; }
    
    for (int by = 0; by < blocos_altura; by++) {
        for (int bx = 0; bx < blocos_largura; bx++) {
            BlocoYCbCr* bloco = &blocos[by * blocos_largura + bx];
            
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int px = bx * 8 + x;
                    int py = by * 8 + y;
                    
                    if (px < largura && py < altura) {
                        int idx = py * largura + px;
                        bloco->Y[y][x]  = round(imagem[idx].Y);
                        bloco->Cb[y][x] = round(imagem[idx].Cb);
                        bloco->Cr[y][x] = round(imagem[idx].Cr);

                    } else {
                        // Lógica de padding
                        if (x > 0) {
                            bloco->Y[y][x] = bloco->Y[y][x-1];
                            bloco->Cb[y][x] = bloco->Cb[y][x-1];
                            bloco->Cr[y][x] = bloco->Cr[y][x-1];
                        } else if (y > 0) {
                             bloco->Y[y][x] = bloco->Y[y-1][x];
                             bloco->Cb[y][x] = bloco->Cb[y-1][x];
                             bloco->Cr[y][x] = bloco->Cr[y-1][x];
                        } else {
                            bloco->Y[y][x] = 128; // Cinza
                            bloco->Cb[y][x] = 128;
                            bloco->Cr[y][x] = 128;
                        }
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
long comprimirJPEGSemPerdas(PixelYCbCr* imagem_ycbcr, int largura, int altura, const char* output_jpeg) {    
    // Dividir em blocos 8x8
    int num_blocos;
    BlocoYCbCr* blocos = dividirBlocos(imagem_ycbcr, largura, altura, &num_blocos);
    
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
        return -1;
    }
    
    // Criar cabeçalho
    JLSHeader header;
    memcpy(header.name, "JLS1", 4);
    header.width = largura;
    header.height = altura;
    header.dataSize = num_blocos;

    // Escrever cabeçalho
    fwrite(&header, sizeof(JLSHeader), 1, out);
    
    // Comprimir cada bloco
    printf("[DEBUG] Iniciando compressao dos blocos...\n");
    for (int i = 0; i < num_blocos; i++) {
        comprimeBloco(blocos[i], tabela_Y, tabela_Cb, tabela_Cr, out);
    }
    long compressed_size = ftell(out);
    fclose(out);
    
    // Liberar memória
    free(blocos);
    destruirTabelaHuffman(tabela_Y);
    destruirTabelaHuffman(tabela_Cb);
    destruirTabelaHuffman(tabela_Cr);
    return compressed_size;
}

