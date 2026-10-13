#ifndef BMSSP_H
#define BMSSP_H

#include "graph.h"

// Esegue l'algoritmo BMSSP con parametro Delta per l'ampiezza dei bucket
void bmssp(Graph* graph, int src, double delta);

#endif