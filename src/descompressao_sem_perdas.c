#include "descompressao_sem_perdas.h"

// Função para debug: verificar perdas
void verificarPerdas(BlocoYCbCr* originais, BlocoYCbCr* reconstruidos, int num_blocos) {
    for (int i = 0; i < num_blocos; i++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (originais[i].Y[y][x] != reconstruidos[i].Y[y][x] ||
                    originais[i].Cb[y][x] != reconstruidos[i].Cb[y][x] ||
                    originais[i].Cr[y][x] != reconstruidos[i].Cr[y][x]) {
                    printf("[ERRO] Perda detectada no bloco %d, pos (%d,%d)! Original Y: %d, Reconstruído Y: %d\n", 
                           i, y, x, originais[i].Y[y][x], reconstruidos[i].Y[y][x]);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    printf("[SUCESSO] Nenhuma perda detectada!\n");
}

BlocoYCbCr* carregarBlocosOriginais(const char* filename, int* num_blocos_out) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("[ERRO] Ao abrir arquivo de blocos originais");
        return NULL;
    }
    fread(num_blocos_out, sizeof(int), 1, f);
    BlocoYCbCr* blocos = malloc(*num_blocos_out * sizeof(BlocoYCbCr));
    fread(blocos, sizeof(BlocoYCbCr), *num_blocos_out, f);
    fclose(f);
    return blocos;
}

void decodificarBlocoComponente(FILE* input, HuffmanNode* raiz, int componente[8][8], unsigned char* buffer, int* pos_bit) {
    HuffmanNode* atual = raiz;
    if (!raiz) {
        fprintf(stderr, "[ERRO] Árvore Huffman nula passada para decodificação.\n");
        return;
    }

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            atual = raiz; // Reseta para a raiz para cada novo símbolo
            while (atual->esquerda || atual->direita) {
                // Se já usamos todos os bits do buffer, lê um novo byte
                if (*pos_bit >= 8) {
                    if (fread(buffer, 1, 1, input) != 1) {
                        fprintf(stderr, "[ERRO] Fim inesperado do arquivo durante a decodificação.\n");
                        return;
                    }
                    *pos_bit = 0; // Reseta o ponteiro de bit
                }
                
                // Lê o bit atual de forma não-destrutiva
                int bit = (*buffer >> (7 - *pos_bit)) & 1;
                (*pos_bit)++; // Avança o ponteiro de bit

                atual = bit ? atual->direita : atual->esquerda;

                if (!atual) {
                    fprintf(stderr, "[ERRO] Caminho inválido na árvore Huffman.\n");
                    return;
                }
            }
            // Chegamos a uma folha, o valor está em 'atual->valor'
            componente[y][x] = atual->valor;
        }
    }
}

void decodificarBloco(FILE* input, TabelaHuffman* tabela_Y, TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr, 
                      BlocoYCbCr* bloco, unsigned char* buffer, int* pos_bit) {
    decodificarBlocoComponente(input, tabela_Y->raiz, bloco->Y, buffer, pos_bit);
    decodificarBlocoComponente(input, tabela_Cb->raiz, bloco->Cb, buffer, pos_bit);
    decodificarBlocoComponente(input, tabela_Cr->raiz, bloco->Cr, buffer, pos_bit);
}

void DCPMInversa(BlocoYCbCr* blocos, int num_blocos) {
    int DC_Y = 0, DC_CB = 0, DC_CR = 0;

    for (int i = 0; i < num_blocos; i++) {
        // Reconstitui os valores originais somando a diferença ao DC anterior
        blocos[i].Y[0][0] += DC_Y;
        blocos[i].Cb[0][0] += DC_CB;
        blocos[i].Cr[0][0] += DC_CR;

        // O valor DC atualizado para o próximo bloco é o valor reconstituído
        DC_Y = blocos[i].Y[0][0];
        DC_CB = blocos[i].Cb[0][0];
        DC_CR = blocos[i].Cr[0][0];
    }
}

