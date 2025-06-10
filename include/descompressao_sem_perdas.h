#ifndef DESCOMPRESSAO_SEM_PERDAS_H
#define DESCOMPRESSAO_SEM_PERDAS_H

#include "jpeg.h"

typedef struct {
    char name[4];        // Identificador do arquivo
    uint32_t width;       // largura da imagem
    uint32_t height;      // altura da imagem
    uint32_t dataSize;    // tamanho dos dados comprimidos em bytes
} JLSHeader;

void descomprimirJPEGSemPerdas(const char* input_compressed_jpeg, const char* output_bmp);

#endif