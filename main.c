#include <stdio.h>
#include <stdlib.h>
#include "include/compressao_sem_perdas.h"
#include "include/descompressao_sem_perdas.h"

#define IMG "test_images/paisagem_32x32.bmp"
#define OUTPUT_JPEG "paisagem_compressed.jls"  // JLS = JPEG Lossless
#define OUTPUT_BMP "paisagem_decompressed.bmp"

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

    // 5. Fazer um menu que permite ao usuário selecionar a opção que ele quer
    printf("====== Escolha a opcao que deseja: =====\n");
    printf("1. Comprimir JPEG sem perdas\n");
    printf("2. Comprimir JPEG com perdas\n");
    printf("3. Descomprimir JPEG sem perdas\n");
    printf("4. Descomprimir JPEG com perdas\n");
    printf("0. Cancelar operacao\n");

    int opcao;
    printf("Digite sua opcao: ");
    scanf("%d", &opcao);

    switch (opcao) {
        case 1:
            printf("Iniciando compressao sem perdas...\n");
            comprimirJPEGSemPerdas(IMG, OUTPUT_JPEG);
            printf("Imagem comprimida com sucesso em: %s\n", OUTPUT_JPEG);
            break;
        case 2:
            printf("Iniciando compressao com perdas...\n");
            // comprimirJPEGComPerdas(IMG, OUTPUT_JPEG);
            printf("Imagem comprimida com sucesso em: %s\n", OUTPUT_JPEG);
            break;
        case 3:
            printf("Iniciando descompressao sem perdas...\n");
            descomprimirJPEGSemPerdas(OUTPUT_JPEG, OUTPUT_BMP);
            printf("Imagem descomprimida com sucesso em: %s\n", OUTPUT_BMP);
            break;
        case 4:
            printf("Iniciando descompressao com perdas...\n");
            // descomprimirJPEGSemPerdas(OUTPUT_JPEG, OUTPUT_BMP);
            // printf("Imagem descomprimida com sucesso em: %s\n", OUTPUT_JPEG);
            break;
        default:
            printf("Saindo do programa...");
            exit(0);
    }

    // 6. Libera a memória
    free(Image);
    
    return 0;
}