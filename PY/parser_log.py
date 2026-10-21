"""
ltspice_compare.py
------------------
Parsing e confronto di file .log generati da LTSpice.
Stile grafico IEEE (Times New Roman, assi minimi, colori sobri).
"""

import re
from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# ── Stile IEEE ─────────────────────────────────────────────────────────────────
IEEE_STYLE = {
    # Font
    "font.family":        "serif",
    "font.serif":         ["Times New Roman", "Times", "DejaVu Serif"],
    "font.size":          9,
    "axes.titlesize":     9,
    "axes.labelsize":     9,
    "xtick.labelsize":    8,
    "ytick.labelsize":    8,
    "legend.fontsize":    8,
    # Linee
    "axes.linewidth":     0.8,
    "grid.linewidth":     0.5,
    "lines.linewidth":    1.2,
    "patch.linewidth":    0.8,
    # Griglia
    "axes.grid":          True,
    "grid.alpha":         0.35,
    "grid.linestyle":     "--",
    # Tick
    "xtick.direction":    "in",
    "ytick.direction":    "in",
    "xtick.top":          True,
    "ytick.right":        True,
    "xtick.minor.visible": True,
    "ytick.minor.visible": True,
    # Figure
    "figure.dpi":         300,
    "savefig.dpi":        300,
    "savefig.bbox":       "tight",
    "savefig.pad_inches": 0.05,
}

# Palette sobria, distinguibile anche in B&W con marker diversi
_COLORS  = ["#1f4e79", "#c00000", "#375623", "#7030a0", "#833c00", "#156082"]
_MARKERS = ["o",       "s",       "^",       "D",       "v",       "P"]
_HATCHES = ["/",       "\\",      "x",       ".",       "-",       "+"]


