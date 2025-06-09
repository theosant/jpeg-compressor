#include "../include/huffman.h"

/* Início das funções min-heap*/
/* Início das funções Auxiliares*/
static void trocarNos(HuffmanNode** a, HuffmanNode** b) {
    HuffmanNode* temp = *a;
    *a = *b;
    *b = temp;
}

/* Função para manter a propriedade da min-heap (o valor de cada nó deve ser menor ou 
igual ao dos seus filhos). Usado após inserir um novo elemento no final da heap */
static void heapifyUp(MinHeap* heap, int indice) {
    int pai = (indice - 1) / 2;
    while (indice > 0 && heap->nos[indice]->frequencia < heap->nos[pai]->frequencia) {
        trocarNos(&heap->nos[indice], &heap->nos[pai]);
        indice = pai;
        pai = (indice - 1) / 2;
    }
}

/* Outra função para manter a propriedade da min-heap. Usado após remover a raiz */
static void heapifyDown(MinHeap* heap, unsigned indice) {
    unsigned menor = indice;
    unsigned esquerda = 2 * indice + 1;
    unsigned direita = 2 * indice + 2;

    if (esquerda < heap->tamanho && 
        heap->nos[esquerda]->frequencia < heap->nos[menor]->frequencia) {
        menor = esquerda;
    }
    if (direita < heap->tamanho && 
        heap->nos[direita]->frequencia < heap->nos[menor]->frequencia) {
        menor = direita;
    }
    if (menor != indice) {
        trocarNos(&heap->nos[indice], &heap->nos[menor]);
        heapifyDown(heap, menor);
    }
}

/* Fim das funções auxiliares*/

/* Início das funções principais*/
MinHeap* criarMinHeap(unsigned capacidade) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->tamanho = 0;
    heap->capacidade = capacidade;
    heap->nos = (HuffmanNode**)malloc(capacidade * sizeof(HuffmanNode*));
    return heap;
}

void inserirMinHeap(MinHeap* heap, HuffmanNode* no) {
    if (heap->tamanho == heap->capacidade) {
        fprintf(stderr, "Erro: Heap cheia\n");
        return;
    }
    heap->nos[heap->tamanho] = no;
    heapifyUp(heap, heap->tamanho);
    heap->tamanho++;
}

/* Extrai o nó com menor frequenência*/
HuffmanNode* extrairMin(MinHeap* heap) {
    if (heap->tamanho == 0) {
        fprintf(stderr, "Erro: Heap vazia\n");
        return NULL;
    }
    HuffmanNode* min = heap->nos[0];
    heap->nos[0] = heap->nos[heap->tamanho - 1];
    heap->tamanho--;
    heapifyDown(heap, 0);
    return min;
}

void destruirMinHeap(MinHeap* heap) {
    free(heap->nos);
    free(heap);
}
/* Fim das funções principais*/
/* Fim das funções min-heap*/

/* Início das funções da árvore Huffman*/
/* Início das funções principais*/
HuffmanNode* criarNo(int valor, unsigned frequencia) {
    HuffmanNode* novoNo = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    
    if (novoNo == NULL) {
        fprintf(stderr, "Erro: Falha ao alocar memoria para o no.\n");
        exit(EXIT_FAILURE);
    }

    novoNo->valor = valor;
    novoNo->frequencia = frequencia;
    novoNo->esquerda = NULL;
    novoNo->direita = NULL;

    return novoNo;
}

/* Funções da árvore Huffman */
HuffmanNode* construirArvoreHuffman(const int* dados, unsigned tamanho) {
    // 1. Contar frequências
    int frequencias[256] = {0};
    for (unsigned i = 0; i < tamanho; i++) {
        frequencias[dados[i]]++;
    }

    // 2. Criar min-heap
    MinHeap* heap = criarMinHeap(256);
    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0) {
            inserirMinHeap(heap, criarNo(i, frequencias[i]));
        }
    }

    // 3. Construir árvore
    while (heap->tamanho > 1) {
        HuffmanNode* esquerda = extrairMin(heap);
        HuffmanNode* direita = extrairMin(heap);
        
        HuffmanNode* top = criarNo('$', esquerda->frequencia + direita->frequencia);
        top->esquerda = esquerda;
        top->direita = direita;
        
        inserirMinHeap(heap, top);
    }

    HuffmanNode* raiz = extrairMin(heap);
    destruirMinHeap(heap);
    return raiz;
}

