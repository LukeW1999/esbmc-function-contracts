#!/usr/bin/env bash
# ============================================================================
# rerun_all.sh -- one clean re-baseline of every timing table in the paper.
#
# Why this exists. The per-experiment scripts used to pick their own ESBMC
# binary, which is how the version-mixed numbers the paper had to withdraw
# were produced. This script takes exactly one binary and refuses to start
# without a recorded commit for it.
#
#   ./rerun_all.sh --esbmc /path/to/esbmc --sha <git-sha> --out results-YYYY-MM-DD
#
# Everything the paper's Setup paragraph claims is written to $OUT/ENVIRONMENT
# BEFORE any measurement, including a copy and sha256 of the binary itself, so
# the run stays reproducible after the host is released.
# ============================================================================
set -u

ESBMC=""; SHA=""; OUT=""; REPS=5; TO=120; TO_BIG=600; COOLDOWN=240
while [ $# -gt 0 ]; do
  case "$1" in
    --esbmc)    ESBMC=$2; shift 2 ;;
    --sha)      SHA=$2; shift 2 ;;
    --out)      OUT=$2; shift 2 ;;
    --reps)     REPS=$2; shift 2 ;;
    --timeout)  TO=$2; shift 2 ;;
    --timeout-big) TO_BIG=$2; shift 2 ;;
    --cooldown) COOLDOWN=$2; shift 2 ;;
    *) echo "unknown argument: $1" >&2; exit 2 ;;
  esac
done

die(){ echo "ERROR: $*" >&2; exit 1; }
[ -n "$ESBMC" ] || die "--esbmc is required"
[ -x "$ESBMC" ] || die "not executable: $ESBMC"
[ -n "$OUT" ]   || die "--out is required"
# The pin is the whole point. A run without one cannot be checked later, which
# is exactly the position the 8.4.0 numbers are in.
[ -n "$SHA" ]   || die "--sha is required: the esbmc commit this binary was built from"
command -v cbmc >/dev/null || die "cbmc not on PATH"
command -v goto-cc >/dev/null || die "goto-cc not on PATH"
command -v goto-instrument >/dev/null || die "goto-instrument not on PATH"

ROOT="$(cd "$(dirname "$0")/../benchmarks" && pwd)"
ESBMC="$(cd "$(dirname "$ESBMC")" && pwd)/$(basename "$ESBMC")"
export ESBMC
mkdir -p "$OUT" || die "cannot create $OUT"
OUT="$(cd "$OUT" && pwd)"
LOG="$OUT/run.log"; : > "$LOG"
say(){ echo "[$(date -u '+%Y-%m-%dT%H:%M:%SZ')] $*" | tee -a "$LOG" >&2; }

# ---- preflight: everything the Setup paragraph has to be able to state ------
cp "$ESBMC" "$OUT/esbmc-pinned" || die "could not archive the binary"
{
  echo "run_started_utc=$(date -u +%FT%TZ)"
  echo "esbmc_sha=$SHA"
  echo "esbmc_sha256=$(sha256sum "$ESBMC" | cut -d' ' -f1)"
  echo "esbmc_version=$("$ESBMC" --version 2>&1 | head -1)"
  echo "cbmc_version=$(cbmc --version 2>&1 | head -1)"
  echo "goto_cc_version=$(goto-cc --version 2>&1 | head -1)"
  echo "reps=$REPS timeout=$TO timeout_big=$TO_BIG cooldown=$COOLDOWN"
  echo "--- uname"; uname -a
  echo "--- lscpu"; lscpu
  echo "--- memory"; free -g
  echo "--- governor"; cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "(unavailable)"
} > "$OUT/ENVIRONMENT"
say "pinned esbmc=$SHA -> $OUT/esbmc-pinned"
say "cooling down ${COOLDOWN}s before the first measurement"
sleep "$COOLDOWN"

# ---- one measurement -------------------------------------------------------
# csv rep label N K timeout -- esbmc args...
erun(){
  local csv=$1 rep=$2 label=$3 N=$4 K=$5 to=$6; shift 6
  local s e out rc res bmc load mhz
  load=$(awk '{print $1}' /proc/loadavg)
  mhz=$(awk -F': ' '/MHz/{print $2; exit}' /proc/cpuinfo)
  s=$(date +%s.%N)
  out=$(timeout --kill-after=15 "$to" "$ESBMC" "$@" 2>&1); rc=$?
  e=$(date +%s.%N)
  if [ $rc -eq 124 ] || [ $rc -eq 137 ]; then res="TIMEOUT_$to"; bmc="NA"
  else
    res=$(echo "$out" | grep -oE "VERIFICATION (SUCCESSFUL|FAILED)" | head -1)
    bmc=$(echo "$out" | grep -oE "BMC program time: [0-9.]+s" | grep -oE "[0-9.]+")
  fi
  printf "%s,%s,%s,%s,%s,%s,%s,%.2f,%s,%s\n" \
    "$rep" "$label" "$N" "$K" "${res:-ERR($rc)}" "${bmc:-NA}" \
    "$(awk "BEGIN{printf \"%.2f\", $e-$s}")" 0 "$load" "$mhz" \
    | sed 's/,0\.00,/,/' >> "$csv"
  sleep 3
}
HDR="rep,label,N,K,result,bmc_s,wall_s,load,cpu_mhz"

# ---- experiments -----------------------------------------------------------
# Each block is isolated: one failing experiment does not stop the rest.

