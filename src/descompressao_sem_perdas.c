/* Pseudocodigo pra lembrar amanha*/

/* 
Ler o cabeçalho do arquivo comprimido (CHECK)

Reconstruir as tabelas Huffman (elas precisam ser armazenadas no arquivo ou recalculadas da mesma forma) (CHECK)

Ler e decodificar os dados comprimidos (CHECK)

Aplicar a DPCM inversa

Converter de YCbCr para RGB

Salvar a imagem descomprimida
*/

#include "descompressao_sem_perdas.h"

// Função auxiliar para recosntriruir as tabelas Huffman
TabelaHuffman* lertabelaHuffman(FILE* input) {
    TabelaHuffman* tabela = malloc(sizeof(TabelaHuffman));

    // Ler tamanho da tabela
    fread(&tabela->tamanho, sizeof(unsigned), 1, input);
    
    // Alocar memória
    tabela->codigos = calloc(tabela->tamanho, sizeof(char*));
    tabela->simbolos = malloc(tabela->tamanho * sizeof(int));

    // Ler cada entrada
    for (unsigned i = 0; i < tabela->tamanho; i++) {
        int simbolo;
        int comprimento;
        
        fread(&simbolo, sizeof(int), 1, input);
        fread(&comprimento, sizeof(int), 1, input);
        
        tabela->simbolos[i] = simbolo;
        
        if (comprimento > 0) {
            tabela->codigos[i] = malloc(comprimento + 1);
            fread(tabela->codigos[i], sizeof(char), comprimento, input);
            tabela->codigos[i][comprimento] = '\0'; // Adiciona terminador
        }
    }
    
    return tabela;
}

// Função auxiliar para reconstruir a árvore a partir da tabela
HuffmanNode* reconstruirArvore(TabelaHuffman* tabela) {
    HuffmanNode* raiz = criarNo('$', 0);

    for (int i = i; i < tabela->tamanho; i++){
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

    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            // Percorre a árvore até encontrar uma folha
            while (atual->esquerda != NULL || atual->direita != NULL) {
                if (bits_restantes == 0) {
                    if (fread(&buffer, 1, 1, input) != 1) {
                        fprintf(stderr, "Erro ao ler dados comprimidos\n");
                        return;
                    }
                    bits_restantes = 8;
                }
                
                int bit = (buffer >> 7) & 1; // Pega o bit mais significativo
                buffer <<= 1;
                bits_restantes--;
                
                if (bit == 0) {
                    atual = atual->esquerda;
                } else {
                    atual = atual->direita;
                }
            }
            componente[i][j] = (float) atual -> valor;
            atual = raiz; // volta para a raiz para o próximo símbolo
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

// Função principal para descompressão do JPEG
void descomprimirJPEGSemPerdas(const char* input_compressed_jpeg, const char* output_bmp) {
    // 1. Abrir arquivo de entrada e ler o cabeçalho
    FILE* in = fopen(input_compressed_jpeg, "rb");
    if (!in) {
        perror("Erro ao abrir arquivo JPEG");
        return;
    }

    JLSHeader header;
    fread(&header, sizeof(JLSHeader), 1, in);
    
    // Verificar cabeçalho
    if (strncmp(header.name, "JLS1", 4) != 0) {
        fprintf(stderr, "Arquivo não é um JLS válido\n");
        fclose(in);
        return;
    }

    // 2. Recosntruir Tabelas Huffman
    TabelaHuffman* tabela_Y = lertabelaHuffman(input_compressed_jpeg);
    TabelaHuffman* tabela_Cb = lertabelaHuffman(input_compressed_jpeg);
    TabelaHuffman* tabela_Cr = lertabelaHuffman(input_compressed_jpeg);

    // 3. Decodificar blocos
    BlocoYCbCr* blocos = malloc(header.dataSize * sizeof(BlocoYCbCr));
    if (!blocos) {
        perror("Erro ao alocar memória para blocos");
        fclose(in);
        return;
    }

    for (int i = 0; i < header.dataSize; i++) {
        decodificarBloco(in, tabela_Y, tabela_Cb, tabela_Cr, &blocos[i]);
    }
}