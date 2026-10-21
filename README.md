# ESBMC function contracts

Benchmarks, proofs and measurements for the ICSE 2027 tool demonstration of
`frame_enforcer`, the module that gives ESBMC `requires`/`ensures`/`assigns`
verification.

**The tool is not here.** It is merged into ESBMC `master`:
<https://github.com/esbmc/esbmc>, in `src/goto-programs/frame_enforcer.cpp`.
`esbmc --help` describes `--enforce-contract` and
`--replace-call-with-contract`, and `regression/function_contract` in that
repository holds runnable examples of every clause. This repository holds what
the paper measures.

## Try it

The quickest route is the prebuilt image (x86_64, about 1 GB), which is the
container below as pushed on 2026-10-15 and checked against the logs in
`results/`:

    docker pull ghcr.io/lukew1999/esbmc-function-contracts
    docker run --rm ghcr.io/lukew1999/esbmc-function-contracts

Otherwise build the container, which compiles ESBMC from source at the
measured commit `2f719f59e0` with the goto-contractor off, and adds CBMC 6.8.0
(about an hour):

    docker build -t esbmc-contracts .
    docker run --rm esbmc-contracts

or build ESBMC at that commit yourself the same way, put `esbmc` on `PATH`, and run:

    scripts/smoke.sh

Older binaries do not reproduce the paper. 8.4 rejects a stack-allocated
argument to an `is_fresh` parameter on the Replace side, so the third command
below fails there. The 8.5 release lacks the five `is_fresh` fixes between it
and the measured commit `2f719f59e0`, so X.509 scenario B fails on it.

Both releases also start with the FPU rounding mode set upward by the ibex
interval library (ESBMC issue #8043). Bitwuzla, the default solver, then runs
out of memory on the Direct baselines the paper reports in seconds. The
measured build was configured without ibex and does not show the effect,
which is what the container reproduces. Upstream fixed the rounding mode on
2026-09-29 (#8045), but the `v8.6` line that carries the fix also changes the
Enforce harness again (#8042) and fails the ML-KEM contracts the paper
verifies, so it does not reproduce the paper either.

Three commands, about two seconds. Expected:

    prog.c  --enforce-contract           62 VCC, 14 properties, SUCCESSFUL
    bad.c   --enforce-contract           assigns compliance: c:@g_other ... FAILED
    prog.c  --replace-call-with-contract  4 VCC,  3 properties, SUCCESSFUL

`examples/prog.c` is the annotated function the paper lists, and `bad.c` is the
same function with one write to an undeclared global.

`examples/descend.c` is a recursive function under a depth bound. Both flags
on the same function check the recursive call against the contract, so no
unwinding is needed:

    esbmc examples/descend.c          --enforce-contract descend --replace-call-with-contract descend --function descend
    esbmc examples/descend.c -DCAPPED --enforce-contract descend --replace-call-with-contract descend --function descend

Expected: 1 VCC each; the first fails at `contract requires` on the recursive
call because nothing in the body owns the bound, the second, with the guard,
is SUCCESSFUL.

## Layout

| Path | What |
|---|---|
| `examples/` | the annotated function, its frame-violating twin and a bounded recursive function |
| `benchmarks/mlkem/` | `poly_reduce`, `poly_add`, `poly_tomont`, with contracts; `poly_reduce_cbmc.c` is the CBMC arm |
| `benchmarks/x509/` | the ANSSI parser functions, plain and contracted, the propagation harness and the controls |
| `benchmarks/freertos/heap1/` | `heap_1` with contracts, and the harness |
| `benchmarks/rq1_vcc/`, `benchmarks/rq1_w_sweep/` | the RQ1 programs: the single- and ten-call pair, and the generator of the $W$-sweep pair |
| `benchmarks/rq2_synth/` | `synth_compute.c`, the $N \times K$ synthetic program behind RQ2 |
| `benchmarks/rq3_boundary/`, `benchmarks/exp3_temp_write/`, `benchmarks/ce_agreement/` | the field-sensitivity pair, the temporary-write program, and the six seeded mutants |
| `benchmarks/svcomp/` | the SV-COMP and VerifyThis tasks, plain and contracted |
| `lean/` | `ContractSoundness.lean`, 237 lines, no `sorry`, `admit` or added `axiom` |
| `results/` | every measurement the paper reports, one run |
| `scripts/` | `smoke.sh`, `rerun_all.sh`, `verify_paper.py`, `gen_figures.py` (the data figures, from `results/`) |
| `Dockerfile` | ESBMC built from source at `2f719f59e0` without ibex, CBMC 6.8.0, and the examples and benchmarks on top |

Every script takes the binary from `esbmc` on `PATH`, or from `$ESBMC`.

## The measurements

Every table and figure comes from one run, recorded in `results/`. The host, the
ESBMC commit, the binary's SHA-256 and the CPU clock beside each measurement are
in `results/ENVIRONMENT`.

    ESBMC 8.5.0 at master 2f719f59e0, Bitwuzla 0.9.0, CBMC 6.8.0
    16-vCPU Xeon 6982P-C, 61 GiB, Ubuntu 24.04, governor pinned

`scripts/verify_paper.py` re-derives each table cell and each timing quoted in
the prose from the CSVs and exits non-zero on a mismatch.

`scripts/rerun_all.sh` re-measures everything. It refuses to start without
`--sha`, and records the commit and the binary's hash beside the numbers. It
takes hours and the wall-clock will not match ours on other hardware. The
verdicts and the ratios do.

### Building the pinned commit

`2f719f59e0` needs mpfr 4.2.1 or newer, which Ubuntu 22.04 does not carry.
Configuring it there fails in Bitwuzla's meson step with `Invalid version, need
'mpfr' ['>=4.2.1'] found '4.1.0'`. Ubuntu 24.04 works.

## Licensing

Our contracts, harnesses, proofs and scripts are Apache-2.0. Several benchmarks
derive from upstream projects and keep their own terms and their file headers:

- `benchmarks/mlkem/` from [mlkem-native](https://github.com/pq-code-package/mlkem-native).
  The contracts are theirs, re-expressed with the `__ESBMC_*` intrinsics. Two
  adaptations are recorded in the file headers: the constant-time selection is a
  branch, and the loop invariants are dropped because ESBMC unrolls the bound
  the standard fixes.
- `benchmarks/x509/` from the ANSSI X.509 parser.
- `benchmarks/svcomp/` from the SV-COMP collection
  ([sv-benchmarks](https://gitlab.com/sosy-lab/benchmarking/sv-benchmarks))
  and the VerifyThis challenges; a file that derives from sv-benchmarks names
  its origin and the bounds we tightened in its header.
- `benchmarks/freertos/` from the FreeRTOS kernel. `heap_1` is self-contained:
  `freertos_stubs.h` stands in for `FreeRTOS.h` and `task.h`, so no kernel
  checkout is needed.
