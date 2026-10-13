#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "thorup.h"

#define MAX_BUCKETS 33 // 32 bit per gli interi + 1 bucket per il minimo corrente
#define INF_INT UINT_MAX

/* Nodo per la lista doppiamente concatenata dentro i bucket. 
   Permette rimozioni in O(1). */
typedef struct TNode {
    int v;
    unsigned int dist;
    int bucket_idx;
    struct TNode* prev;
    struct TNode* next;
} TNode;

/* Struttura della Bucket Queue (Radix Heap) */
typedef struct {
    TNode* buckets[MAX_BUCKETS];
    TNode* array; // Array di accesso diretto a tutti i nodi in O(1)
    unsigned int last_extracted; // Il minimo corrente (riferimento per lo XOR)
    int size;
} BucketQueue;

/* ---------------- OPERAZIONI SUI BIT ---------------- */

// Trova il Most Significant Bit (MSB). Ritorna 0 per il primo bit, 1 per il secondo, ecc.
int getMSB(unsigned int x) {
    if (x == 0) return -1;
    int msb = 0;
    while (x >>= 1) {
        msb++;
    }
    return msb;
}

// IL MOTORE DI THORUP: Calcola l'indice del bucket usando XOR (^) e MSB
int getBucketIndex(unsigned int dist, unsigned int last_extracted) {
    if (dist == last_extracted) return 0; // Bucket 0: nodi con distanza minima assoluta
    
    // Ecco la formula della slide: MSB(d_old ^ d_new)
    // Nel nostro caso confrontiamo la distanza del nodo con l'ultimo minimo estratto
    unsigned int xor_val = dist ^ last_extracted;
    return getMSB(xor_val) + 1;
}

/* ---------------- GESTIONE DELLA CODA ---------------- */

void pushNode(BucketQueue* q, TNode* node) {
    int idx = getBucketIndex(node->dist, q->last_extracted);
    node->bucket_idx = idx;
    node->prev = NULL;
    node->next = q->buckets[idx];
    
    if (q->buckets[idx] != NULL) {
        q->buckets[idx]->prev = node;
    }
    q->buckets[idx] = node;
}

void removeNode(BucketQueue* q, TNode* node) {
    int idx = node->bucket_idx;
    if (idx == -1) return; // Già estratto

    if (node->prev != NULL) node->prev->next = node->next;
    else q->buckets[idx] = node->next; // Era la testa

    if (node->next != NULL) node->next->prev = node->prev;
}

int extractMinBucket(BucketQueue* q) {    
    int i = 0;
    // Cerca il primo bucket non vuoto
    while (i < MAX_BUCKETS && q->buckets[i] == NULL) i++;
    
    if (i == MAX_BUCKETS) return -1; // Coda vuota

    // Se il minimo non è nel bucket 0, dobbiamo "spacchettare" il bucket trovato
    if (i > 0) {
        unsigned int min_dist = INF_INT;
        TNode* curr = q->buckets[i];
        
        // Trova il vero minimo in questo bucket
        while (curr != NULL) {
            if (curr->dist < min_dist) min_dist = curr->dist;
            curr = curr->next;
        }

        // Aggiorna il riferimento globale
        q->last_extracted = min_dist;

        // Ridistribuisce tutti i nodi nei bucket inferiori in base al nuovo last_extracted
        curr = q->buckets[i];
        q->buckets[i] = NULL;
        while (curr != NULL) {
            TNode* next = curr->next;
            pushNode(q, curr); // Lo rimette calcolando il nuovo XOR
            curr = next;
        }
    }

    // Ora siamo certi che il minimo si trovi nel bucket 0. Lo estraiamo.
    TNode* min_node = q->buckets[0];
    q->buckets[0] = min_node->next;
    if (q->buckets[0] != NULL) q->buckets[0]->prev = NULL;

    min_node->bucket_idx = -1; // Segna come estratto
    q->size--;
    
    return min_node->v;
}

/* ---------------- L'ALGORITMO PRINCIPALE ---------------- */

void thorup(Graph* graph, int src) {
    int V = graph->V;
    unsigned int* dist = (unsigned int*)malloc(V * sizeof(unsigned int));
    
    BucketQueue q;
    q.last_extracted = 0;
    q.size = V;
    q.array = (TNode*)malloc(V * sizeof(TNode));
    
    for (int i = 0; i < MAX_BUCKETS; i++) q.buckets[i] = NULL;

    // Inizializzazione
    for (int v = 0; v < V; v++) {
        dist[v] = INF_INT;
        q.array[v].v = v;
        q.array[v].dist = INF_INT;
        q.array[v].bucket_idx = -2;
    }

    dist[src] = 0;
    q.array[src].dist = 0;
    pushNode(&q, &q.array[src]);

    while (q.size > 0) {
        int u = extractMinBucket(&q);
        if (u == -1 || dist[u] == INF_INT) break;

        AdjListNode* pCrawl = graph->array[u].head;
        while (pCrawl != NULL) {
            int v = pCrawl->dest;
            
            // ATTENZIONE: Cast esplicito! Thorup non può gestire i double.
            // Questo è il fulcro di tutta la tesi a favore del BMSSP.
            unsigned int weight_int = (unsigned int)pCrawl->weight;

            // Rilassamento
            if (q.array[v].bucket_idx != -1 && dist[u] + weight_int < dist[v]) {
                dist[v] = dist[u] + weight_int;
                
                // O(1) Decrease Key: Rimuove dal vecchio bucket SOLO se ci era già entrato (>= 0)
                if (q.array[v].bucket_idx >= 0) {
                    removeNode(&q, &q.array[v]);
                }
                
                q.array[v].dist = dist[v];
                pushNode(&q, &q.array[v]);
            }
            pCrawl = pCrawl->next;
        }
    }

    // Stampa i risultati
    //printf("Vertice \t Distanza (INTERA) dalla Sorgente\n");
    //for (int i = 0; i < V; ++i) {
      //  if (dist[i] == INF_INT) printf("%d \t\t INF\n", i);
        //else printf("%d \t\t %u\n", i, dist[i]);
    //}

    free(dist);
    free(q.array);
}