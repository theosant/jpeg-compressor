#ifndef COMPRESSAO_SEM_PERDAS_H
#define COMPRESSAO_SEM_PERDAS_H

#include "huffman.h"

typedef struct {
    int Y[8][8];    // Luminância
    int Cb[8][8];   // Crominância Azul
    int Cr[8][8];   // Crominância Vermelha
} BlocoYCbCr;

void converterRGBparaYCbCr(unsigned char* bitmap, int largura, int altura, 
                          BlocoYCbCr** blocos, int* numBlocos);
void dividirEmBlocos8x8(int* canal, int largura, int altura, 
                        BlocoYCbCr** blocos, int* numBlocos);
void DPCM(BlocoYCbCr* blocos, int numBlocos);
unsigned char* comprimirJPEGSemPerdas(unsigned char* bitmap, int largura, 
                                    int altura, unsigned* tamanhoSaida);
void liberarBlocosYCbCr(BlocoYCbCr* blocos, int numBlocos);

#endif