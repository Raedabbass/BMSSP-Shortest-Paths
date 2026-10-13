#include "graph.h"

/* Funzione interna (privata) per creare un nuovo nodo dell'arco */
static AdjListNode* newAdjListNode(int dest, double weight) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (newNode == NULL) {
        fprintf(stderr, "Errore: impossibile allocare memoria per il nodo.\n");
        exit(EXIT_FAILURE);
    }
    newNode->dest = dest;
    newNode->weight = weight;
    newNode->next = NULL;
    return newNode;
}

/* Crea un grafo con V vertici */
Graph* createGraph(int V) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (graph == NULL) {
        fprintf(stderr, "Errore: impossibile allocare memoria per il grafo.\n");
        exit(EXIT_FAILURE);
    }
    
    graph->V = V;
    
    // Crea un array di liste di adiacenza. La dimensione è V
    graph->array = (AdjList*)malloc(V * sizeof(AdjList));
    if (graph->array == NULL) {
        fprintf(stderr, "Errore: impossibile allocare memoria per le liste.\n");
        exit(EXIT_FAILURE);
    }
    
    // Inizializza tutte le liste come vuote (head = NULL)
    for (int i = 0; i < V; ++i) {
        graph->array[i].head = NULL;
    }
    
    return graph;
}

/* Aggiunge un arco DIRETTO da src a dest. 
   L'inserimento avviene in testa alla lista per essere O(1) */
void addEdge(Graph* graph, int src, int dest, double weight) {
    // Controllo di sicurezza sui vertici
    if (src >= graph->V || dest >= graph->V || src < 0 || dest < 0) {
        fprintf(stderr, "Errore: vertici %d o %d fuori dai limiti (0-%d).\n", src, dest, graph->V - 1);
        return;
    }

    AdjListNode* newNode = newAdjListNode(dest, weight);
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;
    
    // NOTA: Poiché il grafo è DIRETTO (come richiesto dalle assunzioni), 
    // non aggiungiamo l'arco inverso da dest a src.
}

/* Stampa il grafo (utile per il debugging) */
void printGraph(Graph* graph) {
    for (int v = 0; v < graph->V; ++v) {
        AdjListNode* pCrawl = graph->array[v].head;
        printf("Lista di adiacenza del vertice %d:\n head ", v);
        while (pCrawl != NULL) {
            printf("-> %d (peso: %.2f) ", pCrawl->dest, pCrawl->weight);
            pCrawl = pCrawl->next;
        }
        printf("\n");
    }
}

/* Libera tutta la memoria allocata per evitare memory leaks */
void freeGraph(Graph* graph) {
    if (graph == NULL) return;
    
    for (int v = 0; v < graph->V; ++v) {
        AdjListNode* current = graph->array[v].head;
        AdjListNode* next;
        
        while (current != NULL) {
            next = current->next;
            free(current);
            current = next;
        }
    }
    free(graph->array);
    free(graph);
}