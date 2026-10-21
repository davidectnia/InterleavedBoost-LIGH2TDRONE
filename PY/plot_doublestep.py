import pandas as pd
import matplotlib.pyplot as plt
import re

# 1. Read the log file
file_path = "Boost Stack/boost_stack.log"
with open(file_path, "r") as file:
    data = file.read()

# 2. Parse step definitions (Duty Cycle 'd' and Stack Voltage 'v_stack')
step_pattern = re.compile(r'\.step\s+d=([\d\.]+)\s+v_stack=([\d\.]+)', re.IGNORECASE)
steps = step_pattern.findall(data)
step_map = {i + 1: {'d': float(d), 'v_stack': float(v_stack)} for i, (d, v_stack) in enumerate(steps)}

# 3. Dynamically parse ALL measurements in the log file
measurement_pattern = re.compile(r'Measurement:\s+(\w+)\n.*?\n(.*?)(?:\n\n|\Z)', re.DOTALL)
measurements = measurement_pattern.findall(data)

# Dictionary to hold all parsed data mapped by step number
parsed_data = {step: {} for step in step_map.keys()}

for meas_name, meas_data in measurements:
    meas_name = meas_name.lower()  # Normalize column names
    for line in meas_data.strip().split('\n'):
        parts = line.split()
        # Verify the line starts with a step number
        if len(parts) >= 2 and parts[0].isdigit():
            step_idx = int(parts[0])
            if step_idx in parsed_data:
                try:
                    # Always take the absolute value of the parsed measurement
                    parsed_data[step_idx][meas_name] = abs(float(parts[1]))
                except ValueError:
                    pass

# 4. Build a pandas DataFrame to structure the data
records = []
for step_idx, params in step_map.items():
    record = {'step': step_idx, 'd': params['d'], 'v_stack': params['v_stack']}
    record.update(parsed_data.get(step_idx, {}))
    records.append(record)

df = pd.DataFrame(records)

# Drop the duplicated evaluation steps at the end of each sweep
df = df.drop_duplicates(subset=['d', 'v_stack'], keep='first')

# ---------------------------------------------------------
# 5. Generic Plotting Configuration
# Define the list of parameters you want to plot here:
params_to_plot = ['eta', 'iin_ripple', 'pout', 'pin']
# ---------------------------------------------------------

# Filter out any parameters that were not found in the log
params_to_plot = [p for p in params_to_plot if p in df.columns]

if not params_to_plot:
    print("None of the specified parameters were found in the log file.")
else:
    # Dynamically create subplots based on the number of requested parameters
    num_plots = len(params_to_plot)
    fig, axes = plt.subplots(num_plots, 1, figsize=(10, 4 * num_plots), sharex=True)

    # Ensure axes is iterable even if there's only 1 parameter to plot
    if num_plots == 1:
        axes = [axes]

    # Create dynamic color map based on unique v_stack values
    voltages = df['v_stack'].unique()
    cmap = plt.get_cmap('tab10')
    color_map = {v: cmap(i % cmap.N) for i, v in enumerate(voltages)}

    # Plot each parameter in its respective subplot
    for ax, param in zip(axes, params_to_plot):
        for v in voltages:
            subset = df[df['v_stack'] == v].sort_values('d')

            # Optional: Scale efficiency to percentage for readability
            y_values = subset[param] * 100 if param == 'eta' else subset[param]
            unit_label = '[%]' if param == 'eta' else ''

            ax.plot(subset['d'], y_values, marker='o', linestyle='-',
                    color=color_map[v], label=f'v_stack = {v} V')

        ax.set_ylabel(f'Absolute |{param}| {unit_label}')
        ax.set_title(f'|{param}| vs. Duty Cycle (D)')
        ax.grid(True, linestyle=':', alpha=0.7)
        # Place legend outside the plot box to avoid overlapping with data lines
        ax.legend(loc='center left', bbox_to_anchor=(1.02, 0.5), title="Stack Voltage")

    axes[-1].set_xlabel('Duty Cycle (D)')
    plt.tight_layout()
    plt.show()