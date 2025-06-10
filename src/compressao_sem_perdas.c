#include "../include/compressao_sem_perdas.h"

void DPCM(BlocoYCbCr* blocos, int num_blocos) {
    float dc_prev_Y = 0, dc_prev_Cb = 0, dc_prev_Cr = 0;

    for (int i = 0; i < num_blocos; i++) {
        float dc_Y = blocos[i].Y[0][0];
        float dc_Cb = blocos[i].Cb[0][0];
        float dc_Cr = blocos[i].Cr[0][0];

        blocos[i].Y[0][0] = dc_Y - dc_prev_Y;
        blocos[i].Cb[0][0] = dc_Cb - dc_prev_Cb;
        blocos[i].Cr[0][0] = dc_Cr - dc_prev_Cr;

        dc_prev_Y = dc_Y;
        dc_prev_Cb = dc_Cb;
        dc_prev_Cr = dc_Cr;
    }
}

// Função auxiliar para compirmir_bloco
const char* buscarCodigo(TabelaHuffman* tabela, int valor) {
    valor = valor < 0 ? 0 : (valor > 255 ? 255 : valor);
    return tabela->codigos[valor];
}

// Função auxiliar para escrever um código Huffman no buffer
void escreveCodigo(const char* codigo, unsigned char buffer, int pos, FILE* output) {
    for (int i = 0; codigo[i] != '\0'; i++) {
        if (codigo[i] == '1') {
            buffer |= (1 << (7 - pos));
        }
        pos++;
        
        if (pos == 8) {
            fwrite(&buffer, 1, 1, output);
            buffer = 0;
            pos = 0;
        }
    }
}

// Função auxiliar para processar uma componente 8x8
void processarComponente(float componente[8][8], TabelaHuffman* tabela, unsigned char buffer, int pos, FILE* output) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // Garante que o valor está no intervalo [0, 255]
            int valor = (int)round(componente[y][x]);
            valor = valor < 0 ? 0 : (valor > 255 ? 255 : valor);
            // printf("[DEBUG] Processando valor %d na pos (%d,%d)\n", valor, y, x);
            
            const char* codigo = buscarCodigo(tabela, valor);
            if (!codigo) {
                printf("[ERRO CRITICO] Codigo Huffman nao encontrado para valor %d\n", valor);
                exit(EXIT_FAILURE);
            }
            // printf("[DEBUG] Codigo encontrado: %s\n", codigo);

            escreveCodigo(codigo, buffer, pos, output);
        }
    }
}

void comprimeBloco(BlocoYCbCr bloco, TabelaHuffman* tabela_Y, 
                    TabelaHuffman* tabela_Cb, TabelaHuffman* tabela_Cr, 
                    FILE* output) {
    unsigned char buffer = 0;
    int pos = 0;

    // 1. Comprime componente Y (luminância)
    processarComponente(bloco.Y, tabela_Y, buffer, pos, output);

    // 2. Comprime componente Cb (crominância azul)
    processarComponente(bloco.Cb, tabela_Cb, buffer, pos, output);

    // 3. Comprime componente Cr (crominância vermelha)
    processarComponente(bloco.Cr, tabela_Cr, buffer, pos, output);

    // 4. Escreve quaisquer bits restantes no buffer
    if (pos > 0) {
        fwrite(&buffer, 1, 1, output);
    }
}

// Função auxiliar para escrever no arquivo .jls
void escreverTabelaHuffman(TabelaHuffman* tabela, FILE* output) {
    // tamanho fixo de 256
    fwrite(&tabela->tamanho, sizeof(unsigned), 1, output);

    // Escreve cada símbolo
    for (unsigned i = 0; i < tabela->tamanho; i++){
        fwrite(&i, sizeof(int), 1, output);

        // Escreve o comprimento do código (0 se não existir)
        int comprimento = tabela->codigos[i] ? strlen(tabela->codigos[i]) : 0;
        fwrite(&comprimento, sizeof(int), 1, output);

        // Escreve o código
        if (comprimento > 0) {
            fwrite(tabela->codigos[i], sizeof(char), comprimento, output);
        }
    }
}

// Função principal para comprimir JPEG sem perdas
void comprimirJPEGSemPerdas(const char* input_bmp, const char* output_jpeg) {
    printf("\n[DEBUG] Iniciando compressao JPEG sem perdas...\n");
    
    FILE* fp = fopen(input_bmp, "rb");
    if (!fp) {
        perror("Erro ao abrir arquivo BMP");
        return;
    }
    
    // Ler cabeçalhos e imagem
    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;
    loadBmpHeaders(fp, &fileHeader, &infoHeader);

    Pixel* imagem_rgb = loadBmpImage(fp, infoHeader);
    fclose(fp);
    
    // Converter para YCbCr
    PixelYCbCr* imagem_ycbcr = convertRgbToYCbCr(imagem_rgb, infoHeader);
    
    // Dividir em blocos 8x8
    int num_blocos;
    BlocoYCbCr* blocos = dividirBlocos(imagem_ycbcr, infoHeader.width, infoHeader.height, &num_blocos);
    
    // Aplicar DPCM
    DPCM(blocos, num_blocos);
    
    // Construir tabelas Huffman
    TabelaHuffman* tabela_Y = construirTabelaHuffman(blocos, num_blocos, COMP_Y);
    TabelaHuffman* tabela_Cb = construirTabelaHuffman(blocos, num_blocos, COMP_CB);
    TabelaHuffman* tabela_Cr = construirTabelaHuffman(blocos, num_blocos, COMP_CR);
    
    // Escrever arquivo JPEG
    FILE* out = fopen(output_jpeg, "wb");
    if (!out) {
        perror("Erro ao criar arquivo de saída");
        return;
    }
    
    // Criar cabeçalho
    JLSHeader header;
    memcpy(header.name, "JLS1", 4);
    header.width = infoHeader.width;
    header.height = infoHeader.height;
    header.dataSize = num_blocos;

    // Escrever cabeçalho
    fwrite(&header, sizeof(JLSHeader), 1, out);

    // Escrever tabelas Huffan
    escreverTabelaHuffman(tabela_Y, out);
    escreverTabelaHuffman(tabela_Cb, out);
    escreverTabelaHuffman(tabela_Cr, out);
    
    // Comprimir cada bloco
    printf("[DEBUG] Iniciando compressao dos blocos...\n");
    for (int i = 0; i < num_blocos; i++) {
        comprimeBloco(blocos[i], tabela_Y, tabela_Cb, tabela_Cr, out);
    }
    
    fclose(out);
    
    // Liberar memória
    free(imagem_rgb);
    free(imagem_ycbcr);
    free(blocos);
    destruirTabelaHuffman(tabela_Y);
    destruirTabelaHuffman(tabela_Cb);
    destruirTabelaHuffman(tabela_Cr);
}