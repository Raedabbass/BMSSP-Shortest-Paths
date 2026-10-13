#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Per misurare i tempi
#include "graph.h"
#include "dijkstra.h"
#include "thorup.h"
#include "bmssp.h"

// Funzione per generare archi casuali
void generateRandomGraph(Graph* graph, int E, double max_weight) {
    int V = graph->V;
    
    // Inizializza il generatore di numeri casuali (seed)
    srand(time(NULL));

    for (int i = 0; i < E; i++) {
        int src = rand() % V;
        int dest = rand() % V;

        // Evitiamo auto-anelli (un nodo che punta a se stesso)
        while (src == dest) {
            dest = rand() % V;
        }

        // Genera un peso casuale (es. tra 1.0 e max_weight)
        double weight = 1.0 + ((double)rand() / RAND_MAX) * (max_weight - 1.0);

        addEdge(graph, src, dest, weight);
    }
}

int main(int argc, char *argv[]) {
    // Valori di default (se lanciato senza parametri)
    int V = 10000;          
    int E = 50000;          
    double max_weight = 20.0; 
    double delta = 5.0;       

    // Se lo script Python passa i parametri, li usiamo!
    if (argc == 5) {
        V = atoi(argv[1]);
        E = atoi(argv[2]);
        max_weight = atof(argv[3]);
        delta = atof(argv[4]);
    }

    printf("=== INIZIO BENCHMARK ===\n");
    printf("Generazione grafo casuale (V = %d, E = %d)...\n", V, E);
    
    Graph* graph = createGraph(V);
    generateRandomGraph(graph, E, max_weight);

    printf("Grafo generato con successo. Avvio competizione...\n\n");

    clock_t start, end;
    double cpu_time_used;

    // --- 1. DIJKSTRA ---
    printf("Esecuzione Dijkstra (Min-Heap)... ");
    start = clock();
    dijkstra(graph, 0);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC * 1000.0; // In millisecondi
    printf("Completato in: %.2f ms\n", cpu_time_used);

    // --- 2. THORUP ---
    printf("Esecuzione Thorup (Bucket & XOR)... ");
    start = clock();
    thorup(graph, 0);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Completato in: %.2f ms\n", cpu_time_used);

    // --- 3. BMSSP ---
    printf("Esecuzione BMSSP (Batch Queue, Delta=%.1f)... ", delta);
    start = clock();
    bmssp(graph, 0, delta);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Completato in: %.2f ms\n", cpu_time_used);

    printf("\n=== BENCHMARK CONCLUSO ===\n");

    freeGraph(graph);
    return 0;
}