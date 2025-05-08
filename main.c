#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "jpeg.h"

#define IMG "test_images/paisagem_32x32.bmp"

int main(){
    FILE *input;
 
    if(!(input = fopen(IMG, "rb"))){
        printf("Error: could not open input file." );
        exit(1);
    }

    BitmapFileHeader FileHeader;      
    BitmapInfoHeader InfoHeader; 

    loadBmpHeaders (input, &FileHeader, &InfoHeader);
    printHeaders(&FileHeader, &InfoHeader);
   
    Pixel* Image = loadBmpImage(input, InfoHeader);
    fclose(input);

    saveBmpImage("output.bmp", FileHeader, InfoHeader, Image);

    free(Image);
    
    return 0;
}