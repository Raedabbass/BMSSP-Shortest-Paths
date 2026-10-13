#ifndef MINHEAP_H
#define MINHEAP_H

#include <stdbool.h>

/* Nodo del Min-Heap: memorizza il vertice e la sua distanza attuale */
typedef struct MinHeapNode {
    int v;
    double dist;
} MinHeapNode;

/* Struttura del Min-Heap */
typedef struct MinHeap {
    int size;           // Numero di elementi attualmente nel min heap
    int capacity;       // Capacità massima
    int *pos;           // Array fondamentale: pos[i] contiene l'indice del vertice i nell'array del min heap. Serve per la Decrease-Key!
    MinHeapNode **array;
} MinHeap;

MinHeapNode* newMinHeapNode(int v, double dist);
MinHeap* createMinHeap(int capacity);
void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b);
void minHeapify(MinHeap* minHeap, int idx);
int isEmpty(MinHeap* minHeap);
MinHeapNode* extractMin(MinHeap* minHeap);
void decreaseKey(MinHeap* minHeap, int v, double dist);
bool isInMinHeap(MinHeap *minHeap, int v);
void freeMinHeap(MinHeap* minHeap);

#endif