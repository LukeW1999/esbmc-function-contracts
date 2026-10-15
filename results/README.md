# Re-baseline, 2026-09-03

Every measured table in the paper comes from this one run. Verify with:

    scripts/verify_paper.py results

## Host and build

Idle 16-vCPU Intel Xeon 6982P-C, 61 GiB, Ubuntu 24.04, governor pinned to
`performance`, clock steady between 3.5 and 3.9 GHz. ESBMC 8.5.0 at `master`
commit `2f719f59e0`, default solver Bitwuzla 0.9.0; CBMC 6.8.0. Full record in
`ENVIRONMENT`, including the binary's SHA-256 and the clock beside each
measurement.

## What is here

| file | table |
|---|---|
| `rq1_w_sweep.csv`, `rq1_cbmc.csv` | `tab:rq1` |
| `rq2_synth.csv` | `tab:rq2`, and `tab:kstar` by derivation |
| `mlkem_reduce.csv` | `tab:mlkem-esbmc` |
| `mlkem_cbmc.csv`, `mlkem_cbmc_enforce.csv` | `tab:mlkem-cbmc` |
| `mlkem_scaling.csv`, `mlkem_scaling_cbmc.csv` | `tab:mlkem-scaling` |
| `mlkem_kernels.csv` | `tab:mlkem-kernels` |
| `x509.txt`, `x509_controls.txt` | `tab:x509_timing` |
| `freertos_heap1.txt` | `tab:freertos` |
| `svcomp.txt`, `ce_agreement.txt`, `rq3_boundary.txt` | the remaining studies |
| `mlkem_replace_z3.csv`, `anomaly.csv` | solver comparison, esbmc/esbmc#7514 |

Negative controls: `mlkem_controls.txt` 4/4 and `x509_controls.txt` 7/7 fire.
An all-passing table looks the same as one that checks nothing, so read those
first.

## Two things the run established

The CBMC VCC counts in `tab:rq1` reproduce exactly (22, 31, 71, 121, 221, 521),
confirming the `20+W+E` line, and ESBMC stays at 5 for every W. That result is a
count rather than a time, so it does not depend on the host or the solver.

At `MLKEM_N=256` CBMC's monolithic run finishes at 599.7 s against a 600 s
budget. On two other hosts the same run measured 579 s and past 700 s, so
whether it lands inside a fixed budget is a property of the machine. The paper
reports the modular route's headroom instead.

## Which script wrote which log

| log | script |
|---|---|
| `rq2_synth.csv`, `mlkem_reduce.csv`, `mlkem_scaling.csv` | `scripts/rerun_all.sh` |
| `mlkem_cbmc.csv`, `mlkem_cbmc_enforce.csv`, `mlkem_kernels.csv`, `mlkem_scaling_cbmc.csv` | the `exp_mlkem_cbmc` and `exp_mlkem_kernels` stages `run.log` lists, from a later `rerun_all.sh` that was lost with the host; the columns are those of its `erun` |
| `rq1_vcc.txt` | `benchmarks/rq1_vcc/run_experiment2.sh` |
| `rq1_w_sweep.csv`, `rq1_cbmc.csv` | `benchmarks/rq1_w_sweep/run_w_sweep.sh`, written afterwards from the paper's Exp 1a definition; the original driver was not kept. Its counts (5 for ESBMC, 20+W+1 for CBMC) match these files on ESBMC 8.4.0 and CBMC 6.10.0 |
| `rq3_boundary.txt` | `benchmarks/rq3_boundary/run_comparison.sh` |
| `ce_agreement.txt` | `benchmarks/ce_agreement/run_ce.sh`. Verdicts only, from 2026-06-26 on the development build of that day, not the pinned one: `run.log` shows the 2026-09-03 `exp_ce` stage did not start. Re-run on the pinned commit on 2026-10-15 with the `is_fresh` clause below, identical |
| `svcomp.txt` | `benchmarks/svcomp/run_sv_comp.sh` |
| `x509.txt`, `x509_controls.txt` | `benchmarks/x509/run_experiment3.sh`, `run_controls.sh` |
| `freertos_heap1.txt` | `benchmarks/freertos/heap1/run_verify.sh` |
| `mlkem_controls.txt` | `benchmarks/mlkem/run_controls.sh` |

The temporary-write row of `tab:rq3_boundary` (ESBMC passes, CBMC fails) comes
from `benchmarks/exp3_temp_write/run_boundary_test.sh`. The two logs beside it
are from ESBMC 8.0.0 (2026-03) and CBMC 6.8.0, verdicts only; ESBMC 8.4.0 and
CBMC 6.10.0 give the same verdicts.

## Two things the committed sources did not show

`synth_compute.c` and `ce_agree.c` as committed on 2026-06-26 stated
`arr != NULL` and no extent for `arr`. Since ESBMC #6484 (2026-07-30) the
Enforce harness gives such a pointer a nondeterministic extent, and the
`arr[i]` read then fails its bounds check on every build after that date,
including the pinned one. The logged Enforce rows pass, and their times are
reproduced within 2% by the clause the files now carry,
`__ESBMC_is_fresh(arr, n * sizeof(int))`, which is also the form the tool
paper prints. The files were brought in line on 2026-10-15.

The `Command:` line printed for X.509 scenario B in `x509.txt` shows a
comma-joined pair of names. ESBMC rejects that spelling since #6502
(2026-07-31), so the run executed the two separate flags the script has
always passed. The echo was corrected the day after the run.

A reproduction with the 8.4 or 8.5 Linux release, or with a default
`scripts/build.sh` build of the pinned commit, times out or runs out of
memory on the Direct columns. That is the FPU rounding mode the ibex library
leaves upward (ESBMC #8043, fixed upstream in #8045), not a property of the
benchmarks. With the rounding mode restored, the pinned commit gives every
verdict and VCC count in these logs and the times within a few percent on the
same CPU model. The later `v8.6` line (checked at `199741e8d8`) does not: its
Enforce harness (#8042) fails the ML-KEM contracts and the M5 mutant.

## The container reproduces these logs

A `docker build` of this tree on 2026-10-15 (Xeon 6982P-C, the CPU model of
the 2026-09-03 host) followed by `scripts/rerun_all.sh --reps 1` inside the
container gave the same verdict and VCC count in every row above, and all
eleven negative controls fired with a `VERIFICATION FAILED` verdict. Its
`bmc_s` for the cells that take over a second came out between 3% and 7%
below the Xeon means. The first image lacked `python3` and `bc`, which the
control scripts and the speedup columns need. Both are in the image now and
the control scripts refuse to count anything but a failed verdict.
