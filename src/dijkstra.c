#include <stdio.h>
#include <stdlib.h>
#include <float.h> // Per DBL_MAX (il nostro "infinito" per i pesi reali)
#include "dijkstra.h"
#include "minheap.h"

// Funzione di supporto per stampare le distanze calcolate
void printArr(double dist[], int n) {
    printf("Vertice \t Distanza dalla Sorgente\n");
    for (int i = 0; i < n; ++i) {
        if (dist[i] == DBL_MAX)
            printf("%d \t\t INF\n", i);
        else
            printf("%d \t\t %.2f\n", i, dist[i]);
    }
}

void dijkstra(Graph* graph, int src) {
    int V = graph->V;
    double* dist = (double*)malloc(V * sizeof(double));
    MinHeap* minHeap = createMinHeap(V);

    // Inizializzazione
    for (int v = 0; v < V; ++v) {
        dist[v] = DBL_MAX;
        minHeap->array[v] = newMinHeapNode(v, dist[v]);
        minHeap->pos[v] = v;
    }

    // La distanza della sorgente da se stessa è sempre 0
    minHeap->array[src] = newMinHeapNode(src, dist[src]);
    minHeap->pos[src] = src;
    dist[src] = 0.0;
    decreaseKey(minHeap, src, dist[src]);

    minHeap->size = V;

    // Ciclo principale: finché ci sono nodi da esplorare
    while (!isEmpty(minHeap)) {
        // Estraiamo il vertice con distanza minima
        MinHeapNode* minHeapNode = extractMin(minHeap);
        int u = minHeapNode->v;

        // Esploriamo tutti i vicini del vertice estratto
        AdjListNode* pCrawl = graph->array[u].head;
        while (pCrawl != NULL) {
            int v = pCrawl->dest;

            // RILASSAMENTO (La famosa formula delle nostre slide!)
            // Se v è ancora nel min-heap e la nuova distanza è minore di quella vecchia
            if (isInMinHeap(minHeap, v) && dist[u] != DBL_MAX && pCrawl->weight + dist[u] < dist[v]) {
                dist[v] = dist[u] + pCrawl->weight;
                // Aggiorniamo la distanza nel Min-Heap
                decreaseKey(minHeap, v, dist[v]);
            }
            pCrawl = pCrawl->next;
        }
        free(minHeapNode); // Puliamo la memoria del nodo estratto
    }

    // Stampa i risultati
    //printArr(dist, V);

    // Pulizia finale
    free(dist);
    freeMinHeap(minHeap);
}