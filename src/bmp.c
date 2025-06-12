#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

void loadBmpHeaders(FILE *fp, BitmapFileHeader *fileHeader, BitmapInfoHeader *infoHeader) {
    *fileHeader = readFileHeader(fp);
    *infoHeader = readInfoHeader(fp);

    if (infoHeader->compression != 0) {
        printf("This is a compressed BMP!\n");
        fclose(fp);
        return;
    }
}


BitmapFileHeader readFileHeader(FILE *fp) {
    BitmapFileHeader header;

    fread(&header.type, sizeof(unsigned short), 1, fp);
    fread(&header.size, sizeof(unsigned int), 1, fp);
    fread(&header.reserved1, sizeof(unsigned short), 1, fp);
    fread(&header.reserved2, sizeof(unsigned short), 1, fp);
    fread(&header.offsetBits, sizeof(unsigned int), 1, fp);

    return header;
}

BitmapInfoHeader readInfoHeader(FILE *fp) {
    BitmapInfoHeader header;

    fread(&header.size, sizeof(unsigned int), 1, fp);
    fread(&header.width, sizeof(int), 1, fp);
    fread(&header.height, sizeof(int), 1, fp);
    fread(&header.planes, sizeof(unsigned short), 1, fp);
    fread(&header.bitCount, sizeof(unsigned short), 1, fp);
    fread(&header.compression, sizeof(unsigned int), 1, fp);
    fread(&header.imageSize, sizeof(unsigned int), 1, fp);
    fread(&header.xResolution, sizeof(int), 1, fp);
    fread(&header.yResolution, sizeof(int), 1, fp);
    fread(&header.colorsUsed, sizeof(unsigned int), 1, fp);
    fread(&header.importantColors, sizeof(unsigned int), 1, fp);

    return header;
}

Pixel* loadBmpImage(FILE *fp, BitmapInfoHeader infoHeader) {
    int totalPixels = infoHeader.width * infoHeader.height;
    Pixel *image = malloc(totalPixels * sizeof(Pixel));

    fseek(fp, 54, SEEK_SET); // skip header (54 bytes)

    for (int i = 0; i < totalPixels; i++) {
        image[i].B = fgetc(fp);
        image[i].G = fgetc(fp);
        image[i].R = fgetc(fp);
    }

    return image;
}

void printHeaders(BitmapFileHeader *fileHeader, BitmapInfoHeader *infoHeader)
 {
      printf("*************** File Header ***************\n\n");
      
      printf("Magic number for file: %x\n", fileHeader->type);   
      printf("File's size: %d\n",fileHeader->size);           
      printf("Offset to bitmap data: %d\n", fileHeader->offsetBits);
      
      printf("\n\n");
      printf("*************** Info Header ***************\n\n");
      printf("Info header's size: %d\n", infoHeader->size);
      printf("Width: %d\n", infoHeader->width);          
      printf("Height: %d\n",infoHeader->height);
      printf("Color planes: %d\n", infoHeader->planes);
      printf("Bits per pixel: %d\n", infoHeader->bitCount);
      printf("Compression type (0 = no compression): %d\n", infoHeader->compression);
      printf("Image's data size: %d\n", infoHeader->imageSize);
      printf("X Pixels per meter: %d\n", infoHeader->xResolution);
      printf("Y Pixels per meter: %d\n", infoHeader->yResolution);
      printf("Number of colors: %d\n", infoHeader->colorsUsed);
      printf("Numberof important colors: %d\n", infoHeader->importantColors); 
      
 }
 
void printPixelValues(Pixel *image, BitmapInfoHeader infoHeader) {
    int width = infoHeader.width;
    int height = infoHeader.height;

    for (int y = 0; y < height; y++) {
        printf("Row %d:\n", y);
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            printf("(%3d,%3d,%3d) ", image[idx].R, image[idx].G, image[idx].B);
        }
        printf("\n");
    }
}

void writeFileHeader(FILE *fp, BitmapFileHeader fileHeader) {
    fwrite(&fileHeader.type, sizeof(unsigned short), 1, fp);
    fwrite(&fileHeader.size, sizeof(unsigned int), 1, fp);
    fwrite(&fileHeader.reserved1, sizeof(unsigned short), 1, fp);
    fwrite(&fileHeader.reserved2, sizeof(unsigned short), 1, fp);
    fwrite(&fileHeader.offsetBits, sizeof(unsigned int), 1, fp);
}

void writeInfoHeader(FILE *fp, BitmapInfoHeader infoHeader) {
    fwrite(&infoHeader.size, sizeof(unsigned int), 1, fp);
    fwrite(&infoHeader.width, sizeof(int), 1, fp);
    fwrite(&infoHeader.height, sizeof(int), 1, fp);
    fwrite(&infoHeader.planes, sizeof(unsigned short), 1, fp);
    fwrite(&infoHeader.bitCount, sizeof(unsigned short), 1, fp);
    fwrite(&infoHeader.compression, sizeof(unsigned int), 1, fp);
    fwrite(&infoHeader.imageSize, sizeof(unsigned int), 1, fp);
    fwrite(&infoHeader.xResolution, sizeof(int), 1, fp);
    fwrite(&infoHeader.yResolution, sizeof(int), 1, fp);
    fwrite(&infoHeader.colorsUsed, sizeof(unsigned int), 1, fp);
    fwrite(&infoHeader.importantColors, sizeof(unsigned int), 1, fp);
}

void saveBmpImage(const char *filename, BitmapFileHeader fileHeader, BitmapInfoHeader infoHeader, Pixel *image) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error opening output file");
        return;
    }

    int totalPixels = infoHeader.width * infoHeader.height;

    writeFileHeader(fp, fileHeader);
    writeInfoHeader(fp, infoHeader);
    // talvez precise de padding
    // talvez esteja de ponta cabeça
    for (int i = 0; i < totalPixels; i++) {
        fputc(image[i].B, fp);
        fputc(image[i].G, fp);
        fputc(image[i].R, fp); 
    }

    fclose(fp);
}


