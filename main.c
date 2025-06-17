#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "jpeg.h"



#define IMG "test_images/paisagem_32x32.bmp"
#define OUTPUT_JPEG "paisagem_compressed.jls"  // JLS = JPEG Lossless

int main(){
    FILE *input;
 
    // 1. Abre e verifica o arquivo BMP de entrada
    if(!(input = fopen(IMG, "rb"))) {
        printf("Error: could not open input file.");
        exit(1);
    }
    // Leitura dos Headers

    // 2. Lê os cabeçalhos do BMP
    BitmapFileHeader FileHeader;      
    BitmapInfoHeader InfoHeader; 
    loadBmpHeaders(input, &FileHeader, &InfoHeader);
    printHeaders(&FileHeader, &InfoHeader);
   
    // 3. Carrega os pixels da imagem
    Pixel* Image = loadBmpImage(input, InfoHeader);
    fclose(input);

    // Converte para YCbCr
    PixelYCbCr* converted = convertRgbToYCbCr(Image, InfoHeader);

    int *quantized_Y_out;
    int *quantized_Cb_out; 
    int *quantized_Cr_out;
    processImageDCT(converted, InfoHeader, &quantized_Y_out, &quantized_Cb_out, &quantized_Cr_out);
   
    int width_chroma = InfoHeader.width / 2;
    int height_chroma = InfoHeader.height / 2;
    int totalPixels_chroma = width_chroma * height_chroma;
    // 10.1 Calcula número de blocos
    int blocosY_w = InfoHeader.width / 8, blocosY_h = InfoHeader.height / 8;
    int blocosCb_w = width_chroma / 8, blocosCb_h = height_chroma / 8;
    int total_blocos_Y = blocosY_w * blocosY_h;
    int total_blocos_C = blocosCb_w * blocosCb_h;

    // 10.2 Aloca saída zig-zag
    int **zigzag_Y = malloc(total_blocos_Y * sizeof(int*));
    int **zigzag_Cb = malloc(total_blocos_C * sizeof(int*));
    int **zigzag_Cr = malloc(total_blocos_C * sizeof(int*));
    for (int i = 0; i < total_blocos_Y; i++) zigzag_Y[i] = malloc(64 * sizeof(int));
    for (int i = 0; i < total_blocos_C; i++) {
        zigzag_Cb[i] = malloc(64 * sizeof(int));
        zigzag_Cr[i] = malloc(64 * sizeof(int));
    }

    reconstructImageFromDCT(quantized_Y_out,quantized_Cb_out,quantized_Cr_out,InfoHeader,converted); 

    // Converte de volta para RGB
    Pixel *Image2 = convertYCbCrToRgb(converted, InfoHeader);

    // Salva imagem resultante
    saveBmpImage("output.bmp", FileHeader, InfoHeader, Image2);

    //  Chama a função de compressão JPEG sem perdas
    comprimirJPEGSemPerdas(IMG, OUTPUT_JPEG);

    printf("Imagem comprimida com sucesso em: %s\n", OUTPUT_JPEG);

    // 6. Libera a memória
    free(Image);
    free(Image2);
    free(converted);

    for (int i = 0; i < total_blocos_Y; i++) free(zigzag_Y[i]);
    for (int i = 0; i < total_blocos_C; i++) {
        free(zigzag_Cb[i]);
        free(zigzag_Cr[i]);
    }
    free(zigzag_Y);
    free(zigzag_Cb);
    free(zigzag_Cr);
    return 0;
}