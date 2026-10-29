#!/usr/bin/env python3

import json
from datetime import datetime
from collections import defaultdict

import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from matplotlib.lines import Line2D

INPUT_JSON = "merged.js"
OUTPUT_PNG = "plot3_binary_size.png"

FLAGS = ["-O1", "-O2", "-O3", "-Os", "-Oz"]
STRIPPED_FLAGS = ["-Os", "-Oz"]

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

STRIPPED_STYLES = {
    "-Os": {"linestyle": "-", "marker": "X", "alpha": 1.0},
    "-Oz": {"linestyle": "-", "marker": "*", "alpha": 1.0},
}


def darken(hex_color, factor=0.55):
    r, g, b = mcolors.to_rgb(hex_color)
    return (r * factor, g * factor, b * factor)


def classify(version: str) -> str:
    v = version.lower()
    if v.startswith("gcc") or v.startswith("gcc"):
        return "gcc"
    return "clang"


def load_data(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    for row in data:
        row["compiler"] = classify(row["version"])
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


def main():
    data = load_data(INPUT_JSON)

    series_raw = build_series(data, "binary_size", "binary_size_diff_pct")
    series_stripped = build_series(data, "binary_size_stripped", "binary_size_diff_pct")

    for compiler in ["gcc", "clang"]:
        for flag in STRIPPED_FLAGS:
            n = len(series_stripped.get((compiler, flag), []))
            print(f"stripped {compiler.upper()} {flag}: {n} points")

    fig, ax = plt.subplots(figsize=(15, 8))

    for compiler in ["gcc", "clang"]:
        for flag in FLAGS:
            points = series_raw.get((compiler, flag))
            if not points:
                continue

            dates = [p[0] for p in points]
            values = [p[1] for p in points]
            halfs = [p[2] for p in points]

            if all(h == 0 for h in halfs):
                continue

            color = COMPILER_COLORS[compiler][flag]
            lower = [v - h for v, h in zip(values, halfs)]
            upper = [v + h for v, h in zip(values, halfs)]
            ax.fill_between(
                dates, lower, upper,
                color=color, alpha=0.08, linewidth=0, zorder=1,
            )

    group_handles = {"gcc": [], "clang": []}
    group_labels = {"gcc": [], "clang": []}

    for compiler in ["gcc", "clang"]:
        for flag in FLAGS:
            points = series_raw.get((compiler, flag))
            if not points:
                continue

            dates = [p[0] for p in points]
            values = [p[1] for p in points]

            color = COMPILER_COLORS[compiler][flag]
            style = FLAG_STYLES[flag]

            line, = ax.plot(
                dates, values,
                color=color,
                linestyle=style["linestyle"],
                marker=style["marker"],
                linewidth=1.8,
                markersize=6,
                alpha=style["alpha"],
                label=flag,
                zorder=2,
            )
            group_handles[compiler].append(line)
            group_labels[compiler].append(flag)

    for compiler in ["gcc", "clang"]:
        for flag in STRIPPED_FLAGS:
            points = series_stripped.get((compiler, flag))
            if not points:
                continue

            dates = [p[0] for p in points]
            values = [p[1] for p in points]

            color = darken(COMPILER_COLORS[compiler][flag], 0.55)
            style = STRIPPED_STYLES[flag]

            line, = ax.plot(
                dates, values,
                color=color,
                linestyle=style["linestyle"],
                marker=style["marker"],
                linewidth=2.6,
                markersize=8,
                alpha=style["alpha"],
                markeredgecolor="white",
                markeredgewidth=0.8,
                label=f"{flag} (stripped)",
                zorder=4,
            )
            group_handles[compiler].append(line)
            group_labels[compiler].append(f"{flag} (stripped)")

    ax.set_xlabel("Release date")
    ax.set_ylabel("Binary size")
    ax.set_title("Binary size: GCC vs Clang (with stripped -Os/-Oz)", fontsize=13)
    ax.grid(True, alpha=0.3)

    blank = Line2D([], [], linestyle="none", marker="", color="none")

    handles, labels = [], []
    if group_handles["gcc"]:
        handles.append(blank); labels.append("GCC")
        handles += group_handles["gcc"]; labels += group_labels["gcc"]
    if group_handles["clang"]:
        handles.append(blank); labels.append("CLANG")
        handles += group_handles["clang"]; labels += group_labels["clang"]

    leg = ax.legend(
        handles, labels,
        loc="best", ncol=1, fontsize=9, framealpha=0.9,
        handlelength=2.4,
    )
    for text in leg.get_texts():
        if text.get_text() in ("GCC", "CLANG"):
            text.set_fontweight("bold")

    ax.yaxis.set_major_formatter(
        plt.FuncFormatter(lambda x, _: f"{x/1024:.0f} KB")
    )

    plt.tight_layout()
    plt.savefig(OUTPUT_PNG, dpi=150)
    print(f"Saved: {OUTPUT_PNG}")
    plt.close(fig)


if __name__ == "__main__":
    main()