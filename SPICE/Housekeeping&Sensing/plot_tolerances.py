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
    "legend.fontsize": 5,
    "figure.dpi": 300,
    "lines.linewidth": 0.8,
    "axes.grid": True,
    "grid.alpha": 0.4,
    "grid.linestyle": "--"
})

# 2. File and Output Paths
raw_file = "voltage_sense_output.raw"
output_plot_path = "output_tolerance.png"

if not os.path.exists(raw_file):
    raise FileNotFoundError(f"File '{raw_file}' was not found in the current directory.")

# 3. Parse LTspice Data
l = ltspice.Ltspice(raw_file)
l.parse()

n_cases = l.case_count if hasattr(l, "case_count") and l.case_count > 0 else 1

x_var_name = "V(V_lipo)"
node_out = "V(n001)"

# 4. Create Single-Panel IEEE Figure (Column Width = 3.5 in)
fig, ax = plt.subplots(figsize=(3.5, 2.2))
colors = plt.cm.plasma(np.linspace(0.1, 0.85, max(n_cases, 1)))

last_x = None

for case_idx in range(n_cases):
    try:
        x_data = l.get_data(x_var_name, case=case_idx)
        v_out_data = l.get_data(node_out, case=case_idx)
    except TypeError:
        x_data = l.get_data(x_var_name)[case_idx]
        v_out_data = l.get_data(node_out)[case_idx]

    last_x = x_data

    ax.plot(
        x_data,
        v_out_data,
        color=colors[case_idx] if n_cases > 1 else "#d62728",
        alpha=0.85,
        label=f"Run {case_idx + 1}" if n_cases <= 5 else None
    )

# 5. IEEE Labels and Axis Limits
ax.set_ylabel(r"$V_{\text{sense}}$ [V]")
ax.set_xlabel(r"$V_{\text{LiPo}}$ [V]")
ax.set_title("Tolerance Analysis")
if last_x is not None:
    ax.set_xlim([last_x[0], last_x[-1]])

if n_cases <= 5 and n_cases > 1:
    ax.legend(loc="upper left", framealpha=0.85)

# 6. Margins and Export
plt.tight_layout()
plt.savefig(output_plot_path, dpi=300, format="png")
plt.show()