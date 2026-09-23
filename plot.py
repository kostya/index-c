#!/usr/bin/env python3

import json
from datetime import datetime
from collections import defaultdict

import matplotlib.pyplot as plt

INPUT_JSON  = "history.js"
OUTPUT_RUN  = "plot_runtime.png"
OUTPUT_COMP = "plot_compile.png"

EXCLUDE_FLAGS = ["-O0"]

COMPILER_COLORS = {
    "gcc":   ["#c0392b", "#e74c3c", "#e67e22", "#f39c12"],
    "clang": ["#2980b9", "#3498db", "#16a085", "#1abc9c"],
}

FLAG_STYLES = [
    {"linestyle": "-",  "marker": "o"},
    {"linestyle": "--", "marker": "s"},
    {"linestyle": ":",  "marker": "^"},
    {"linestyle": "-.", "marker": "D"},
]


def load_data(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    for row in data:
        row["compiler"] = "gcc" if row["version"].lower().startswith("gcc") else "clang"
        row["year"] = datetime.strptime(row["release_date"], "%Y-%m-%d").year

    return data


def collect_dimensions(data):
    compilers = sorted({r["compiler"] for r in data})
    flags = sorted({r["flags"] for r in data if r["flags"] not in EXCLUDE_FLAGS})
    return compilers, flags


def group_by_year(data):
    groups = defaultdict(lambda: {"compile": [], "bench": [], "versions": []})

    for row in data:
        if row["flags"] in EXCLUDE_FLAGS:
            continue
        key = (row["compiler"], row["flags"], row["year"])
        groups[key]["compile"].append(row["compile_time"])
        groups[key]["bench"].append(row["bench_time"])
        groups[key]["versions"].append(row["version"])

    return groups


def plot_metric(groups, metric_key, ylabel, title, output_path):
    compilers = sorted({c for (c, f, y) in groups})
    flags = sorted({f for (c, f, y) in groups})

    fig, ax = plt.subplots(figsize=(13, 7))

    for ci, compiler in enumerate(compilers):
        base_colors = COMPILER_COLORS.get(compiler, ["#555555"] * len(flags))

        for fi, flag in enumerate(flags):
            years = sorted({y for (c, f, y) in groups if c == compiler and f == flag})
            if not years:
                continue

            xs, ys = [], []
            for year in years:
                vals = groups[(compiler, flag, year)][metric_key]
                xs.append(year)
                ys.append(sum(vals) / len(vals))

            color = base_colors[fi % len(base_colors)]
            style = FLAG_STYLES[fi % len(FLAG_STYLES)]

            ax.plot(
                xs, ys,
                color=color,
                linestyle=style["linestyle"],
                marker=style["marker"],
                linewidth=2,
                markersize=8,
                label=f"{compiler.upper()} {flag}",
            )

    ax.set_xlabel("Year")
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", ncol=2, fontsize=10, framealpha=0.9)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Saved: {output_path}")
    plt.close(fig)


def main():
    data = load_data(INPUT_JSON)
    groups = group_by_year(data)

    compilers, flags = collect_dimensions(data)
    print(f"Compilers: {compilers}")
    print(f"Flags:     {flags}")
    print(f"Total rows (excluding {EXCLUDE_FLAGS}): {sum(len(v['bench']) for v in groups.values())}")

    plot_metric(
        groups,
        metric_key="bench",
        ylabel="Runtime (seconds)",
        title="Runtime by year: GCC vs Clang by optimization flag",
        output_path=OUTPUT_RUN,
    )

    plot_metric(
        groups,
        metric_key="compile",
        ylabel="Compile time (seconds)",
        title="Compile time by year: GCC vs Clang by optimization flag",
        output_path=OUTPUT_COMP,
    )


if __name__ == "__main__":
    main()