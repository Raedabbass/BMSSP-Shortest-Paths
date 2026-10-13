#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "bmssp.h"

#define MAX_BUCKETS 10000 // Dimensione di sicurezza per i bucket

/* Nodo per la lista doppiamente concatenata nei bucket */
typedef struct BNode {
    int v;
    int bucket_idx;
    struct BNode* prev;
    struct BNode* next;
} BNode;

/* Funzione per inserire un nodo in un determinato bucket in O(1) */
void pushBNode(BNode** buckets, BNode* node, int idx) {
    node->bucket_idx = idx;
    node->prev = NULL;
    node->next = buckets[idx];
    
    if (buckets[idx] != NULL) {
        buckets[idx]->prev = node;
    }
    buckets[idx] = node;
}

/* Funzione per rimuovere un nodo dal suo bucket attuale in O(1) */
void removeBNode(BNode** buckets, BNode* node) {
    int idx = node->bucket_idx;
    if (idx < 0) return; // Non è in nessun bucket
    
    if (node->prev != NULL) node->prev->next = node->next;
    else buckets[idx] = node->next;
    
    if (node->next != NULL) node->next->prev = node->prev;
    
    node->bucket_idx = -1; // Segna come "fuori dalla coda"
}

/* --- L'ALGORITMO BMSSP PRINCIPALE --- */
void bmssp(Graph* graph, int src, double delta) {
    int V = graph->V;
    double* dist = (double*)malloc(V * sizeof(double));
    BNode* nodes = (BNode*)malloc(V * sizeof(BNode));
    BNode** buckets = (BNode**)calloc(MAX_BUCKETS, sizeof(BNode*)); // Inizializza tutto a NULL

    // Inizializzazione dei vertici
    for (int i = 0; i < V; i++) {
        dist[i] = DBL_MAX;
        nodes[i].v = i;
        nodes[i].bucket_idx = -1;
        nodes[i].prev = NULL;
        nodes[i].next = NULL;
    }

    // Impostiamo la sorgente
    dist[src] = 0.0;
    pushBNode(buckets, &nodes[src], 0);

    int curr_b = 0; // Indice del bucket corrente
    
    // Esploriamo i bucket in ordine
    while (curr_b < MAX_BUCKETS) {
        // Se il bucket corrente è vuoto, passiamo al successivo
        if (buckets[curr_b] == NULL) {
            curr_b++;
            continue;
        }

        /* 1. ESTRAZIONE DEL BATCH (LOTTO) 
           Scolleghiamo l'intera lista dal bucket corrente in O(1).
           Tutti i nodi in questa lista verranno processati insieme. */
        BNode* batch = buckets[curr_b];
        buckets[curr_b] = NULL;

        // Segniamo tutti i nodi del batch come "estratti"
        BNode* temp = batch;
        while (temp != NULL) {
            temp->bucket_idx = -1; 
            temp = temp->next;
        }

        /* 2. RILASSAMENTO A CASCATA (Stile Bellman-Ford locale)
           Processiamo tutti gli archi uscenti dei nodi nel batch */
        BNode* curr = batch;
        while (curr != NULL) {
            int u = curr->v;
            AdjListNode* pCrawl = graph->array[u].head;
            
            while (pCrawl != NULL) {
                int v = pCrawl->dest;
                double w = pCrawl->weight;

                // Rilassamento standard
                if (dist[u] + w < dist[v]) {
                    // Se v era in un altro bucket, lo rimuoviamo
                    if (nodes[v].bucket_idx >= 0) {
                        removeBNode(buckets, &nodes[v]);
                    }

                    dist[v] = dist[u] + w;
                    
                    // Calcolo del nuovo bucket basato sulla finestra Delta
                    int new_b = (int)(dist[v] / delta);
                    if (new_b >= MAX_BUCKETS) new_b = MAX_BUCKETS - 1; // Sicurezza

                    // Lo inseriamo nel nuovo bucket
                    pushBNode(buckets, &nodes[v], new_b);
                }
                pCrawl = pCrawl->next;
            }
            curr = curr->next;
        }
        
        /* MAGIA DEL BMSSP: Non incrementiamo subito curr_b!
           Se il rilassamento ha ri-inserito dei nodi nel bucket corrente 
           (perché la distanza era ravvicinata e ricadeva nello stesso Delta),
           il ciclo while li ripescherà al prossimo giro come un nuovo sotto-batch,
           generando l'effetto "a cascata" finché il bucket non si svuota! */
    }

    // Stampa dei risultati
    //printf("Vertice \t Distanza (REALE) dalla Sorgente\n");
    //for (int i = 0; i < V; ++i) {
      //  if (dist[i] == DBL_MAX) printf("%d \t\t INF\n", i);
        //else printf("%d \t\t %.2f\n", i, dist[i]);
    //}

    // Pulizia memoria
    free(dist);
    free(nodes);
    free(buckets);
}