// Função principal para descompressão do JPEG
void descomprimirJPEGSemPerdas(const char* input_jpeg, const char* output_bmp) {
    printf("\n[DEBUG] Iniciando descompressao de %s para %s\n", input_jpeg, output_bmp);

    FILE* input = fopen(input_jpeg, "rb");
    if (!input) {
        perror("[ERRO] Falha ao abrir arquivo JPEG");
        return;
    }

    JLSHeader header;
    fread(&header, sizeof(JLSHeader), 1, input);

    if (strncmp(header.name, "JLS1", 4) != 0) {
        fprintf(stderr, "[ERRO] Arquivo nao e um JLS valido\n");
        fclose(input);
        return;
    }

    TabelaHuffman* tabela_Y = lerTabelaHuffman(input);
    TabelaHuffman* tabela_Cb = lerTabelaHuffman(input);
    TabelaHuffman* tabela_Cr = lerTabelaHuffman(input);

    BlocoYCbCr* blocos = malloc(header.dataSize * sizeof(BlocoYCbCr));
    if (!blocos) {
        perror("[ERRO] Falha ao alocar blocos");
        return;
    }

    unsigned char buffer_leitura = 0;
    int pos_bit_leitura = 8;

    printf("[DEBUG] Decodificando %u blocos...\n", header.dataSize);
    for (unsigned i = 0; i < header.dataSize; i++) {
        decodificarBloco(input, tabela_Y, tabela_Cb, tabela_Cr, &blocos[i], &buffer_leitura, &pos_bit_leitura);
    }

    // DEBUG
    int num_blocos_originais;

    // Aplicar DPCM Inversa
    printf("[DEBUG] Aplicando DPCM inversa...\n");
    DCPMInversa(blocos, header.dataSize);
    
    BlocoYCbCr* blocos_originais_finais = carregarBlocosOriginais("temp_originais.bin", &num_blocos_originais);
    printf("\n[VERIFICAÇÃO] Comparando blocos finais (APÓS DPCM inversa):\n");
    verificarPerdas(blocos_originais_finais, blocos, header.dataSize);
    free(blocos_originais_finais);

    // 5. Reconstruir imagem YCbCr a partir dos blocos
    printf("[DEBUG] Reconstruindo imagem YCbCr...\n");
    PixelYCbCr* ycbcr = malloc(header.width * header.height * sizeof(PixelYCbCr));
    if (!ycbcr) {
        perror("[ERRO] Falha ao alocar YCbCr");
        free(blocos);
        fclose(input);
        return;
    }
    int blocos_por_linha = (header.width + 7) / 8;
    
    for (unsigned by = 0; by < (header.height + 7)/8; by++) {
        for (int bx = 0; bx < blocos_por_linha; bx++) {
            BlocoYCbCr* bloco = &blocos[by * blocos_por_linha + bx];
            
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    unsigned px = bx * 8 + x;
                    unsigned py = by * 8 + y;
                    
                    if (px < header.width && py < header.height) {
                        int idx = py * header.width + px;
                        ycbcr[idx].Y = bloco->Y[y][x];
                        ycbcr[idx].Cb = bloco->Cb[y][x];
                        ycbcr[idx].Cr = bloco->Cr[y][x];
                    }
                }
            }
        }
    }

    printf("[TESTE] Verificando blocos YCbCr reconstruidos:\n");
    for (int b = 0; b < 3; b++) { // Mostra 3 primeiros blocos
        printf("Bloco %d - Y[0][0]=%.2f, Cb[0][0]=%.2f, Cr[0][0]=%.2f\n",
            b, blocos[b].Y[0][0], blocos[b].Cb[0][0], blocos[b].Cr[0][0]);
        
        // Mostra toda a matriz 8x8 do componente Y do primeiro bloco
        if (b == 0) {
            printf("Matriz Y completa do Bloco 0:\n");
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    printf("%6.1f ", blocos[b].Y[i][j]);
                }
                printf("\n");
            }
        }
    }

    int blocos_por_coluna = (header.height + 7) / 8;
    printf("\n[TESTE] Dimensoes:\n");
    printf("Blocos totais: %d\n", header.dataSize);
    printf("Blocos por linha: %d\n", blocos_por_linha);
    printf("Blocos por coluna: %d\n", blocos_por_coluna);
    printf("Calculado: %d x %d = %d blocos\n", 
        blocos_por_linha, blocos_por_coluna, blocos_por_linha * blocos_por_coluna);

    assert((size_t)header.dataSize == (size_t)(blocos_por_linha * blocos_por_coluna));

    // 6. Converter YCbCr para RGB
    Pixel* rgb = convertYCbCrToRgb(ycbcr, header.width, header.height);

    // 7. Salvando as mensagens
    BitmapFileHeader fileHeader = {
        .type = 0x4D42, // 'BM'
        .size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + (header.width * header.height * 3),
        .offsetBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader)
    };

    BitmapInfoHeader infoHeader = {
        .size = sizeof(BitmapInfoHeader),
        .width = header.width,
        .height = header.height,
        .planes = 1,
        .bitCount = 24,
        .compression = 0,
        .imageSize = header.width * header.height * 3,
        .xResolution = 0,
        .yResolution = 0,
        .colorsUsed = 0,
        .importantColors = 0
    };
    saveBmpImage(output_bmp, fileHeader, infoHeader, rgb);

    // [9] Liberar memória
    free(blocos);
    free(ycbcr);
    free(rgb);
    destruirTabelaHuffman(tabela_Y);
    destruirTabelaHuffman(tabela_Cb);
    destruirTabelaHuffman(tabela_Cr);
    fclose(input);
}