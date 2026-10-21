"""
ltspice_advanced_compare.py
---------------------------
Advanced parsing and comparison of LTSpice .log files.
Features:
 - IEEE graphical style
 - Dynamic extraction of stepped variables
 - Missing data (NaN) handling for failed convergence
 - Pandas DataFrame integration for easy data manipulation
 - Support for multi-dimensional sweeps
"""

import re
from pathlib import Path
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import pandas as pd

# ── IEEE Style (from your previous setup) ──────────────────────────────────────
IEEE_STYLE = {
    "font.family": "serif",
    "font.serif": ["Times New Roman", "Times", "DejaVu Serif"],
    "font.size": 9,
    "axes.titlesize": 9,
    "axes.labelsize": 9,
    "xtick.labelsize": 8,
    "ytick.labelsize": 8,
    "legend.fontsize": 8,
    "axes.linewidth": 0.8,
    "grid.linewidth": 0.5,
    "lines.linewidth": 1.2,
    "patch.linewidth": 0.8,
    "axes.grid": True,
    "grid.alpha": 0.35,
    "grid.linestyle": "--",
    "xtick.direction": "in",
    "ytick.direction": "in",
    "xtick.top": True,
    "ytick.right": True,
    "xtick.minor.visible": True,
    "ytick.minor.visible": True,
    "figure.dpi": 300,
    "savefig.dpi": 300,
    "savefig.bbox": "tight",
    "savefig.pad_inches": 0.05,
}

_COLORS = ["#1f4e79", "#c00000", "#375623", "#7030a0", "#833c00", "#156082"]
_MARKERS = ["o", "s", "^", "D", "v", "P"]


# ── Advanced Parser ─────────────────────────────────────────────────────────────
def parse_log_to_dataframe(log_path: str | Path, circuit_label: str) -> pd.DataFrame:
    """
    Parses an LTspice .log file into a Pandas DataFrame.
    Automatically detects stepped variables and measurement tables, matching
    step numbers to avoid misalignment if a simulation step fails.
    """
    step_map = {}
    measurements = {}
    current_meas = None
    step_counter = 1

    try:
        with open(log_path, 'r', encoding='latin-1') as file:
            for line in file:
                line = line.strip()

                # 1. Detect Stepped Variables (e.g., ".step d=0.5" or ".step d=0.5 Rload=10")
                if line.lower().startswith(".step"):
                    # Find all key=value pairs in the step line
                    pairs = re.findall(r"([a-zA-Z0-9_]+)=([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)", line)
                    if pairs:
                        step_map[step_counter] = {k.lower(): float(v) for k, v in pairs}
                        step_counter += 1
                    continue

                # 2. Detect Measurement Blocks (e.g., "Measurement: eta")
                meas_match = re.match(r"^Measurement:\s+(\w+)", line, re.IGNORECASE)
                if meas_match:
                    current_meas = meas_match.group(1).lower()
                    if current_meas not in measurements:
                        measurements[current_meas] = {}
                    continue

                # 3. Extract Data Inside Measurement Blocks
                if current_meas:
                    # Look for step number and value: "1    0.902944"
                    data_match = re.match(r"^(\d+)\s+([+-]?\d+\.?\d*(?:[eE][+-]?\d+)?)", line)
                    if data_match:
                        step_idx = int(data_match.group(1))
                        val = float(data_match.group(2))
                        measurements[current_meas][step_idx] = val
                    elif not line and measurements[current_meas]:
                        # Empty line signals the end of the block
                        current_meas = None

    except FileNotFoundError:
        print(f"Error: Log file not found at {log_path}")
        return pd.DataFrame()

    # 4. Compile into a DataFrame
    data_rows = []

    # If there are no steps found (single run), fake a step 1
    if not step_map:
        step_map[1] = {"run": 1}

    for step_idx, step_vars in step_map.items():
        row = {"Circuit": circuit_label, "Step": step_idx}
        row.update(step_vars)  # Add the dynamic stepped variables (like 'd')

        for meas_name, meas_data in measurements.items():
            # Use .get() to insert NaN if LTspice failed to calculate this specific step
            row[meas_name] = meas_data.get(step_idx, np.nan)

        data_rows.append(row)

    return pd.DataFrame(data_rows)


