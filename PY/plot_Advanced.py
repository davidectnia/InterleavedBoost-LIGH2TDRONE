import argparse
import pandas as pd
import re
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import plotly.express as px
import sys


def main():
    # 1. Command-Line Interface Setup
    parser = argparse.ArgumentParser(description="Dynamically parse and plot LTspice .log files interactively.")
    parser.add_argument("log_file", help="Path to the LTspice .log file (e.g., boost_stack.log)")
    parser.add_argument("-p", "--params", nargs='+', required=True,
                        help="List of parameters to plot (e.g., -p eta m_ccm v_out)")

    args = parser.parse_args()

    # 2. Read the log file
    try:
        with open(args.log_file, "r") as file:
            data = file.read()
    except FileNotFoundError:
        print(f"Error: The file '{args.log_file}' was not found.")
        sys.exit(1)

    # 3. Dynamically parse step definitions
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
        print("Error: No .step directives found in the log file.")
        sys.exit(1)

    step_df = pd.DataFrame(step_data)

    # 4. Auto-detect X-axis and Grouping variables
    param_cols = [c for c in step_df.columns if c != 'step']
    if len(param_cols) > 1:
        changes = {col: (step_df[col] != step_df[col].shift()).sum() for col in param_cols}
        x_col = max(changes, key=changes.get)
        group_cols = [col for col in param_cols if col != x_col]
    else:
        x_col = param_cols[0]
        group_cols = []

    # 5. Dynamically parse ALL measurements
    measurement_pattern = re.compile(r'Measurement:\s+(\w+)\n.*?\n(.*?)(?:\n\n|\Z)', re.DOTALL)
    measurements = measurement_pattern.findall(data)

    parsed_data = {step: {} for step in step_df['step']}

    for meas_name, meas_data in measurements:
        meas_name = meas_name.lower()
        for line in meas_data.strip().split('\n'):
            parts = line.split()
            if len(parts) >= 2 and parts[0].isdigit():
                step_idx = int(parts[0])
                if step_idx in parsed_data:
                    try:
                        parsed_data[step_idx][meas_name] = abs(float(parts[1]))
                    except ValueError:
                        pass

    # 6. Build the final DataFrame
    records = []
    for step_idx in step_df['step']:
        record = step_df[step_df['step'] == step_idx].iloc[0].to_dict()
        record.update(parsed_data.get(step_idx, {}))
        records.append(record)

    df = pd.DataFrame(records)
    df = df.drop_duplicates(subset=param_cols, keep='first')

    # 7. Interactive Plotting with Plotly
    valid_params = [p.lower() for p in args.params if p.lower() in df.columns]

    if not valid_params:
        print(f"Error: None of the requested parameters {args.params} were found in the log file.")
        print(f"Available parameters are: {list(df.columns)}")
        sys.exit(1)

    # Handle grouping (for legend and colors)
    if group_cols:
        df['group_label'] = df[group_cols].apply(lambda row: ', '.join([f"{c}={row[c]}" for c in group_cols]), axis=1)
        unique_groups = df['group_label'].unique()
    else:
        df['group_label'] = "Default"
        unique_groups = ["Default"]

    # Create Subplots
    fig = make_subplots(rows=len(valid_params), cols=1, shared_xaxes=True,
                        subplot_titles=[f"|{p}| vs {x_col.upper()}" for p in valid_params],
                        vertical_spacing=0.05)

    # Assign consistent colors to each group across subplots
    colors = px.colors.qualitative.Plotly
    color_map = {grp: colors[i % len(colors)] for i, grp in enumerate(unique_groups)}

    for i, param in enumerate(valid_params):
        for grp in unique_groups:
            subset = df[df['group_label'] == grp].sort_values(x_col)

            y_values = subset[param] * 100 if param == 'eta' else subset[param]
            unit = '[%]' if param == 'eta' else ''

            label = grp if grp != "Default" else "Data"
            # Only show legend on the first subplot to prevent duplicate legend entries
            show_legend = True if i == 0 else False

            fig.add_trace(go.Scatter(
                x=subset[x_col],
                y=y_values,
                mode='lines+markers',
                name=label,
                legendgroup=label,  # Links traces so clicking one hides it across all subplots
                showlegend=show_legend,
                line=dict(color=color_map[grp])
            ), row=i + 1, col=1)

        fig.update_yaxes(title_text=f"|{param}| {unit}", row=i + 1, col=1)

    fig.update_xaxes(title_text=f"Sweep Variable ({x_col.upper()})", row=len(valid_params), col=1)

    fig.update_layout(
        title=f"LTspice Data: {args.log_file}",
        height=350 * len(valid_params),
        template="plotly_white",
        hovermode="x unified",
        legend_title_text="Step Parameters" if group_cols else ""
    )

    fig.show()


if __name__ == "__main__":
    main()