#ifndef _BITMAP_H_
#  define _BITMAP_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct                       /**** BMP file header structure ****/
{
unsigned short type;           /* Magic number for file */
unsigned int   size;           /* Size of file */
unsigned short reserved1;      /* Reserved */
unsigned short reserved2;      /* ... */
unsigned int   offsetBits;        /* Offset to bitmap data */
} BitmapFileHeader;

#  define BF_TYPE 0x4D42             /* "MB" */

typedef struct                       /**** BMP file info structure ****/
{
unsigned int   size;           /* Size of info header */
int            width;          /* Width of image */
int            height;         /* Height of image */
unsigned short planes;         /* Number of color planes */
unsigned short bitCount;       /* Number of bits per pixel */
unsigned int   compression;    /* Type of compression to use */
unsigned int   imageSize;      /* Size of image data */
int            xResolution;    /* X pixels per meter */
int            yResolution;    /* Y pixels per meter */
unsigned int   colorsUsed;        /* Number of colors used */
unsigned int   importantColors;   /* Number of important colors */
} BitmapInfoHeader;

typedef struct
{
        unsigned char R;
        unsigned char G;
        unsigned char B;      
} Pixel;

void loadBmpHeaders(FILE *fp, BitmapFileHeader *fileHeader, BitmapInfoHeader *infoHeader);
BitmapFileHeader readFileHeader(FILE *fp);
BitmapInfoHeader readInfoHeader(FILE *fp);
Pixel* loadBmpImage(FILE *fp, BitmapInfoHeader infoHeader);
void printHeaders(BitmapFileHeader *fileHeader, BitmapInfoHeader *infoHeader);
void printPixelValues(Pixel *image, BitmapInfoHeader infoHeader);
void writeFileHeader(FILE *fp, BitmapFileHeader fileHeader);
void writeInfoHeader(FILE *fp, BitmapInfoHeader infoHeader);
void saveBmpImage(const char *filename, BitmapFileHeader fileHeader, BitmapInfoHeader infoHeader, Pixel *image);

#endif