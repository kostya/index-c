#!/usr/bin/env python3

import json
from datetime import datetime

import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch

INPUT_JSON = "merged.js"

FLAG = "-O2"

X_KEY, X_LABEL = "bench_time_avg",   "Runtime (seconds)"
Y_KEY, Y_LABEL = "compile_time_avg", "Compile time (seconds)"

COMPILER_COLORS = {
    "gcc":   "#c0392b",
    "clang": "#2980b9",
}

COMPILER_MARKERS = {
    "gcc":   "s",
    "clang": "o",
}


def load_data(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    for row in data:
        row["compiler"] = "gcc" if row["version"].lower().startswith("gcc") else "clang"
        row["date"] = datetime.strptime(row["release_date"], "%Y-%m-%d")
    return data


def plot_evolution(points, output_path):
    fig, ax = plt.subplots(figsize=(14, 9))

    for compiler, color in COMPILER_COLORS.items():
        rows = [d for d in points if d["compiler"] == compiler]
        rows.sort(key=lambda d: d["date"])

        if not rows:
            continue

        xs = [d[X_KEY] for d in rows]
        ys = [d[Y_KEY] for d in rows]

        for i in range(len(rows) - 1):
            arrow = FancyArrowPatch(
                (xs[i], ys[i]), (xs[i + 1], ys[i + 1]),
                arrowstyle="-|>",
                mutation_scale=14,
                color=color,
                alpha=0.55,
                linewidth=1.3,
                shrinkA=6, shrinkB=6,
                zorder=2,
            )
            ax.add_patch(arrow)

        ax.scatter(
            xs, ys,
            s=70, c=color,
            marker=COMPILER_MARKERS[compiler],
            alpha=0.85, edgecolors="white", linewidths=0.7,
            label=compiler.upper(),
            zorder=3,
        )

        for d in rows:
            ax.annotate(
                d["version"],
                (d[X_KEY], d[Y_KEY]),
                xytext=(6, 5), textcoords="offset points",
                fontsize=7, color="#333333",
                zorder=5,
            )

    ax.set_xlabel(X_LABEL)
    ax.set_ylabel(Y_LABEL)
    ax.set_title(f"{FLAG}: compile time vs runtime",
                 fontsize=13)
    ax.grid(True, alpha=0.3)
    ax.legend(loc="best", fontsize=10, framealpha=0.9)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Saved: {output_path}")
    plt.close(fig)


def main():
    data = load_data(INPUT_JSON)
    points = [d for d in data if d["flags"] == FLAG]

    if not points:
        raise SystemExit(f"No rows with flags == {FLAG!r}")

    plot_evolution(points, "plot6.png")


if __name__ == "__main__":
    main()