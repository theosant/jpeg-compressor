/* Pseudocodigo pra lembrar amanha*/

/* 
Ler o cabeçalho do arquivo comprimido (CHECK)

Reconstruir as tabelas Huffman (elas precisam ser armazenadas no arquivo ou recalculadas da mesma forma)

Ler e decodificar os dados comprimidos

Aplicar a DPCM inversa

Converter de YCbCr para RGB

Salvar a imagem descomprimida
*/

#include "descompressao_sem_perdas.h"

void descomprimirJPEGSemPerdas(const char* input_compressed_jpeg, const char* output_bmp) {
    // Abrir arquivo de entrada e ler o cabeçalho
    FILE* in = fopen(input_compressed_jpeg, "rb");
    if (!in) {
        perror("Erro ao abrir arquivo JPEG");
        return;
    }

    JLSHeader header;
    fread(&header, sizeof(JLSHeader), 1, in);
    
    // Verificar cabeçalho
    if (strncmp(header.name, "JLS1", 4) != 0) {
        fprintf(stderr, "Arquivo não é um JLS válido\n");
        fclose(in);
        return;
    }


}