#include "huffman.h"

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
static void heapifyDown(MinHeap* heap, int indice) {
    int menor = indice;
    int esquerda = 2 * indice + 1;
    int direita = 2 * indice + 2;

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

/* TESTE */

int main() {
    // 1. Cria uma heap vazia
    MinHeap* heap = criarMinHeap(10);
    
    // 2. Insere nós com frequências aleatórias
    inserirMinHeap(heap, criarNo('A', 5));
    inserirMinHeap(heap, criarNo('B', 3));
    inserirMinHeap(heap, criarNo('C', 8));
    inserirMinHeap(heap, criarNo('D', 1));

    // 3. Extrai os nós em ordem crescente
    printf("Ordem de extracao (deveria ser: D, B, A, C):\n");
    while (heap->tamanho > 0) {
        HuffmanNode* no = extrairMin(heap);
        printf("Simbolo: %c, Frequencia: %u\n", no->valor, no->frequencia);
        free(no);
    }

    // 4. Libera a heap
    destruirMinHeap(heap);
    return 0;
}