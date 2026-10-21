"""
ltspice_plotter.py
==================
Generic parser and plotter for LTspice .log files with .step sweeps.
Supports multi-file overlays and IEEE-style publication figures.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
QUICK START — single file
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    from ltspice_plotter import parse_log, parse_logs, filter_steps, plot, info

    df = parse_log("/path/to/boost_stack_h100.log")
    info(df)                          # print available params & measurements

    # Full sweep visible — all v_stack traces
    plot(df, x='d', y_left=['eta'],
         group_by='v_stack', abs_value=['eta'])

    # Dual-axis: pout (left) and eta (right)
    plot(df, x='d', y_left=['pout'], y_right=['eta'],
         group_by='v_stack', layout='subplots', abs_value=['eta'])

    # Keep only max-eta row per v_stack group (optional)
    best = filter_steps(df, max_only='eta', group_by='v_stack')
    plot(best, x='d', y_left=['pout'], y_right=['eta'],
         group_by='v_stack', abs_value=['eta'])

    # Filter to specific parameter values
    sub = filter_steps(df, v_stack=[38.4, 50.4], d=[0.65, 0.7, 0.75])
    plot(sub, x='d', y_left=['il_rms', 'isw_rms'], group_by='v_stack')

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
QUICK START — multiple files (same sweep structure)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    df = parse_logs({
        "H100": "/data/boost_stack_h100.log",
        "H200": "/data/boost_stack_h200.log",
    })
    # Color = file (source),  line style = v_stack group
    plot(df, x='d', y_left=['eta'],
         group_by='v_stack', source_col='source',
         abs_value=['eta'], layout='single')

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
IEEE FIGURE SIZE PRESETS  (pass to figsize=)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    IEEE_SINGLE = (3.5, 2.625)   # single-column         [default]
    IEEE_DOUBLE = (7.16, 2.625)  # double-column, half-height
    IEEE_SQUARE = (3.5, 3.5)     # single-column, square
    IEEE_TALL   = (3.5, 4.5)     # single-column, tall

    plot(df, x='d', y_left=['eta'], figsize=IEEE_DOUBLE)
"""

import re
import os
import matplotlib as mpl
import matplotlib.pyplot as plt
import pandas as pd

# ──────────────────────────────────────────────────────────────────────────────
# IEEE figure size presets (inches)
# ──────────────────────────────────────────────────────────────────────────────
IEEE_SINGLE = (3.5,  2.625)
IEEE_DOUBLE = (7.16, 2.625)
IEEE_SQUARE = (3.5,  3.5)
IEEE_TALL   = (3.5,  4.5)

# ──────────────────────────────────────────────────────────────────────────────
# IEEE matplotlib style parameters
# ──────────────────────────────────────────────────────────────────────────────
_IEEE_RC = {
    'font.family':                   'serif',
    'font.serif':                    ['Times New Roman', 'DejaVu Serif', 'serif'],
    'font.size':                     8,
    'axes.titlesize':                8,
    'axes.labelsize':                8,
    'xtick.labelsize':               7,
    'ytick.labelsize':               7,
    'legend.fontsize':               7,
    'legend.title_fontsize':         7,
    'figure.dpi':                    150,
    'savefig.dpi':                   300,
    'savefig.bbox':                  'tight',
    'axes.linewidth':                0.6,
    'axes.grid':                     True,
    'grid.linestyle':                ':',
    'grid.linewidth':                0.4,
    'grid.alpha':                    0.7,
    'lines.linewidth':               1.0,
    'lines.markersize':              4,
    'xtick.direction':               'in',
    'ytick.direction':               'in',
    'xtick.major.width':             0.6,
    'ytick.major.width':             0.6,
    'xtick.minor.visible':           True,
    'ytick.minor.visible':           True,
    'xtick.minor.width':             0.4,
    'ytick.minor.width':             0.4,
    'legend.framealpha':             0.9,
    'legend.edgecolor':              '0.75',
    'legend.borderpad':              0.4,
    'legend.labelspacing':           0.3,
    'figure.constrained_layout.use': True,
}

# Colorblind-safe palette (Wong 2011)
_FILE_COLORS = [
    '#0072B2',  # blue
    '#D55E00',  # vermillion
    '#009E73',  # green
    '#CC79A7',  # pink/purple
    '#56B4E9',  # sky blue
    '#E69F00',  # orange
    '#F0E442',  # yellow
    '#000000',  # black
]

# Line styles — one per group value (e.g. different v_stack)
_GROUP_LINESTYLES = ['-', '--', '-.', ':', (0, (3, 1, 1, 1))]

