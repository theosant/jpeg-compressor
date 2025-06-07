#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"

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