#!/usr/bin/env python3

import json
from datetime import datetime
from collections import defaultdict

import matplotlib.pyplot as plt

INPUT_JSON = "merged.js"

FLAGS = ["-O1", "-O2", "-O3", "-Os", "-Oz"]

COMPILER_COLORS = {
    "gcc": {
        "-O1": "#f1c40f",
        "-O2": "#e67e22",
        "-O3": "#c0392b",
        "-Os": "#8e44ad",
        "-Oz": "#5b2c6f",
    },
    "clang": {
        "-O1": "#1abc9c",
        "-O2": "#2980b9",
        "-O3": "#1a5276",
        "-Os": "#27ae60",
        "-Oz": "#145a32",
    },
}

FLAG_STYLES = {
    "-O1": {"linestyle": ":",  "marker": "o", "alpha": 0.7},
    "-O2": {"linestyle": "-",  "marker": "s", "alpha": 0.9},
    "-O3": {"linestyle": "-.", "marker": "^", "alpha": 1.0},
    "-Os": {"linestyle": "--", "marker": "D", "alpha": 0.85},
    "-Oz": {"linestyle": "-",  "marker": "v", "alpha": 1.0},
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

            color = COMPILER_COLORS[compiler][flag]

            lower = [v - h for v, h in zip(values, halfs)]
            upper = [v + h for v, h in zip(values, halfs)]
            ax.fill_between(
                dates, lower, upper,
                color=color,
                alpha=0.08,
                linewidth=0,
                zorder=1,
            )

    for compiler in ["gcc", "clang"]:
        for flag in FLAGS:
            points = series.get((compiler, flag))
            if not points:
                continue

            dates  = [p[0] for p in points]
            values = [p[1] for p in points]

            color = COMPILER_COLORS[compiler][flag]
            style = FLAG_STYLES[flag]

            ax.plot(
                dates, values,
                color=color,
                linestyle=style["linestyle"],
                marker=style["marker"],
                linewidth=1.8,
                markersize=6,
                alpha=style["alpha"],
                label=f"{compiler.upper()} {flag}",
                zorder=2,
            )

    ax.set_xlabel("Release date")
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=13)
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", ncol=2, fontsize=10, framealpha=0.9)

    ax.yaxis.set_major_formatter(
        plt.FuncFormatter(lambda x, _: f"{x/1024:.0f} KB")
    )

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Saved: {output_path}")
    plt.close(fig)


def main():
    data = load_data(INPUT_JSON)

    series_raw = build_series(
        data,
        metric="binary_size",
        diff_key="binary_size_diff_pct",
    )
    plot_combined(
        series_raw,
        ylabel="Binary size (raw)",
        title="Binary size (raw): GCC vs Clang",
        output_path="plot3_binary_size.png",
    )

    series_strip = build_series(
        data,
        metric="binary_size_stripped",
        diff_key="binary_size_diff_pct",
    )
    plot_combined(
        series_strip,
        ylabel="Binary size (stripped)",
        title="Binary size (stripped): GCC vs Clang",
        output_path="plot3_binary_size_stripped.png",
    )


if __name__ == "__main__":
    main()
