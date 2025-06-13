#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// --- Constantes ---
// O tamanho do nosso alfabeto de símbolos. Para um processo sem perdas
// com predição, o erro geralmente fica em um intervalo como [-255, 255].
// O offset garante que todos os índices do array de frequência sejam positivos.
#define HUFFMAN_TAMANHO 511
#define HUFFMAN_OFFSET 255

// Componentes da imagem
#define COMP_Y 'Y'
#define COMP_CB 'C'
#define COMP_CR 'R'

// --- Estruturas de Dados ---

// A estrutura BlocoYCbCr foi mantida, mas seus membros agora são do tipo 'int'
// para garantir que a compressão seja sem perdas (lossless).
typedef struct {
    int Y[8][8];
    int Cb[8][8];
    int Cr[8][8];
} BlocoYCbCr;

// Nó da árvore de Huffman
typedef struct HuffmanNode {
    int valor;
    unsigned frequencia;
    struct HuffmanNode *esquerda, *direita;
} HuffmanNode;

// Min Heap para construir a árvore de Huffman
typedef struct {
    unsigned tamanho;
    unsigned capacidade;
    HuffmanNode** nos;
} MinHeap;

// Tabela de códigos de Huffman
typedef struct {
    char** codigos;
    int* simbolos;
    unsigned tamanho;
    HuffmanNode* raiz; // Raiz da árvore para descompressão
} TabelaHuffman;


// --- Protótipos das Funções ---

// Funções da Árvore e Tabela de Huffman
TabelaHuffman* construirTabelaHuffman(BlocoYCbCr* blocos, int num_blocos, char componente);
void escreverTabelaHuffman(TabelaHuffman* tabela, FILE* output);
TabelaHuffman* lerTabelaHuffman(FILE* input);
void destruirTabelaHuffman(TabelaHuffman* tabela);
const char* buscarCodigo(TabelaHuffman* tabela, int valor);
HuffmanNode* reconstruirArvore(TabelaHuffman* tabela);
void liberarArvoreHuffman(HuffmanNode* raiz);

#endif // HUFFMAN_H