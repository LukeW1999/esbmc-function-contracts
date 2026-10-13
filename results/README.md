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
| `ce_agreement.txt` | `benchmarks/ce_agreement/run_ce.sh`. Verdicts only, from 2026-06-26 on the development build of that day, not the pinned one: `run.log` shows the 2026-09-03 `exp_ce` stage did not start. Re-run on ESBMC 8.4.0 aarch64 on 2026-10-13, byte-identical |
| `svcomp.txt` | `benchmarks/svcomp/run_sv_comp.sh` |
| `x509.txt`, `x509_controls.txt` | `benchmarks/x509/run_experiment3.sh`, `run_controls.sh` |
| `freertos_heap1.txt` | `benchmarks/freertos/heap1/run_verify.sh` |
| `mlkem_controls.txt` | `benchmarks/mlkem/run_controls.sh` |

The temporary-write row of `tab:rq3_boundary` (ESBMC passes, CBMC fails) comes
from `benchmarks/exp3_temp_write/run_boundary_test.sh`. The two logs beside it
are from ESBMC 8.0.0 (2026-03) and CBMC 6.8.0, verdicts only; ESBMC 8.4.0 and
CBMC 6.10.0 give the same verdicts.
