#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "jpeg.h"
#include "lossy.h"


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <modo> <entrada.bin> <saida.bmp>\n", argv[0]);
        fprintf(stderr, "Modos: -lossy, -lossless\n");
        return 1;
    }

    char* modo = argv[1];
    char* inputFile_nome = argv[2];
    char* outputFile_nome = argv[3];

    // Abre arquivo comprimido para leitura
    FILE *inputFile = fopen(inputFile_nome, "rb");
    if (!inputFile) {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo de entrada.\n");
        return 1;
    }

    // Informações da imagem original devem ser lidas ou inferidas
    BitmapFileHeader FileHeader;
    BitmapInfoHeader InfoHeader;

    PixelYCbCr* converted = NULL;

    if (strcmp(modo, "-lossy") == 0) {
        printf("Modo Descompressão com perdas ativado.\n");

        int *quantized_Y, *quantized_Cb, *quantized_Cr;

        // Leitura dos dados comprimidos (RLE + Huffman)
        decompressEntropy(inputFile, &quantized_Y, &quantized_Cb, &quantized_Cr, &InfoHeader);

        // Reverter quantização, IDCT, upsampling
        converted = reconstructImageFromDCT(quantized_Y, quantized_Cb, quantized_Cr, InfoHeader);
        free(quantized_Y);
        free(quantized_Cb);
        free(quantized_Cr);
    } else if (strcmp(modo, "-lossless") == 0) {
        printf("Modo Descompressão sem perdas ativado.\n");

        //converted = descomprimirJPEGSemPerdas(inputFile, &FileHeader, &InfoHeader);
    } else {
        fprintf(stderr, "Erro: Modo '%s' inválido.\n", modo);
        fclose(inputFile);
        return 1;
    }

    fclose(inputFile);

    // Converter YCbCr de volta para RGB
    Pixel* imageRGB = convertYCbCrToRgb(converted, InfoHeader);
    
    saveBmpImage(outputFile_nome, FileHeader, InfoHeader, imageRGB);

    printf("Imagem reconstruída salva em '%s'.\n", outputFile_nome);

    free(converted);
    free(imageRGB);

    return 0;
}
