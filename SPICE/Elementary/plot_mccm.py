import matplotlib.pyplot as plt
import numpy as np
import ltspice
import os

# =============================================================================
# 1. Configurazione Stile IEEE Standard
# =============================================================================
plt.rcParams.update({
    "font.family": "serif",
    "font.serif": ["Times New Roman"],
    "font.size": 10,
    "axes.labelsize": 10,
    "axes.titlesize": 10,
    "xtick.labelsize": 8,
    "ytick.labelsize": 8,
    "legend.fontsize": 8,
    "figure.figsize": (3.5, 2.5),  # Larghezza singola colonna IEEE (3.5 inch)
    "figure.dpi": 300,
    "lines.linewidth": 1.0,
    "axes.grid": True,
    "grid.alpha": 0.5,
    "grid.linestyle": "--"
})

# =============================================================================
# 2. Parametri e Percorsi dei file
# =============================================================================
# Sostituisci questi nomi generici con i tuoi file .log.raw effettivi
log_files = ["boostideal.log.raw", "boostideal_inter2.log.raw", "boostideal_inter4.log.raw"]
plot_labels = ["Conventional Boost", "2-Phases IBC", "4-Phases IBC"]
output_plot_path = "ieee_mccm_comparison.png"

# Stili per la differenziazione in stampa (bianco e nero / colori)
colors = ['green', 'blue', 'red']
line_styles = ['-', '--', '-.']

fig, ax = plt.subplots()

# =============================================================================
# 3. Lettura e Plot di ogni file
# =============================================================================
for i, log_file in enumerate(log_files):
    if not os.path.exists(log_file):
        print(f"Warning: File '{log_file}' non trovato nella directory corrente.")
        continue
        
    # Carica e analizza il file .log.raw
    l = ltspice.Ltspice(log_file)
    l.parse()

    # Estrai la variabile del parametro di step (asse X) e la misura (asse Y)
    duty_cycle = l.get_data('d')
    m_ccm = l.get_data('m_ccm')

    # Plotta i dati estratti
    ax.plot(duty_cycle, m_ccm, label=plot_labels[i], color=colors[i], linestyle=line_styles[i])

# =============================================================================
# 4. Etichette, Limiti e Salvataggio
# =============================================================================
ax.set_xlabel('Duty Cycle ($D$)')
ax.set_ylabel('$M_{CCM}$')

# Posizionamento automatico della legenda per evitare sovrapposizioni
ax.legend(loc='best', frameon=True, edgecolor='black')

plt.tight_layout()

# Salva il grafico in formato PDF (ideale per LaTeX / Word) e PNG
plt.savefig(output_plot_path, format='png', bbox_inches='tight')
# Opzionale: salva anche in PNG ad alta risoluzione
# plt.savefig(output_plot_path.replace('.pdf', '.png'), format='png', bbox_inches='tight')

# Mostra il plot a schermo
plt.show()