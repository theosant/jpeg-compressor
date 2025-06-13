#include "../include/compressao_sem_perdas.h"

void DPCM(BlocoYCbCr* blocos, int num_blocos) {
    int dc_prev_Y = 0, dc_prev_Cb = 0, dc_prev_Cr = 0;

    for (int i = 0; i < num_blocos; i++) {
        // Armazena os valores originais antes de calcular as diferenças
        int original_Y = blocos[i].Y[0][0];
        int original_Cb = blocos[i].Cb[0][0];
        int original_Cr = blocos[i].Cr[0][0];

        // Calcula as diferenças usando apenas inteiros
        blocos[i].Y[0][0] = original_Y - dc_prev_Y;
        blocos[i].Cb[0][0] = original_Cb - dc_prev_Cb;
        blocos[i].Cr[0][0] = original_Cr - dc_prev_Cr;

        // Atualiza os valores anteriores
        dc_prev_Y = original_Y;
        dc_prev_Cb = original_Cb;
        dc_prev_Cr = original_Cr;
    }
}


// Função de processamento de componentes
void processarComponente(int componente[8][8], TabelaHuffman* tabela, unsigned char* buffer, int* pos, FILE* output) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int valor = componente[y][x];
            
            const char* codigo = buscarCodigo(tabela, valor);
            if (!codigo) {
                fprintf(stderr, "[ERRO] Valor %d não encontrado na tabela Huffman!\n", valor);
                exit(EXIT_FAILURE);
            }

            // Escreve os bits no buffer
            for (int i = 0; codigo[i] != '\0'; i++) {
                if (codigo[i] == '1') {
                    *buffer |= (1 << (7 - *pos));
                }
                (*pos)++;
                
                if (*pos == 8) {
                    fwrite(buffer, 1, 1, output);
                    *buffer = 0;
                    *pos = 0;
                }
            }
        }
    }
}

void comprimeBloco(BlocoYCbCr bloco, TabelaHuffman* tabela_Y, TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr,
                   unsigned char* buffer, int* pos, FILE* output) {
    processarComponente(bloco.Y, tabela_Y, buffer, pos, output);
    processarComponente(bloco.Cb, tabela_Cb, buffer, pos, output);
    processarComponente(bloco.Cr, tabela_Cr, buffer, pos, output);
}


void comprimirJPEGSemPerdas(const char* input_bmp, const char* output_jpeg) {
    printf("\n[DEBUG] Iniciando compressao JPEG sem perdas...\n");
    
    // 1. Carregar a imagem BMP e seus cabeçallhos
    FILE* fp = fopen(input_bmp, "rb");
    if (!fp) {
        perror("Erro ao abrir arquivo BMP");
        return;
    }
    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;
    loadBmpHeaders(fp, &fileHeader, &infoHeader);
    Pixel* imagem_rgb = loadBmpImage(fp, infoHeader);
    fclose(fp);
    
    // 2. Converter para o espaço de cores YCbCr
    PixelYCbCr* imagem_ycbcr = convertRgbToYCbCr(imagem_rgb, infoHeader);
    
    // 3. Dividir a imagem em blocos 8x8
    int num_blocos;
    BlocoYCbCr* blocos = dividirBlocos(imagem_ycbcr, infoHeader.width, infoHeader.height, &num_blocos);

    // 4. Salvar uma cópia dos blocos originais para verificação posterior na descompressão
    BlocoYCbCr* blocos_originais = malloc(num_blocos * sizeof(BlocoYCbCr));
    memcpy(blocos_originais, blocos, num_blocos * sizeof(BlocoYCbCr));
    FILE* f_orig = fopen("temp_originais.bin", "wb");
    fwrite(&num_blocos, sizeof(int), 1, f_orig);
    fwrite(blocos_originais, sizeof(BlocoYCbCr), num_blocos, f_orig);
    fclose(f_orig);
    free(blocos_originais);
    
    // 5. Aplicar DPCM para codificar as diferenças dos coeficientes DC
    DPCM(blocos, num_blocos);
    
    // 6. Construir as tabelas de Huffman para cada componente
    TabelaHuffman* tabela_Y = construirTabelaHuffman(blocos, num_blocos, COMP_Y);
    TabelaHuffman* tabela_Cb = construirTabelaHuffman(blocos, num_blocos, COMP_CB);
    TabelaHuffman* tabela_Cr = construirTabelaHuffman(blocos, num_blocos, COMP_CR);
    
    // 7. Iniciar a escrita do arquivo de saída
    FILE* out = fopen(output_jpeg, "wb");
    if (!out) {
        perror("Erro ao criar arquivo de saída");
        return;
    }
    
    // 8. Escrever o cabeçalho do arquivo JLS
    JLSHeader header;
    memcpy(header.name, "JLS1", 4);
    header.width = infoHeader.width;
    header.height = infoHeader.height;
    header.dataSize = num_blocos;
    header.bmp_file_header = fileHeader;
    header.bmp_info_header = infoHeader;

    fwrite(&header, sizeof(JLSHeader), 1, out);

    // 9. Escrever as tabelas de Huffman no arquivo
    escreverTabelaHuffman(tabela_Y, out);
    escreverTabelaHuffman(tabela_Cb, out);
    escreverTabelaHuffman(tabela_Cr, out);
    
    // 10. Comprimir os dados dos blocos em um fluxo de bits contínuo
    unsigned char buffer_escrita = 0;
    int pos_bit_escrita = 0;
    printf("[DEBUG] Comprimindo %d blocos...\n", num_blocos);
    for (int i = 0; i < num_blocos; i++) {
        comprimeBloco(blocos[i], tabela_Y, tabela_Cb, tabela_Cr, 
                      &buffer_escrita, &pos_bit_escrita, out);
    }
    
    // Após o loop, se sobraram bits no buffer, escreve o último byte.
    if (pos_bit_escrita > 0) {
        printf("[DEBUG] Escrevendo último byte com %d bits válidos.\n", pos_bit_escrita);
        fwrite(&buffer_escrita, 1, 1, out);
    }
    
    long bytes_escritos = ftell(out);
    printf("[DEBUG] Compressão finalizada. Total de bytes escritos no corpo: %ld\n", bytes_escritos);
    
    fclose(out);
    
    // 11. Liberar toda a memória alocada
    free(imagem_rgb);
    free(imagem_ycbcr);
    free(blocos);
    destruirTabelaHuffman(tabela_Y);
    destruirTabelaHuffman(tabela_Cb);
    destruirTabelaHuffman(tabela_Cr);
}