void liberarArvoreHuffman(HuffmanNode* raiz) {
    if (raiz) {
        liberarArvoreHuffman(raiz->esquerda);
        liberarArvoreHuffman(raiz->direita);
        free(raiz);
    }
}

/* Funções da tabela Huffman */
void gerarCodigos(HuffmanNode* raiz, char* codigo, int top, TabelaHuffman* tabela) {
    if (!raiz) return;

    if (raiz->esquerda) {
        codigo[top] = '0';
        gerarCodigos(raiz->esquerda, codigo, top + 1, tabela);
    }
    if (raiz->direita) {
        codigo[top] = '1';
        gerarCodigos(raiz->direita, codigo, top + 1, tabela);
    }
    if (!raiz->esquerda && !raiz->direita) {
        // Verifica se o valor está dentro do intervalo válido
        if (raiz->valor >= 0 && raiz->valor < 256) {
            tabela->codigos[raiz->valor] = malloc(top + 1);
            if (!tabela->codigos[raiz->valor]) {
                perror("Falha ao alocar código");
                exit(EXIT_FAILURE);
            }
            memcpy(tabela->codigos[raiz->valor], codigo, top);
            tabela->codigos[raiz->valor][top] = '\0';
        } else {
            printf("[ERRO] Valor inválido: %d\n", raiz->valor);
        }
    }
}

// Função aux para gerar tabela huffman
TabelaHuffman* construirTabelaHuffmanComFrequencias(const int* frequencias) {
    MinHeap* heap = criarMinHeap(256);
    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0) {
            inserirMinHeap(heap, criarNo(i, frequencias[i]));
        }
    }

    // Construir árvore
    while (heap->tamanho > 1) {
        HuffmanNode* esquerda = extrairMin(heap);
        HuffmanNode* direita = extrairMin(heap);
        HuffmanNode* top = criarNo('$', esquerda->frequencia + direita->frequencia);
        top->esquerda = esquerda;
        top->direita = direita;
        inserirMinHeap(heap, top);
    }

    HuffmanNode* raiz = extrairMin(heap);
    destruirMinHeap(heap);

    TabelaHuffman* tabela = malloc(sizeof(TabelaHuffman));
    tabela->codigos = calloc(256, sizeof(char*));
    tabela->simbolos = malloc(256 * sizeof(int));
    tabela->tamanho = 256;

    char codigo[256];
    gerarCodigos(raiz, codigo, 0, tabela);
    liberarArvoreHuffman(raiz);
    return tabela;
}

TabelaHuffman* construirTabelaHuffman(BlocoYCbCr* blocos, int num_blocos, char componente) {
    int frequencias[256] = {0};

    for (int i = 0; i < num_blocos; i++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int valor;
                if (componente == COMP_Y) {
                    valor = (int)round(blocos[i].Y[y][x]);
                } else if (componente == COMP_CB) {
                    valor = (int)round(blocos[i].Cb[y][x]);
                } else {
                    valor = (int)round(blocos[i].Cr[y][x]);
                }

                valor = valor < 0 ? 0 : (valor > 255 ? 255 : valor);
                frequencias[valor]++;
            }
        }
    }

    for (int i = 0; i < 256; i++) {
        if (frequencias[i] == 0) {
            frequencias[i] = 1;
        }
    }

    return construirTabelaHuffmanComFrequencias(frequencias);
}


void destruirTabelaHuffman(TabelaHuffman* tabela) {
    if (tabela) {
        for (unsigned i = 0; i < 256; i++) {
            if (tabela->codigos[i]) {
                free(tabela->codigos[i]);
            }
        }
        free(tabela->codigos);
        free(tabela->simbolos);
        free(tabela);
    }
}