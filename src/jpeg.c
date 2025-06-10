#include "../include/jpeg.h"

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader) {
    int totalPixels = infoHeader.width * infoHeader.height;
    PixelYCbCr *converted = malloc(totalPixels * sizeof(PixelYCbCr));
    if (!converted) return NULL;

    for (int i = 0; i < totalPixels; i++) {
        double r = input[i].R;
        double g = input[i].G;
        double b = input[i].B;

        converted[i].Y  = 0.299 * r + 0.587 * g + 0.114 * b;
        converted[i].Cb = -0.1687 * r - 0.3313 * g + 0.5 * b + 128;
        converted[i].Cr =  0.5 * r - 0.4187 * g - 0.0813 * b + 128;
    }

    return converted;
}

BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos) {
    // Calcula quantos blocos serão necessários na horizontal e vertical (arredonda para cima a divisão)
    int blocos_largura = (largura + 7) / 8;
    int blocos_altura = (altura + 7) / 8;
    *num_blocos = blocos_largura * blocos_altura;
    
    BlocoYCbCr* blocos = malloc(*num_blocos * sizeof(BlocoYCbCr));
    
    for (int by = 0; by < blocos_altura; by++) {
        for (int bx = 0; bx < blocos_largura; bx++) {
            BlocoYCbCr* bloco = &blocos[by * blocos_largura + bx];
            
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    // Calcula a posição do pixel na imagem original
                    int px = bx * 8 + x;
                    int py = by * 8 + y;
                    
                    if (px < largura && py < altura) {
                        // Se estiver dentro dos limites, copia o pixel
                        int idx = py * largura + px;
                        bloco->Y[y][x] = imagem[idx].Y;
                        bloco->Cb[y][x] = imagem[idx].Cb;
                        bloco->Cr[y][x] = imagem[idx].Cr;
                    } else {
                        // Padding com últimos valores válidos
                        bloco->Y[y][x] = bloco->Y[y][x-1];
                        bloco->Cb[y][x] = bloco->Cb[y][x-1];
                        bloco->Cr[y][x] = bloco->Cr[y][x-1];
                    }
                }
            }
        }
    }
    
    return blocos;
}