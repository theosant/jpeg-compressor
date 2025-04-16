#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

#define IMG "test_images/paisagem_32x32.bmp"

int main(){
    FILE *input;
    // *output;
    //BITMAPFILEHEADER FileHeader;       /* File header */
    //BITMAPINFOHEADER InfoHeader; 
    //Pixel *Image;
    if(!(input = fopen(IMG, "rb"))){
        printf("Error: could not open input file." );
        exit(1);
}
    loadBMPHeaders (input);
    printf("Hello World!");
    return 0;
}