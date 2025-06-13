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

Pixel* convertYCbCrToRgb(PixelYCbCr* imagem_ycbcr, int largura, int altura) {
    int total_pixels = largura * altura;
    Pixel* imagem_rgb = malloc(total_pixels * sizeof(Pixel));
    
    if (!imagem_rgb) {
        perror("Erro ao alocar memória para imagem RGB");
        return NULL;
    }

    for (int i = 0; i < total_pixels; i++) {
        // Obter componentes YCbCr
        float Y = imagem_ycbcr[i].Y;
        float Cb = imagem_ycbcr[i].Cb - 128.0f;  // Centraliza em 0
        float Cr = imagem_ycbcr[i].Cr - 128.0f;  // Centraliza em 0

        // Fórmulas de conversão padrão ITU-R BT.601
        float R = Y + 1.402f * Cr;
        float G = Y - 0.344136f * Cb - 0.714136f * Cr;
        float B = Y + 1.772f * Cb;

        // Garantir que os valores estão no intervalo [0, 255]
        imagem_rgb[i].R = (unsigned char)(R < 0 ? 0 : (R > 255 ? 255 : R));
        imagem_rgb[i].G = (unsigned char)(G < 0 ? 0 : (G > 255 ? 255 : G));
        imagem_rgb[i].B = (unsigned char)(B < 0 ? 0 : (B > 255 ? 255 : B));
    }

    return imagem_rgb;
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

    for (int i = 0; i < *num_blocos; i++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (blocos[i].Y[y][x] < 0 || blocos[i].Y[y][x] > 255 ||
                    blocos[i].Cb[y][x] < 0 || blocos[i].Cb[y][x] > 255 ||
                    blocos[i].Cr[y][x] < 0 || blocos[i].Cr[y][x] > 255) {
                    printf("[WARNING DIVISAO] Bloco %d (%d,%d) - Y=%.2f, Cb=%.2f, Cr=%.2f\n",
                        i, y, x, blocos[i].Y[y][x], blocos[i].Cb[y][x], blocos[i].Cr[y][x]);
                }
            }
        }
    }
    
    return blocos;
}