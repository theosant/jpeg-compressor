/* Pseudocodigo pra lembrar amanha*/

/* 
Ler o cabeçalho do arquivo comprimido (CHECK)

Reconstruir as tabelas Huffman (elas precisam ser armazenadas no arquivo ou recalculadas da mesma forma) (CHECK)

Ler e decodificar os dados comprimidos (CHECK)

Aplicar a DPCM inversa (CHECK)

Converter de YCbCr para RGB (CHECK)

Salvar a imagem descomprimida
*/

#include "descompressao_sem_perdas.h"

// Função auxiliar para recosntriruir as tabelas Huffman
TabelaHuffman* lertabelaHuffman(FILE* input) {
    printf("[DEBUG] Lendo tabela Huffman...\n");
    TabelaHuffman* tabela = malloc(sizeof(TabelaHuffman));
    if (!tabela) {
        perror("[ERRO] Falha ao alocar tabela");
        return NULL;
    }

    if (fread(&tabela->tamanho, sizeof(unsigned), 1, input) != 1) {
        perror("[ERRO] Falha ao ler tamanho da tabela");
        free(tabela);
        return NULL;
    }
    printf("[DEBUG] Tamanho da tabela: %u\n", tabela->tamanho);

    tabela->codigos = calloc(tabela->tamanho, sizeof(char*));
    tabela->simbolos = malloc(tabela->tamanho * sizeof(int));
    
    for (unsigned i = 0; i < tabela->tamanho; i++) {
        if (fread(&tabela->simbolos[i], sizeof(int), 1, input) != 1) {
            perror("[ERRO] Falha ao ler simbolo");
            destruirTabelaHuffman(tabela);
            return NULL;
        }
        
        int comprimento;
        if (fread(&comprimento, sizeof(int), 1, input) != 1) {
            perror("[ERRO] Falha ao ler comprimento do codigo");
            destruirTabelaHuffman(tabela);
            return NULL;
        }
        
        if (comprimento > 0) {
            tabela->codigos[i] = malloc(comprimento + 1);
            if (fread(tabela->codigos[i], sizeof(char), comprimento, input) != comprimento) {
                perror("[ERRO] Falha ao ler codigo Huffman");
                destruirTabelaHuffman(tabela);
                return NULL;
            }
            tabela->codigos[i][comprimento] = '\0';
            printf("[DEBUG] Simbolo %d: Codigo %s\n", tabela->simbolos[i], tabela->codigos[i]);
        }
    }
    
    return tabela;
}

// Função auxiliar para reconstruir a árvore a partir da tabela
HuffmanNode* reconstruirArvore(TabelaHuffman* tabela) {
    HuffmanNode* raiz = criarNo('$', 0);

    for (unsigned i = 0; i < tabela->tamanho; i++){
        if (tabela->codigos[i] != NULL) {
            HuffmanNode* atual = raiz;
            const char* codigo = tabela->codigos[i];

            // Percorre cada bit do código
            for (int j = 0; codigo[j] != '\0'; j++) {
                if (codigo[j] == '0') {
                    if (atual->esquerda == NULL) {
                        atual->esquerda = criarNo('$', 0);
                    }
                    atual = atual->esquerda;
                } else { // '1'
                    if (atual->direita == NULL) {
                        atual->direita = criarNo('$', 0);
                    }
                    atual = atual->direita;
                }
            }
            atual->valor = i;
        }
    }
    return raiz;
}

// Função auxiliar para decodificar o bloco por componente
void decodificarBlocoComponente(FILE* input, HuffmanNode* raiz, float componente[8][8]) {
    HuffmanNode* atual = raiz;
    unsigned char buffer;
    int bits_restantes = 0;
    int bits_lidos = 0;

    printf("[DEBUG] Iniciando decodificacao de bloco componente...\n");
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            while (atual->esquerda != NULL || atual->direita != NULL) {
                if (bits_restantes == 0) {
                    printf("[DEBUG] Lendo novo byte...\n");
                    if (fread(&buffer, 1, 1, input) != 1) {
                        fprintf(stderr, "[ERRO] Falha ao ler byte na posicao %ld\n", ftell(input));
                        perror("Detalhe");
                        return;
                    }
                    bits_restantes = 8;
                    printf("[DEBUG] Byte lido: 0x%02X\n", buffer);
                }
                
                int bit = (buffer >> 7) & 1;
                buffer <<= 1;
                bits_restantes--;
                bits_lidos++;
                
                printf("[DEBUG] Bit %d: %d\n", bits_lidos, bit);
                
                atual = (bit == 0) ? atual->esquerda : atual->direita;
                
                if (!atual) {
                    fprintf(stderr, "[ERRO] Caminho invalido na arvore Huffman\n");
                    return;
                }
            }
            
            componente[i][j] = (float)atual->valor;
            printf("[DEBUG] Valor decodificado [%d][%d] = %.2f\n", i, j, componente[i][j]);
            atual = raiz;
        }
    }
}

