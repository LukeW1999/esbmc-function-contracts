#!/bin/bash
# Experiment 2: Contract Verification Efficiency Comparison
#
# Comparing three verification modes:
# 1. Direct run (no contract): esbmc 1_no_contract.c
# 2. Enforce(--function) + Replace: enforce from --function entry + replace
# 3. Enforce(from main) + Replace: enforce from main entry (using 1_1.c) + replace

ESBMC=${ESBMC:-esbmc}
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FUNC_NAME="add_and_check"

echo "=============================================================="
echo "Experiment 2: Contract Verification Efficiency Comparison"
echo "=============================================================="
echo ""

# Timed execution function (returns milliseconds)
run_timed() {
    local cmd="$1"
    local start=$(date +%s%3N)
    eval "$cmd" > /dev/null 2>&1
    local result=$?
    local end=$(date +%s%3N)
    local elapsed=$((end - start))
    echo $elapsed
}

echo "=== Tool Versions ==="
$ESBMC --version 2>&1 | head -1
cbmc --version 2>&1 | head -1
echo ""

######################################################################
# ESBMC Single Call Test
######################################################################
echo "=============================================================="
echo "ESBMC Single Call Test"
echo "=============================================================="
cd "$SCRIPT_DIR/esbmc/single_call"

echo ""
echo "--- Mode 1: Direct run (no contract) ---"
T1=$(run_timed "$ESBMC 1_no_contract.c")
echo "Time: ${T1}ms"

echo ""
echo "--- Mode 2: Enforce(--function) + Replace ---"
T2_E=$(run_timed "$ESBMC 1.c --enforce-contract $FUNC_NAME --function $FUNC_NAME")
T2_R=$(run_timed "$ESBMC 1.c --replace-call-with-contract $FUNC_NAME")
T2=$((T2_E + T2_R))
echo "Enforce time: ${T2_E}ms"
echo "Replace time: ${T2_R}ms"
echo "Total time: ${T2}ms"

echo ""
echo "--- Mode 3: Enforce(from main, 1_1.c) + Replace ---"
T3_E=$(run_timed "$ESBMC 1_1.c --enforce-contract $FUNC_NAME")
T3_R=$(run_timed "$ESBMC 1.c --replace-call-with-contract $FUNC_NAME")
T3=$((T3_E + T3_R))
echo "Enforce time: ${T3_E}ms"
echo "Replace time: ${T3_R}ms"
echo "Total time: ${T3}ms"

ESBMC_SINGLE_T1=$T1
ESBMC_SINGLE_T2=$T2
ESBMC_SINGLE_T3=$T3
ESBMC_SINGLE_T2_E=$T2_E
ESBMC_SINGLE_T3_E=$T3_E

######################################################################
# ESBMC Multiple Calls Test
######################################################################
echo ""
echo "=============================================================="
echo "ESBMC Multiple Calls Test (10 calls)"
echo "=============================================================="
cd "$SCRIPT_DIR/esbmc/multiple_calls"

echo ""
echo "--- Mode 1: Direct run (no contract) ---"
T1=$(run_timed "$ESBMC 1_no_contract.c")
echo "Time: ${T1}ms"

echo ""
echo "--- Mode 2: Enforce(--function) + Replace ---"
T2_E=$(run_timed "$ESBMC 1.c --enforce-contract $FUNC_NAME --function $FUNC_NAME")
T2_R=$(run_timed "$ESBMC 1.c --replace-call-with-contract $FUNC_NAME")
T2=$((T2_E + T2_R))
echo "Enforce time: ${T2_E}ms"
echo "Replace time: ${T2_R}ms"
echo "Total time: ${T2}ms"

echo ""
echo "--- Mode 3: Enforce(from main, 1_1.c) + Replace ---"
T3_E=$(run_timed "$ESBMC 1_1.c --enforce-contract $FUNC_NAME")
T3_R=$(run_timed "$ESBMC 1.c --replace-call-with-contract $FUNC_NAME")
T3=$((T3_E + T3_R))
echo "Enforce time: ${T3_E}ms"
echo "Replace time: ${T3_R}ms"
echo "Total time: ${T3}ms"

ESBMC_MULTI_T1=$T1
ESBMC_MULTI_T2=$T2
ESBMC_MULTI_T3=$T3
ESBMC_MULTI_T2_E=$T2_E
ESBMC_MULTI_T3_E=$T3_E

######################################################################
# CBMC Single Call Test
######################################################################
echo ""
echo "=============================================================="
echo "CBMC Single Call Test"
echo "=============================================================="
cd "$SCRIPT_DIR/cbmc/single_call"
rm -f *.goto

echo ""
echo "--- Mode 1: Direct run (no contract) ---"
goto-cc 1_no_contract.c -o 1_nc.goto 2>/dev/null
T1=$(run_timed "cbmc 1_nc.goto")
echo "Time: ${T1}ms"

echo ""
echo "--- Mode 2: Enforce(--function) + Replace ---"
goto-cc 1.c -o 1.goto 2>/dev/null
goto-instrument --enforce-contract $FUNC_NAME 1.goto 1_e.goto 2>/dev/null
T2_E=$(run_timed "cbmc --function $FUNC_NAME 1_e.goto")
goto-instrument --replace-call-with-contract $FUNC_NAME 1.goto 1_r.goto 2>/dev/null
T2_R=$(run_timed "cbmc 1_r.goto")
T2=$((T2_E + T2_R))
echo "Enforce time: ${T2_E}ms"
echo "Replace time: ${T2_R}ms"
echo "Total time: ${T2}ms"

