#ifndef COMPRESSAO_SEM_PERDAS_H
#define COMPRESSAO_SEM_PERDAS_H

#include "jpeg.h"

typedef struct {
    char name[4];        // Identificador do arquivo
    uint32_t width;       // largura da imagem
    uint32_t height;      // altura da imagem
    uint32_t dataSize;    // tamanho dos dados comprimidos em bytes
} JLSHeader;


void DPCM(BlocoYCbCr* blocos, int num_blocos);
void comprimeBloco(BlocoYCbCr bloco, TabelaHuffman* tabela_Y, TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr, FILE* output);
void comprimirJPEGSemPerdas(const char* input_bmp, const char* output_jpeg);

#endif