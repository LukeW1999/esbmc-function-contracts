#!/bin/bash
# ESBMC vs CBMC Contract Field-Sensitivity Comparison
# Usage: bash run_comparison.sh [/path/to/esbmc]

ESBMC=${1:-${ESBMC:-esbmc}}
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=============================================================="
echo "ESBMC vs CBMC: Field-Sensitive Assigns Comparison"
echo "=============================================================="
echo ""

echo "=== Tool Versions ==="
echo -n "ESBMC: "
$ESBMC --version 2>&1 | head -1
echo -n "CBMC:  "
cbmc --version 2>&1 | head -1
echo ""

#######################################################################
# ESBMC Tests
#######################################################################
echo "=============================================================="
echo "                      ESBMC Tests"
echo "=============================================================="

cd "$SCRIPT_DIR"

echo ""
echo "=== ESBMC PASS: field-sensitive assigns ==="
START=$(date +%s.%N)
$ESBMC field_sensitive_esbmc.c --replace-call-with-contract "*" 2>&1
ESBMC_PASS_RESULT=$?
END=$(date +%s.%N)
ESBMC_PASS_TIME=$(echo "$END - $START" | bc)

echo ""
echo "=== ESBMC FAIL: weak ensures ==="
START=$(date +%s.%N)
$ESBMC field_sensitive_fail_esbmc.c --replace-call-with-contract "*" 2>&1
ESBMC_FAIL_RESULT=$?
END=$(date +%s.%N)
ESBMC_FAIL_TIME=$(echo "$END - $START" | bc)

#######################################################################
# CBMC Tests
#######################################################################
echo ""
echo "=============================================================="
echo "                      CBMC Tests"
echo "=============================================================="

cd "$SCRIPT_DIR"
rm -f *.goto

echo ""
echo "=== CBMC PASS: field-sensitive assigns ==="
goto-cc field_sensitive_cbmc.c -o test.goto 2>&1
goto-instrument --replace-call-with-contract set_x test.goto t1.goto 2>/dev/null
goto-instrument --replace-call-with-contract modify_array_element t1.goto t2.goto 2>/dev/null
goto-instrument --replace-call-with-contract update_multiple_fields t2.goto t3.goto 2>/dev/null
goto-instrument --replace-call-with-contract modify_global_and_ptr t3.goto t4.goto 2>/dev/null
goto-instrument --replace-call-with-contract increment_counter t4.goto final.goto 2>/dev/null

START=$(date +%s.%N)
cbmc final.goto 2>&1
CBMC_PASS_RESULT=$?
END=$(date +%s.%N)
CBMC_PASS_TIME=$(echo "$END - $START" | bc)

echo ""
echo "=== CBMC FAIL: weak ensures ==="
goto-cc field_sensitive_fail_cbmc.c -o fail.goto 2>&1
goto-instrument --replace-call-with-contract weak_ensures_contract fail.goto f1.goto 2>/dev/null
goto-instrument --replace-call-with-contract trivial_ensures f1.goto f2.goto 2>/dev/null

START=$(date +%s.%N)
cbmc f2.goto 2>&1
CBMC_FAIL_RESULT=$?
END=$(date +%s.%N)
CBMC_FAIL_TIME=$(echo "$END - $START" | bc)

rm -f *.goto

#######################################################################
# Summary
#######################################################################
echo ""
echo "=============================================================="
echo "                      Results Summary"
echo "=============================================================="
echo ""
printf "%-20s | %-12s | %-12s | %-10s\n" "Test" "ESBMC" "CBMC" "Expected"
printf "%-20s-+-%-12s-+-%-12s-+-%-10s\n" "--------------------" "------------" "------------" "----------"

[ $ESBMC_PASS_RESULT -eq 0 ] && ESBMC_PASS_STR="PASS" || ESBMC_PASS_STR="FAIL"
[ $CBMC_PASS_RESULT -eq 0 ] && CBMC_PASS_STR="PASS" || CBMC_PASS_STR="FAIL"
printf "%-20s | %-12s | %-12s | %-10s\n" "Field-sensitive" "$ESBMC_PASS_STR" "$CBMC_PASS_STR" "PASS"

[ $ESBMC_FAIL_RESULT -ne 0 ] && ESBMC_FAIL_STR="FAIL" || ESBMC_FAIL_STR="PASS"
[ $CBMC_FAIL_RESULT -ne 0 ] && CBMC_FAIL_STR="FAIL" || CBMC_FAIL_STR="PASS"
printf "%-20s | %-12s | %-12s | %-10s\n" "Weak ensures" "$ESBMC_FAIL_STR" "$CBMC_FAIL_STR" "FAIL"

echo ""
printf "%-20s | ESBMC: %.3fs | CBMC: %.3fs\n" "PASS time" "$ESBMC_PASS_TIME" "$CBMC_PASS_TIME"
printf "%-20s | ESBMC: %.3fs | CBMC: %.3fs\n" "FAIL time" "$ESBMC_FAIL_TIME" "$CBMC_FAIL_TIME"
echo ""