echo ""
echo "--- Mode 3: Enforce(from main, 1_1.c) + Replace ---"
goto-cc 1_1.c -o 1_1.goto 2>/dev/null
goto-instrument --enforce-contract $FUNC_NAME 1_1.goto 1_1_e.goto 2>/dev/null
T3_E=$(run_timed "cbmc 1_1_e.goto")
T3_R=$(run_timed "cbmc 1_r.goto")
T3=$((T3_E + T3_R))
echo "Enforce time: ${T3_E}ms"
echo "Replace time: ${T3_R}ms"
echo "Total time: ${T3}ms"

CBMC_SINGLE_T1=$T1
CBMC_SINGLE_T2=$T2
CBMC_SINGLE_T3=$T3
CBMC_SINGLE_T2_E=$T2_E
CBMC_SINGLE_T3_E=$T3_E

rm -f *.goto

######################################################################
# CBMC Multiple Calls Test
######################################################################
echo ""
echo "=============================================================="
echo "CBMC Multiple Calls Test (10 calls)"
echo "=============================================================="
cd "$SCRIPT_DIR/cbmc/multiple_calls"
rm -f *.goto

echo ""
echo "--- Mode 1: Direct run (no contract) ---"
goto-cc 1_no_contract.c -o 1_nc.goto 2>/dev/null
T1=$(run_timed "cbmc 1_nc.goto")
echo "Time: ${T1}ms"

echo ""
echo "--- Mode 2: Enforce(--function) + Replace ---"
goto-cc 1.c -o 1.goto 2>/dev/null
goto-instrument --enforce-contract $FUNC_NAME 1.goto 1_e.goto 2>/dev/null
T2_E=$(run_timed "cbmc --function $FUNC_NAME 1_e.goto")
goto-instrument --replace-call-with-contract $FUNC_NAME 1.goto 1_r.goto 2>/dev/null
T2_R=$(run_timed "cbmc 1_r.goto")
T2=$((T2_E + T2_R))
echo "Enforce time: ${T2_E}ms"
echo "Replace time: ${T2_R}ms"
echo "Total time: ${T2}ms"

echo ""
echo "--- Mode 3: Enforce(from main, 1_1.c) + Replace ---"
goto-cc 1_1.c -o 1_1.goto 2>/dev/null
goto-instrument --enforce-contract $FUNC_NAME 1_1.goto 1_1_e.goto 2>/dev/null
T3_E=$(run_timed "cbmc 1_1_e.goto")
T3_R=$(run_timed "cbmc 1_r.goto")
T3=$((T3_E + T3_R))
echo "Enforce time: ${T3_E}ms"
echo "Replace time: ${T3_R}ms"
echo "Total time: ${T3}ms"

CBMC_MULTI_T1=$T1
CBMC_MULTI_T2=$T2
CBMC_MULTI_T3=$T3
CBMC_MULTI_T2_E=$T2_E
CBMC_MULTI_T3_E=$T3_E

rm -f *.goto

######################################################################
# Results Summary
######################################################################
echo ""
echo "=============================================================="
echo "                      Results Summary"
echo "=============================================================="
echo ""
echo "=== ESBMC ==="
printf "%-30s | %-15s | %-15s\n" "Mode" "Single Call" "Multiple (10x)"
printf "%-30s-+-%-15s-+-%-15s\n" "------------------------------" "---------------" "---------------"
printf "%-30s | %12sms | %12sms\n" "1. Direct run (no contract)" "$ESBMC_SINGLE_T1" "$ESBMC_MULTI_T1"
printf "%-30s | %12sms | %12sms\n" "2. Enforce(--function)+Replace" "$ESBMC_SINGLE_T2" "$ESBMC_MULTI_T2"
printf "%-30s | %12sms | %12sms\n" "3. Enforce(from main)+Replace" "$ESBMC_SINGLE_T3" "$ESBMC_MULTI_T3"

echo ""
echo "=== CBMC ==="
printf "%-30s | %-15s | %-15s\n" "Mode" "Single Call" "Multiple (10x)"
printf "%-30s-+-%-15s-+-%-15s\n" "------------------------------" "---------------" "---------------"
printf "%-30s | %12sms | %12sms\n" "1. Direct run (no contract)" "$CBMC_SINGLE_T1" "$CBMC_MULTI_T1"
printf "%-30s | %12sms | %12sms\n" "2. Enforce(--function)+Replace" "$CBMC_SINGLE_T2" "$CBMC_MULTI_T2"
printf "%-30s | %12sms | %12sms\n" "3. Enforce(from main)+Replace" "$CBMC_SINGLE_T3" "$CBMC_MULTI_T3"

echo ""
echo "=== Enforce Entry Point Comparison (--function entry vs from main entry) ==="
printf "%-10s | %-25s | %-25s\n" "Tool" "Single Call" "Multiple Calls"
printf "%-10s-+-%-25s-+-%-25s\n" "----------" "-------------------------" "-------------------------"
printf "%-10s | --func:%5sms main:%5sms | --func:%5sms main:%5sms\n" "ESBMC" "$ESBMC_SINGLE_T2_E" "$ESBMC_SINGLE_T3_E" "$ESBMC_MULTI_T2_E" "$ESBMC_MULTI_T3_E"
printf "%-10s | --func:%5sms main:%5sms | --func:%5sms main:%5sms\n" "CBMC" "$CBMC_SINGLE_T2_E" "$CBMC_SINGLE_T3_E" "$CBMC_MULTI_T2_E" "$CBMC_MULTI_T3_E"

echo ""
echo "=============================================================="
