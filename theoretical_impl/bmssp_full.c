#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define INFINITY 1e9

// --- COSTANTI GLOBALI DEL PAPER ---
int k = 3; 
int t = 5; 

// ==========================================
// 1. STRUTTURE DATI DI BASE (Grafo e Insiemi)
// ==========================================

typedef struct Edge {
    int to;
    double weight;
    struct Edge* next;
} Edge;

typedef struct Graph {
    int n;
    Edge** head;
} Graph;

double* d_hat; 
int* PRED;

typedef struct {
    int* vertices;
    int size;
    int capacity;
} VertexSet;

VertexSet* create_set(int cap) {
    VertexSet* s = (VertexSet*)malloc(sizeof(VertexSet));
    s->vertices = (int*)malloc(cap * sizeof(int));
    s->size = 0;
    s->capacity = cap;
    return s;
}

void add_to_set(VertexSet* s, int v) {
    if (s->size < s->capacity) s->vertices[s->size++] = v;
}

// ==========================================
// 2. STRUTTURE DATI: RED-BLACK TREE E BLOCCHI
// ==========================================

typedef enum { RED, BLACK } Color;

// Elemento della lista concatenata dentro un blocco
typedef struct Element {
    int vertex;
    double distance;
    struct Element* next;
} Element;

// Blocco che contiene al massimo M elementi
typedef struct Block {
    Element* head;
    int size;
    double upper_bound;
    struct Block* next_block;
} Block;

// Nodo dell'albero Red-Black
typedef struct RBNode {
    double upper_bound;  // Chiave dell'albero
    Block* block;        // Puntatore al blocco associato
    Color color;
    struct RBNode *left, *right, *parent;
} RBNode;

// Struttura principale dell'albero
typedef struct RBTree {
    RBNode* root;
    RBNode* nil;
} RBTree;

// Libera tutti gli Element dentro un blocco e il blocco stesso
void free_block(Block* b) {
    if (b == NULL) return;
    Element* e = b->head;
    while (e != NULL) {
        Element* next = e->next;
        free(e);
        e = next;
    }
    free(b);
}

// Libera tutta la lista D0 (catena di blocchi con i loro elementi)
void free_d0_list(Block* list) {
    Block* curr = list;
    while (curr != NULL) {
        Block* next = curr->next_block;
        free_block(curr);
        curr = next;
    }
}

// Libera ricorsivamente tutti i nodi del RBTree (e i blocchi associati)
void free_rbtree_nodes(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return;
    free_rbtree_nodes(tree, node->left);
    free_rbtree_nodes(tree, node->right);
    free_block(node->block);
    free(node);
}

// Libera l'intero RBTree
void free_rbtree(RBTree* tree) {
    if (tree == NULL) return;
    free_rbtree_nodes(tree, tree->root);
    free(tree->nil);
    free(tree);
}

// Libera un VertexSet
void free_set(VertexSet* s) {
    if (s == NULL) return;
    free(s->vertices);
    free(s);
}

RBTree* create_rbtree() {
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    tree->nil = (RBNode*)malloc(sizeof(RBNode));
    tree->nil->color = BLACK;
    tree->nil->left = tree->nil->right = tree->nil->parent = tree->nil;
    tree->nil->block = NULL;
    tree->root = tree->nil;
    return tree;
}

void left_rotate(RBTree* tree, RBNode* x) {
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left != tree->nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == tree->nil) tree->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

void right_rotate(RBTree* tree, RBNode* x) {
    RBNode* y = x->left;
    x->left = y->right;
    if (y->right != tree->nil) y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == tree->nil) tree->root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
}

void insert_fixup(RBTree* tree, RBNode* z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                right_rotate(tree, z->parent->parent);
            }
        } else {
            RBNode* y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
}

