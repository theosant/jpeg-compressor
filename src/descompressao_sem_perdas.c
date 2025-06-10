/* Pseudocodigo pra lembrar amanha*/

/* 
Ler o cabeçalho do arquivo comprimido (CHECK)

Reconstruir as tabelas Huffman (elas precisam ser armazenadas no arquivo ou recalculadas da mesma forma) (CHECK)

Ler e decodificar os dados comprimidos

Aplicar a DPCM inversa

Converter de YCbCr para RGB

Salvar a imagem descomprimida
*/

#include "descompressao_sem_perdas.h"

// Função auxiliar para recosntriruir as tabelas Huffman
TabelaHuffman* lertabelaHuffman(FILE* input) {
    TabelaHuffman* tabela = malloc(sizeof(TabelaHuffman));

    // Ler tamanho da tabela
    fread(&tabela->tamanho, sizeof(unsigned), 1, input);
    
    // Alocar memória
    tabela->codigos = calloc(tabela->tamanho, sizeof(char*));
    tabela->simbolos = malloc(tabela->tamanho * sizeof(int));

    // Ler cada entrada
    for (unsigned i = 0; i < tabela->tamanho; i++) {
        int simbolo;
        int comprimento;
        
        fread(&simbolo, sizeof(int), 1, input);
        fread(&comprimento, sizeof(int), 1, input);
        
        tabela->simbolos[i] = simbolo;
        
        if (comprimento > 0) {
            tabela->codigos[i] = malloc(comprimento + 1);
            fread(tabela->codigos[i], sizeof(char), comprimento, input);
            tabela->codigos[i][comprimento] = '\0'; // Adiciona terminador
        }
    }
    
    return tabela;
}

// Função principal para descompressão do JPEG
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

    // Recosntruir Tabelas Huffman
    TabelaHuffman* tabela_Y = lertabelaHuffman(input_compressed_jpeg);
    TabelaHuffman* tabela_Cb = lertabelaHuffman(input_compressed_jpeg);
    TabelaHuffman* tabela_Cr = lertabelaHuffman(input_compressed_jpeg);


}