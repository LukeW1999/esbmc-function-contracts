#!/usr/bin/env python3
"""Draw the data figures of the paper from the logs in results/.

  ./scripts/gen_figures.py [results-dir] [out-dir]

Every plotted number is the median over the repetitions in the CSV, the same
statistic the tables use, so a figure cannot drift from its table. The two
figures that carry no measurement (the VCC heatmap and the temporary-write
sketch) are not drawn here.
"""
import collections
import csv
import re
import statistics as st
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402

LOG = Path(sys.argv[1] if len(sys.argv) > 1 else "results")
OUT = Path(sys.argv[2] if len(sys.argv) > 2 else "figures")
OUT.mkdir(parents=True, exist_ok=True)
TIMEOUT_S = 120.0
GREEN, RED, ORANGE, PURPLE, BLUE = "#2a8f2a", "#d62728", "#ff7f0e", "#8e44ad", "#1f77b4"


def rows(name):
    with (LOG / name).open() as f:
        return [r for r in csv.DictReader(f) if r.get("label") or r.get("tool")]


def med(values):
    return st.median(values) if values else None


def save(fig, name):
    fig.savefig(OUT / name, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print("wrote", OUT / name)


def rq1():
    """fig3a: VCC count vs W. fig3d: wall-clock vs W with the CBMC/ESBMC ratio."""
    et, ct, ev, cv = (collections.defaultdict(list) for _ in range(4))
    for r in rows("rq1_w_sweep.csv"):
        if r["tool"] != "esbmc":
            continue
        W = int(r["W"])
        et[W].append(float(r["wall_ms"]))
        if r["vccs"] != "NA":
            ev[W].append(int(r["vccs"]))
    for r in rows("rq1_cbmc.csv"):
        W = int(r["W"])
        ct[W].append(float(r["wall_ms"]))
        cv[W].append(int(r["props"]))
    Ws = sorted(et)
    for W in Ws:
        assert len(set(ev[W])) == 1 and len(set(cv[W])) == 1, f"VCC count varies at W={W}"
    e_vcc, c_vcc = [ev[W][0] for W in Ws], [cv[W][0] for W in Ws]
    e_ms, c_ms = [med(et[W]) for W in Ws], [med(ct[W]) for W in Ws]
    ratio = [c / e for c, e in zip(c_ms, e_ms)]
    print("rq1 W:", Ws)
    print("  esbmc vcc", e_vcc, "ms", e_ms)
    print("  cbmc  vcc", c_vcc, "ms", c_ms)
    print("  ratio", [round(x, 1) for x in ratio])

    fig, ax = plt.subplots(figsize=(6, 4.2))
    ax.plot(Ws, e_vcc, "o-", color=GREEN, lw=2, ms=7,
            label=f"ESBMC (observational): $(G{{-}}M)+E={e_vcc[0]}$")
    ax.plot(Ws, c_vcc, "s-", color=RED, lw=2, ms=7, label="CBMC (DFCC): $20+W+E$")
    ax.set_xlabel("Write instructions $W$")
    ax.set_ylabel("VCC count")
    ax.set_title("VCC count vs write count")
    ax.legend(loc="upper left")
    save(fig, "fig3a_vcc_counts_w.png")

    fig, ax = plt.subplots(figsize=(6, 4.2))
    ax.plot(Ws, e_ms, "o-", color=GREEN, lw=2, ms=7, label="ESBMC time")
    ax.plot(Ws, c_ms, "s-", color=RED, lw=2, ms=7, label="CBMC time")
    ax.set_xlabel("Write instructions $W$")
    ax.set_ylabel("Time (ms)")
    ax.set_title(f"Large-scale timing ($W$ up to {Ws[-1]})")
    ax2 = ax.twinx()
    ax2.plot(Ws, ratio, "k--", lw=1.8, label="CBMC/ESBMC ratio")
    ax2.set_ylabel("CBMC/ESBMC time ratio")
    ax2.set_ylim(bottom=0)
    ax2.annotate(f"{ratio[-1]:.1f}x", (Ws[-1], ratio[-1]), textcoords="offset points",
                 xytext=(-30, 6), fontsize=10)
    h1, l1 = ax.get_legend_handles_labels()
    h2, l2 = ax2.get_legend_handles_labels()
    ax.legend(h1 + h2, l1 + l2, loc="upper left")
    save(fig, "fig3d_large_scale.png")


def rq2():
    """fig2: Direct vs Modular over (N, K): log-scale times, speedup heatmap, cost breakdown."""
    t = collections.defaultdict(list)
    timed_out = set()
    for r in rows("rq2_synth.csv"):
        key = (r["label"], int(r["N"]), None if r["K"] == "-" else int(r["K"]))
        if "TIMEOUT" in r["result"]:
            timed_out.add(key)
        else:
            t[key].append(float(r["bmc_s"]))
    Ns = sorted({k[1] for k in t})
    Ks = sorted({k[2] for k in t if k[2] is not None})
    T_E = {N: med(t[("Enforce", N, None)]) for N in Ns}
    T_R = {(N, K): med(t[("Replace", N, K)]) for N in Ns for K in Ks}
    T_D = {(N, K): med(t[("Direct", N, K)]) for N in Ns for K in Ks}
    print("rq2 N:", Ns, "K:", Ks)
    for N in Ns:
        print(f"  N={N} enforce {T_E[N]:.3f}s direct", [T_D[(N, K)] for K in Ks], "timeouts",
              [K for K in Ks if ("Direct", N, K) in timed_out])

    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    ax = axes[0]
    markers = ["o", "s", "^", "D"]
    for N, m in zip(Ns, markers):
        d = [T_D[(N, K)] * 1000 if T_D[(N, K)] is not None else TIMEOUT_S * 1000 for K in Ks]
        mod = [(T_E[N] + T_R[(N, K)]) * 1000 for K in Ks]
        ax.plot(Ks, d, marker=m, color=ORANGE, label=f"Direct N={N}")
        ax.plot(Ks, mod, marker=m, color=PURPLE, ls="--", label=f"Modular N={N}")
        for K, v in zip(Ks, d):
            if ("Direct", N, K) in timed_out:
                ax.plot([K], [v], marker=m, mfc="white", mec=ORANGE, ms=9)
    ax.set_yscale("log")
    ax.set_xlabel("Call count K")
    ax.set_ylabel("Time (ms, log)")
    ax.set_title("Direct (orange) vs Modular (purple); hollow = timeout")
    ax.legend(ncol=2, fontsize=8)

    ax = axes[1]
    grid = np.zeros((len(Ns), len(Ks)))
    labels = [[""] * len(Ks) for _ in Ns]
    for i, N in enumerate(Ns):
        for j, K in enumerate(Ks):
            mod = T_E[N] + T_R[(N, K)]
            if ("Direct", N, K) in timed_out:
                grid[i, j] = TIMEOUT_S / mod
                labels[i][j] = f">{TIMEOUT_S / mod:.1f}x"
            else:
                grid[i, j] = T_D[(N, K)] / mod
                labels[i][j] = f"{grid[i, j]:.1f}x"
    im = ax.imshow(grid, cmap="YlOrRd", aspect="auto")
    for i in range(len(Ns)):
        for j in range(len(Ks)):
            ax.text(j, i, labels[i][j], ha="center", va="center",
                    color="white" if grid[i, j] > grid.max() / 2 else "black", fontsize=10)
    ax.set_xticks(range(len(Ks)), Ks)
    ax.set_yticks(range(len(Ns)), Ns)
    ax.set_xlabel("K")
    ax.set_ylabel("N")
    ax.set_title("Speedup = Direct/Modular; '>' = Direct timed out")
    fig.colorbar(im, ax=ax)

    ax = axes[2]
    names = [f"N{N}\nK{K}" for N in Ns for K in Ks]
    enf = [T_E[N] * 1000 for N in Ns for K in Ks]
    rep = [T_R[(N, K)] * 1000 for N in Ns for K in Ks]
    x = np.arange(len(names))
    ax.bar(x, enf, color="#2e7d32", label="Enforce")
    ax.bar(x, rep, bottom=enf, color="#4fc3f7", label="Replace")
    ax.set_xticks(x, names, fontsize=8)
    ax.set_ylabel("Time (ms)")
    ax.set_title("Modular breakdown (E+R)")
    ax.legend()
    save(fig, "fig2_scalability.png")


def x509():
    """fig7b: verdict and wall-clock per scenario for both tools, from x509.txt."""
    tool, times, verdicts = None, {"ESBMC": [], "CBMC": []}, {"ESBMC": [], "CBMC": []}
    for line in (LOG / "x509.txt").read_text().splitlines():
        if "ESBMC Tests" in line:
            tool = "ESBMC"
        elif "CBMC Tests" in line:
            tool = "CBMC"
        elif "Results Summary" in line:
            tool = None
        if tool:
            m = re.search(r"Time: (\d+)ms|PASS \((\d+)ms\)", line)
            if m:
                times[tool].append(int(m.group(1) or m.group(2)))
        m = re.match(r"^(A|B|C|D\d)\. .*\| (PASS|FAIL)\s+\| (PASS|FAIL)", line)
        if m:
            verdicts["ESBMC"].append(m.group(2))
            verdicts["CBMC"].append(m.group(3))
    scen = ["A. Direct\n(no contracts)", "B. Replace\nleaves only", "C. Replace\nall functions",
            "D1. Enforce\nparse_id_len", "D2. Enforce\nparse_x509_cert",
            "D3. Enforce parent\n+replace leaves", "D4. Replace all\n+verify main"]
    for k in times:
        assert len(times[k]) == len(scen) == len(verdicts[k]), (k, times[k], verdicts[k])
    print("x509 ms:", times)
    print("x509 verdicts:", verdicts)

    fig, (a, b) = plt.subplots(2, 1, figsize=(8, 8))
    fig.suptitle("ANSSI X.509 Contract Propagation", fontweight="bold")
    x, w = np.arange(len(scen)), 0.36
    for ax, data, title, ylabel in [
        (a, {k: [1 if v == "PASS" else 0 for v in verdicts[k]] for k in verdicts},
         "(a) Verification Results", ""),
        (b, times, "(b) Full-Pipeline Verification Time", "Time (ms)"),
    ]:
        bars_e = ax.bar(x - w / 2, data["ESBMC"], w, color="#3b9ae1", label="ESBMC")
        bars_c = ax.bar(x + w / 2, data["CBMC"], w, color="#f5a623", label="CBMC")
        ax.set_xticks(x, scen, rotation=25, ha="right", fontsize=8)
        ax.set_title(title)
        ax.legend(loc="upper right")
        if ylabel:
            ax.set_ylabel(ylabel)
            for bars in (bars_e, bars_c):
                ax.bar_label(bars, fontsize=8, fontweight="bold")
            ax.set_ylim(0, max(max(times["ESBMC"]), max(times["CBMC"])) * 1.25)
        else:
            ax.set_yticks([0, 1], ["FAIL", "PASS"])
            ax.set_ylim(0, 1.15)
    fig.tight_layout()
    save(fig, "fig7b_x509_propagation.png")


if __name__ == "__main__":
    rq1()
    rq2()
    x509()