exp_mlkem_reduce(){
  cd "$ROOT/mlkem" || return 1
  local C="$OUT/mlkem_reduce.csv"; echo "$HDR" > "$C"
  for rep in $(seq 1 "$REPS"); do
    for N in 16 24 32 48; do local U=$((N+1))
      erun "$C" "$rep" Enforce "$N" - "$TO" poly_reduce.c -DCONTRACTS -DMLKEM_N=$N -DMLKEM_K=3 \
        --enforce-contract mlk_poly_reduce --function mlk_poly_reduce --unwind $U
      for K in 2 3 4; do
        erun "$C" "$rep" Direct "$N" "$K" "$TO" poly_reduce.c -DHARNESS_DIRECT -DMLKEM_N=$N -DMLKEM_K=$K --unwind $U
        erun "$C" "$rep" Replace "$N" "$K" "$TO" poly_reduce.c -DCONTRACTS -DMLKEM_N=$N -DMLKEM_K=$K \
          --enforce-contract mlk_polyvec_reduce --function mlk_polyvec_reduce \
          --replace-call-with-contract mlk_poly_reduce --unwind $U
      done
    done
    say "  mlkem_reduce rep $rep"
  done
}

exp_mlkem_scaling(){
  cd "$ROOT/mlkem" || return 1
  local C="$OUT/mlkem_scaling.csv"; echo "$HDR" > "$C"
  # N=48 is not a power of two and is anomalously slow relative to N=64; the
  # grid keeps it for continuity with the paper and adds 64/128/256 so the
  # trend can be read off powers of two alone.
  for N in 64 128 256; do local U=$((N+1))
    for rep in 1 2 3; do
      erun "$C" "$rep" Enforce "$N" - "$TO_BIG" poly_reduce.c -DCONTRACTS -DMLKEM_N=$N -DMLKEM_K=3 \
        --enforce-contract mlk_poly_reduce --function mlk_poly_reduce --unwind $U
      erun "$C" "$rep" Replace "$N" 4 "$TO_BIG" poly_reduce.c -DCONTRACTS -DMLKEM_N=$N -DMLKEM_K=4 \
        --enforce-contract mlk_polyvec_reduce --function mlk_polyvec_reduce \
        --replace-call-with-contract mlk_poly_reduce --unwind $U
    done
    # Direct at these sizes costs the whole budget; one run per N, as the paper says.
    erun "$C" 1 Direct "$N" 4 "$TO_BIG" poly_reduce.c -DHARNESS_DIRECT -DMLKEM_N=$N -DMLKEM_K=4 --unwind $U
    say "  mlkem_scaling N=$N"
  done
}

exp_rq2(){
  cd "$ROOT/rq2_synth" || return 1
  local C="$OUT/rq2_synth.csv"; echo "$HDR" > "$C"
  for rep in $(seq 1 "$REPS"); do
    for N in 5 10 20; do
      erun "$C" "$rep" Enforce "$N" - "$TO" synth_compute.c -DCONTRACTS -DN=$N -DK=1 \
        --enforce-contract compute --function compute --unwind $((N+1))
      for K in 1 3 5 10; do local U=$(( (N>K?N:K)+1 ))
        erun "$C" "$rep" Direct "$N" "$K" "$TO" synth_compute.c -DHARNESS -DN=$N -DK=$K --unwind $U
        erun "$C" "$rep" Replace "$N" "$K" "$TO" synth_compute.c -DHARNESS -DCONTRACTS -DN=$N -DK=$K \
          --replace-call-with-contract compute --unwind $U
      done
    done
    say "  rq2 rep $rep"
  done
}

# The remaining experiments keep their own scripts. Every one of them reads
# $ESBMC, which is exported above, so none can fall back to a developer build.
exp_x509(){     cd "$ROOT/x509"           && ./run_experiment3.sh > "$OUT/x509.txt" 2>&1; }
ctl_x509(){     cd "$ROOT/x509"           && ./run_controls.sh    > "$OUT/x509_controls.txt" 2>&1; }
ctl_mlkem(){    cd "$ROOT/mlkem"          && ./run_controls.sh    > "$OUT/mlkem_controls.txt" 2>&1; }
exp_svcomp(){   cd "$ROOT/svcomp"         && ./run_sv_comp.sh     > "$OUT/svcomp.txt" 2>&1; }
exp_freertos(){ cd "$ROOT/freertos/heap1" && ./run_verify.sh      > "$OUT/freertos_heap1.txt" 2>&1; }
exp_rq1(){      cd "$ROOT/rq1_vcc"        && ./run_experiment2.sh > "$OUT/rq1_vcc.txt" 2>&1; }
exp_w_sweep(){  cd "$ROOT/rq1_w_sweep"    && REPS=$REPS ./run_w_sweep.sh "$OUT" > "$OUT/rq1_w_sweep.log" 2>&1; }
exp_rq3(){      cd "$ROOT/rq3_boundary"   && ./run_comparison.sh  > "$OUT/rq3_boundary.txt" 2>&1; }
exp_ce(){       cd "$ROOT/ce_agreement"   && ./run_ce.sh          > "$OUT/ce_agreement.txt" 2>&1; }

for stage in exp_mlkem_reduce ctl_mlkem exp_mlkem_scaling exp_rq2 exp_rq1 exp_w_sweep exp_rq3 exp_ce \
             exp_x509 ctl_x509 exp_svcomp exp_freertos; do
  say "START $stage"
  if ( $stage ); then say "OK    $stage"; else say "FAIL  $stage (continuing)"; fi
done

say "ALL DONE -> $OUT"
echo
echo "Before trusting any number in $OUT, check:"
echo "  1. ENVIRONMENT records a real esbmc_sha, not a placeholder"
echo "  2. every *_controls.txt reports its mutations REJECTED; a control that"
echo "     did not fire means that experiment proved nothing"
echo "  3. cpu_mhz in the CSVs is stable across the run; a falling column is"
echo "     thermal throttling and the timings are not comparable"
