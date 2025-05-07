#ifndef _BITMAP_H_
#  define _BITMAP_H_


typedef struct                       /**** BMP file header structure ****/
{
unsigned short Type;           /* Magic number for file */
unsigned int   Size;           /* Size of file */
unsigned short Reserved1;      /* Reserved */
unsigned short Reserved2;      /* ... */
unsigned int   OffBits;        /* Offset to bitmap data */
} BITMAPFILEHEADER;

#  define BF_TYPE 0x4D42             /* "MB" */

typedef struct                       /**** BMP file info structure ****/
{
unsigned int   Size;           /* Size of info header */
int            Width;          /* Width of image */
int            Height;         /* Height of image */
unsigned short Planes;         /* Number of color planes */
unsigned short BitCount;       /* Number of bits per pixel */
unsigned int   Compression;    /* Type of compression to use */
unsigned int   SizeImage;      /* Size of image data */
int            XResolution;    /* X pixels per meter */
int            YResolution;    /* Y pixels per meter */
unsigned int   NColours;        /* Number of colors used */
unsigned int   ImportantColours;   /* Number of important colors */
} BITMAPINFOHEADER;

typedef struct
{
        unsigned char R;
        unsigned char G;
        unsigned char B;      
} Pixel;

void loadBMPHeaders (FILE *fp,  BITMAPFILEHEADER *FileHeader, BITMAPINFOHEADER *InfoHeader);


void printHeaders (BITMAPFILEHEADER *FileHeader,  BITMAPINFOHEADER *InfoHeader);
BITMAPINFOHEADER leituraInfoHeader(FILE *F);
BITMAPFILEHEADER leituraHeader(FILE *F);
Pixel* loadBMPImage(FILE *input, BITMAPINFOHEADER InfoHeader);

#endif