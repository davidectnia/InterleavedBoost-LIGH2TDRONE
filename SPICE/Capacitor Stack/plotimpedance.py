import os
import matplotlib.pyplot as plt
import numpy as np
import ltspice

# 1. IEEE Standard Formatting Configuration
plt.rcParams.update({
    "font.family": "serif",
    "font.serif": ["Times New Roman"],
    "font.size": 7,
    "axes.labelsize": 7,
    "axes.titlesize": 8,
    "xtick.labelsize": 6,
    "ytick.labelsize": 6,
    "legend.fontsize": 6,
    "figure.dpi": 300,
    "lines.linewidth": 0.9,
    "axes.grid": True,
    "grid.alpha": 0.4,
    "grid.linestyle": "--"
})

# 2. File Path and Parameters
raw_file = "capacitor_stack_input50V.raw"
output_plot_path = "inputstack.png"

if not os.path.exists(raw_file):
    raise FileNotFoundError(f"File '{raw_file}' was not found in the current directory.")

# 3. Parse LTspice Data
l = ltspice.Ltspice(raw_file)
l.parse()

frequency = l.get_frequency()

# Extract node voltage and branch current
# Adjust node/device names if observing a different branch
v_trace = l.get_data("V(n002)")
i_trace = l.get_data("I(I7)")

# 4. Calculate Impedance
z_complex = v_trace / i_trace
z_mag_db = 20 * np.log10(np.abs(z_complex))
z_phase_deg = np.angle(z_complex, deg=True)

# 5. Create IEEE Single-Column Dual-Axis Plot (Width = 3.5 in)
fig, ax1 = plt.subplots(figsize=(3.5, 2.3))

# Left Y-Axis: Impedance Magnitude
line1 = ax1.semilogx(
    frequency,
    z_mag_db,
    color="#1f77b4",
    linestyle="-",
    label=r"$|Z|$"
)
ax1.set_xlabel("Frequency (Hz)")
ax1.set_ylabel(r"Impedance Magnitude, $|Z|\ (\text{dB}\Omega)$", color="#1f77b4")
ax1.tick_params(axis="y", labelcolor="#1f77b4")
ax1.set_xlim([frequency[0], frequency[-1]])

# Right Y-Axis: Impedance Phase (Dashed Black Line)
ax2 = ax1.twinx()
line2 = ax2.semilogx(
    frequency,
    z_phase_deg,
    color="black",
    linestyle="--",
    label=r"$\angle Z$"
)
ax2.set_ylabel(r"Phase Angle, $\angle Z\ (^\circ)$", color="black")
ax2.tick_params(axis="y", labelcolor="black")
# Disable secondary grid to prevent overlapping dashed grid lines
ax2.grid(False)

# Combined Legend
lines = line1 + line2
labels = [l.get_label() for l in lines]
ax1.legend(lines, labels, loc="upper right", framealpha=0.85)

# 6. Fine-Tune Margins & Save
plt.tight_layout()
plt.savefig(output_plot_path, dpi=300, format="png")

plt.show()