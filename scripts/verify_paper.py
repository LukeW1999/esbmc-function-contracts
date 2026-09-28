#!/usr/bin/env python3
"""Re-derive every measured number in the paper from the raw logs.

The paper is treated as unknown and the logs as ground truth: each table cell
is recomputed from the CSV or text log that produced it, and every timing
figure quoted in the prose has to be traceable to one of them. Twelve
mismatches were found the first time this ran, none of which a read-through had
caught, because the failure mode is a table being updated while the sentence
quoting it is not.

  ./check/verify_paper.py [results-dir] [main.tex]

Exit status is non-zero if anything disagrees.
"""
import csv, re, statistics as st, sys
from pathlib import Path

LOG = Path(sys.argv[1] if len(sys.argv) > 1 else "results-xeon-2026-09-03")
TEX = Path(sys.argv[2] if len(sys.argv) > 2
           else Path.home()/"research/papers/ESBMC_Frame_func/main.tex")
tex = TEX.read_text()
FAIL = []


def rows(*names):
    out = []
    for n in names:
        f = LOG/n
        if f.exists():
            out += [r for r in csv.DictReader(f.open()) if r.get("label")]
    return out


def med(rs, pred, col="wall_s"):
    """Median over repetitions, and whether any repetition timed out."""
    v = [float(r[col]) for r in rs if pred(r) and "TIMEOUT" not in r["result"]
         and r.get(col) not in (None, "NA", ">120", ">900")]
    return (st.median(v) if v else None), any(
        pred(r) and "TIMEOUT" in r["result"] for r in rs)


def table(label):
    """The numeric body rows of the tabular carrying \\label{label}."""
    i = tex.index("\\label{%s}" % label)
    j = tex.index("\\begin{tabular}", i)
    k = tex.index("\\end{tabular}", j)
    out = []
    for line in tex[j:k].split("\n"):
        line = line.strip()
        if not line.endswith("\\\\"):
            continue
        cells = [c.strip() for c in line[:-2].split("&")]
        if re.match(r"^\d+$", cells[0]):
            out.append(cells)
    return out


def num(cell):
    m = re.search(r"(\d+\.?\d*)", cell.replace("\\,", "").replace("$", ""))
    return float(m.group(1)) if m else None


def cmp(tag, paper, computed, tol=0.03):
    if paper is None or computed is None:
        FAIL.append(f"{tag}: paper={paper} log={computed}")
    elif abs(paper - computed) > max(tol, abs(computed)*tol):
        FAIL.append(f"{tag}: paper={paper} log={computed:.2f}")


def phase_table(label, src, cols=(2, 3, 4, 5)):
    """Direct / Enforce / Replace / Modular against one CSV set."""
    rs = rows(*src)
    for r in table(label):
        if len(r) < max(cols)+1:
            continue
        N, K = r[0], r[1]
        e, _ = med(rs, lambda x: x["label"] == "Enforce" and x["N"] == N)
        p, _ = med(rs, lambda x: x["label"] == "Replace" and x["N"] == N and x["K"] == K)
        d, dt = med(rs, lambda x: x["label"] == "Direct" and x["N"] == N and x["K"] == K)
        if not dt:
            cmp(f"{label} N={N} K={K} Direct", num(r[cols[0]]), d)
        cmp(f"{label} N={N} K={K} Enforce", num(r[cols[1]]), e)
        cmp(f"{label} N={N} K={K} Replace", num(r[cols[2]]), p)
        if e is not None and p is not None:
            cmp(f"{label} N={N} K={K} Modular", num(r[cols[3]]), e+p)


phase_table("tab:mlkem-esbmc", ["mlkem_reduce.csv"])
phase_table("tab:mlkem-cbmc", ["mlkem_cbmc.csv", "mlkem_cbmc_enforce.csv"])

# tab:mlkem-scaling pairs two tools in one row.
ES, CS = rows("mlkem_scaling.csv"), rows("mlkem_scaling_cbmc.csv")
for r in table("tab:mlkem-scaling"):
    N = r[0]
    for tag, rs, dcol, mcol in (("ESBMC", ES, 1, 3), ("CBMC", CS, 2, 4)):
        e, _ = med(rs, lambda x: x["label"] == "Enforce" and x["N"] == N)
        p, _ = med(rs, lambda x: x["label"] == "Replace" and x["N"] == N)
        d, dt = med(rs, lambda x: x["label"] == "Direct" and x["N"] == N)
        if "t/o" in r[dcol]:
            if not dt:
                FAIL.append(f"scaling N={N} {tag}: paper says t/o, log completed at {d}")
        else:
            cmp(f"scaling N={N} {tag} Direct", num(r[dcol]), d, 0.05)
        cmp(f"scaling N={N} {tag} Modular", num(r[mcol]), e+p, 0.05)

# tab:rq2 reports solver time in ms.
Q = rows("rq2_synth.csv")
for r in table("tab:rq2"):
    N, K = r[0], r[1]
    e, _ = med(Q, lambda x: x["label"] == "Enforce" and x["N"] == N, "bmc_s")
    p, _ = med(Q, lambda x: x["label"] == "Replace" and x["N"] == N and x["K"] == K, "bmc_s")
    d, _ = med(Q, lambda x: x["label"] == "Direct" and x["N"] == N and x["K"] == K, "bmc_s")
    if "120" not in r[2]:
        cmp(f"rq2 N={N} K={K} Direct", num(r[2])/(1000 if "ms" in r[2] else 1), d, 0.05)
    cmp(f"rq2 N={N} K={K} Enforce", num(r[3])/1000, e, 0.05)
    cmp(f"rq2 N={N} K={K} Replace", num(r[4])/1000, p, 0.30)

