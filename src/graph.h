#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>

/* Struttura per un nodo della lista di adiacenza (rappresenta un arco) */
typedef struct AdjListNode {
    int dest;                   // Vertice di destinazione
    double weight;              // Peso dell'arco (double per i reali del BMSSP)
    struct AdjListNode* next;   // Puntatore al prossimo arco uscente
} AdjListNode;

/* Struttura per la lista di adiacenza vera e propria */
typedef struct AdjList {
    AdjListNode* head;          // Puntatore al primo nodo della lista
} AdjList;

/* Struttura principale del Grafo */
typedef struct Graph {
    int V;                      // Numero di vertici
    AdjList* array;             // Array di liste di adiacenza (grande quanto V)
} Graph;

/* Prototipi delle funzioni */
Graph* createGraph(int V);
void addEdge(Graph* graph, int src, int dest, double weight);
void printGraph(Graph* graph);
void freeGraph(Graph* graph);

#endif