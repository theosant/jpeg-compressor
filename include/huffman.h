#ifndef HUFFAMN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

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
TabelaHuffman* ConstruirTabelaHuffman(const int* dados, unsigned tamanho);
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