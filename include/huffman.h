#ifndef HUFFAMN_H
#   define HUFFMAN_H

#include <stdint.h>

/* Decodifica código -> símbolo, ou seja, gera a tabela de códigos */
typedef struct {
    int valor;
    unsigned frequencia;
    HuffmanNode* esquerda;
    HuffmanNode* direita;
} HuffmanNode;

/* Mapeia símbolo -> código, ou seja, susbtitui símbolos por códigos */
typedef struct {
    int* simbolos;
    char** codigos;
    unsigned tamanho;
} TabelaHuffman;

TabelaHuffman* ConstruirTabelaHuffman(const int* dados, unsigned tamanho);
void destruirTabelaHuffman(TabelaHuffman *tabela);
void comprimirDadosHuffman(const int* dados, unsigned tamanho, 
    const TabelaHuffman* tabela, unsigned char** bufferSaida, 
    unsigned* tamanhoSaida);
HuffmanNode* construirArvoreHuffman(const int* dados, unsigned tamanho);
void liberarArvoreHuffman(HuffmanNode* raiz);


#endif