# ── Parser ──────────────────────────────────────────────────────────────────────
def parse_ltspice_meas(log_path: str | Path) -> dict[str, float]:
    """
    Legge un file .log di LTSpice e restituisce un dizionario
    { nome_parametro (lowercase): valore_float } per tutte le
    misure generate da direttive .meas/.measure.

    Gestisce i formati:
      nome: AVG(I(D1))=2.236 FROM 0.002 TO 0.003
      nome: (Vomax-Vomin)=12.378
      nome: 'I_in*V_in'=-119.127
    """
    MEAS_RE = re.compile(
        r"^(?P<name>\w+)\s*:.*="
        r"\s*(?P<value>[+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)"
        r"(?:\s+FROM\b.*)?$",
        re.IGNORECASE,
    )
    results: dict[str, float] = {}
    with open(log_path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = MEAS_RE.match(line.strip())
            if m:
                results[m.group("name").lower()] = float(m.group("value"))
    return results


# ── Plot ────────────────────────────────────────────────────────────────────────
def compare_circuits(
    log_files: dict[str, str | Path],
    parameters: list[str],
    *,
    units: dict[str, str] | None = None,
    labels: dict[str, str] | None = None,
    output_dir: str | Path = ".",
    fig_width_col: float = 3.5,
    fig_height: float = 2.6,
    bar_width: float = 0.55,
    show: bool = True,
) -> dict[str, Path]:
    """
    Genera un plot IEEE per ogni parametro in `parameters`, confrontando
    i valori estratti dai file .log in `log_files`.

    Parametri
    ---------
    log_files : dict[str, path]
        Chiave = nome del circuito (usato come label), valore = percorso al .log.
        Esempio: {"Boost A": "boost_a.log", "Boost B": "boost_b.log"}

    parameters : list[str]
        Nomi dei parametri (come appaiono nel .log, case-insensitive).
        Esempio: ["eta", "ripple", "il_rms"]

    units : dict[str, str], opzionale
        Unità di misura per asse Y. Esempio: {"eta": "-", "ripple": "V"}

    labels : dict[str, str], opzionale
        Label leggibili per l'asse X / titolo.
        Esempio: {"eta": "Efficiency η", "ripple": "Output Voltage Ripple"}

    output_dir : path
        Directory dove salvare i PNG/PDF. Default: cartella corrente.

    fig_width_col : float
        Larghezza figura in pollici. 3.5 = singola colonna IEEE, 7.0 = doppia.

    fig_height : float
        Altezza figura in pollici.

    bar_width : float
        Larghezza delle barre (0–1).

    show : bool
        Se True chiama plt.show() dopo ogni figura.

    Ritorna
    -------
    dict[str, Path]
        { nome_parametro: percorso_file_salvato }
    """
    units  = units  or {}
    labels = labels or {}
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    # Parsing di tutti i file
    data: dict[str, dict[str, float]] = {
        circuit: parse_ltspice_meas(path)
        for circuit, path in log_files.items()
    }

    circuit_names = list(log_files.keys())
    n_circuits    = len(circuit_names)
    x_pos         = np.arange(n_circuits)

    saved: dict[str, Path] = {}

    with matplotlib.rc_context(IEEE_STYLE):
        for param in parameters:
            key    = param.lower()
            values = [data[c].get(key, float("nan")) for c in circuit_names]

            fig, ax = plt.subplots(figsize=(fig_width_col, fig_height))

            for i, (val, name) in enumerate(zip(values, circuit_names)):
                color  = _COLORS[i % len(_COLORS)]
                hatch  = _HATCHES[i % len(_HATCHES)]
                ax.bar(
                    x_pos[i], val,
                    width=bar_width,
                    color=color,
                    hatch=hatch,
                    edgecolor="white",
                    linewidth=0.6,
                    label=name,
                )
                # Valore sopra la barra
                if not np.isnan(val):
                    ax.text(
                        x_pos[i], val,
                        f"{val:.3g}",
                        ha="center", va="bottom",
                        fontsize=7, color=color,
                    )

            # Asse Y
            y_label = labels.get(key, param)
            unit    = units.get(key, "")
            ax.set_ylabel(f"{y_label} [{unit}]" if unit else y_label)

            # Asse X: nomi circuiti
            ax.set_xticks(x_pos)
            rot_angle = 0 if n_circuits <= 4 else 30
            h_align = "center" if rot_angle == 0 else "right"

            ax.set_xticklabels(circuit_names, rotation=rot_angle, ha=h_align)
            ax.set_xlim(-0.6, n_circuits - 0.4)

            # Tick minori sull'asse Y
            ax.yaxis.set_minor_locator(ticker.AutoMinorLocator(4))

            # Rimozione bordi superiore/destro (opzionale, più pulito)
            ax.spines["top"].set_linewidth(0.4)
            ax.spines["right"].set_linewidth(0.4)

            fig.tight_layout(pad=0.4)

            # Salvataggio
            png_fname = output_dir / f"compare_{key}.png"
            fig.savefig(png_fname)
            saved[param] = png_fname

            if show:
                plt.show()
            plt.close(fig)

    return saved


def generate_metadata(parameters):
    units = {}
    labels = {}

    for param in parameters:
        p_lower = param.lower()

        # 1. Determine Units
        # Logic: 'v' for Voltage (V), 'i' for Current (A), 'eta' for efficiency (-)
        if "eta" in p_lower:
            units[param] = "-"
        elif "v" in p_lower or param == "ripple":  # 'ripple' defaults to Voltage in your example
            units[param] = "V"
        elif "i" in p_lower:
            units[param] = "A"
        else:
            units[param] = ""  # Dimensionless/Unknown

        # 2. Determine Labels
        # Logic: Use Greek eta for efficiency, \Delta_{name} for ripples
        if "eta" in p_lower:
            labels[param] = r"$\eta$"
        elif "ripple" in p_lower:
            # Cleaning name to extract the base parameter for the subscript
            base_name = param.replace("_ripple", "").replace("ripple_", "")
            labels[param] = f"$\Delta_{{{base_name}}}$"
        else:
            labels[param] = param.replace("_", " ").capitalize()

    return units, labels




# ── Esempio d'uso ───────────────────────────────────────────────────────────────
if __name__ == "__main__":

    log_files = {
        "Conventional BC": "Boost Real/boost.log",
        "2 Phases IBC": "Boost Interleaved Real/boost_inter2.log",
        "2 Phases Soft": "Boost Interleaved Real/boost_soft2.log",
        "3 Phases IBC": "Boost Interleaved Real/boost_inter3.log",
        "4 Phases IBC": "Boost Interleaved Real/boost_inter4.log",

    }

    parameters = ["eta", "iin_ripple", "vo_ripple"]

    units = {
        "eta":              "-",
        "iin_ripple":       "A",
        "vo_ripple":           "V",
    }

    labels = {
        "eta":          "Efficiency",
        "iin_ripple":       "Input current ripple",
        "vo_ripple":        "Output Voltage Ripple",
    }


    #units, labels = generate_metadata(parameters)

    saved = compare_circuits(
        log_files,
        parameters,
        units=units,
        labels=labels,
        output_dir="plots_newcomponents",
        fig_width_col=3.5,   # singola colonna IEEE
        show=True,
    )

    for param, path in saved.items():
        print(f"  {param:20s} → {path}")