# Markers — one per y variable
_MARKERS = ['o', 's', '^', 'D', 'v', 'P', '*', 'X']


# ══════════════════════════════════════════════════════════════════════════════
# 1.  PARSING
# ══════════════════════════════════════════════════════════════════════════════

def parse_log(file_path: str, label: str = None) -> pd.DataFrame:
    """
    Parse a single LTspice .log file and return a tidy DataFrame.

    Parameters
    ----------
    file_path : str  – absolute or relative path to the .log file
    label     : str  – label stored in the 'source' column.
                       Defaults to the filename without extension.

    Columns returned
    ----------------
    source        : str   – file label (useful when merging files)
    step          : int   – simulation step index (1-based)
    <sweep_param> : float – one column per .step parameter (e.g. d, v_stack)
    <measurement> : float – one column per Measurement block
    """
    file_path = os.path.abspath(file_path)
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f"Log file not found: {file_path}")

    source_label = label or os.path.splitext(os.path.basename(file_path))[0]

    with open(file_path, 'r') as fh:
        raw = fh.read()

    # ── Step definitions ────────────────────────────────────────────────────
    step_param_re = re.compile(
        r'\.step\s+((?:\w+=[\d\.eE+\-]+\s*)+)', re.IGNORECASE
    )
    step_rows = []
    for match in step_param_re.finditer(raw):
        pairs = re.findall(r'(\w+)=([\d\.eE+\-]+)', match.group(1))
        step_rows.append({k: float(v) for k, v in pairs})

    if not step_rows:
        raise ValueError(f"No .step definitions found in: {file_path}")

    n_steps = len(step_rows)
    df = pd.DataFrame(step_rows)
    df.insert(0, 'source', source_label)
    df.insert(1, 'step',   range(1, n_steps + 1))

    sweep_params = [c for c in df.columns if c not in ('step', 'source')]

    # ── Measurement blocks ──────────────────────────────────────────────────
    # Handles both:
    #   "step  value  FROM  TO"   (4-col, time-windowed)
    #   "step  value"             (2-col derived: ripple, ratio, etc.)
    meas_block_re = re.compile(
        r'Measurement:\s+(\S+)\s*\n'
        r'(?:.*?\n)'
        r'((?:\s+\d+\s+.*\n?)+)',
        re.IGNORECASE
    )
    for m in meas_block_re.finditer(raw):
        name  = m.group(1).lower()
        lines = m.group(2).strip().splitlines()
        values = {}
        for line in lines:
            parts = line.split()
            if len(parts) >= 2 and parts[0].isdigit():
                try:
                    values[int(parts[0])] = float(parts[1])
                except ValueError:
                    pass
        if values and name not in sweep_params:
            df[name] = [values.get(i) for i in df['step']]

    # Cast measurement columns to float
    meta      = ['source', 'step'] + sweep_params
    meas_cols = [c for c in df.columns if c not in meta]
    df[meas_cols] = df[meas_cols].astype(float)

    print(f"  '{source_label}' — {n_steps} steps | "
          f"sweep: {sweep_params} | "
          f"{len(meas_cols)} measurements: {meas_cols}")
    return df


def parse_logs(files: dict) -> pd.DataFrame:
    """
    Parse multiple .log files with the same sweep structure and stack them.

    Parameters
    ----------
    files : dict  –  {label: file_path}
            Example: {"H100": "/data/boost_h100.log",
                      "H200": "/data/boost_h200.log"}

    Returns
    -------
    Single concatenated DataFrame with a 'source' column containing the label.

    Example
    -------
    df = parse_logs({
        "H100": "/path/to/boost_stack_h100.log",
        "H200": "/path/to/boost_stack_h200.log",
    })
    plot(df, x='d', y_left=['eta'],
         group_by='v_stack', source_col='source', abs_value=['eta'])
    """
    print(f"parse_logs: loading {len(files)} file(s)...")
    frames = [parse_log(path, label=lbl) for lbl, path in files.items()]
    combined = pd.concat(frames, ignore_index=True)
    print(f"  Total: {len(combined)} rows from {len(frames)} files")
    return combined


# ══════════════════════════════════════════════════════════════════════════════
# 2.  FILTERING
# ══════════════════════════════════════════════════════════════════════════════

