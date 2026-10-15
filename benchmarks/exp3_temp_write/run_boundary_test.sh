#!/bin/bash
# Exp3: Black-box vs White-box Soundness Boundary
# Tests the semantic difference between observational and strict write-set compliance

ESBMC="${ESBMC:-esbmc}"
GOTO_CC="goto-cc"
GOTO_INSTRUMENT="goto-instrument"
CBMC="cbmc"
DIR="$(cd "$(dirname "$0")" && pwd)"

echo "============================================================"
echo "Exp3: Soundness Boundary (Observational vs Strict)"
echo "============================================================"
echo ""
echo "Test: foo() writes h=99 then restores the value it found. Assigns only declares g."
echo ""

# === ESBMC ===
echo "--- ESBMC (observational compliance) ---"
esbmc_out=$("$ESBMC" "$DIR/test_temp_modify_esbmc.c" \
    --function foo --enforce-contract foo 2>&1)
esbmc_result="ERROR"
echo "$esbmc_out" | grep -q "VERIFICATION SUCCESSFUL" && esbmc_result="PASS"
echo "$esbmc_out" | grep -q "VERIFICATION FAILED" && esbmc_result="FAIL"
echo "  Result: $esbmc_result"
echo "  (h holds its entry value at exit, so observational compliance holds)"
echo ""

# === CBMC ===
echo "--- CBMC (strict write-set compliance / DFCC) ---"
tmp_goto=$(mktemp /tmp/cbmc_XXXXX.goto)
tmp_instr=$(mktemp /tmp/cbmc_instr_XXXXX.goto)

"$GOTO_CC" "$DIR/test_temp_modify_cbmc.c" -o "$tmp_goto" 2>/dev/null
"$GOTO_INSTRUMENT" "$tmp_goto" "$tmp_instr" \
    --dfcc main --enforce-contract foo 2>/dev/null
cbmc_out=$("$CBMC" "$tmp_instr" 2>&1)

cbmc_result="ERROR"
echo "$cbmc_out" | grep -q "VERIFICATION SUCCESSFUL" && cbmc_result="PASS"
echo "$cbmc_out" | grep -q "VERIFICATION FAILED" && cbmc_result="FAIL"
echo "  Result: $cbmc_result"

# Show the specific violation
if [ "$cbmc_result" = "FAIL" ]; then
    echo "  Violations found:"
    echo "$cbmc_out" | grep -A1 "FAILURE" | head -6
fi
echo ""

echo "--- Summary ---"
echo "ESBMC: $esbmc_result (observational: h is restored, compliance OK)"
echo "CBMC:  $cbmc_result (strict: h was written, violation detected)"
echo ""
echo "This demonstrates the key semantic difference:"
echo "- Observational (ESBMC): only checks final state at function boundary"
echo "- Strict (CBMC DFCC): instruments every write, detects ANY modification"

rm -f "$tmp_goto" "$tmp_instr"

# Save full outputs
echo "$esbmc_out" > "$DIR/esbmc_output.txt"
echo "$cbmc_out" > "$DIR/cbmc_output.txt"
