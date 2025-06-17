#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "jpeg.h"

#define BLOCK_SIZE 8
#define K_FACTOR 1.0

#define IMG "test_images/paisagem_32x32.bmp"
#define OUTPUT_JPEG "paisagem_compressed.jls"  // JLS = JPEG Lossless

const int LUMINANCE_Q_TABLE[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

const int CHROMINANCE_Q_TABLE[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}
};
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

    // Aplica DCT 
    int width_chroma = InfoHeader.width / 2;
    int height_chroma = InfoHeader.height / 2;
    int totalPixels_chroma = width_chroma * height_chroma;

    double *dctCoeffs_Y = malloc(totalPixels * sizeof(double));
    double *dctCoeffs_Cr = malloc(totalPixels_chroma * sizeof(double));
    double *dctCoeffs_Cb = malloc(totalPixels_chroma * sizeof(double));
    applyDctToImage(Y_component, InfoHeader.width, InfoHeader.height, dctCoeffs_Y, C);
    applyDctToImage(CrDown, width_chroma, height_chroma, dctCoeffs_Cr, C);
    applyDctToImage(CbDown, width_chroma, height_chroma, dctCoeffs_Cb, C);

    int *quantized_Cb = malloc(totalPixels_chroma * sizeof(int));
    int *quantized_Cr = malloc(totalPixels_chroma * sizeof(int));
    int *quantized_Y = malloc(totalPixels * sizeof(int));
    quantizeImage(dctCoeffs_Y, quantized_Y, InfoHeader.width, InfoHeader.height, LUMINANCE_Q_TABLE, K_FACTOR);
    quantizeImage(dctCoeffs_Cb, quantized_Cb, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);
    quantizeImage(dctCoeffs_Cr, quantized_Cr, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);

   // 10. Zig-Zag scan

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

    // 10.3 Aplica zig-zag Y
    int idx = 0;
    for (int y = 0; y < InfoHeader.height; y += 8) {
        for (int x = 0; x < InfoHeader.width; x += 8) {
            aplicarZigZagBloco(quantized_Y, InfoHeader.width, y, x, zigzag_Y[idx++]);
        }
    }

    // 10.4 Aplica zig-zag Cb
    idx = 0;
    for (int y = 0; y < height_chroma; y += 8) {
        for (int x = 0; x < width_chroma; x += 8) {
            aplicarZigZagBloco(quantized_Cb, width_chroma, y, x, zigzag_Cb[idx++]);
        }
    }

    // 10.5 Aplica zig-zag Cr
    idx = 0;
    for (int y = 0; y < height_chroma; y += 8) {
        for (int x = 0; x < width_chroma; x += 8) {
            aplicarZigZagBloco(quantized_Cr, width_chroma, y, x, zigzag_Cr[idx++]);
        }
    }
    
    aplicarDPCMnosDCs(zigzag_Y, total_blocos_Y);
    aplicarDPCMnosDCs(zigzag_Cb, total_blocos_C);
    aplicarDPCMnosDCs(zigzag_Cr, total_blocos_C);

    double *dequantized_dct_Y = malloc(totalPixels * sizeof(double));
    double *dequantized_dct_Cb = malloc(totalPixels_chroma * sizeof(double));
    double *dequantized_dct_Cr = malloc(totalPixels_chroma * sizeof(double));
    dequantizeImage(quantized_Y, dequantized_dct_Y, InfoHeader.width, InfoHeader.height, LUMINANCE_Q_TABLE, K_FACTOR);
    dequantizeImage(quantized_Cb, dequantized_dct_Cb, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);
    dequantizeImage(quantized_Cr, dequantized_dct_Cr, width_chroma, height_chroma, CHROMINANCE_Q_TABLE, K_FACTOR);
    
    // Aplica IDCT
    double *reconstructedCbDown = malloc(totalPixels_chroma * sizeof(double));
    double *reconstructedCrDown = malloc(totalPixels_chroma * sizeof(double));
    double *reconstructedY = malloc(totalPixels * sizeof(double));
    applyIdctToImage(dequantized_dct_Cb, width_chroma, height_chroma, reconstructedCbDown, C);
    applyIdctToImage(dequantized_dct_Cr, width_chroma, height_chroma, reconstructedCrDown, C);
    applyIdctToImage(dequantized_dct_Y, InfoHeader.width, InfoHeader.height, reconstructedY, C);

    // Atualiza canal Y com reconstruído
    for (int i = 0; i < totalPixels; i++) {
        converted[i].Y = reconstructedY[i];
    }

    // Upsampling de Cb e Cr
    double *CbFull, *CrFull;
    upSampling(reconstructedCbDown , reconstructedCrDown , InfoHeader, &CbFull, &CrFull);

     // Atualiza canais Cb e Cr no struct de YCbCr
    for (int i = 0; i < totalPixels; i++) {
        converted[i].Cb = CbFull[i];
        converted[i].Cr = CrFull[i];
    }
    // Converte de volta para RGB
    Pixel *Image2 = convertYCbCrToRgb(converted, InfoHeader);

    // Salva imagem resultante
    saveBmpImage("output.bmp", FileHeader, InfoHeader, Image2);

    // 5. Chama a função de compressão JPEG sem perdas
    comprimirJPEGSemPerdas(IMG, OUTPUT_JPEG);

    printf("Imagem comprimida com sucesso em: %s\n", OUTPUT_JPEG);

    // 6. Libera a memória
    free(Image);
    free(Image2);
    free(converted);
    free(Y_component);
    free(CbDown);
    free(CrDown);

    free(dctCoeffs_Y);
    free(quantized_Y);
    free(dequantized_dct_Y);
    free(reconstructedY);
    
    free(dctCoeffs_Cb);
    free(quantized_Cb);
    free(dequantized_dct_Cb);
    free(reconstructedCbDown);
    
    free(dctCoeffs_Cr);
    free(quantized_Cr);
    free(dequantized_dct_Cr);
    free(reconstructedCrDown);

    free(CbFull);
    free(CrFull);
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