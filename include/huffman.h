#ifndef HUFFAMN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef enum {
    COMP_Y = 1,  // Luminância
    COMP_CB = 2, // Crominância Azul
    COMP_CR = 3  // Crominância Vermelha
} Componente;

typedef struct {
    float Y[8][8];
    float Cb[8][8];
    float Cr[8][8];
} BlocoYCbCr;

/* Decodifica código -> símbolo, ou seja, gera a tabela de códigos */
typedef struct HuffmanNode HuffmanNode;

struct HuffmanNode{
    int valor;
    unsigned frequencia;
    HuffmanNode* esquerda;
    HuffmanNode* direita;
};

/* Mapeia símbolo -> código, ou seja, susbtitui símbolos por códigos */
typedef struct {
    int* simbolos;
    char** codigos;
    unsigned tamanho;
} TabelaHuffman;

/* Estrtura para construir a árvore de forma mais eficiente*/
typedef struct {
    HuffmanNode** nos;
    unsigned tamanho;
    unsigned capacidade;
} MinHeap;

/* Funções da tabela*/
TabelaHuffman* construirTabelaHuffman(BlocoYCbCr* blocos, int num_blocos, char componente);
void destruirTabelaHuffman(TabelaHuffman *tabela);
void comprimirDadosHuffman(const int* dados, unsigned tamanho, 
    const TabelaHuffman* tabela, unsigned char** bufferSaida, 
    unsigned* tamanhoSaida);

/* Funções da árvore*/
HuffmanNode* criarNo(int valor, unsigned frequencia);
HuffmanNode* construirArvoreHuffman(const int* dados, unsigned tamanho);
void liberarArvoreHuffman(HuffmanNode* raiz);

/* Funções min-heap*/
MinHeap* criarMinHeap(unsigned capacidade);
void inserirMinHeap(MinHeap* heap, HuffmanNode* no);
HuffmanNode* extrairMin(MinHeap* heap);
void destruirMinHeap(MinHeap* heap);

#endif