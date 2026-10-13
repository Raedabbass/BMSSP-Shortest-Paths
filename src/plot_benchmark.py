import matplotlib.pyplot as plt
import csv

# Liste per memorizzare i dati
nodi = []
dijkstra_tempi = []
thorup_tempi = []
bmssp_tempi = []

# Leggiamo i dati dal CSV che abbiamo appena generato
csv_filename = "risultati_benchmark.csv"

try:
    with open(csv_filename, mode='r') as file:
        reader = csv.reader(file)
        next(reader) # Saltiamo la prima riga (l'intestazione)
        
        for row in reader:
            nodi.append(int(row[0]))
            dijkstra_tempi.append(float(row[2]))
            thorup_tempi.append(float(row[3]))
            bmssp_tempi.append(float(row[4]))
except FileNotFoundError:
    print(f"Errore: file {csv_filename} non trovato. Assicurati di aver lanciato prima benchmark.py!")
    exit()

# --- CREAZIONE DEL GRAFICO ---
# Impostiamo la dimensione della figura (perfetta per le slide, formato 16:9)
plt.figure(figsize=(10, 6))

# Disegniamo le tre linee. Ho scelto colori "semaforici" per evidenziare il vincitore
plt.plot(nodi, dijkstra_tempi, marker='o', linewidth=2.5, markersize=8, color='#d62728', label='Dijkstra (Min-Heap)') # Rosso
plt.plot(nodi, thorup_tempi, marker='s', linewidth=2.5, markersize=8, color='#ff7f0e', label='Thorup (Bucket & XOR)') # Arancione
plt.plot(nodi, bmssp_tempi, marker='^', linewidth=2.5, markersize=10, color='#2ca02c', label='BMSSP (Batch Queue)')  # Verde

# Aggiungiamo titoli e etichette eleganti
plt.title('Confronto Tempi di Esecuzione SSSP (Single Source Shortest Path)', fontsize=16, fontweight='bold', pad=15)
plt.xlabel('Dimensione del Grafo (Numero di Nodi)', fontsize=12, fontweight='bold')
plt.ylabel('Tempo di Esecuzione (Millisecondi)', fontsize=12, fontweight='bold')

# Formattiamo l'asse X per evitare i numeri scientifici (es. 5e5)
plt.ticklabel_format(style='plain', axis='x')

# Aggiungiamo una griglia di sfondo per facilitare la lettura
plt.grid(True, linestyle='--', alpha=0.6)

# Aggiungiamo la legenda
plt.legend(fontsize=12, loc='upper left')

# Abbelliamo i bordi
plt.tight_layout()

# Salviamo l'immagine in altissima risoluzione (300 dpi) pronta per PowerPoint/Keynote
output_img = 'grafico_prestazioni_slide.png'
plt.savefig(output_img, dpi=300)

print(f"🎨 Grafico generato con successo! Lo trovi salvato come: {output_img}")

# Mostriamo anche il grafico a schermo
plt.show()