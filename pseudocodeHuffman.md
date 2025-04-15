# Pseudocódigo Compressão Sem Perdas
Usando o Algoritmo de Huffman. Um método conhecido dessa categoria e também eficiente. 

## Pseudocódigo
### Estruturas
```
struct HuffmanNode {
    int valor
    int frequencia
    HuffmanNode* esquerda
    HuffmanNode* direita
}

struct BlocoYCbCr {  
    int[8][8] Y    // Luminância  
    int[8][8] Cb   // Crominância Azul  
    int[8][8] Cr   // Crominância Vermelha  
} 

struct TabelaHuffman {  
    dicionario<int, string> codigos  // Mapeia símbolos/valores para códigos binários  
}  

```
### Funções auxiliares
```
função ConverterRGBparaYCbCr(bitmap: *, largura: int, altura: int) -> Matriz[YCbCr]

função divisãoBlocos8(matrizYCbCr: Matriz[YCbCr], largura: int, altura: int) -> Vetor[BlocoYCbCr]

função ConstruirTabelaHuffman(blocos: Vetor[BlocoYCbCr], componente: char) -> TabelaHuffman

função ComprimirBlocoHuffman(  
    bloco: BlocoYCbCr,  
    bufferSaida: Vetor[Byte],  
    tabelaHuffman_Y: TabelaHuffman,  
    tabelaHuffman_Cb: TabelaHuffman,  
    tabelaHuffman_Cr: TabelaHuffman  
) -> buffer

```

### Função principal
```
função Comprimir JPEGSemPerdas(bitmap: *, largura: int, altura: int) -> Vetor[Byte]
{
    Converter PGB para YCbCr

    Dividir em blocos de 8x8

    // usar DCPM para melhorar a eficiencia da compressao
    Aplicar DCPM

    Gerar tabelas Huffman para Y, Cb, Cr

    Comprimir e escrever em um buffer de saída

    Escreve cabeçalho (largura, altura, tabelas Huffman)

    Comprime cada bloco usando Huffman

    retornar bufferSaida 
}
```