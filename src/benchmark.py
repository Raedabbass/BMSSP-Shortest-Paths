import subprocess
import re
import csv

# --- CONFIGURAZIONE DEI TEST ---
# Formato: (Numero di Nodi V, Numero di Archi E)
# Nota: L'ultimo test da 500mila nodi potrebbe richiedere qualche secondo in più!
test_cases = [
    (10000, 50000),
    (50000, 250000),
    (100000, 500000),
    (250000, 1250000),
    (500000, 2500000) 
]

max_weight = 20.0
delta = 5.0
output_csv = "risultati_benchmark.csv"

# 1. Compiliamo il codice C per assicurarci che sia aggiornato
print("Compilazione del progetto C in corso...")
compile_cmd = ["gcc", "-O3", "main.c", "graph.c", "minheap.c", "dijkstra.c", "thorup.c", "bmssp.c", "-o", "main"]
subprocess.run(compile_cmd, check=True)
print("Compilazione completata con successo (flag -O3 per le massime prestazioni attivati).\n")

# Prepariamo il file CSV
with open(output_csv, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Nodi (V)", "Archi (E)", "Dijkstra (ms)", "Thorup (ms)", "BMSSP (ms)"])

    # 2. Cicliamo attraverso i test
    for V, E in test_cases:
        print(f"--- Esecuzione Test: {V} nodi, {E} archi ---")
        
        # Lanciamo il programma C passandogli i parametri
        run_cmd = ["./main", str(V), str(E), str(max_weight), str(delta)]
        result = subprocess.run(run_cmd, capture_output=True, text=True)
        output = result.stdout
        
        # 3. Estraiamo i millisecondi usando le Espressioni Regolari (Regex)
        # Cerca i numeri che stanno tra "Completato in: " e " ms"
        tempi = re.findall(r"Completato in: ([0-9.]+) ms", output)
        
        if len(tempi) == 3:
            t_dijkstra, t_thorup, t_bmssp = tempi
            print(f"Risultati estratti -> Dijkstra: {t_dijkstra}ms | Thorup: {t_thorup}ms | BMSSP: {t_bmssp}ms\n")
            
            # Scriviamo la riga nel CSV
            writer.writerow([V, E, t_dijkstra, t_thorup, t_bmssp])
        else:
            print("Errore nella lettura dell'output del C!")
            print(output)

print(f"🎉 Benchmark concluso! I dati sono stati salvati in: {output_csv}")