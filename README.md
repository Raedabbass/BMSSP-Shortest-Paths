# Breaking the Sorting Barrier for Directed Single-Source Shortest Paths (BMSSP)

University project developed for the Computer Engineering course at **Politecnico di Milano**  
**Author:** Raed Abbas  
**Supervisor:** Prof. Alessandro Barenghi  

---

## Project Overview
This repository contains the theoretical study, C implementation, and performance benchmark of the **BMSSP (Bounded Multi-Source Shortest Path)** algorithm, based on the paper *"Breaking the Sorting Barrier for Directed Single-Source Shortest Paths"* (Duan, Mao, Shu, Yin - 2025).

The goal is to overcome the frontier-sorting bottleneck ($\Omega(\vert{}V\vert{} \log \vert{}V\vert{})$) typical of the classical Dijkstra's algorithm on sparse directed graphs with non-negative real edge weights ($\mathbb{R}_{\ge 0}$), achieving a time complexity of $\mathcal{O}(\vert{}E\vert{} \log^{2/3} \vert{}V\vert{})$ in the *Comparison-Addition* model.

Three different approaches are implemented and compared in this project:
1. **Classical Dijkstra (with Min-Heap):** Time complexity $\mathcal{O}(\vert{}E\vert{} + \vert{}V\vert{} \log \vert{}V\vert{})$.
2. **Thorup's Algorithm (Bucket Queue & XOR/MSB):** Near-linear time complexity in the *Word RAM* model, but strictly limited to **integer** weights ($\mathbb{N}$).
3. **BMSSP:** Batch-based exploration combined with local Bellman-Ford-style relaxations, capable of handling **real** weights ($\mathbb{R}_{\ge 0}$) without maintaining a total order over the entire frontier.

---

## Benchmark Results

Tests were executed on randomly generated directed graphs of increasing size (compiled with `-O3` optimization flags):

| Vertices ($V$) | Edges ($E$) | Dijkstra (ms) | Thorup (ms) | BMSSP (ms) |
| :--- | :--- | :--- | :--- | :--- |
| 10,000 | 50,000 | 2.22 ms | 1.10 ms | **0.42 ms** |
| 50,000 | 250,000 | 17.57 ms | 4.76 ms | **1.37 ms** |
| 100,000 | 500,000 | 38.07 ms | 17.68 ms | **5.94 ms** |
| 250,000 | 1,250,000 | 142.19 ms | 76.98 ms | **24.20 ms** |
| 500,000 | 2,500,000 | 342.20 ms | 200.81 ms | **57.74 ms** |

![Performance Graph](src/grafico_prestazioni_slide.png)

> **Note:** *The labels in the performance chart above, the presentation slides, and the inline code comments are in Italian, as they are part of the original project material developed in Italian for Politecnico di Milano.*

---

## Repository Structure
* `src/`: C implementation of the three algorithms (`dijkstra.c`, `thorup.c`, `bmssp.c`), supporting data structures (`graph.c`, `minheap.c`), and Python scripts for benchmark automation and plotting.
* `theoretical_impl/`: Full C implementation of the hybrid data structure proposed in the paper (Block sequence $D_0$ + Red-Black Tree $D_1$, `FINDPIVOTS` procedure, and level-$l$ recursion).
* `docs/`: LaTeX Beamer presentation (`Slides_BMSSP.pdf` and `.tex` source file, in Italian) and the reference paper (`paperBMSSP.pdf`).

---

## How to Compile and Run the Benchmark

### 1. Manual Execution (C)
Navigate to the `src/` directory and compile using `gcc`:
```bash
cd src
gcc -O3 main.c graph.c minheap.c dijkstra.c thorup.c bmssp.c -o main
./main 100000 500000 20.0 5.0
```
*(Parameters: `<Number of Vertices V> <Number of Edges E> <Max Weight> <Delta>`)*

### 2. Automated Benchmark Suite (Python)
To run the full test suite, generate the CSV file, and plot the performance graph using `matplotlib`:
```bash
cd src
python3 benchmark.py
python3 plot_benchmark.py
```
