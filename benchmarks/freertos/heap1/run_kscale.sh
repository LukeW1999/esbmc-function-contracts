#!/bin/bash
#
# run_kscale.sh — K-scaling experiment for FreeRTOS heap_1 pvPortMalloc
#
# Measures Direct (real implementation, no contracts) vs Replace
# (contract stub) verification time for K = 1, 2, 5, 10 call sites.
#
# Usage: ./run_kscale.sh

ESBMC=${ESBMC:-esbmc}
DIR="$(cd "$(dirname "$0")" && pwd)"
CONTRACTS="$DIR/heap1_contracts.c"
PLAIN="$DIR/heap1_plain.c"
KSCALE="$DIR/harness_kscale.c"
UNWIND="--unwind 5"

pass_fail() { [ "$1" -eq 0 ] && echo "PASS" || echo "FAIL($1)"; }

echo "================================================================"
echo "  FreeRTOS heap_1 K-Scaling Experiment"
echo "  Direct (inline impl) vs Replace (contract stub)"
echo "================================================================"
echo ""
echo -n "ESBMC: "
$ESBMC --version 2>&1 | head -1
echo ""
echo "Config: heap size = 256 bytes, portBYTE_ALIGNMENT = 1"
echo "        MAX_ALLOC per call = 16 bytes, $UNWIND"
echo ""

declare -A DIRECT_TIME DIRECT_RC
declare -A REPLACE_TIME REPLACE_RC

# Collect Direct times
for K in 1 2 5 10; do
    func="verify_k${K}"
    START=$(date +%s%3N)
    $ESBMC "$PLAIN" "$KSCALE" --function "$func" $UNWIND \
        > "/tmp/heap1_kscale_direct_k${K}.log" 2>&1
    RC=$?
    END=$(date +%s%3N)
    DIRECT_TIME[$K]=$((END - START))
    DIRECT_RC[$K]=$RC
done

# Collect Replace times
for K in 1 2 5 10; do
    func="verify_k${K}"
    START=$(date +%s%3N)
    $ESBMC "$CONTRACTS" "$KSCALE" \
        --replace-call-with-contract pvPortMalloc \
        --function "$func" $UNWIND \
        > "/tmp/heap1_kscale_replace_k${K}.log" 2>&1
    RC=$?
    END=$(date +%s%3N)
    REPLACE_TIME[$K]=$((END - START))
    REPLACE_RC[$K]=$RC
done

# Print summary
printf "\n%-6s  %-16s  %-16s  %-10s\n" "K" "Direct (ms)" "Replace (ms)" "Speedup"
echo "------------------------------------------------------------"
for K in 1 2 5 10; do
    D=${DIRECT_TIME[$K]}
    R=${REPLACE_TIME[$K]}
    DR=$(pass_fail ${DIRECT_RC[$K]})
    RR=$(pass_fail ${REPLACE_RC[$K]})
    if [ "$R" -gt 0 ]; then
        SPEEDUP=$(echo "scale=1; $D / $R" | bc 2>/dev/null || echo "n/a")
    else
        SPEEDUP="inf"
    fi
    printf "%-6s  %-16s  %-16s  %-10s\n" \
        "$K" \
        "${D} [${DR}]" \
        "${R} [${RR}]" \
        "${SPEEDUP}x"
done

echo ""
echo "Log files: /tmp/heap1_kscale_*.log"