def filter_steps(
    df: pd.DataFrame,
    max_only: str = None,
    group_by: str = None,
    **param_filters,
) -> pd.DataFrame:
    """
    Subset the DataFrame by sweep-parameter values and/or keep only the
    row with the maximum (absolute) value of a measurement per group.

    Parameters
    ----------
    df             : DataFrame from parse_log() / parse_logs()
    max_only       : measurement name → keep only the max-|value| row per
                     group (defined by group_by). If group_by is None,
                     keeps the single global maximum row.
    group_by       : column to group by when using max_only (e.g. 'v_stack')
    **param_filters : sweep parameters to filter, e.g.
                      v_stack=[38.4, 50.4]  or  d=0.7
                      Scalar or list of accepted values.
                      Use source=[...] to restrict to specific file labels.

    Examples
    --------
    filter_steps(df, v_stack=[38.4, 50.4])
    filter_steps(df, d=0.7, v_stack=44.4)
    filter_steps(df, max_only='eta', group_by='v_stack')
    filter_steps(df, v_stack=[38.4], max_only='eta', group_by='v_stack')
    filter_steps(df, source=['H100'], v_stack=[38.4, 44.4])
    """
    out = df.copy()

    for param, values in param_filters.items():
        if param not in out.columns:
            raise ValueError(f"Column '{param}' not found. "
                             f"Available: {out.columns.tolist()}")
        if not isinstance(values, (list, tuple)):
            values = [values]
        out = out[out[param].isin(values)]

    if out.empty:
        raise ValueError("filter_steps: no rows remain after filtering.")

    if max_only is not None:
        if max_only not in out.columns:
            raise ValueError(f"max_only='{max_only}' not found in columns.")
        col = out[max_only].abs()
        if group_by:
            idx = col.groupby(out[group_by]).idxmax()
        else:
            idx = [col.idxmax()]
        out = out.loc[idx].sort_values(group_by or out.columns[0])

    print(f"filter_steps: {len(out)} rows remaining")
    return out.reset_index(drop=True)


# ══════════════════════════════════════════════════════════════════════════════
# 3.  PLOTTING
# ══════════════════════════════════════════════════════════════════════════════

