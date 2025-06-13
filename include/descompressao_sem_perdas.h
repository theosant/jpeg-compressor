#ifndef DESCOMPRESSAO_SEM_PERDAS_H
#define DESCOMPRESSAO_SEM_PERDAS_H

#include "jpeg.h"
#include <assert.h> 

void DCPMInversa(BlocoYCbCr* blocos, int num_blocos);
void descomprimirJPEGSemPerdas(const char* input_compressed_jpeg, const char* output_bmp);

#endif