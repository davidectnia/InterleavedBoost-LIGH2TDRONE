import pandas as pd
import matplotlib.pyplot as plt
import re

# 1. Read the log file
file_path = "Boost Stack/boost_stack_h100.log"
try:
    with open(file_path, "r") as file:
        data = file.read()
except FileNotFoundError:
    print(f"Error: Could not find {file_path}")
    exit()

# 2. Parse step definitions (Duty Cycle 'd' and Stack Voltage 'v_stack')
step_pattern = re.compile(r'\.step\s+d=([\d\.]+)\s+v_stack=([\d\.]+)', re.IGNORECASE)
steps = step_pattern.findall(data)
step_map = {i+1: {'d': float(d), 'v_stack': float(v_stack)} for i, (d, v_stack) in enumerate(steps)}

# 3. Parse eta measurements
eta_pattern = re.compile(r'Measurement:\s+eta\n.*?\n(.*?)(?:\n\n|\Z)', re.DOTALL | re.IGNORECASE)
eta_match = eta_pattern.search(data)

eta_data = {}
if eta_match:
    for line in eta_match.group(1).strip().split('\n'):
        parts = line.split()
        if len(parts) >= 2 and parts[0].isdigit():
            eta_data[int(parts[0])] = abs(float(parts[1]))

# 4. Build DataFrame
records = []
for step_idx, params in step_map.items():
    if step_idx in eta_data:
        records.append({
            'step': step_idx,
            'd': params['d'],
            'v_stack': params['v_stack'],
            'eta': eta_data[step_idx]
        })

df = pd.DataFrame(records)
df = df.drop_duplicates(subset=['d', 'v_stack'], keep='first')

# 5. Extract Maximum Efficiency for each v_stack
# Group the data by v_stack and find the index of the maximum eta in each group
max_eta_idx = df.groupby('v_stack')['eta'].idxmax()
# Extract those specific rows
max_eta_df = df.loc[max_eta_idx].sort_values('v_stack')

print("--- Optimal Points Extracted ---")
print(max_eta_df[['v_stack', 'd', 'eta']].to_string(index=False))

# 6. Plot Maximum Efficiency vs. Corresponding Duty Cycle
plt.figure(figsize=(8, 5))

# Plot the optimal points
plt.plot(max_eta_df['d'], max_eta_df['eta'] * 100, marker='o', linestyle='-', color='#1f77b4', markersize=8)

# Annotate each point with its v_stack value so you know which voltage it belongs to
for i, row in max_eta_df.iterrows():
    plt.annotate(f"{row['v_stack']} V",
                 (row['d'], row['eta'] * 100),
                 textcoords="offset points",
                 xytext=(0,10),
                 ha='center',
                 fontsize=10,
                 fontweight='bold')

plt.title('Maximum Absolute Efficiency (|η|) vs Optimal Duty Cycle')
plt.xlabel('Optimal Duty Cycle (D)')
plt.ylabel('Maximum Absolute Efficiency (|η|) [%]')
plt.grid(True, linestyle=':', alpha=0.7)

# Adjust y-axis slightly to make room for annotations
ymin, ymax = plt.ylim()
plt.ylim(ymin, ymax + (ymax-ymin)*0.1)

plt.tight_layout()
plt.show()