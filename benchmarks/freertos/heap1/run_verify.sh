#!/bin/bash
#
# run_verify.sh — ESBMC verification of FreeRTOS heap_1 contracts
#

ESBMC=${ESBMC:-esbmc}
DIR="$(cd "$(dirname "$0")" && pwd)"
CONTRACTS="$DIR/heap1_contracts.c"
PLAIN="$DIR/heap1_plain.c"
HARNESS="$DIR/harness.c"
UNWIND="--unwind 5"

pass_fail() { [ "$1" -eq 0 ] && echo "PASS" || echo "FAIL"; }

echo "================================================================"
echo "  FreeRTOS heap_1 ESBMC Contract Verification"
echo "================================================================"
echo ""
echo -n "ESBMC: "
$ESBMC --version 2>&1 | head -1
echo ""

# ------------------------------------------------------------------
# Phase 1: Enforce — verify pvPortMalloc satisfies its own contracts
# (ESBMC creates a nondeterministic harness for pvPortMalloc)
# ------------------------------------------------------------------
echo "=== Phase 1: Enforce ==="
echo "Command: esbmc heap1_contracts.c --enforce-contract pvPortMalloc --function pvPortMalloc $UNWIND"
START=$(date +%s%3N)
$ESBMC "$CONTRACTS" \
    --enforce-contract pvPortMalloc \
    --function pvPortMalloc $UNWIND \
    > /tmp/heap1_enforce.log 2>&1
RC=$?
END=$(date +%s%3N)
echo "Result : $(pass_fail $RC)   Time: $((END - START))ms"
[ $RC -ne 0 ] && grep -E "(Violated|FAILED|assert)" /tmp/heap1_enforce.log | head -5
echo ""

# ------------------------------------------------------------------
# Phase 2: Direct harness — inline pvPortMalloc, no contract stubs
# ------------------------------------------------------------------
echo "=== Phase 2: Direct harness ==="
for func in verify_single_alloc verify_exhaustion verify_two_allocs; do
    echo -n "  $func ... "
    START=$(date +%s%3N)
    $ESBMC "$PLAIN" "$HARNESS" --function "$func" $UNWIND \
        > /tmp/heap1_direct_${func}.log 2>&1
    RC=$?
    END=$(date +%s%3N)
    echo "$(pass_fail $RC)   $((END - START))ms"
    [ $RC -ne 0 ] && grep -E "(Violated|FAILED|assert)" \
        /tmp/heap1_direct_${func}.log | head -3
done
echo ""

# ------------------------------------------------------------------
# Phase 3: Replace — harness calls pvPortMalloc contract stub
# ------------------------------------------------------------------
echo "=== Phase 3: Replace ==="
for func in verify_single_alloc verify_exhaustion verify_two_allocs; do
    echo -n "  $func ... "
    START=$(date +%s%3N)
    $ESBMC "$CONTRACTS" "$HARNESS" \
        --replace-call-with-contract pvPortMalloc \
        --function "$func" $UNWIND \
        > /tmp/heap1_replace_${func}.log 2>&1
    RC=$?
    END=$(date +%s%3N)
    echo "$(pass_fail $RC)   $((END - START))ms"
    [ $RC -ne 0 ] && grep -E "(Violated|FAILED|assert)" \
        /tmp/heap1_replace_${func}.log | head -3
done
echo ""

echo "================================================================"
echo "Log files: /tmp/heap1_*.log"
echo "================================================================"
