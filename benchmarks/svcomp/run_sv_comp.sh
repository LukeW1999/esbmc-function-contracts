#!/bin/bash
# SV-COMP Contract Benchmark: Direct vs Modular Verification
#
# Usage: bash run_sv_comp.sh [path-to-esbmc]
# Default ESBMC path: esbmc (from PATH)
#
# Benchmarks:
#   verifythis/    : lcp, prefixsum, prefixsum_rec, duplets, elimination_max_rec, elimination_max_rec_onepoint
#   recursive/     : Addition, EvenOdd, MultCommutative
#   recursive-simple/ : fibo, sum
#   loop-lit/      : hhk2008, cggmp2005  (loop invariant + k-induction)
#
# For each benchmark we measure:
#   Plain    = direct BMC, no contracts
#   Enforce  = per-function contract verification (sum of all functions)
#   Replace  = system-level with --replace-call-with-contract
#   Total    = Enforce + Replace (modular workflow)
#   Speedup  = Plain / Total
#
# loop-lit section uses --loop-invariant instead of enforce/replace workflow

E=${1:-${ESBMC:-esbmc}}
D="$(cd "$(dirname "$0")" && pwd)"
VT="$D/verifythis"
RS="$D/recursive-simple"
REC="$D/recursive"
LL="$D/loop-lit"
T=120
PTR="--no-pointer-check --no-align-check"

# ── helpers ──────────────────────────────────────────────────────────────────

# Run ESBMC, return "<ms> <VCC>" or "TO <VCC>" or "ERR <VCC>"
run_ms_vcc() {
    local start end out rc ms vcc result
    start=$(date +%s%N)
    out=$(timeout $T "$@" 2>&1)
    rc=$?
    end=$(date +%s%N)
    ms=$(( (end - start) / 1000000 ))
    vcc=$(echo "$out" | grep -oP 'Generated \K[0-9]+(?= VCC)' | tail -1)
    vcc=${vcc:-"?"}
    if   [ $rc -eq 124 ]; then result="TO"
    elif echo "$out" | grep -q "VERIFICATION SUCCESSFUL"; then result="OK"
    elif echo "$out" | grep -q "VERIFICATION FAILED";    then result="FAIL"
    else result="ERR"
    fi
    echo "$ms $vcc $result"
}

run_plain() {
    local file="$1"; shift
    run_ms_vcc $E "$file" "$@"
}

run_enforce() {
    local file="$1" func="$2"; shift 2
    run_ms_vcc $E "$file" --function "$func" --enforce-contract "$func" "$@"
}

run_replace() {
    local file="$1"; shift
    run_ms_vcc $E "$file" --replace-call-with-contract '*' "$@"
}

# Sum millisecond column from multiple "ms vcc result" strings
sum_ms() {
    local total=0
    for row in "$@"; do
        local ms
        ms=$(echo "$row" | awk '{print $1}')
        if [[ "$ms" =~ ^[0-9]+$ ]]; then total=$((total + ms)); fi
    done
    echo $total
}

# Print one benchmark row
print_row() {
    local name="$1" plain_ms="$2" plain_vcc="$3" plain_res="$4" \
          enf_ms="$5" enf_vcc="$6" rep_ms="$7" rep_vcc="$8" rep_res="$9"
    local total_ms speedup
    if [[ "$enf_ms" =~ ^[0-9]+$ ]] && [[ "$rep_ms" =~ ^[0-9]+$ ]]; then
        total_ms=$((enf_ms + rep_ms))
    else
        total_ms="?"
    fi
    if [[ "$plain_ms" =~ ^[0-9]+$ ]] && [[ "$total_ms" =~ ^[0-9]+$ ]] && [ "$total_ms" -gt 0 ]; then
        speedup=$(echo "scale=2; $plain_ms / $total_ms" | bc)x
    else
        speedup="-"
    fi
    printf "%-32s | %6s ms %-4s %-3s | %8s ms %-4s | %6s ms %-4s %-3s | %8s ms | %6s\n" \
        "$name" \
        "$plain_ms" "V=$plain_vcc" "$plain_res" \
        "$enf_ms" "V=$enf_vcc" \
        "$rep_ms" "V=$rep_vcc" "$rep_res" \
        "$total_ms" \
        "$speedup"
}

