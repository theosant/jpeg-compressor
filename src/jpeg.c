#include "../include/jpeg.h"

PixelYCbCr* convertRgbToYCbCr(Pixel *input, BitmapInfoHeader infoHeader) {
    int totalPixels = infoHeader.width * infoHeader.height;
    PixelYCbCr *converted = malloc(totalPixels * sizeof(PixelYCbCr));
    if (!converted) return NULL;

    for (int i = 0; i < totalPixels; i++) {
        double r = input[i].R;
        double g = input[i].G;
        double b = input[i].B;

        converted[i].Y = round(0.299 * r + 0.587 * g + 0.114 * b);
        converted[i].Cb = round(-0.168736 * r - 0.331264 * g + 0.5 * b + 128);
        converted[i].Cr = round(0.5 * r - 0.418688 * g - 0.081312 * b + 128);
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
        float Y  = imagem_ycbcr[i].Y;
        float Cb = imagem_ycbcr[i].Cb;
        float Cr = imagem_ycbcr[i].Cr;
        
        float Cb_centered = Cb - 128.0f;
        float Cr_centered = Cr - 128.0f;

        // Fórmulas de conversão padrão (ITU-R BT.601)
        float R = Y + 1.402f * Cr_centered;
        float G = Y - 0.344136f * Cb_centered - 0.714136f * Cr_centered;
        float B = Y + 1.772f * Cb_centered;

        imagem_rgb[i].R = (unsigned char)(fmax(0, fmin(255, R)));
        imagem_rgb[i].G = (unsigned char)(fmax(0, fmin(255, G)));
        imagem_rgb[i].B = (unsigned char)(fmax(0, fmin(255, B)));
    }

    return imagem_rgb;
}

BlocoYCbCr* dividirBlocos(PixelYCbCr* imagem, int largura, int altura, int* num_blocos) {
    int blocos_largura = (largura + 7) / 8;
    int blocos_altura = (altura + 7) / 8;
    *num_blocos = blocos_largura * blocos_altura;
    
    BlocoYCbCr* blocos = malloc(*num_blocos * sizeof(BlocoYCbCr));
    if (!blocos) { return NULL; }
    
    for (int by = 0; by < blocos_altura; by++) {
        for (int bx = 0; bx < blocos_largura; bx++) {
            BlocoYCbCr* bloco = &blocos[by * blocos_largura + bx];
            
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int px = bx * 8 + x;
                    int py = by * 8 + y;
                    
                    if (px < largura && py < altura) {
                        int idx = py * largura + px;
                        bloco->Y[y][x]  = round(imagem[idx].Y);
                        bloco->Cb[y][x] = round(imagem[idx].Cb);
                        bloco->Cr[y][x] = round(imagem[idx].Cr);

                    } else {
                        // Lógica de padding
                        if (x > 0) {
                            bloco->Y[y][x] = bloco->Y[y][x-1];
                            bloco->Cb[y][x] = bloco->Cb[y][x-1];
                            bloco->Cr[y][x] = bloco->Cr[y][x-1];
                        } else if (y > 0) {
                             bloco->Y[y][x] = bloco->Y[y-1][x];
                             bloco->Cb[y][x] = bloco->Cb[y-1][x];
                             bloco->Cr[y][x] = bloco->Cr[y-1][x];
                        } else {
                            bloco->Y[y][x] = 128; // Cinza
                            bloco->Cb[y][x] = 128;
                            bloco->Cr[y][x] = 128;
                        }
                    }
                }
            }
        }
    }
    
    return blocos;
}