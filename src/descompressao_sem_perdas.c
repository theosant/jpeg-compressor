#include "descompressao_sem_perdas.h"

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

        if (i == 10) {
            printf("[DESCOMPRESSAO] Bloco do pixel central (DC) apos DPCM Inversa:\n");
            printf("  -> Y=%d, Cb=%d, Cr=%d\n", blocos[i].Y[0][0], blocos[i].Cb[0][0], blocos[i].Cr[0][0]);
            printf("--------------------------------------------------\n");
        }
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

    // 1. Lê cabeçalho JLS
    JLSHeader header;
    fread(&header, sizeof(JLSHeader), 1, input);

    if (strncmp(header.name, "JLS1", 4) != 0) {
        fprintf(stderr, "[ERRO] Arquivo nao e um JLS valido\n");
        fclose(input);
        return;
    }

    // 2. Lê as três tabelas de Huffman do arquivo
    TabelaHuffman* tabela_Y = lerTabelaHuffman(input);
    TabelaHuffman* tabela_Cb = lerTabelaHuffman(input);
    TabelaHuffman* tabela_Cr = lerTabelaHuffman(input);

    // 3. Aloca memória para os blocos
    BlocoYCbCr* blocos = malloc(header.dataSize * sizeof(BlocoYCbCr));
    if (!blocos) {
        perror("[ERRO] Falha ao alocar blocos");
        return;
    }
    unsigned char buffer_leitura = 0;
    int pos_bit_leitura = 8;

    // 4. Decodifica cada bloco da imagem usando as tabelas de Huffman.
    printf("[DEBUG] Decodificando %u blocos...\n", header.dataSize);
    for (unsigned i = 0; i < header.dataSize; i++) {
        decodificarBloco(input, tabela_Y, tabela_Cb, tabela_Cr, &blocos[i], &buffer_leitura, &pos_bit_leitura);
    }

    // 5. Aplica a DPCM inversa para restaurar os valores DC originais.
    printf("[DEBUG] Aplicando DPCM inversa...\n");
    DCPMInversa(blocos, header.dataSize);

    // 6. Remonta a imagem YCbCr completa a partir dos blocos 8x8.
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

    // 7. Converte a imagem YCbCr de volta para o formato RGB.
    Pixel* rgb = convertYCbCrToRgb(ycbcr, header.width, header.height);

    // 8. Salvando a imagem
    printf("[DEBUG] Salvando imagem BMP final com cabeçalhos originais...\n");
    saveBmpImage(output_bmp, header.bmp_file_header, header.bmp_info_header, rgb);

    // 9. Liberar memória
    free(blocos);
    free(ycbcr);
    free(rgb);
    destruirTabelaHuffman(tabela_Y);
    destruirTabelaHuffman(tabela_Cb);
    destruirTabelaHuffman(tabela_Cr);
    fclose(input);
}