#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "jpeg.h"
#include "lossy.h"


#define IMG "test_images/paisagem_32x32.bmp"
#define OUTPUT_JPEG "paisagem_compressed.jls"  // JLS = JPEG Lossless

int main(int argc, char *argv[]){
     if (argc != 4) {
        fprintf(stderr, "Uso: %s <modo> <entrada.bmp> <saida.bin>\n", argv[0]);
        fprintf(stderr, "Modos: -lossy, -lossless\n");
        return 1;
    }

    char* modo = argv[1];
    char* inputFile_nome = argv[2];
    char* outputFile_nome = argv[3];

    FILE *inputFile = fopen(inputFile_nome, "rb");
    if(!inputFile) {
        printf("Error: could not open input file.");
        exit(1);
    }

    // Leitura dos Pixels e Headers
    BitmapFileHeader FileHeader;      
    BitmapInfoHeader InfoHeader; 
    loadBmpHeaders(inputFile, &FileHeader, &InfoHeader);
    Pixel* Image = loadBmpImage(inputFile, InfoHeader);
    fclose(inputFile);

    // Converte para YCbCr
    PixelYCbCr* converted = convertRgbToYCbCr(Image, InfoHeader);
    long tamanho_comprimido = 0;

    // Chamar a rotina de compressão correta
    if (strcmp(modo, "-lossy") == 0) {
        printf("Modo Compressão com Perdas ativado.\n");
        int *quantized_Y_out, *quantized_Cb_out, *quantized_Cr_out;
        // Realiza DownSampling, DCT e Quantização
        processImageDCT(converted, InfoHeader, &quantized_Y_out, &quantized_Cb_out, &quantized_Cr_out);
        tamanho_comprimido = entropy_encode(quantized_Y_out, quantized_Cb_out, quantized_Cr_out,  InfoHeader.width, InfoHeader.height);
        free(quantized_Y_out);
        free(quantized_Cb_out);
        free(quantized_Cr_out);

    } else if (strcmp(modo, "-lossless") == 0) {
        printf("Modo Compressão sem Perdas ativado.\n");
        tamanho_comprimido = comprimirJPEGSemPerdas(converted, InfoHeader.width, InfoHeader.height, outputFile_nome);
    } else {
        fprintf(stderr, "Erro: Modo '%s' inválido.\n", modo);
        
        free(Image);
        free(converted);
        return 1;
    }


     if (tamanho_comprimido > 0) {
        long tamanho_original_dados = InfoHeader.width * InfoHeader.height * 3;
        double taxa_compressao = (double)tamanho_original_dados / tamanho_comprimido;

        printf("Compressao finalizada com sucesso!\n");
        printf("Tamanho original (dados): %ld bytes\n", tamanho_original_dados);
        printf("Tamanho comprimido:      %ld bytes\n", tamanho_comprimido);
        printf("Taxa de compressao:      %.2f : 1\n", taxa_compressao);
    } else {
        fprintf(stderr, "A compressao falhou ou resultou em um arquivo de tamanho 0.\n");
    }

    free(Image);
    free(converted);

    return 0;
}