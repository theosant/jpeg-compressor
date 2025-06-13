#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define HUFFMAN_TAMANHO 511
#define HUFFMAN_OFFSET 255

// Componentes da imagem
#define COMP_Y 'Y'
#define COMP_CB 'C'
#define COMP_CR 'R'

typedef struct {
    int Y[8][8];
    int Cb[8][8];
    int Cr[8][8];
} BlocoYCbCr;

typedef struct HuffmanNode {
    int valor;
    unsigned frequencia;
    struct HuffmanNode *esquerda, *direita;
} HuffmanNode;

typedef struct {
    unsigned tamanho;
    unsigned capacidade;
    HuffmanNode** nos;
} MinHeap;

typedef struct {
    char** codigos;
    int* simbolos;
    unsigned tamanho;
    HuffmanNode* raiz;
} TabelaHuffman;


TabelaHuffman* construirTabelaHuffman(BlocoYCbCr* blocos, int num_blocos, char componente);
void escreverTabelaHuffman(TabelaHuffman* tabela, FILE* output);
TabelaHuffman* lerTabelaHuffman(FILE* input);
void destruirTabelaHuffman(TabelaHuffman* tabela);
const char* buscarCodigo(TabelaHuffman* tabela, int valor);
HuffmanNode* reconstruirArvore(TabelaHuffman* tabela);
void liberarArvoreHuffman(HuffmanNode* raiz);

#endif