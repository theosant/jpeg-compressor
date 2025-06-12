#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "jpeg.h"

#define BLOCK_SIZE 8

#define IMG "test_images/paisagem_32x32.bmp"

int main(){
    FILE *input;
 
    if(!(input = fopen(IMG, "rb"))){
        printf("Error: could not open input file." );
        exit(1);
    }
    // Leitura dos Headers
    BitmapFileHeader FileHeader;      
    BitmapInfoHeader InfoHeader; 
    loadBmpHeaders (input, &FileHeader, &InfoHeader);
    printHeaders(&FileHeader, &InfoHeader);
   
    // Carrega RGB
    Pixel* Image = loadBmpImage(input, InfoHeader);
    fclose(input);

    // Converte para YCbCr
    PixelYCbCr* converted = convertRgbToYCbCr(Image, InfoHeader);

    // Downsampling dos canais Cb e Cr
    double *CbDown, *CrDown;
    downSampling(converted, InfoHeader, &CbDown, &CrDown);

    // Extrai canal Y para DCT
    int totalPixels = InfoHeader.width * InfoHeader.height;
    double *Y_component = malloc(totalPixels * sizeof(double));
    for (int i = 0; i < totalPixels; i++) {
        Y_component[i] = converted[i].Y;
    }

    // Prepara coeficientes da DCT
    double C[BLOCK_SIZE][BLOCK_SIZE];
    precalcC(C);

    // Aplica DCT no canal Y
    double *dctCoeffs = malloc(totalPixels * sizeof(double));
    applyDctToImage(Y_component, InfoHeader.width, InfoHeader.height, dctCoeffs, C);

     // Aplica IDCT
    double *reconstructedY = malloc(totalPixels * sizeof(double));
    applyIdctToImage(dctCoeffs, InfoHeader.width, InfoHeader.height, reconstructedY, C);

    // Atualiza canal Y com reconstruído
    for (int i = 0; i < totalPixels; i++) {
        converted[i].Y = reconstructedY[i];
    }

    // Upsampling de Cb e Cr
    double *CbFull, *CrFull;
    upSampling(CbDown, CrDown, InfoHeader, &CbFull, &CrFull);

     // Atualiza canais Cb e Cr no struct de YCbCr
    for (int i = 0; i < totalPixels; i++) {
        converted[i].Cb = CbFull[i];
        converted[i].Cr = CrFull[i];
    }
    // Converte de volta para RGB
    Pixel *Image2 = convertYCbCrToRgb(converted, InfoHeader);

    // Salva imagem resultante
    saveBmpImage("output.bmp", FileHeader, InfoHeader, Image2);

    // Libera memória
    free(Image);
    free(Image2);
    free(converted);
    free(Y_component);
    free(dctCoeffs);
    free(reconstructedY);
    free(CbDown);
    free(CrDown);
    free(CbFull);
    free(CrFull);

    return 0;
}