# ── Plotting Routine ────────────────────────────────────────────────────────────
def plot_stepped_comparisons(
        master_df: pd.DataFrame,
        parameters: list[str],
        stepped_var: str = None,
        units: dict[str, str] = None,
        labels: dict[str, str] = None,
        output_dir: str | Path = "."
):
    """
    Generates comparative plots for stepped parameters.
    """
    units = units or {}
    labels = labels or {}
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    # Auto-detect the stepped variable if not provided
    if stepped_var is None:
        # Exclude known non-stepped columns
        exclude = ["Circuit", "Step"] + parameters
        potential_vars = [col for col in master_df.columns if col not in exclude]
        if potential_vars:
            stepped_var = potential_vars[0]  # Default to the first found stepped variable (e.g., 'd')
        else:
            print("Error: No stepped variable detected in the logs to use for the X-axis.")
            return

    print(f"Using '{stepped_var}' as the stepped X-axis variable.")
    circuits = master_df["Circuit"].unique()

    with matplotlib.rc_context(IEEE_STYLE):
        for param in parameters:
            key = param.lower()
            if key not in master_df.columns:
                print(f"Skipping {key}: Not found in parsed data.")
                continue

            fig, ax = plt.subplots(figsize=(3.5, 2.6))  # Single IEEE column

            for i, circuit in enumerate(circuits):
                # Filter data for current circuit
                df_circ = master_df[master_df["Circuit"] == circuit].dropna(subset=[stepped_var, key])

                # Sort to ensure lines draw cleanly left-to-right
                df_circ = df_circ.sort_values(by=stepped_var)

                color = _COLORS[i % len(_COLORS)]
                marker = _MARKERS[i % len(_MARKERS)]

                ax.plot(
                    df_circ[stepped_var],
                    df_circ[key],
                    marker=marker,
                    color=color,
                    label=circuit,
                    markersize=4,
                    linewidth=1.2
                )

            # Axis Formatting
            x_label = labels.get(stepped_var, stepped_var.upper())
            y_label = labels.get(key, param)
            unit = units.get(key, "")

            ax.set_xlabel(x_label)
            ax.set_ylabel(f"{y_label} [{unit}]" if unit else y_label)

            # Minor ticks and spines
            ax.xaxis.set_minor_locator(ticker.AutoMinorLocator(4))
            ax.yaxis.set_minor_locator(ticker.AutoMinorLocator(4))
            ax.spines["top"].set_linewidth(0.4)
            ax.spines["right"].set_linewidth(0.4)

            ax.legend()
            fig.tight_layout(pad=0.4)

            # Save
            fname = output_dir / f"stepped_compare_{key}.png"
            fig.savefig(fname)
            plt.close(fig)
            print(f"Saved: {fname}")


def find_duty_cycle_for_gain(df: pd.DataFrame, target_gain: float, circuit_label: str) -> float:
    """
    Finds the exact duty cycle required to achieve a target M_ccm
    using linear interpolation on the simulated data points.
    """
    # Filter the DataFrame for the specific circuit and remove NaNs
    df_circ = df[df["Circuit"] == circuit_label].dropna(subset=["d", "m_ccm"])

    if df_circ.empty:
        print(f"No data available for {circuit_label}.")
        return np.nan

    # Sort by m_ccm to ensure numpy's interpolation works correctly
    df_circ = df_circ.sort_values(by="m_ccm")

    # We interpolate to find 'd' given 'm_ccm', so m_ccm is our X-axis for the math
    m_ccm_array = df_circ["m_ccm"].values
    d_array = df_circ["d"].values

    # Check if the target is out of bounds
    if target_gain < m_ccm_array.min() or target_gain > m_ccm_array.max():
        print(f"Warning: Target gain {target_gain} is outside the simulated range for {circuit_label}.")

    # Calculate the interpolated duty cycle
    calculated_d = np.interp(target_gain, m_ccm_array, d_array)

    return calculated_d
# ── Execution ───────────────────────────────────────────────────────────────────
if __name__ == "__main__":

    # 1. Define your logs
    log_files = {
        "BC": "Boost Stack/boost_stack_h100.log",
        "2-IBC": "Boost Stack/boost_stack_inter2.log",
        "4-IBC": "Boost Stack/boost_stack_inter4.log"
    }

    # 2. Parse all logs into a single powerful Pandas DataFrame
    all_dfs = []
    for circuit_name, path in log_files.items():
        df = parse_log_to_dataframe(path, circuit_name)
        if not df.empty:
            all_dfs.append(df)

    if all_dfs:
        master_df = pd.concat(all_dfs, ignore_index=True)
        # --- NEW: Find duty cycle for M_ccm = 3.7 ---
        target_mccm = 3.7
        print(f"\n--- Interpolation Results for M_ccm = {target_mccm} ---")

        for circuit in master_df["Circuit"].unique():
            required_d = find_duty_cycle_for_gain(master_df, target_mccm, circuit)
            if not np.isnan(required_d):
                print(f"To get M_ccm = {target_mccm} in '{circuit}', set Duty Cycle (d) to: {required_d:.4f}")
        print("--------------------------------------------------\n")
        # Optional: Print the first few rows to see how beautifully structured it is
        # print(master_df.head())

        # 3. Define what you want to plot
        parameters_to_plot = ["eta", "m_ccm", "iin_ripple", "isw_rms", "ico_rms", "pout"]

        units = {
            "eta": "-",
            "m_ccm": "-",
            "iin_ripple": "A",
            "isw_rms": "A",
            "ico_rms": "A",
            "pout": "W"
        }

        labels = {
            "d": "Duty Cycle (D)",  # Map the dynamic 'd' variable to a nice string
            "eta": "Efficiency",
            "m_ccm": "Voltage Gain $M_{ccm}$",
            "iin_ripple": "Input Current Ripple",
            "isw_rms": "Switch RMS Current",
            "ico_rms": "Output Capacitor RMS Current",
            "pout": "Output Power"
        }

        # 4. Generate the plots
        plot_stepped_comparisons(
            master_df=master_df,
            parameters=parameters_to_plot,
            units=units,
            labels=labels,
            output_dir="ieee_plots"
        )
        plt.show()