void rb_insert(RBTree* tree, double upper_bound, Block* block) {
    RBNode* z = (RBNode*)malloc(sizeof(RBNode));
    z->upper_bound = upper_bound;
    z->block = block;
    z->left = z->right = z->parent = tree->nil;
    z->color = RED;

    RBNode* y = tree->nil;
    RBNode* x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (z->upper_bound < x->upper_bound) x = x->left;
        else x = x->right;
    }

    z->parent = y;
    if (y == tree->nil) tree->root = z;
    else if (z->upper_bound < y->upper_bound) y->left = z;
    else y->right = z;

    insert_fixup(tree, z);
}

Block* find_appropriate_block(RBTree* tree, double b) {
    RBNode* curr = tree->root;
    RBNode* best_match = tree->nil;

    while (curr != tree->nil) {
        if (curr->upper_bound >= b) {
            best_match = curr;
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    if (best_match != tree->nil) return best_match->block;
    return NULL;
}

// Struttura per tracciare da quale blocco proviene un elemento,
// necessaria per poterlo rimuovere dopo averlo selezionato.
typedef struct {
    int vertex;
    double distance;
    Block* parent_block;
} Candidate;

// Funzione di comparazione per ordinare i candidati per distanza
int compare_candidates(const void* a, const void* b) {
    Candidate* ca = (Candidate*)a;
    Candidate* cb = (Candidate*)b;
    if (ca->distance < cb->distance) return -1;
    if (ca->distance > cb->distance) return 1;
    return 0;
}

// Visita in-order dell'albero Red-Black (da sinistra a destra, ovvero dai limiti minori ai maggiori).
// Raccoglie gli elementi dai blocchi fino a raggiungerne M.
void collect_from_rbt(RBTree* tree, RBNode* node, Candidate* arr, int* total_count, int* d1_count, int limit) {
    if (node == tree->nil || *d1_count >= limit) return;
    
    // Visita il figlio sinistro (valori più piccoli)
    collect_from_rbt(tree, node->left, arr, total_count, d1_count, limit);
    
    if (*d1_count >= limit) return;
    
    // Raccogli dal blocco del nodo corrente
    Block* b = node->block;
    if (b != NULL) {
        Element* curr = b->head;
        while (curr != NULL && *d1_count < limit) {
            arr[*total_count].vertex = curr->vertex;
            arr[*total_count].distance = curr->distance;
            arr[*total_count].parent_block = b;
            (*total_count)++;
            (*d1_count)++;
            curr = curr->next;
        }
    }
    
    if (*d1_count >= limit) return;
    
    // Visita il figlio destro
    collect_from_rbt(tree, node->right, arr, total_count, d1_count, limit);
}
// ==========================================
// 3. LA STRUTTURA DATI "D" (Lemma 3.3)
// ==========================================

typedef struct {
    RBTree* rbt_D1;    // Sequenza D1: usa l'albero Red-Black
    Block* list_D0;    // Sequenza D0: lista semplice per i batch prepend
    int M;             // Dimensione massima del blocco
    int is_empty;      // Flag veloce per sapere se D è vuoto
} CustomDataStructD;

void D_INITIALIZE(CustomDataStructD* D, int M, double B) {
    D->rbt_D1 = create_rbtree();
    D->list_D0 = NULL;
    D->M = M;
    D->is_empty = 1;

    // Crea un blocco iniziale vuoto con upper bound B
    Block* initial_block = (Block*)malloc(sizeof(Block));
    initial_block->head = NULL;
    initial_block->size = 0;
    initial_block->upper_bound = B;
    
    rb_insert(D->rbt_D1, B, initial_block);
}

// Libera l'intera struttura D (D0 + D1)
void free_D(CustomDataStructD* D) {
    free_d0_list(D->list_D0);
    D->list_D0 = NULL;
    free_rbtree(D->rbt_D1);
    D->rbt_D1 = NULL;
}

// Funzione helper per l'ordinamento dell'array di puntatori
int compare_elements(const void* a, const void* b) {
    Element* ea = *(Element**)a;
    Element* eb = *(Element**)b;
    if (ea->distance < eb->distance) return -1;
    if (ea->distance > eb->distance) return 1;
    return 0;
}

// Implementazione della Logica di SPLIT del blocco
void split_block(CustomDataStructD* D, Block* target_block) {
    int total_size = target_block->size;
    
    // 1. Raccogliamo tutti i puntatori agli elementi in un array.
    // Questo ci permette di trovare il mediano e riordinarli facilmente.
    Element** arr = (Element**)malloc(total_size * sizeof(Element*));
    Element* curr = target_block->head;
    for (int i = 0; i < total_size; i++) {
        arr[i] = curr;
        curr = curr->next;
    }
    
    // 2. Ordiniamo l'array per distanza (approssimazione pratica di O(M))
    // Nota: Il paper suggerisce un calcolo del mediano in O(M), ma qsort in O(M log M) 
    // su blocchi piccoli è spessissimo più veloce ed efficiente nella pratica in C.
    qsort(arr, total_size, sizeof(Element*), compare_elements);
    
    int mid = total_size / 2;
    double median_val = arr[mid]->distance; // Il nostro mediano
    
    // 3. Creiamo il NUOVO blocco per la metà INFERIORE (distanze <= mediano)
    Block* new_block = (Block*)malloc(sizeof(Block));
    new_block->upper_bound = median_val;
    new_block->size = mid + 1;
    new_block->head = NULL;
    
    // Ricostruiamo la lista concatenata per il nuovo blocco
    for (int i = mid; i >= 0; i--) {
        arr[i]->next = new_block->head;
        new_block->head = arr[i];
    }
    
    // 4. Aggiorniamo il VECCHIO blocco per tenere la metà SUPERIORE (distanze > mediano).
    // Siccome tiene gli elementi più grandi, il suo upper_bound di partenza
    // rimane corretto per l'albero Red-Black. Nessun bisogno di ribilanciare il suo nodo!
    target_block->size = total_size - (mid + 1);
    target_block->head = NULL;
    
    // Ricostruiamo la lista concatenata per il vecchio blocco
    for (int i = total_size - 1; i > mid; i--) {
        arr[i]->next = target_block->head;
        target_block->head = arr[i];
    }
    
    free(arr);
    
    // 5. Inseriamo il NUOVO blocco nell'albero Red-Black (il vecchio è già lì)
    rb_insert(D->rbt_D1, new_block->upper_bound, new_block);
}
void D_INSERT(CustomDataStructD* D, int v, double dist) {
    D->is_empty = 0;
    
    // 1. Trova il blocco in D1 in tempo O(log(N/M)) usando il Red-Black Tree
    Block* target_block = find_appropriate_block(D->rbt_D1, dist);
    
    if (target_block != NULL) {
        // 2. Inserisci l'elemento in testa alla lista del blocco in O(1)
        Element* new_elem = (Element*)malloc(sizeof(Element));
        new_elem->vertex = v;
        new_elem->distance = dist;
        new_elem->next = target_block->head;
        target_block->head = new_elem;
        target_block->size++;
        
        // 3. LOGICA DI SPLIT: Se la dimensione supera M, eseguiamo la divisione
        if (target_block->size > D->M) {
            split_block(D, target_block);
        }
    }
}


// Funzione helper per ordinare i vertici in base alla loro distanza attuale (d_hat)
int compare_vertices_by_dist(const void* a, const void* b) {
    int va = *(int*)a;
    int vb = *(int*)b;
    if (d_hat[va] < d_hat[vb]) return -1;
    if (d_hat[va] > d_hat[vb]) return 1;
    return 0;
}

// Implementazione della funzione D_BATCHPREPEND
void D_BATCHPREPEND(CustomDataStructD* D, VertexSet* K) {
    if (K->size == 0) return;
    D->is_empty = 0;

    // 1. Ordiniamo i vertici in K per distanza
    qsort(K->vertices, K->size, sizeof(int), compare_vertices_by_dist);

    // 2. Calcoliamo la dimensione massima del blocco
    // Il paper suggerisce blocchi di dimensione ceil(M/2) quando L > M
    // Utilizziamo M per semplicità, l'importante è non superare il limite M del Lemma 3.3.
    int chunk_size = D->M; 

    // 3. Creiamo i blocchi e li inseriamo in testa a list_D0
    int i = K->size - 1; // Partiamo dal fondo per mantenere l'ordine quando prepariamo in testa

    while (i >= 0) {
        Block* new_block = (Block*)malloc(sizeof(Block));
        new_block->size = 0;
        new_block->head = NULL;
        new_block->upper_bound = d_hat[K->vertices[i]]; // Il massimo di questo blocco

        int current_chunk = (i + 1 >= chunk_size) ? chunk_size : (i + 1);

        // Riempiamo la lista concatenata del blocco
        for (int j = 0; j < current_chunk; j++) {
            int v = K->vertices[i - j];
            Element* new_elem = (Element*)malloc(sizeof(Element));
            new_elem->vertex = v;
            new_elem->distance = d_hat[v];
            
            new_elem->next = new_block->head;
            new_block->head = new_elem;
            new_block->size++;
        }

        i -= current_chunk;

        // 4. Inseriamo il blocco appena creato in TESTA a list_D0 (Prepend)
        new_block->next_block = D->list_D0;
        D->list_D0 = new_block;
    }
}

// Restituisce il valore minimo presente in D senza estrarlo (peek).
// Serve per calcolare B' nel caso di esecuzione parziale di BMSSP (Lemma 3.1).
double D_PEEK_MIN(CustomDataStructD* D, double B) {
    double min_val = B;

    // Controlla il primo elemento di D0 (già ordinato, il minimo è in testa)
    if (D->list_D0 != NULL && D->list_D0->head != NULL) {
        Element* e = D->list_D0->head;
        // Il blocco in testa di D0 contiene i valori più piccoli inseriti per batch;
        // scorriamo la sua lista per trovare il minimo effettivo del blocco.
        while (e != NULL) {
            if (e->distance < min_val) min_val = e->distance;
            e = e->next;
        }
    }

    // Controlla il nodo più a sinistra di D1 (Red-Black Tree), che ha l'upper_bound minore
    RBNode* curr = D->rbt_D1->root;
    RBNode* leftmost = D->rbt_D1->nil;
    while (curr != D->rbt_D1->nil) {
        leftmost = curr;
        curr = curr->left;
    }
    if (leftmost != D->rbt_D1->nil && leftmost->block != NULL) {
        Element* e = leftmost->block->head;
        while (e != NULL) {
            if (e->distance < min_val) min_val = e->distance;
            e = e->next;
        }
    }

    return min_val;
}

double D_PULL(CustomDataStructD* D, VertexSet* S_i, double B) {
    // Possiamo raccogliere al massimo M da D0 e M da D1
    int max_candidates = 2 * D->M; 
    Candidate* candidates = (Candidate*)malloc(max_candidates * sizeof(Candidate));
    int c_count = 0;

// STEP 1: Raccogli fino a M elementi da D0 (i batch prepend)
    int d0_collected = 0;
    Block* curr_b0 = D->list_D0; 
    
    // Scorriamo TUTTI i blocchi in list_D0 finché non raccogliamo M elementi
    while (curr_b0 != NULL && d0_collected < D->M) {
        Element* curr_e = curr_b0->head;
        while (curr_e != NULL && d0_collected < D->M) {
            candidates[c_count].vertex = curr_e->vertex;
            candidates[c_count].distance = curr_e->distance;
            candidates[c_count].parent_block = curr_b0;
            c_count++;
            d0_collected++;
            curr_e = curr_e->next;
        }
        curr_b0 = curr_b0->next_block; // Passa al blocco successivo in D0
    }

    // STEP 2: Raccogli fino a M elementi dai nodi più a sinistra di D1 (Red-Black Tree)
    int d1_collected = 0;
    collect_from_rbt(D->rbt_D1, D->rbt_D1->root, candidates, &c_count, &d1_collected, D->M);

    // Se non abbiamo trovato nulla, D è completamente vuota.
    if (c_count == 0) {
        D->is_empty = 1;
        free(candidates);
        return B;
    }

    // STEP 3: Ordina tutti i candidati raccolti per trovare i minimi globali
    qsort(candidates, c_count, sizeof(Candidate), compare_candidates);

    // STEP 4: Estrai al massimo M elementi (se ne abbiamo di meno, li prendiamo tutti)
    int num_to_pull = (c_count < D->M) ? c_count : D->M;
    
    for (int i = 0; i < num_to_pull; i++) {
        // Aggiungi il vertice all'insieme di output S_i
        add_to_set(S_i, candidates[i].vertex);
        
        // Rimuovi fisicamente l'elemento dalla lista concatenata del suo blocco originale
        Block* b = candidates[i].parent_block;
        Element* curr = b->head;
        Element* prev = NULL;
        while (curr != NULL) {
            if (curr->vertex == candidates[i].vertex && curr->distance == candidates[i].distance) {
                if (prev == NULL) b->head = curr->next;
                else prev->next = curr->next;
                b->size--;
                free(curr);
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    // STEP 5: Calcola il nuovo lower bound
    double new_bound;
    if (c_count > num_to_pull) {
        // Se c'erano più elementi di quanti ne abbiamo estratti, 
        // il nuovo lower bound è la distanza del primo elemento rimasto.
        new_bound = candidates[num_to_pull].distance;
    } else {
        // Abbiamo svuotato completamente tutto quello che avevamo raccolto.
        // In un'implementazione perfetta, qui si cercherebbe il minimo tra gli elementi *non raccolti*.
        // Ma siccome abbiamo raccolto i più piccoli, se c_count < M significa che
        // l'intera struttura D è ora vuota.
        new_bound = B;
        D->is_empty = 1;
    }

    free(candidates);
    return new_bound;
}


// ==========================================
// 4. ALGORITMI PRINCIPALI DEL PAPER
// ==========================================

// Struttura di un nodo all'interno del Min-Heap
typedef struct {
    int vertex;
    double distance;
} HeapNode;

// Struttura del Min-Heap
typedef struct {
    HeapNode* nodes; // Array che rappresenta l'albero binario
    int* pos;        // pos[v] mappa il vertice 'v' al suo indice in 'nodes' (o -1 se non presente)
    int size;
    int capacity;
} MinHeap;

// Crea e inizializza il Min-Heap
MinHeap* create_min_heap(int capacity) {
    MinHeap* h = (MinHeap*)malloc(sizeof(MinHeap));
    h->nodes = (HeapNode*)malloc(capacity * sizeof(HeapNode));
    h->pos = (int*)malloc(capacity * sizeof(int));
    h->size = 0;
    h->capacity = capacity;
    for (int i = 0; i < capacity; i++) {
        h->pos[i] = -1; // -1 significa che il vertice non è nel heap
    }
    return h;
}

// Scambia due nodi nel heap (e aggiorna l'array delle posizioni)
void swap_heap_nodes(MinHeap* h, int i, int j) {
    HeapNode temp = h->nodes[i];
    h->nodes[i] = h->nodes[j];
    h->nodes[j] = temp;
    
    h->pos[h->nodes[i].vertex] = i;
    h->pos[h->nodes[j].vertex] = j;
}

// Ripristina la proprietà del Min-Heap salendo verso la radice
void heapify_up(MinHeap* h, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->nodes[idx].distance < h->nodes[parent].distance) {
            swap_heap_nodes(h, idx, parent);
            idx = parent;
        } else {
            break;
        }
    }
}

// Ripristina la proprietà del Min-Heap scendendo verso le foglie
void heapify_down(MinHeap* h, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < h->size && h->nodes[left].distance < h->nodes[smallest].distance)
        smallest = left;
    if (right < h->size && h->nodes[right].distance < h->nodes[smallest].distance)
        smallest = right;

    if (smallest != idx) {
        swap_heap_nodes(h, idx, smallest);
        heapify_down(h, smallest);
    }
}

// Inserisce un nuovo vertice nel heap (corrisponde a H.INSERT nel paper)
void push_heap(MinHeap* h, int v, double dist) {
    int idx = h->size;
    h->size++;
    h->nodes[idx].vertex = v;
    h->nodes[idx].distance = dist;
    h->pos[v] = idx;
    heapify_up(h, idx);
}

// Estrae e restituisce il vertice con distanza minima (corrisponde a H.EXTRACT_MIN nel paper)
int pop_heap(MinHeap* h) {
    if (h->size == 0) return -1;
    
    int root_vertex = h->nodes[0].vertex;
    h->pos[root_vertex] = -1; // Rimosso dal heap
    
    h->size--;
    if (h->size > 0) {
        h->nodes[0] = h->nodes[h->size];
        h->pos[h->nodes[0].vertex] = 0;
        heapify_down(h, 0);
    }
    
    return root_vertex;
}

// Aggiorna la distanza di un vertice già presente (corrisponde a H.DECREASE_KEY nel paper)
void decrease_key(MinHeap* h, int v, double new_dist) {
    int idx = h->pos[v];
    if (idx != -1 && new_dist < h->nodes[idx].distance) {
        h->nodes[idx].distance = new_dist;
        heapify_up(h, idx);
    }
}

// Libera la memoria del heap
void free_min_heap(MinHeap* h) {
    free(h->nodes);
    free(h->pos);
    free(h);
}

// --- ALGORITMO 2: BASE CASE DI BMSSP ---
double BASECASE(Graph* G, double B, VertexSet* S, VertexSet* U) {
    int x = S->vertices[0]; // S è un singoletto
    
    // Inizializza U_0 con x
    VertexSet* U_0 = create_set(k + 1); 
    
    // Inizializza il Min-Heap
    MinHeap* H = create_min_heap(G->n);
    push_heap(H, x, d_hat[x]); // Linea 3
    
    double max_d_in_U0 = 0.0; // Ci serve per la linea 16 (max d[u] in U_0)
    
    // Linea 4: while H is non-empty and |U_0| < k + 1
    while (H->size > 0 && U_0->size < k + 1) {
        
        // Linea 5: Estrai il minimo
        int u = pop_heap(H);
        
        // Linea 6: Aggiungi u a U_0
        add_to_set(U_0, u);
        
        // Tracciamo il valore massimo della vera distanza d_hat estratto per il calcolo di B_prime
        if (d_hat[u] > max_d_in_U0) {
            max_d_in_U0 = d_hat[u];
        }
        
        // Linea 7: Per ogni arco (u, v)
        for (Edge* e = G->head[u]; e != NULL; e = e->next) {
            int v = e->to;
            double new_dist = d_hat[u] + e->weight;
            
            // Linea 8: se il rilassamento è valido ed è minore di B
            if (new_dist <= d_hat[v] && new_dist < B) {
                d_hat[v] = new_dist; // Linea 9
                PRED[v] = u;         // Aggiorniamo l'albero dei cammini minimi
                
                // Linea 10-13: Inserisci in H o aggiorna (Decrease-Key)
                if (H->pos[v] == -1) {
                    push_heap(H, v, d_hat[v]);
                } else {
                    decrease_key(H, v, d_hat[v]);
                }
            }
        }
    }
    
    // Pulizia memoria del heap
    free_min_heap(H);
    
    double B_prime;
    
    // Linea 14: if |U_0| <= k
    if (U_0->size <= k) {
        B_prime = B; // Linea 15
        // U = U_0
        for (int i = 0; i < U_0->size; i++) {
            add_to_set(U, U_0->vertices[i]);
        }
    } 
    // Linea 16: else
    else {
        B_prime = max_d_in_U0; // Linea 17: B' <- max_{v in U_0} d[v]
        
        // U = {v in U_0 : d[v] < B_prime}
        for (int i = 0; i < U_0->size; i++) {
            int v = U_0->vertices[i];
            if (d_hat[v] < B_prime) {
                add_to_set(U, v);
            }
        }
    }
    
    // Pulizia memoria set temporaneo
    free(U_0->vertices);
    free(U_0);
    
    return B_prime;
}

// Funzione ricorsiva per contare i nodi nel sottoalbero all'interno della foresta F
int count_subtree(int u, int* first_child, int* next_sibling) {
    int count = 1; // Conta se stesso
    int child = first_child[u];
    
    // Visita tutti i figli di u nella foresta
    while (child != -1) {
        count += count_subtree(child, first_child, next_sibling);
        child = next_sibling[child];
    }
    
    return count;
}

// --- ALGORITMO 1: FIND PIVOTS ---
void FINDPIVOTS(Graph* G, double B, VertexSet* S, VertexSet* P, VertexSet* W) {
    VertexSet* W_prev = create_set(G->n);
    bool* is_in_W = (bool*)calloc(G->n, sizeof(bool)); // Per evitare duplicati in W
    
    // Inizializzazione: W = S, W_0 = S
    for(int i = 0; i < S->size; i++) {
        int v = S->vertices[i];
        add_to_set(W, v);
        add_to_set(W_prev, v);
        is_in_W[v] = true;
    }
    
    // Linee 4-11: Rilassamento per k step
    for (int i = 1; i <= k; i++) {
        VertexSet* W_curr = create_set(G->n);
        
        for (int j = 0; j < W_prev->size; j++) {
            int u = W_prev->vertices[j];
            
            for (Edge* e = G->head[u]; e != NULL; e = e->next) {
                int v = e->to;
                double new_dist = d_hat[u] + e->weight;
                
                if (new_dist <= d_hat[v]) {
                    d_hat[v] = new_dist;
                    PRED[v] = u;
                    
                    if (new_dist < B) {
                        add_to_set(W_curr, v);
                        if (!is_in_W[v]) {
                            add_to_set(W, v);
                            is_in_W[v] = true;
                        }
                    }
                }
            }
        }
        
        // Linea 12-14: Early exit se il carico di lavoro supera k*|S|
        if (W->size > k * S->size) {
            for(int j = 0; j < S->size; j++) {
                add_to_set(P, S->vertices[j]);
            }
            free(W_curr->vertices); free(W_curr);
            free(W_prev->vertices); free(W_prev);
            free(is_in_W);
            return;
        }
        
        free(W_prev->vertices); free(W_prev);
        W_prev = W_curr; // Prepara per il prossimo step
    }
    free(W_prev->vertices); free(W_prev);

    // ==============================================================
    // TODO COMPLETATO: Calcolo componenti foresta F e riempimento P
    // Linee 15-17 del paper
    // ==============================================================
    
    // Strutture per rappresentare la foresta F in tempo O(|W|)
    int* first_child = (int*)malloc(G->n * sizeof(int));
    int* next_sibling = (int*)malloc(G->n * sizeof(int));
    for (int i = 0; i < G->n; i++) {
        first_child[i] = -1;
        next_sibling[i] = -1;
    }
    
    // Costruzione della foresta F
    for (int i = 0; i < W->size; i++) {
        int v = W->vertices[i];
        int u = PRED[v];
        
        // Se u è in W e (u,v) fa parte dell'albero dei cammini minimi
        if (u != -1 && is_in_W[u] && u != v) {
            // Aggiungi 'v' come figlio di 'u'
            next_sibling[v] = first_child[u];
            first_child[u] = v;
        }
    }
    
    // Riempimento di P: troviamo le radici in S con sottoalbero di dimensione >= k
    for (int i = 0; i < S->size; i++) {
        int root = S->vertices[i];
        
        int tree_size = count_subtree(root, first_child, next_sibling);
        
        if (tree_size >= k) {
            add_to_set(P, root);
        }
    }
    
    // Pulizia finale della memoria
    free(first_child);
    free(next_sibling);
    free(is_in_W);
}

double BMSSP(Graph* G, int l, double B, VertexSet* S, VertexSet* U) {
    if (l == 0) return BASECASE(G, B, S, U);
    
    VertexSet* P = create_set(G->n);
    VertexSet* W = create_set(G->n);
    FINDPIVOTS(G, B, S, P, W); 
    
    CustomDataStructD D;
    int M = pow(2, (l - 1) * t);
    D_INITIALIZE(&D, M, B);
    
    double B_prime_0 = INFINITY;
    for (int idx = 0; idx < P->size; idx++) {
        int x = P->vertices[idx];
        D_INSERT(&D, x, d_hat[x]);
        if (d_hat[x] < B_prime_0) B_prime_0 = d_hat[x];
    }
    if (P->size == 0) B_prime_0 = B;
    
    int i = 0;
    int limit = k * pow(2, l * t);
    
    while (U->size < limit && !D.is_empty) {
        i++;
        VertexSet* S_i = create_set(G->n);
        double B_i = D_PULL(&D, S_i, B);

        VertexSet* U_i = create_set(G->n);
        double B_prime_i = BMSSP(G, l - 1, B_i, S_i, U_i);

        for (int j = 0; j < U_i->size; j++) add_to_set(U, U_i->vertices[j]);

        VertexSet* K = create_set(G->n);

        for (int j = 0; j < U_i->size; j++) {
            int u = U_i->vertices[j];
            for (Edge* e = G->head[u]; e != NULL; e = e->next) {
                int v = e->to;
                double new_dist = d_hat[u] + e->weight;

                if (new_dist <= d_hat[v]) {
                    d_hat[v] = new_dist;
                    PRED[v] = u;

                    if (new_dist >= B_i && new_dist < B) D_INSERT(&D, v, new_dist);
                    else if (new_dist >= B_prime_i && new_dist < B_i) add_to_set(K, v);
                }
            }
        }
        // Aggiungiamo a K anche i vertici di S_i che rientrano nel range richiesto
        for (int j = 0; j < S_i->size; j++) {
            int x = S_i->vertices[j];
            if (d_hat[x] >= B_prime_i && d_hat[x] < B_i) {
                add_to_set(K, x);
            }
        }
        D_BATCHPREPEND(&D, K);

        // Libera i set temporanei di questa iterazione
        free_set(S_i);
        free_set(U_i);
        free_set(K);
    }

    // --- Calcolo corretto di B' (Lemma 3.1) ---
    double B_prime;
    if (D.is_empty) {
        B_prime = B;
    } else {
        B_prime = D_PEEK_MIN(&D, B);
    }

    // Libera P, W e la struttura D prima di ritornare
    free_set(P);
    free_set(W);
    free_D(&D);

    return B_prime;
}

// ==========================================
// 5. MAIN
// ==========================================

int main() {
    // Esempio di inizializzazione base
    int num_nodes = 10;
    Graph G;
    G.n = num_nodes;
    G.head = (Edge**)calloc(num_nodes, sizeof(Edge*));
    
    d_hat = (double*)malloc(num_nodes * sizeof(double));
    PRED = (int*)malloc(num_nodes * sizeof(int));
    
    for(int i=0; i<num_nodes; i++) {
        d_hat[i] = INFINITY;
        PRED[i] = -1;
    }
    
    // Nodo sorgente 0
    d_hat[0] = 0;
    VertexSet* S_iniziale = create_set(1);
    add_to_set(S_iniziale, 0);
    VertexSet* U_finale = create_set(num_nodes);
    
    int l = ceil(log2(num_nodes) / t); 
    if (l < 0) l = 0;

    printf("Inizio esecuzione BMSSP...\n");
    // BMSSP(&G, l, INFINITY, S_iniziale, U_finale);
    printf("Esecuzione completata (Scheletro pronto).\n");

    // Pulizia memoria globale
    free_set(S_iniziale);
    free_set(U_finale);
    free(d_hat);
    free(PRED);
    free(G.head);

    return 0;
}