#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

#define IMG "test_images/paisagem_32x32.bmp"

int main(){
    FILE *input;
    // *output;
    if(!(input = fopen(IMG, "rb"))){
        printf("Error: could not open input file." );
        exit(1);
    }
    BITMAPFILEHEADER FileHeader;       /* File header */
    BITMAPINFOHEADER InfoHeader; 
    // carregar dois headers: info header e file header
    loadBMPHeaders (input, &FileHeader, &InfoHeader);
    printHeaders(&FileHeader, &InfoHeader);
   
    // carregar pixels
    loadBMPImage(input, InfoHeader);
    return 0;
}