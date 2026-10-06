#!/usr/bin/env python3

import json
from datetime import datetime
from collections import defaultdict

import matplotlib.pyplot as plt

INPUT_JSON = "merged.js"

FLAG = "-O0"

COMPILER_COLORS = {
    "gcc":   "#c0392b",
    "clang": "#2980b9",
}

COMPILER_MARKERS = {
    "gcc":   "o",
    "clang": "s",
}


def load_data(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    for row in data:
        row["compiler"] = "gcc" if row["version"].lower().startswith("gcc") else "clang"
        row["date"] = datetime.strptime(row["release_date"], "%Y-%m-%d")

    return data


def build_series(data, metric, diff_key):
    """
    (compiler) -> [(date, avg, half_range), ...]
    """
    series = defaultdict(list)

    for row in data:
        if row["flags"] != FLAG:
            continue

        avg = row.get(metric)
        if avg is None:
            continue

        pct = row.get(diff_key)
        half_range = 0.0 if pct is None else abs(avg) * pct / 100.0

        series[row["compiler"]].append((row["date"], avg, half_range))

    for key in series:
        series[key].sort(key=lambda t: t[0])

    return series


def plot_combined(series, ylabel, title, output_path):
    fig, ax = plt.subplots(figsize=(15, 8))

    for compiler in ["gcc", "clang"]:
        points = series.get(compiler)
        if not points:
            continue

        dates  = [p[0] for p in points]
        values = [p[1] for p in points]
        halfs  = [p[2] for p in points]

        if all(h == 0 for h in halfs):
            continue

        color = COMPILER_COLORS[compiler]
        lower = [v - h for v, h in zip(values, halfs)]
        upper = [v + h for v, h in zip(values, halfs)]
        ax.fill_between(
            dates, lower, upper,
            color=color, alpha=0.15, linewidth=0, zorder=1,
        )

    for compiler in ["gcc", "clang"]:
        points = series.get(compiler)
        if not points:
            continue

        dates  = [p[0] for p in points]
        values = [p[1] for p in points]

        color  = COMPILER_COLORS[compiler]
        marker = COMPILER_MARKERS[compiler]

        ax.plot(
            dates, values,
            color=color,
            linestyle="-",
            marker=marker,
            linewidth=2.0,
            markersize=7,
            label=compiler.upper(),
            zorder=2,
        )

    ax.set_xlabel("Release date")
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=13)
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", fontsize=11, framealpha=0.9)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Saved: {output_path}")
    plt.close(fig)


def main():
    data = load_data(INPUT_JSON)

    # ---- compile time ----
    series_compile = build_series(
        data,
        metric="compile_time_avg",
        diff_key="compile_time_diff_pct",
    )
    plot_combined(
        series_compile,
        ylabel="Compile time (seconds)",
        title=f"Compile time at {FLAG}: GCC vs Clang",
        output_path="plot2_compile.png",
    )

    # ---- runtime ----
    series_runtime = build_series(
        data,
        metric="bench_time_avg",
        diff_key="bench_time_diff_pct",
    )
    plot_combined(
        series_runtime,
        ylabel="Runtime (seconds)",
        title=f"Runtime at {FLAG}: GCC vs Clang",
        output_path="plot2_runtime.png",
    )


if __name__ == "__main__":
    main()