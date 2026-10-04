#!/usr/bin/env python3

import json
from datetime import datetime
from collections import defaultdict

import matplotlib.pyplot as plt

INPUT_JSON = "merged.js"

FLAGS = ["-O2", "-Os", "-Oz"]

COMPILER_COLORS = {
    "gcc":   "#c0392b",
    "clang": "#2980b9",
}

FLAG_STYLES = {
    "-O2": {"linestyle": "-",  "marker": "o", "linewidth": 3.0, "alpha": 1.0,  "markersize": 7},
    "-Os": {"linestyle": "--", "marker": "s", "linewidth": 1.8, "alpha": 0.85, "markersize": 6},
    "-Oz": {"linestyle": ":",  "marker": "^", "linewidth": 1.8, "alpha": 0.85, "markersize": 6},
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
    (compiler, flag) -> [(date, avg, half_range), ...]
    """
    series = defaultdict(list)

    for row in data:
        flag = row["flags"]
        if flag not in FLAGS:
            continue

        avg = row.get(metric)
        if avg is None:
            continue

        pct = row.get(diff_key)
        half_range = 0.0 if pct is None else abs(avg) * pct / 100.0

        series[(row["compiler"], flag)].append((row["date"], avg, half_range))

    for key in series:
        series[key].sort(key=lambda t: t[0])

    return series


def plot_combined(series, ylabel, title, output_path):
    fig, ax = plt.subplots(figsize=(15, 8))

    for compiler in ["gcc", "clang"]:
        for flag in FLAGS:
            points = series.get((compiler, flag))
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

            alpha = 0.12 if flag == "-O2" else 0.08

            ax.fill_between(
                dates, lower, upper,
                color=color, alpha=alpha, linewidth=0, zorder=1,
            )

    for compiler in ["gcc", "clang"]:
        for flag in FLAGS:
            points = series.get((compiler, flag))
            if not points:
                continue

            dates  = [p[0] for p in points]
            values = [p[1] for p in points]

            color = COMPILER_COLORS[compiler]
            style = FLAG_STYLES[flag]

            ax.plot(
                dates, values,
                color=color,
                linestyle=style["linestyle"],
                marker=style["marker"],
                linewidth=style["linewidth"],
                markersize=style["markersize"],
                alpha=style["alpha"],
                label=f"{compiler.upper()} {flag}",
                zorder=2 if flag == "-O2" else 3,
            )

    ax.set_xlabel("Release date")
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=13)
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", ncol=2, fontsize=10, framealpha=0.9)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Saved: {output_path}")
    plt.close(fig)


def main():
    data = load_data(INPUT_JSON)

    # ---- runtime ----
    series_run = build_series(data, metric="bench_time_avg", diff_key="bench_time_diff_pct")
    plot_combined(
        series_run,
        ylabel="Runtime (seconds)",
        title="Runtime: -Os / -Oz vs -O2 (GCC and Clang)",
        output_path="plot4_runtime.png",
    )

    # ---- compile time ----
    series_comp = build_series(data, metric="compile_time_avg", diff_key="compile_time_diff_pct")
    plot_combined(
        series_comp,
        ylabel="Compile time (seconds)",
        title="Compile time: -Os / -Oz vs -O2 (GCC and Clang)",
        output_path="plot4_compile.png",
    )


if __name__ == "__main__":
    main()