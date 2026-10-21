import pandas as pd
import matplotlib.pyplot as plt
import re
import os

# ---------------------------------------------------------
# Configuration
# Add the names of the log files you want to compare
log_files = ["Boost Stack/boost_stack.log", "Boost Stack - GaN/boost_stackGaN.log"]


# ---------------------------------------------------------

def get_peak_efficiencies(file_path):
    try:
        with open(file_path, "r") as file:
            data = file.read()
    except FileNotFoundError:
        print(f"Error: Could not find '{file_path}'")
        return None, None

    # 1. Dynamically parse step definitions
    step_lines = re.findall(r'^\.step\s+(.*)$', data, re.MULTILINE | re.IGNORECASE)
    step_data = []
    for i, line in enumerate(step_lines):
        pairs = re.findall(r'([a-zA-Z0-9_]+)=([0-9\.\-\+eE]+)', line)
        if pairs:
            record = {'step': i + 1}
            for key, val in pairs:
                record[key.lower()] = float(val)
            step_data.append(record)

    if not step_data:
        return None, None

    step_df = pd.DataFrame(step_data)

    # 2. Auto-detect Sweep Variables
    param_cols = [c for c in step_df.columns if c != 'step']
    if len(param_cols) > 1:
        # The inner loop changes most often (usually 'd')
        changes = {col: (step_df[col] != step_df[col].shift()).sum() for col in param_cols}
        x_col = max(changes, key=changes.get)
        group_cols = [col for col in param_cols if col != x_col]
    else:
        x_col = param_cols[0]
        group_cols = []

    # 3. Parse eta measurements
    eta_pattern = re.compile(r'Measurement:\s+eta\n.*?\n(.*?)(?:\n\n|\Z)', re.DOTALL | re.IGNORECASE)
    eta_match = eta_pattern.search(data)

    eta_data = {}
    if eta_match:
        for line in eta_match.group(1).strip().split('\n'):
            parts = line.split()
            if len(parts) >= 2 and parts[0].isdigit():
                eta_data[int(parts[0])] = abs(float(parts[1]))

    # 4. Build DataFrame and clean duplicates
    records = []
    for step_idx in step_df['step']:
        if step_idx in eta_data:
            record = step_df[step_df['step'] == step_idx].iloc[0].to_dict()
            record['eta'] = eta_data[step_idx]
            records.append(record)

    df = pd.DataFrame(records)
    df = df.drop_duplicates(subset=param_cols, keep='first')

    # 5. Extract Maximum Efficiency
    if group_cols:
        # Group by the outer loop variable(s) and find the index of the max eta
        max_eta_idx = df.groupby(group_cols)['eta'].idxmax()
        max_eta_df = df.loc[max_eta_idx].sort_values(group_cols)
    else:
        # If there's only one sweep variable, just find the absolute max
        max_eta_idx = df['eta'].idxmax()
        max_eta_df = df.loc[[max_eta_idx]]

    return max_eta_df, x_col


# ---------------------------------------------------------
# Main Execution and Plotting
# ---------------------------------------------------------

plt.figure(figsize=(10, 6))
cmap = plt.get_cmap('tab10')
markers = ['o', 's', '^', 'D', 'v', 'p', '*']

print("--- Optimal Points Extracted ---")

for i, file_path in enumerate(log_files):
    peak_df, x_col = get_peak_efficiencies(file_path)

    if peak_df is not None and not peak_df.empty:
        file_name = os.path.basename(file_path)
        print(f"\n[{file_name}]")
        print(peak_df.to_string(index=False))

        # Plot the data
        plt.plot(peak_df[x_col], peak_df['eta'] * 100,
                 marker=markers[i % len(markers)],
                 linestyle='-',
                 color=cmap(i % cmap.N),
                 markersize=8,
                 linewidth=2,
                 label=file_name)

        # Add labels to the points (Optional: labels the outer sweep variable)
        group_cols = [c for c in peak_df.columns if c not in [x_col, 'eta', 'step']]
        for _, row in peak_df.iterrows():
            if group_cols:
                # E.g., formats to "38.4 V" or "20.0 Ω"
                label_text = ", ".join([f"{row[col]}" for col in group_cols])
            else:
                label_text = f"{row['eta'] * 100:.1f}%"

            plt.annotate(label_text,
                         (row[x_col], row['eta'] * 100),
                         textcoords="offset points",
                         xytext=(0, 10),
                         ha='center',
                         fontsize=9,
                         color=cmap(i % cmap.N))

plt.title('Comparison of Peak Absolute Efficiency (|η|) vs Optimal Duty Cycle')
plt.xlabel('Optimal Duty Cycle (D)')
plt.ylabel('Peak Absolute Efficiency (|η|) [%]')
plt.grid(True, linestyle=':', alpha=0.7)
plt.legend(title="Log Files", loc='best')

# Dynamically adjust y-axis limits to accommodate the text annotations
ymin, ymax = plt.ylim()
plt.ylim(ymin, ymax + (ymax - ymin) * 0.15)

plt.tight_layout()
plt.show()