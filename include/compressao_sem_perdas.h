#ifndef COMPRESSAO_SEM_PERDAS_H
#define COMPRESSAO_SEM_PERDAS_H

#include "jpeg.h"

void DPCM(BlocoYCbCr* blocos, int num_blocos);
void comprimirJPEGSemPerdas(const char* input_bmp, const char* output_jpeg);

#endif