# tab:kstar is derived from rq2, so check the derivation too.
def m2(l, N, K):
    v = [float(r["bmc_s"]) for r in Q if r["label"] == l and r["N"] == str(N)
         and r["K"] == str(K) and r["bmc_s"] not in ("NA", ">120")]
    return st.median(v) if v else None


for r in table("tab:kstar"):
    N = r[0]
    td, te, tr = m2("Direct", N, 1), m2("Enforce", N, "-"), m2("Replace", N, 1)
    if None in (td, te, tr):
        FAIL.append(f"kstar N={N}: missing log rows"); continue
    cmp(f"kstar N={N} T_D", num(r[1]), td*1000, 0.05)
    cmp(f"kstar N={N} T_E", num(r[2]), te*1000, 0.05)
    cmp(f"kstar N={N} K*", num(r[4]), te/(td-tr), 0.05)

# tab:rq1 is a VCC count as well as a time, and the count must match exactly.
Ew = [r for r in csv.DictReader((LOG/"rq1_w_sweep.csv").open())] if (LOG/"rq1_w_sweep.csv").exists() else []
Cw = [r for r in csv.DictReader((LOG/"rq1_cbmc.csv").open())] if (LOG/"rq1_cbmc.csv").exists() else []
for r in table("tab:rq1"):
    W = r[0]
    e = [x for x in Ew if x["tool"] == "esbmc" and x["W"] == W]
    c = [x for x in Cw if x["W"] == W and x.get("rep") not in (None, "0")]
    if not e or not c:
        FAIL.append(f"rq1 W={W}: missing log rows"); continue
    if num(r[1]) != float(e[0]["vccs"]):
        FAIL.append(f"rq1 W={W} ESBMC VCC: paper={r[1]} log={e[0]['vccs']}")
    if num(r[3]) != float(c[0]["props"]):
        FAIL.append(f"rq1 W={W} CBMC VCC: paper={r[3]} log={c[0]['props']}")
    cmp(f"rq1 W={W} ESBMC time", num(r[2]),
        st.median(float(x["wall_ms"]) for x in e), 3)
    cmp(f"rq1 W={W} CBMC time", num(r[4]),
        st.median(float(x["wall_ms"]) for x in c), 3)

# X.509 and FreeRTOS report verdicts in the paper, with their timings folded
# into prose ranges, so check the ranges the prose states against the logs.
def prose_range(pattern, values, tag):
    m = re.search(pattern, tex)
    if not m:
        FAIL.append(f"{tag}: range sentence not found"); return
    lo, hi = float(m.group(1)), float(m.group(2))
    if not values:
        FAIL.append(f"{tag}: no log values"); return
    if abs(lo - min(values)) > 1 or abs(hi - max(values)) > 1:
        FAIL.append(f"{tag}: paper={lo}-{hi} log={min(values)}-{max(values)}")

if (LOG/"x509.txt").exists():
    x = (LOG/"x509.txt").read_text()
    split = x.index("CBMC Tests")
    def times(block):
        return [float(m) for m in re.findall(r"(?:Time: |PASS \((\d+)ms\))", block) if m] + \
               [float(m) for m in re.findall(r"Time: (\d+)ms", block)]
    prose_range(r"every\s+\\toolname\{\}\s+run lands in \$(\d+)\$--\$(\d+)\$", times(x[:split]), "x509 ESBMC range")
    prose_range(r"every \\baseline\{\} run in\s*\n?\$(\d+)\$--\$(\d+)\$", times(x[split:]), "x509 CBMC range")
    # every scenario must be a pass in the log, since the table says so
    for tool, block in (("ESBMC", x[:split]), ("CBMC", x[split:])):
        if block.count("VERIFICATION FAILED") or "FAIL (" in block:
            FAIL.append(f"x509 {tool}: log has a failing scenario, table says all pass")

if (LOG/"freertos_heap1.txt").exists():
    f = (LOG/"freertos_heap1.txt").read_text()
    vals = [float(m) for m in re.findall(r"(?:Time: |\.\.\. PASS\s+)(\d+)ms", f)]
    prose_range(r"lands between \$(\d+)\$ and \$(\d+)\$\\,ms", vals, "freertos range")
    m = re.search(r"All (seven|nine) checks pass", tex)
    want = {"seven": 7, "nine": 9}.get(m.group(1) if m else "", None)
    if want is None:
        FAIL.append("freertos: check-count sentence not found")
    elif len(vals) != want:
        FAIL.append(f"freertos: paper says {want} checks, log has {len(vals)}")
    if "FAIL" in f:
        FAIL.append("freertos: log has a failing check, paper says all nine pass")

print("all figures agree with the logs" if not FAIL
      else f"{len(FAIL)} disagreements")
for f_ in FAIL:
    print("  MISMATCH", f_)
sys.exit(1 if FAIL else 0)