def plot(
    df: pd.DataFrame,
    x: str,
    y_left: list,
    y_right: list = None,
    group_by: str = None,
    source_col: str = None,
    layout: str = 'single',
    abs_value: list = None,
    title: str = None,
    figsize: tuple = IEEE_SINGLE,
    savefig: str = None,
):
    """
    IEEE-style plotter for LTspice log DataFrames.

    Visual encoding
    ---------------
    Color      → file / source  (when source_col is set)
                 or y variable  (single-file mode)
    Line style → group value    (e.g. v_stack=38.4 → solid,
                                      v_stack=44.4 → dashed, …)
    Marker     → y variable index (○ for first y, □ for second, …)

    Parameters
    ----------
    df         : DataFrame from parse_log() or parse_logs()
    x          : column for the X axis (e.g. 'd')
    y_left     : list of measurements for the primary (left) Y axis
    y_right    : list of measurements for the secondary (right) Y axis.
                 Triggers a twin-axis. Default None.
    group_by   : column whose distinct values produce separate traces
                 with different line styles (e.g. 'v_stack').
    source_col : column that identifies the file label (e.g. 'source').
                 Each unique value gets a distinct color.
                 If None, colors cycle over y-variables instead.
    layout     : 'single'           – all traces on one axes pair
                 'subplots'         – one subplot row per group value
                 'separate_windows' – one figure per group value
    abs_value  : list of measurement names to plot as |x|
    title      : figure title (auto-generated if None)
    figsize    : (width, height) in inches.
                 Use IEEE_SINGLE / IEEE_DOUBLE / IEEE_SQUARE presets.
    savefig    : path to save the figure (e.g. 'fig1.pdf').
                 Supports .pdf .svg .png.  None = don't save.
                 For 'separate_windows', the group value is appended
                 to the filename automatically.

    Layout details
    --------------
    'single'
        All group × source combinations share one axes (+ optional twin).

    'subplots'
        One subplot row per group value, sharing the X axis.
        Each subplot gets its own optional twin axis.

    'separate_windows'
        One independent figure per group value.

    Examples
    --------
    # Full sweep — all v_stack traces on one IEEE single-column figure
    plot(df, x='d', y_left=['eta'],
         group_by='v_stack', abs_value=['eta'])

    # Dual-axis pout/eta, one subplot per v_stack, double-column width
    plot(df, x='d', y_left=['pout'], y_right=['eta'],
         group_by='v_stack', layout='subplots',
         abs_value=['eta'], figsize=IEEE_DOUBLE)

    # Multi-file: color = file, line style = v_stack
    plot(df_multi, x='d', y_left=['eta'],
         group_by='v_stack', source_col='source',
         abs_value=['eta'], layout='single')

    # Save as PDF
    plot(df, x='d', y_left=['il_rms', 'isw_rms'],
         group_by='v_stack', savefig='current_stress.pdf')
    """
    y_right   = y_right   or []
    abs_value = abs_value or []

    # ── Validate columns ────────────────────────────────────────────────────
    needed = [x] + y_left + y_right
    if group_by:   needed.append(group_by)
    if source_col: needed.append(source_col)
    for col in needed:
        if col not in df.columns:
            raise ValueError(f"Column '{col}' not found.\n"
                             f"Available: {df.columns.tolist()}")

    # ── Build group & source lists ──────────────────────────────────────────
    groups  = sorted(df[group_by].unique())   if group_by   else [None]
    sources = sorted(df[source_col].unique()) if source_col else [None]

    # Color: one per source (multi-file) or per y-variable (single-file)
    if source_col:
        color_map = {s: _FILE_COLORS[i % len(_FILE_COLORS)]
                     for i, s in enumerate(sources)}
    else:
        all_y_vars = y_left + y_right
        color_map  = {v: _FILE_COLORS[i % len(_FILE_COLORS)]
                      for i, v in enumerate(all_y_vars)}

    # Line style: one per group value
    ls_map = {g: _GROUP_LINESTYLES[i % len(_GROUP_LINESTYLES)]
              for i, g in enumerate(groups)}

    # ── Helpers ─────────────────────────────────────────────────────────────
    def _s(sub, name):
        s = sub[name]
        return s.abs() if name in abs_value else s

    def _ylabel(names):
        return ', '.join(f"|{n}|" if n in abs_value else n for n in names)

    def _auto_title():
        t = _ylabel(y_left)
        if y_right:
            t += f" / {_ylabel(y_right)}"
        t += f" vs {x}"
        if group_by:   t += f"  [{group_by}]"
        if source_col: t += f"  [{source_col}]"
        return t

    fig_title = title or _auto_title()

    # ── Core draw helper ─────────────────────────────────────────────────────
    def _draw(ax_l, group_val, source_val=None):
        """
        Draw all y_left and y_right traces for one (group, source) slice.
        Returns (handles, labels) for legend assembly.
        """
        mask = pd.Series([True] * len(df), index=df.index)
        if group_val  is not None: mask &= (df[group_by]   == group_val)
        if source_val is not None: mask &= (df[source_col] == source_val)
        sub = df[mask]
        if sub.empty:
            return [], []

        ls       = ls_map.get(group_val, '-')
        handles  = []
        labels   = []

        # Legend suffix: "H100, v_stack=38.4" etc.
        parts = []
        if source_val is not None: parts.append(str(source_val))
        if group_val  is not None: parts.append(f"{group_by}={group_val}")
        suffix = ', '.join(parts)

        # Left-axis traces
        for i, yname in enumerate(y_left):
            c   = color_map[source_val if source_col else yname]
            mk  = _MARKERS[i % len(_MARKERS)]
            lbl = yname + (f" ({suffix})" if suffix else "")
            ln, = ax_l.plot(
                _s(sub, x), _s(sub, yname),
                color=c, linestyle=ls, marker=mk, label=lbl,
            )
            handles.append(ln)
            labels.append(lbl)

        # Right-axis traces (twin axis)
        if y_right:
            ax_r = ax_l.twinx()
            ax_r.spines['right'].set_color('0.45')
            ax_r.tick_params(axis='y', colors='0.45')
            ax_r.yaxis.label.set_color('0.45')
            ax_r.set_ylabel(_ylabel(y_right))

            for i, yname in enumerate(y_right):
                c   = color_map[source_val if source_col else yname]
                mk  = _MARKERS[(len(y_left) + i) % len(_MARKERS)]
                lbl = yname + (f" ({suffix})" if suffix else "")
                ln, = ax_r.plot(
                    _s(sub, x), _s(sub, yname),
                    color=c, linestyle=ls, marker=mk, label=lbl,
                    alpha=0.85,
                )
                handles.append(ln)
                labels.append(lbl)

        ax_l.set_xlabel(x)
        ax_l.set_ylabel(_ylabel(y_left))
        return handles, labels

    def _finalise_ax(ax_l, handles, labels, subtitle=None):
        if subtitle:
            ax_l.set_title(subtitle)
        ax_l.legend(handles, labels, loc='best', handlelength=2.5)

    # ── Apply IEEE style and draw ────────────────────────────────────────────
    with mpl.rc_context(_IEEE_RC):

        # ════════════════════════════════════════════════════════════════════
        # single
        # ════════════════════════════════════════════════════════════════════
        if layout == 'single':
            fig, ax = plt.subplots(figsize=figsize)
            all_h, all_l = [], []
            for src in sources:
                for grp in groups:
                    h, l = _draw(ax, grp, src)
                    all_h += h; all_l += l
            _finalise_ax(ax, all_h, all_l)
            fig.suptitle(fig_title)
            if savefig:
                fig.savefig(savefig)
                print(f"Saved: {savefig}")
            plt.show()

        # ════════════════════════════════════════════════════════════════════
        # subplots  (one row per group value)
        # ════════════════════════════════════════════════════════════════════
        elif layout == 'subplots':
            n     = len(groups)
            sub_h = figsize[1] * max(n, 1)
            fig, axes = plt.subplots(n, 1,
                                     figsize=(figsize[0], sub_h),
                                     sharex=True)
            if n == 1:
                axes = [axes]

            for ax, grp in zip(axes, groups):
                all_h, all_l = [], []
                subtitle = f"{group_by} = {grp}" if grp is not None else None
                for src in sources:
                    h, l = _draw(ax, grp, src)
                    all_h += h; all_l += l
                _finalise_ax(ax, all_h, all_l, subtitle=subtitle)

            # x-label only on the bottom subplot
            axes[-1].set_xlabel(x)
            for ax in axes[:-1]:
                ax.set_xlabel('')

            fig.suptitle(fig_title)
            if savefig:
                fig.savefig(savefig)
                print(f"Saved: {savefig}")
            plt.show()

        # ════════════════════════════════════════════════════════════════════
        # separate_windows  (one figure per group value)
        # ════════════════════════════════════════════════════════════════════
        elif layout == 'separate_windows':
            figs = []
            for grp in groups:
                fig, ax = plt.subplots(figsize=figsize)
                all_h, all_l = [], []
                for src in sources:
                    h, l = _draw(ax, grp, src)
                    all_h += h; all_l += l
                subtitle = (f"{group_by} = {grp}" if grp is not None else None)
                _finalise_ax(ax, all_h, all_l)
                fig.suptitle(f"{fig_title}  —  {subtitle}"
                             if subtitle else fig_title)
                figs.append((fig, grp))

            if savefig:
                base, ext = os.path.splitext(savefig)
                for fig, grp in figs:
                    fname = (f"{base}_{grp}{ext}" if grp is not None
                             else f"{base}{ext}")
                    fig.savefig(fname)
                    print(f"Saved: {fname}")
            plt.show()

        else:
            raise ValueError(
                f"Unknown layout='{layout}'. "
                "Choose 'single', 'subplots', or 'separate_windows'."
            )


