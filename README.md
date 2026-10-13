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

Either build the container, which fetches the ESBMC 8.5 release binary:

    docker build -t esbmc-contracts .
    docker run --rm esbmc-contracts

or install ESBMC 8.5 or newer yourself, put `esbmc` on `PATH`, and run:

    scripts/smoke.sh

8.4 is not enough: its Replace side rejects a stack-allocated argument to an
`is_fresh` parameter, so the third command below fails there.

Three commands, about two seconds. Expected:

    prog.c  --enforce-contract           62 VCC, 14 properties, SUCCESSFUL
    bad.c   --enforce-contract           assigns compliance: c:@g_other ... FAILED
    prog.c  --replace-call-with-contract  4 VCC,  4 properties, SUCCESSFUL

`examples/prog.c` is the annotated function the paper lists, and `bad.c` is the
same function with one write to an undeclared global.

## Layout

| Path | What |
|---|---|
| `examples/` | the annotated function and its frame-violating twin |
| `benchmarks/mlkem/` | `poly_reduce`, `poly_add`, `poly_tomont`, with contracts; `poly_reduce_cbmc.c` is the CBMC arm |
| `benchmarks/x509/` | the ANSSI parser functions, plain and contracted, with their controls |
| `benchmarks/freertos/heap1/` | `heap_1` with contracts, and the harness |
| `lean/` | `ContractSoundness.lean`, 237 lines, no `sorry`, `admit` or added `axiom` |
| `results/` | every measurement the paper reports, one run |
| `scripts/` | `smoke.sh`, `rerun_all.sh`, `verify_paper.py`, `gen_figures.py` (the data figures, from `results/`) |
| `Dockerfile` | the 8.5 release binary with the examples and benchmarks on top |

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
- `benchmarks/freertos/` from the FreeRTOS kernel. `heap_1` is self-contained:
  `freertos_stubs.h` stands in for `FreeRTOS.h` and `task.h`, so no kernel
  checkout is needed.