echo "================================================================"
echo "  SV-COMP Contract Benchmark: Direct vs Modular Verification"
echo "  ESBMC: $E   timeout=${T}s   date=$(date '+%Y-%m-%d %H:%M')"
echo "================================================================"
printf "%-32s | %-19s | %-16s | %-19s | %-12s | %s\n" \
    "Benchmark" "Plain (direct)" "Enforce (sum)" "Replace (system)" "Total (E+R)" "Speedup"
printf "%s\n" "$(printf '─%.0s' {1..120})"

# ════════════════════════════════════════════════════════════════
echo ""
echo "── verifythis ─────────────────────────────────────────────────────────"

# lcp: enforce lcp + enforce check + replace
p=($(run_plain     $VT/lcp_plain.c         --unwind 9 $PTR))
e1=($(run_enforce  $VT/lcp_contracted.c lcp    --unwind 9 $PTR))
e2=($(run_enforce  $VT/lcp_contracted.c check  --unwind 9 $PTR))
r=($(run_replace   $VT/lcp_contracted.c        --unwind 9))
enf_ms=$(( ${e1[0]} + ${e2[0]} ))
print_row "lcp" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# duplets: enforce mkdup + finddup + replace
p=($(run_plain     $VT/duplets_plain.c       --unwind 9 $PTR))
e1=($(run_enforce  $VT/duplets_contracted.c mkdup   --unwind 9 $PTR))
e2=($(run_enforce  $VT/duplets_contracted.c finddup --unwind 9 $PTR))
r=($(run_replace   $VT/duplets_contracted.c          --unwind 9))
enf_ms=$(( ${e1[0]} + ${e2[0]} ))
print_row "duplets" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# elimination_max_rec: enforce check + replace
p=($(run_plain     $VT/elimination_max_rec_plain.c       --unwind 7 $PTR))
e1=($(run_enforce  $VT/elimination_max_rec_contracted.c check --unwind 7 $PTR))
r=($(run_replace   $VT/elimination_max_rec_contracted.c        --unwind 7))
enf_ms=${e1[0]}
print_row "elimination_max_rec" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# elimination_max_rec_onepoint: enforce check + replace
p=($(run_plain     $VT/elimination_max_rec_onepoint_plain.c       --unwind 7 $PTR))
e1=($(run_enforce  $VT/elimination_max_rec_onepoint_contracted.c check --unwind 7 $PTR))
r=($(run_replace   $VT/elimination_max_rec_onepoint_contracted.c        --unwind 7))
enf_ms=${e1[0]}
print_row "elimination_max_rec_onepoint" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# prefixsum: enforce is_pow2 (unwind 33) + upsweep + downsweep + check + replace
p=($(run_plain     $VT/prefixsum_plain.c       --unwind 9 $PTR))
e1=($(run_enforce  $VT/prefixsum_contracted.c is_pow2   --unwind 33))
e2=($(run_enforce  $VT/prefixsum_contracted.c upsweep   --unwind 9 $PTR))
e3=($(run_enforce  $VT/prefixsum_contracted.c downsweep --unwind 9 $PTR))
e4=($(run_enforce  $VT/prefixsum_contracted.c check     --unwind 9 $PTR))
r=($(run_replace   $VT/prefixsum_contracted.c            --unwind 9))
enf_ms=$(( ${e1[0]} + ${e2[0]} + ${e3[0]} + ${e4[0]} ))
print_row "prefixsum" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e2[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# prefixsum_rec: enforce upsweep + downsweep + is_pow2 + check + replace
p=($(run_plain     $VT/prefixsum_rec_plain.c       --unwind 5 $PTR))
e1=($(run_enforce  $VT/prefixsum_rec_contracted.c upsweep   --unwind 5 $PTR))
e2=($(run_enforce  $VT/prefixsum_rec_contracted.c downsweep --unwind 5 $PTR))
e3=($(run_enforce  $VT/prefixsum_rec_contracted.c is_pow2   --unwind 33))
e4=($(run_enforce  $VT/prefixsum_rec_contracted.c check     --unwind 5 $PTR))
r=($(run_replace   $VT/prefixsum_rec_contracted.c            --unwind 5))
enf_ms=$(( ${e1[0]} + ${e2[0]} + ${e3[0]} + ${e4[0]} ))
print_row "prefixsum_rec" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# ════════════════════════════════════════════════════════════════
echo ""
echo "── recursive-simple ───────────────────────────────────────────────────"

# sum: enforce sum + replace
p=($(run_plain     $RS/sum_plain.c           --unwind 11))
e1=($(run_enforce  $RS/sum_contracted.c sum  --unwind 11))
r=($(run_replace   $RS/sum_contracted.c       --unwind 11))
enf_ms=${e1[0]}
print_row "sum" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# fibo: enforce fibo + replace  (enforce may be infeasible — noted in report)
p=($(run_plain     $RS/fibo_plain.c           --unwind 20))
e1=($(run_enforce  $RS/fibo_contracted.c fibo --unwind 20))
r=($(run_replace   $RS/fibo_contracted.c       --unwind 20))
enf_ms=${e1[0]}
print_row "fibo" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# ════════════════════════════════════════════════════════════════
echo ""
echo "── recursive ──────────────────────────────────────────────────────────"

# EvenOdd: enforce isOdd + isEven + replace
p=($(run_plain     $REC/EvenOdd_plain.c          --unwind 22))
e1=($(run_enforce  $REC/EvenOdd_contracted.c isOdd  --unwind 22))
e2=($(run_enforce  $REC/EvenOdd_contracted.c isEven --unwind 22))
r=($(run_replace   $REC/EvenOdd_contracted.c          --unwind 22))
enf_ms=$(( ${e1[0]} + ${e2[0]} ))
print_row "EvenOdd" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# Addition: enforce addition + replace
p=($(run_plain     $REC/Addition_plain.c             --unwind 11))
e1=($(run_enforce  $REC/Addition_contracted.c addition --unwind 11))
r=($(run_replace   $REC/Addition_contracted.c           --unwind 11))
enf_ms=${e1[0]}
print_row "Addition" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# MultCommutative: enforce mult + replace
p=($(run_plain     $REC/MultCommutative_plain.c        --unwind 11))
e1=($(run_enforce  $REC/MultCommutative_contracted.c mult --unwind 11))
r=($(run_replace   $REC/MultCommutative_contracted.c       --unwind 11))
enf_ms=${e1[0]}
print_row "MultCommutative" "${p[0]}" "${p[1]}" "${p[2]}" "$enf_ms" "${e1[1]}" "${r[0]}" "${r[1]}" "${r[2]}"

# ════════════════════════════════════════════════════════════════
echo ""
echo "── loop-lit (loop invariant + k-induction) ────────────────────────────"
echo ""
echo "  Format: benchmark | plain (unwind N) | loop-invariant"
printf "%-28s | %-30s | %-30s\n" "Benchmark" "Plain BMC" "Loop Invariant (--loop-invariant)"
printf "%s\n" "$(printf '─%.0s' {1..92})"

run_loopinv() {
    local file="$1"; shift
    run_ms_vcc $E "$file" --loop-invariant "$@"
}

# hhk2008: plain needs --unwind b+1 (b<=1000000, infeasible); loop invariant proves unboundedly
p_hh=($(run_plain    $LL/hhk2008_plain.c --unwind 10))
i_hh=($(run_loopinv  $LL/hhk2008_inv.c))
printf "%-28s | %6s ms V=%-4s %-4s | %6s ms V=%-4s %-4s\n" \
    "hhk2008 (b<=1000000)" "${p_hh[0]}" "${p_hh[1]}" "${p_hh[2]}" "${i_hh[0]}" "${i_hh[1]}" "${i_hh[2]}"
echo "    Plain note: --unwind 10 only verifies cnt<=10; FAIL = unwinding assertion for cnt>10"

# cggmp2005: plain --unwind 5 works (loop runs exactly 4x); invariant also works, no unwind needed
p_cg=($(run_plain    $LL/cggmp2005_plain.c --unwind 5))
i_cg=($(run_loopinv  $LL/cggmp2005_inv.c))
printf "%-28s | %6s ms V=%-4s %-4s | %6s ms V=%-4s %-4s\n" \
    "cggmp2005 (fixed loop)" "${p_cg[0]}" "${p_cg[1]}" "${p_cg[2]}" "${i_cg[0]}" "${i_cg[1]}" "${i_cg[2]}"
echo "    Plain note: loop runs exactly 4 times; --unwind 5 sufficient; VCC sliced to 0"

echo ""
echo "================================================================"
echo "  Columns: Plain=direct BMC | Enforce=sum of per-fn runs"
echo "           Replace=system-level with contract abstraction"
echo "           Total=Enforce+Replace | Speedup=Plain/Total"
echo "  V=VCC count (Generated VCC from ESBMC output)"
echo "  Result: OK=VERIFICATION SUCCESSFUL, FAIL=FAILED, TO=timeout, ERR=error"
echo "  loop-lit: Plain uses fixed --unwind; Inv uses --loop-invariant (no unroll)"
echo "================================================================"