# ══════════════════════════════════════════════════════════════════════════════
# 4.  INFO
# ══════════════════════════════════════════════════════════════════════════════

def info(df: pd.DataFrame):
    """Print a summary of sources, sweep parameters, and measurements."""
    meta  = ['source', 'step']
    all_c = [c for c in df.columns if c not in meta]
    sweep = [c for c in all_c if df[c].nunique() <= max(10, len(df) // 5)]
    meas  = [c for c in all_c if c not in sweep]

    print("=" * 64)
    if 'source' in df.columns:
        print(f"Sources      : {sorted(df['source'].unique())}")
    print(f"Total steps  : {len(df)}")
    print(f"\nSweep parameters ({len(sweep)}):")
    for c in sweep:
        vals = sorted(df[c].unique())
        print(f"  {c:20s}: {vals}")
    print(f"\nMeasurements ({len(meas)}):")
    for i, c in enumerate(meas):
        print(f"  {c}", end=('\n' if (i + 1) % 5 == 0 else ''))
    print("\n" + "=" * 64)


# ══════════════════════════════════════════════════════════════════════════════
# 5.  DEMO
# ══════════════════════════════════════════════════════════════════════════════

if __name__ == '__main__':
    df = parse_logs({
        "H100 BC": "Boost Stack/boost_stack_h100.log",
        "H100 2-IBC": "Boost Stack/boost_stack_inter2.log",
        "H100 4-IBC": "Boost Stack/boost_stack_inter4.log"
    })
    info(df)
    '''plot(df, x='d', y_left=['eta'], y_right=['pout'],
         group_by='d', abs_value=['eta'],
         layout='single',
         title='Output Power and Efficiency comparison',
         figsize=IEEE_SINGLE)'''

    plot(df, x='d', y_left=['eta'], y_right=['pout'],
         title='Output Power and Efficiency comparison',
         figsize=IEEE_DOUBLE)
