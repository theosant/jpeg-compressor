#include "huffman.h"

// --- Implementações internas do Min Heap (usada para construir a árvore Huffman) ---

static void trocarNos(HuffmanNode** a, HuffmanNode** b) {
    HuffmanNode* temp = *a;
    *a = *b;
    *b = temp;
}

// função para manter a propriedade do Min Heap após a remoção do elemento raiz. 
static void heapifyDown(MinHeap* heap, unsigned indice) {
    unsigned menor = indice;
    unsigned esquerda = 2 * indice + 1;
    unsigned direita = 2 * indice + 2;

    if (esquerda < heap->tamanho && heap->nos[esquerda]->frequencia < heap->nos[menor]->frequencia) {
        menor = esquerda;
    }
    if (direita < heap->tamanho && heap->nos[direita]->frequencia < heap->nos[menor]->frequencia) {
        menor = direita;
    }
    if (menor != indice) {
        trocarNos(&heap->nos[menor], &heap->nos[indice]);
        heapifyDown(heap, menor);
    }
}


// função para manter a propriedade do Min Heap após a adição de um elemento.
static void heapifyUp(MinHeap* heap, int indice) {
    int pai = (indice - 1) / 2;
    while (indice > 0 && heap->nos[indice]->frequencia < heap->nos[pai]->frequencia) {
        trocarNos(&heap->nos[indice], &heap->nos[pai]);
        indice = pai;
        pai = (indice - 1) / 2;
    }
}

static MinHeap* criarMinHeap(unsigned capacidade) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->tamanho = 0;
    heap->capacidade = capacidade;
    heap->nos = (HuffmanNode**)malloc(capacidade * sizeof(HuffmanNode*));
    return heap;
}

static void inserirMinHeap(MinHeap* heap, HuffmanNode* no) {
    if (heap->tamanho == heap->capacidade) return;
    heap->tamanho++;
    int i = heap->tamanho - 1;
    heap->nos[i] = no;
    heapifyUp(heap, i);
}

// Extrai o nó com a menor frequência (que está sempre na raiz do Min Heap).
static HuffmanNode* extrairMin(MinHeap* heap) {
    if (heap->tamanho == 0) return NULL;
    HuffmanNode* temp = heap->nos[0];
    heap->nos[0] = heap->nos[heap->tamanho - 1];
    heap->tamanho--;
    heapifyDown(heap, 0);
    return temp;
}

static void destruirMinHeap(MinHeap* heap) {
    if (heap) {
        free(heap->nos);
        free(heap);
    }
}

// --- Funções da Árvore e Tabela de Huffman ---

static HuffmanNode* criarNo(int valor, unsigned frequencia) {
    HuffmanNode* no = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    no->valor = valor;
    no->frequencia = frequencia;
    no->esquerda = no->direita = NULL;
    return no;
}

void liberarArvoreHuffman(HuffmanNode* raiz) {
    if (raiz) {
        liberarArvoreHuffman(raiz->esquerda);
        liberarArvoreHuffman(raiz->direita);
        free(raiz);
    }
}

// Função recursiva que percorre a árvore de Huffman para gerar os códigos binários.
static void gerarCodigos(HuffmanNode* raiz, char* codigo, int top, TabelaHuffman* tabela) {
    // Caminho para a esquerda adiciona '0' ao código, caminho para a direita adiciona '1'.
    if (raiz->esquerda) {
        codigo[top] = '0';
        gerarCodigos(raiz->esquerda, codigo, top + 1, tabela);
    }
    if (raiz->direita) {
        codigo[top] = '1';
        gerarCodigos(raiz->direita, codigo, top + 1, tabela);
    }

    // Quando um nó folha é alcançado, o código acumulado é salvo na tabela.
    if (!raiz->esquerda && !raiz->direita) {
        int indice = raiz->valor + HUFFMAN_OFFSET; // HUFFMAN_OFFSET ajusta o valor para ser um índice de array >= 0.
        if (indice >= 0 && indice < HUFFMAN_TAMANHO) {
            tabela->codigos[indice] = malloc(top + 1);
            memcpy(tabela->codigos[indice], codigo, top);
            tabela->codigos[indice][top] = '\0';
            tabela->simbolos[indice] = raiz->valor;
        }
    }
}

// Função central que constrói a árvore e a tabela de Huffman a partir de um array de frequências de símbolos.
static TabelaHuffman* construirTabelaHuffmanComFrequencias(const int* frequencias) {
    // 1. Cria um nó folha para cada símbolo
    MinHeap* heap = criarMinHeap(HUFFMAN_TAMANHO);
    for (int i = 0; i < HUFFMAN_TAMANHO; ++i) {
        if (frequencias[i] > 0) {
            inserirMinHeap(heap, criarNo(i - HUFFMAN_OFFSET, frequencias[i]));
        }
    }

    // 2. Constrói a árvore de Huffman.
    while (heap->tamanho > 1) {
        // Extrai os dois nós com as menores frequências.
        HuffmanNode* esq = extrairMin(heap);
        HuffmanNode* dir = extrairMin(heap);

        // Cria um novo nó interno com a soma das frequências.
        HuffmanNode* top = criarNo('$', esq->frequencia + dir->frequencia);
        top->esquerda = esq;
        top->direita = dir;
        inserirMinHeap(heap, top);
    }

    // 3. O nó restante é a raiz da árvore completa.
    HuffmanNode* raiz = extrairMin(heap);
    destruirMinHeap(heap);

    // 4. Cria e preenche a tabela de Huffman.
    TabelaHuffman* tabela = (TabelaHuffman*)malloc(sizeof(TabelaHuffman));
    tabela->codigos = (char**)calloc(HUFFMAN_TAMANHO, sizeof(char*));
    tabela->simbolos = (int*)malloc(HUFFMAN_TAMANHO * sizeof(int));
    tabela->tamanho = HUFFMAN_TAMANHO;
    tabela->raiz = raiz;

    // 5. Gera os códigos binários a partir da árvore.
    char codigo[HUFFMAN_TAMANHO];
    gerarCodigos(raiz, codigo, 0, tabela);
    
    return tabela;
}

