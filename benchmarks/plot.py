#!/usr/bin/env python3
# Regenerates benchmarks.png from results.csv. Needs matplotlib (pip install matplotlib).

import csv
import pathlib

import matplotlib.pyplot as plt

HERE = pathlib.Path(__file__).parent
rows = list(csv.DictReader(open(HERE / "results.csv")))

# fixed per backend so each one is the same color in every chart, instead of
# leaving it to whatever order matplotlib's cycle happens to hit
COLORS = {
    "library backend": "tab:orange",
    "custom backend": "tab:green",
    "gmp-chudnovsky": "tab:blue",
}
# one linestyle/marker per digit count within a scalability panel, same color
STYLES = {
    16000: dict(linestyle="-", marker="o"),
    32000: dict(linestyle="--", marker="s"),
}

fig, (lib_ax, custom_ax, right) = plt.subplots(1, 3, figsize=(14, 4.2))

# left two panels: threads vs wall time, one panel per backend, one line per digit count
scalability = [r for r in rows if r["suite"] == "scalability"]
panels = [("library backend", lib_ax, "library backend: threads vs. time"), ("custom backend", custom_ax, "custom backend: threads vs. time")]
for backend, ax, title in panels:
    for digits in sorted({int(r["digits"]) for r in scalability if r["backend"] == backend}):
        series = sorted((int(r["threads"]), float(r["seconds"])) for r in scalability if r["backend"] == backend and int(r["digits"]) == digits)
        threads, seconds = zip(*series)
        ax.plot(threads, seconds, color=COLORS[backend], label=f"{digits} digits", **STYLES[digits])
    ax.set_xlabel("threads")
    ax.set_ylabel("seconds")
    ax.set_title(title)
    ax.set_xticks(sorted({int(r["threads"]) for r in scalability}))
    ax.legend()

# right: digits vs wall time, ours (both backends) vs gmp-chudnovsky
comparison = [r for r in rows if r["suite"] == "vs_gmp"]
for backend in sorted({r["backend"] for r in comparison}):
    series = sorted((int(r["digits"]), float(r["seconds"])) for r in comparison if r["backend"] == backend)
    digits, seconds = zip(*series)
    right.plot(digits, seconds, marker="o", color=COLORS[backend], label=backend)
right.set_xscale("log")
right.set_yscale("log")
right.set_xlabel("digits")
right.set_ylabel("seconds")
right.set_title("us vs. gmp-chudnovsky")
right.legend()

fig.tight_layout()
fig.savefig(HERE / "benchmarks.png", dpi=150)
