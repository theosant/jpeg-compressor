#include <stdio.h>
#include <stdlib.h>
#include "include/jpeg.h"

#define IMG "test_images/paisagem_32x32.bmp"
#define OUTPUT_JPEG "paisagem_compressed.jls"  // JLS = JPEG Lossless

int main() {
    FILE *input;
 
    // 1. Abre e verifica o arquivo BMP de entrada
    if(!(input = fopen(IMG, "rb"))) {
        printf("Error: could not open input file.");
        exit(1);
    }

    // 2. Lê os cabeçalhos do BMP
    BitmapFileHeader FileHeader;      
    BitmapInfoHeader InfoHeader; 
    loadBmpHeaders(input, &FileHeader, &InfoHeader);
    printHeaders(&FileHeader, &InfoHeader);
   
    // 3. Carrega os pixels da imagem
    Pixel* Image = loadBmpImage(input, InfoHeader);
    fclose(input);

    // 4. Salva uma cópia do BMP original para verificação
    saveBmpImage("output.bmp", FileHeader, InfoHeader, Image);

    // 5. Chama a função de compressão JPEG sem perdas
    comprimirJPEGSemPerdas(IMG, OUTPUT_JPEG);

    printf("Imagem comprimida com sucesso em: %s\n", OUTPUT_JPEG);

    // 6. Libera a memória
    free(Image);
    
    return 0;
}