void decodificarBloco(FILE* input, TabelaHuffman* tabela_Y, TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr, BlocoYCbCr* bloco) {
    // Reconstruir a árvore para cada componente
    HuffmanNode* arvore_Y = reconstruirArvore(tabela_Y);
    HuffmanNode* arvore_Cb = reconstruirArvore(tabela_Cb);
    HuffmanNode* arvore_Cr = reconstruirArvore(tabela_Cr);

    // Decodoficar cada componente
    decodificarBlocoComponente(input, arvore_Y, bloco->Y);
    decodificarBlocoComponente(input, arvore_Cb, bloco->Cb);
    decodificarBlocoComponente(input, arvore_Cr, bloco->Cr);

    // Liberar memória
    liberarArvoreHuffman(arvore_Y);
    liberarArvoreHuffman(arvore_Cb);
    liberarArvoreHuffman(arvore_Cr);
}

void DCPMInversa(BlocoYCbCr* blocos, int num_blocos) {
    float DC_Y = 0.0f;
    float DC_CB = 0.0f;
    float DC_CR = 0.0f;

    for (int i = 0; i < num_blocos; i++){
        // Recupera as diferenças armazenadas
        float diff_Y = blocos[i].Y[0][0];
        float diff_Cb = blocos[i].Cb[0][0];
        float diff_Cr = blocos[i].Cr[0][0];

        // Reconstitui os valores originais
        blocos[i].Y[0][0] = diff_Y + DC_Y;
        blocos[i].Cb[0][0] = diff_Cb + DC_CB;
        blocos[i].Cr[0][0] = diff_Cr + DC_CR;

        // Atualiza valores para o próximo bloco
        DC_Y = blocos[i].Y[0][0];
        DC_CB = blocos[i].Cb[0][0];
        DC_CR = blocos[i].Cr[0][0];
    }
}

// Função principal para descompressão do JPEG
void descomprimirJPEGSemPerdas(const char* input_compressed_jpeg, const char* output_bmp) {
    printf("\n[DEBUG] Iniciando descompressao de %s para %s\n", input_compressed_jpeg, output_bmp);

    // 1. Abrir arquivo de entrada e ler o cabeçalho
    printf("[DEBUG] Abrindo arquivo de entrada...\n");
    FILE* input = fopen(input_compressed_jpeg, "rb");
    if (!input) {
        perror("[ERRO] Falha ao abrir arquivo JPEG");
        return;
    }

    printf("[DEBUG] Lendo cabecalho...\n");
    JLSHeader header;
    if (fread(&header, sizeof(JLSHeader), 1, input) != 1) {
        perror("[ERRO] Falha ao ler cabeçalho JLS");
        fclose(input);
        return;
    }

    printf("[DEBUG] Dados do cabecalho:\n");
    printf("  Assinatura: %.4s\n", header.name);
    printf("  Largura: %u\n", header.width);
    printf("  Altura: %u\n", header.height);
    printf("  Num blocos: %u\n", header.dataSize);
    
    // Verificar cabeçalho
    if (strncmp(header.name, "JLS1", 4) != 0) {
        fprintf(stderr, "[ERRO] Arquivo nao e um JLS valido\n");
        fclose(input);
        return;
    }

    // 2. Recosntruir Tabelas Huffman
    printf("[DEBUG] Lendo tabela Huffman Y...\n");
    TabelaHuffman* tabela_Y = lertabelaHuffman(input);
    printf("[DEBUG] Lendo tabela Huffman Cb...\n");
    TabelaHuffman* tabela_Cb = lertabelaHuffman(input);
    printf("[DEBUG] Lendo tabela Huffman Cr...\n");
    TabelaHuffman* tabela_Cr = lertabelaHuffman(input);

    if (!tabela_Y || !tabela_Cb || !tabela_Cr) {
        fprintf(stderr, "[ERRO] Falha ao ler tabelas Huffman\n");
        fclose(input);
        return;
    }

    // 3. Decodificar blocos
    printf("[DEBUG] Alocando memoria para %u blocos...\n", header.dataSize);
    BlocoYCbCr* blocos = malloc(header.dataSize * sizeof(BlocoYCbCr));
    if (!blocos) {
        perror("[ERRO] Falha ao alocar blocos");
        fclose(input);
        return;
    }

    printf("[DEBUG] Decodificando blocos...\n");
    for (unsigned i = 0; i < header.dataSize; i++) {
        printf("[DEBUG] Decodificando bloco %u/%u\n", i+1, header.dataSize);
        decodificarBloco(input, tabela_Y, tabela_Cb, tabela_Cr, &blocos[i]);
        
        // Debug do primeiro coeficiente de cada componente
        if (i == 0) {
            printf("[DEBUG] Primeiro bloco - Y[0][0]=%.2f, Cb[0][0]=%.2f, Cr[0][0]=%.2f\n", 
                   blocos[i].Y[0][0], blocos[i].Cb[0][0], blocos[i].Cr[0][0]);
        }
    }
    
    // 4. Aplicar DCPM Inversa
    printf("[DEBUG] Aplicando DPCM inversa...\n");
    DCPMInversa(blocos, header.dataSize);
    printf("[DEBUG] Apos DPCM - Y[0][0]=%.2f, Cb[0][0]=%.2f, Cr[0][0]=%.2f\n", 
           blocos[0].Y[0][0], blocos[0].Cb[0][0], blocos[0].Cr[0][0]);

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