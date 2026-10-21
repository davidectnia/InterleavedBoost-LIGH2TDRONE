import matplotlib.pyplot as plt
import numpy as np
import ltspice
import os

# 1. IEEE Standard Formatting Configuration
plt.rcParams.update({
    "font.family": "serif",
    "font.serif": ["Times New Roman"],
    "font.size": 10,
    "axes.labelsize": 10,
    "axes.titlesize": 10,
    "xtick.labelsize": 8,
    "ytick.labelsize": 8,
    "legend.fontsize": 8,
    "figure.figsize": (3.5, 2.5),  # IEEE single-column width
    "figure.dpi": 300,
    "lines.linewidth": 1.0,
    "axes.grid": True,
    "grid.alpha": 0.5,
    "grid.linestyle": "--"
})

# 2. File Paths and Parameters
# Put your actual filenames here since they are in the same folder
raw_files = ["boostideal.raw", "boostideal_inter2.raw", "boostideal_inter4.raw"]
plot_labels = ["Conventional Boost Converter", "2-Phases IBC", "4-Phases IBC"]
output_plot_path = "vout_optimized.png"

# Styling for differentiation (black, blue, and red with different dash patterns)
colors = ['green', 'blue', 'red']
line_styles = ['-', '--', '-.']

# Define the switching frequency to calculate the 4 periods
f_sw = 400e3  # Example: 100 kHz. Update to your converter's frequency!
period = 1 / f_sw
time_window = 4 * period

fig, ax = plt.subplots()
t_start_global = None
t_end_global = None

# 3. Parse and Plot each file
for i, raw_file in enumerate(raw_files):
    if not os.path.exists(raw_file):
        print(f"Warning: File '{raw_file}' not found in the current directory.")
        continue
        
    # Load and parse the raw file
    l = ltspice.Ltspice(raw_file)
    l.parse()

    # Extract time and the I(V1) current
    time = l.get_time()
    #i_v1 = -l.get_data('I(V2)')
    i_v1 = l.get_data('V(out)')
    # Calculate the time window for the last 4 periods of THIS specific file
    t_end = time[-1]
    t_start = t_end - time_window

    # Keep track of the limits for the first file to set the x-axis properly
    if i == 0:
        t_start_global = t_start * 1e6
        t_end_global = t_end * 1e6

    # Mask to filter data
    mask = (time >= t_start) & (time <= t_end)
    t_plot_us = time[mask] * 1e6
    i_v1_plot = i_v1[mask]

    # Plot the masked data
    ax.plot(t_plot_us, i_v1_plot, label=plot_labels[i], color=colors[i], linestyle=line_styles[i])

# 4. Labeling and Limits
ax.set_xlabel('Time ($\\mu$s)')
ax.set_ylabel('Output Voltage (V)')

# Align the x-axis limits to the first file's last 4 periods
if t_start_global is not None and t_end_global is not None:
    ax.set_xlim([t_start_global, t_end_global])

# Legend positioning
ax.legend(loc='best', frameon=True, edgecolor='black')

plt.tight_layout()

# Save and show
plt.savefig(output_plot_path, bbox_inches='tight')
plt.show()