// Função principal para criar uma tabela de Huffman.
TabelaHuffman* construirTabelaHuffman(BlocoYCbCr* blocos, int num_blocos, char componente) {
    // 1. conta a frequência dos valores de um componente (Y, Cb ou Cr)
    int* frequencias = (int*)calloc(HUFFMAN_TAMANHO, sizeof(int));
    
    for (int i = 0; i < num_blocos; ++i) {
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                int valor;
                if (componente == COMP_Y) valor = blocos[i].Y[y][x];
                else if (componente == COMP_CB) valor = blocos[i].Cb[y][x];
                else valor = blocos[i].Cr[y][x];

                int indice = valor + HUFFMAN_OFFSET;
                if (indice >= 0 && indice < HUFFMAN_TAMANHO) {
                    frequencias[indice]++;
                }
            }
        }
    }
    
    for (int i = 0; i < HUFFMAN_TAMANHO; ++i) {
        if (frequencias[i] == 0) {
            frequencias[i] = 1;
        }
    }

    // 2. Constroi a tabela com base nas frequências contadas.
    TabelaHuffman* tabela = construirTabelaHuffmanComFrequencias(frequencias);
    free(frequencias);
    return tabela;
}

void escreverTabelaHuffman(TabelaHuffman* tabela, FILE* output) {
    fwrite(&tabela->tamanho, sizeof(unsigned), 1, output);
    for (unsigned i = 0; i < tabela->tamanho; ++i) {
        int comprimento = tabela->codigos[i] ? strlen(tabela->codigos[i]) : 0;
        fwrite(&comprimento, sizeof(int), 1, output);
        if (comprimento > 0) {
            fwrite(tabela->codigos[i], sizeof(char), comprimento, output);
        }
    }
}

HuffmanNode* reconstruirArvore(TabelaHuffman* tabela) {
    HuffmanNode* raiz = criarNo('$', 0);
    for (int i = 0; i < HUFFMAN_TAMANHO; i++) {
        if (tabela->codigos[i]) {
            HuffmanNode* atual = raiz;
            const char* codigo = tabela->codigos[i];
            for (int j = 0; codigo[j] != '\0'; j++) {
                if (codigo[j] == '0') {
                    if (!atual->esquerda) atual->esquerda = criarNo('$', 0);
                    atual = atual->esquerda;
                } else {
                    if (!atual->direita) atual->direita = criarNo('$', 0);
                    atual = atual->direita;
                }
            }
            atual->valor = tabela->simbolos[i];
        }
    }
    return raiz;
}

// Lê uma tabela de Huffman de um arquivo e a reconstrói na memória.
TabelaHuffman* lerTabelaHuffman(FILE* input) {
    TabelaHuffman* tabela = (TabelaHuffman*)malloc(sizeof(TabelaHuffman));
    fread(&tabela->tamanho, sizeof(unsigned), 1, input);

    tabela->codigos = (char**)calloc(tabela->tamanho, sizeof(char*));
    tabela->simbolos = (int*)malloc(tabela->tamanho * sizeof(int));
    
    for (unsigned i = 0; i < tabela->tamanho; ++i) {
        int comprimento;
        fread(&comprimento, sizeof(int), 1, input);
        if (comprimento > 0) {
            tabela->codigos[i] = (char*)malloc(comprimento + 1);
            fread(tabela->codigos[i], sizeof(char), comprimento, input);
            tabela->codigos[i][comprimento] = '\0';
            tabela->simbolos[i] = i - HUFFMAN_OFFSET;
        } else {
            tabela->codigos[i] = NULL;
        }
    }
    
    tabela->raiz = reconstruirArvore(tabela);
    return tabela;
}

void destruirTabelaHuffman(TabelaHuffman* tabela) {
    if (tabela) {
        for (unsigned i = 0; i < tabela->tamanho; ++i) {
            free(tabela->codigos[i]);
        }
        free(tabela->codigos);
        free(tabela->simbolos);
        liberarArvoreHuffman(tabela->raiz);
        free(tabela);
    }
}

// Retorna o código de Huffman para um dado valor, fazendo uma busca rápida na tabela.
const char* buscarCodigo(TabelaHuffman* tabela, int valor) {
    int indice = valor + HUFFMAN_OFFSET;
    if (indice < 0 || indice >= HUFFMAN_TAMANHO) {
        fprintf(stderr, "[ERRO] Valor %d fora da tabela Huffman!\n", valor);
        exit(EXIT_FAILURE);
    }
    return tabela->codigos[indice];
}