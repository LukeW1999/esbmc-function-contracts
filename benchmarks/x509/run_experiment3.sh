#!/bin/bash
#
# Experiment 3: X.509 Contract Propagation Comparison - ESBMC vs CBMC
#
# Core question: When ESBMC uses --replace-call-with-contract "*" to replace all functions,
#                contract propagation fails. Can CBMC propagate correctly?
#
# Test scenarios:
# A. Direct verification (no contracts) - baseline
# B. Replace leaf functions only - ESBMC can do this
# C. Replace all functions - scenario where ESBMC fails
# D. Layered propagation: enforce leaf contracts first, then replace leaves + enforce parent

ESBMC=${ESBMC:-esbmc}
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "================================================================"
echo "  Experiment 3: X.509 Contract Propagation Comparison - ESBMC vs CBMC"
echo "================================================================"
echo ""

echo "=== Tool Versions ==="
echo -n "ESBMC: "
$ESBMC --version 2>&1 | head -1
echo -n "CBMC:  "
cbmc --version 2>&1 | head -1
echo ""

######################################################################
# ESBMC Tests
######################################################################
echo "================================================================"
echo "                        ESBMC Tests"
echo "================================================================"
cd "$SCRIPT_DIR"

echo ""
echo "--- Scenario A: Direct verification (no contracts) ---"
echo "Command: esbmc x509_esbmc.c --unwind 5"
START=$(date +%s%3N)
$ESBMC x509_esbmc.c --unwind 5 > /tmp/esbmc_A.log 2>&1
ESBMC_A=$?
END=$(date +%s%3N)
ESBMC_A_TIME=$((END - START))
echo "Result: $([ $ESBMC_A -eq 0 ] && echo 'VERIFICATION SUCCESSFUL' || echo 'VERIFICATION FAILED')"
echo "Time: ${ESBMC_A_TIME}ms"
echo ""

echo "--- Scenario B: Replace leaf functions only ---"
echo "Command: esbmc x509_esbmc.c --replace-call-with-contract parse_id_len --replace-call-with-contract parse_x509_cert --unwind 5"
START=$(date +%s%3N)
$ESBMC x509_esbmc.c --replace-call-with-contract parse_id_len --replace-call-with-contract parse_x509_cert --unwind 5 > /tmp/esbmc_B.log 2>&1
ESBMC_B=$?
END=$(date +%s%3N)
ESBMC_B_TIME=$((END - START))
echo "Result: $([ $ESBMC_B -eq 0 ] && echo 'VERIFICATION SUCCESSFUL' || echo 'VERIFICATION FAILED')"
echo "Time: ${ESBMC_B_TIME}ms"
echo ""

echo "--- Scenario C: Replace all functions (using *) ---"
echo "Command: esbmc x509_esbmc.c --replace-call-with-contract \"*\" --unwind 5"
START=$(date +%s%3N)
$ESBMC x509_esbmc.c --replace-call-with-contract "*" --unwind 5 > /tmp/esbmc_C.log 2>&1
ESBMC_C=$?
END=$(date +%s%3N)
ESBMC_C_TIME=$((END - START))
echo "Result: $([ $ESBMC_C -eq 0 ] && echo 'VERIFICATION SUCCESSFUL' || echo 'VERIFICATION FAILED')"
echo "Time: ${ESBMC_C_TIME}ms"
if [ $ESBMC_C -ne 0 ]; then
    echo "Failure details:"
    grep -E "(FAILED|counterexample|eaten|Violated)" /tmp/esbmc_C.log | head -5
fi
echo ""

echo "--- Scenario D: Layered verification ---"
echo "Step 1: enforce leaf function parse_id_len"
START=$(date +%s%3N)
$ESBMC x509_esbmc.c --enforce-contract parse_id_len --function parse_id_len --unwind 5 > /tmp/esbmc_D1.log 2>&1
ESBMC_D1=$?
END=$(date +%s%3N)
ESBMC_D1_TIME=$((END - START))
echo "  parse_id_len enforce: $([ $ESBMC_D1 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${ESBMC_D1_TIME}ms)"

echo "Step 2: enforce leaf function parse_x509_cert"
START=$(date +%s%3N)
$ESBMC x509_esbmc.c --enforce-contract parse_x509_cert --function parse_x509_cert --unwind 5 > /tmp/esbmc_D2.log 2>&1
ESBMC_D2=$?
END=$(date +%s%3N)
ESBMC_D2_TIME=$((END - START))
echo "  parse_x509_cert enforce: $([ $ESBMC_D2 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${ESBMC_D2_TIME}ms)"

echo "Step 3: replace leaves + enforce parent function parse_x509_cert_relaxed"
START=$(date +%s%3N)
$ESBMC x509_esbmc.c --replace-call-with-contract parse_id_len --replace-call-with-contract parse_x509_cert --enforce-contract parse_x509_cert_relaxed --function parse_x509_cert_relaxed --unwind 5 > /tmp/esbmc_D3.log 2>&1
ESBMC_D3=$?
END=$(date +%s%3N)
ESBMC_D3_TIME=$((END - START))
echo "  parse_x509_cert_relaxed enforce+replace: $([ $ESBMC_D3 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${ESBMC_D3_TIME}ms)"

echo "Step 4: replace all verified functions, verify main"
START=$(date +%s%3N)
$ESBMC x509_esbmc.c --replace-call-with-contract parse_id_len --replace-call-with-contract parse_x509_cert --replace-call-with-contract parse_x509_cert_relaxed --unwind 5 > /tmp/esbmc_D4.log 2>&1
ESBMC_D4=$?
END=$(date +%s%3N)
ESBMC_D4_TIME=$((END - START))
echo "  main with all replaced: $([ $ESBMC_D4 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${ESBMC_D4_TIME}ms)"
echo ""

######################################################################
# CBMC Tests
######################################################################
echo "================================================================"
echo "                        CBMC Tests"
echo "================================================================"
# The CBMC scenarios compile and delete *.goto repeatedly. Do that in a scratch
# directory: x509.goto and d4_*.goto are tracked in this one.
CBMC_WORK="$(mktemp -d)"
trap 'rm -rf "$CBMC_WORK"' EXIT
cp "$SCRIPT_DIR"/*.c "$CBMC_WORK"/
cd "$CBMC_WORK"

echo ""
echo "--- Scenario A: Direct verification (no contracts) ---"
echo "Compile: goto-cc x509_cbmc.c -o x509.goto"
goto-cc x509_cbmc.c -o x509.goto 2>/dev/null
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    goto-cc x509_cbmc.c -o x509.goto 2>&1 | tail -5
else
    echo "Command: cbmc x509.goto --unwind 5"
    START=$(date +%s%3N)
    cbmc x509.goto --unwind 5 > /tmp/cbmc_A.log 2>&1
    CBMC_A=$?
    END=$(date +%s%3N)
    CBMC_A_TIME=$((END - START))
    echo "Result: $([ $CBMC_A -eq 0 ] && echo 'VERIFICATION SUCCESSFUL' || echo 'VERIFICATION FAILED')"
    echo "Time: ${CBMC_A_TIME}ms"
fi
echo ""

echo "--- Scenario B: Replace leaf functions only ---"
rm -f *.goto
goto-cc x509_cbmc.c -o x509.goto 2>/dev/null
if [ $? -eq 0 ]; then
    goto-instrument --replace-call-with-contract parse_id_len x509.goto t1.goto 2>/dev/null
    goto-instrument --replace-call-with-contract parse_x509_cert t1.goto t2.goto 2>/dev/null
    echo "Command: cbmc t2.goto --unwind 5"
    START=$(date +%s%3N)
    cbmc t2.goto --unwind 5 > /tmp/cbmc_B.log 2>&1
    CBMC_B=$?
    END=$(date +%s%3N)
    CBMC_B_TIME=$((END - START))
    echo "Result: $([ $CBMC_B -eq 0 ] && echo 'VERIFICATION SUCCESSFUL' || echo 'VERIFICATION FAILED')"
    echo "Time: ${CBMC_B_TIME}ms"
    if [ $CBMC_B -ne 0 ]; then
        echo "Failure details:"
        grep -E "(FAILURE|FAILED|counterexample)" /tmp/cbmc_B.log | head -5
    fi
else
    echo "Compilation failed"
fi
echo ""

echo "--- Scenario C: Replace all functions (including parent) ---"
rm -f *.goto
goto-cc x509_cbmc.c -o x509.goto 2>/dev/null
if [ $? -eq 0 ]; then
    goto-instrument --replace-call-with-contract parse_id_len x509.goto c1.goto 2>/dev/null
    goto-instrument --replace-call-with-contract parse_x509_cert c1.goto c2.goto 2>/dev/null
    goto-instrument --replace-call-with-contract parse_x509_cert_relaxed c2.goto c3.goto 2>/dev/null
    echo "Command: cbmc c3.goto --unwind 5"
    START=$(date +%s%3N)
    cbmc c3.goto --unwind 5 > /tmp/cbmc_C.log 2>&1
    CBMC_C=$?
    END=$(date +%s%3N)
    CBMC_C_TIME=$((END - START))
    echo "Result: $([ $CBMC_C -eq 0 ] && echo 'VERIFICATION SUCCESSFUL' || echo 'VERIFICATION FAILED')"
    echo "Time: ${CBMC_C_TIME}ms"
    if [ $CBMC_C -ne 0 ]; then
        echo "Failure details:"
        grep -E "(FAILURE|FAILED|counterexample)" /tmp/cbmc_C.log | head -5
    fi
else
    echo "Compilation failed"
fi
echo ""

echo "--- Scenario D: Layered propagation verification ---"
rm -f *.goto

echo "Step 1: enforce leaf function parse_id_len"
goto-cc x509_cbmc.c -o x509.goto 2>/dev/null
goto-instrument --enforce-contract parse_id_len x509.goto d1_e.goto 2>/dev/null
START=$(date +%s%3N)
cbmc --function parse_id_len d1_e.goto --unwind 5 > /tmp/cbmc_D1.log 2>&1
CBMC_D1=$?
END=$(date +%s%3N)
CBMC_D1_TIME=$((END - START))
echo "  parse_id_len enforce: $([ $CBMC_D1 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${CBMC_D1_TIME}ms)"
if [ $CBMC_D1 -ne 0 ]; then
    grep -E "FAILURE" /tmp/cbmc_D1.log | head -3
fi

echo "Step 2: enforce leaf function parse_x509_cert"
rm -f *.goto
goto-cc x509_cbmc.c -o x509.goto 2>/dev/null
goto-instrument --enforce-contract parse_x509_cert x509.goto d2_e.goto 2>/dev/null
START=$(date +%s%3N)
cbmc --function parse_x509_cert d2_e.goto --unwind 5 > /tmp/cbmc_D2.log 2>&1
CBMC_D2=$?
END=$(date +%s%3N)
CBMC_D2_TIME=$((END - START))
echo "  parse_x509_cert enforce: $([ $CBMC_D2 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${CBMC_D2_TIME}ms)"
if [ $CBMC_D2 -ne 0 ]; then
    grep -E "FAILURE" /tmp/cbmc_D2.log | head -3
fi

echo "Step 3: replace leaves + enforce parent function parse_x509_cert_relaxed"
rm -f *.goto
goto-cc x509_cbmc.c -o x509.goto 2>/dev/null
goto-instrument --replace-call-with-contract parse_id_len x509.goto d3_1.goto 2>/dev/null
goto-instrument --replace-call-with-contract parse_x509_cert d3_1.goto d3_2.goto 2>/dev/null
goto-instrument --enforce-contract parse_x509_cert_relaxed d3_2.goto d3_3.goto 2>/dev/null
START=$(date +%s%3N)
cbmc --function parse_x509_cert_relaxed d3_3.goto --unwind 5 > /tmp/cbmc_D3.log 2>&1
CBMC_D3=$?
END=$(date +%s%3N)
CBMC_D3_TIME=$((END - START))
echo "  parse_x509_cert_relaxed enforce+replace: $([ $CBMC_D3 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${CBMC_D3_TIME}ms)"
if [ $CBMC_D3 -ne 0 ]; then
    echo "  Failure details:"
    grep -E "FAILURE" /tmp/cbmc_D3.log | head -5
fi

echo "Step 4: replace all verified functions, verify main"
rm -f *.goto
goto-cc x509_cbmc.c -o x509.goto 2>/dev/null
goto-instrument --replace-call-with-contract parse_id_len x509.goto d4_1.goto 2>/dev/null
goto-instrument --replace-call-with-contract parse_x509_cert d4_1.goto d4_2.goto 2>/dev/null
goto-instrument --replace-call-with-contract parse_x509_cert_relaxed d4_2.goto d4_3.goto 2>/dev/null
START=$(date +%s%3N)
cbmc d4_3.goto --unwind 5 > /tmp/cbmc_D4.log 2>&1
CBMC_D4=$?
END=$(date +%s%3N)
CBMC_D4_TIME=$((END - START))
echo "  main with all replaced: $([ $CBMC_D4 -eq 0 ] && echo 'PASS' || echo 'FAIL') (${CBMC_D4_TIME}ms)"
if [ $CBMC_D4 -ne 0 ]; then
    echo "  Failure details:"
    grep -E "FAILURE" /tmp/cbmc_D4.log | head -5
fi
echo ""

######################################################################
# Results Summary
######################################################################
echo "================================================================"
echo "                      Results Summary"
echo "================================================================"
echo ""
printf "%-45s | %-12s | %-12s\n" "Scenario" "ESBMC" "CBMC"
printf "%-45s-+-%-12s-+-%-12s\n" "---------------------------------------------" "------------" "------------"
printf "%-45s | %-12s | %-12s\n" "A. Direct verification (no contracts)" \
    "$([ $ESBMC_A -eq 0 ] && echo 'PASS' || echo 'FAIL')" \
    "$([ ${CBMC_A:-1} -eq 0 ] && echo 'PASS' || echo 'FAIL')"
printf "%-45s | %-12s | %-12s\n" "B. Replace leaf functions only" \
    "$([ $ESBMC_B -eq 0 ] && echo 'PASS' || echo 'FAIL')" \
    "$([ ${CBMC_B:-1} -eq 0 ] && echo 'PASS' || echo 'FAIL')"
printf "%-45s | %-12s | %-12s\n" "C. Replace all functions (incl. parent)" \
    "$([ $ESBMC_C -eq 0 ] && echo 'PASS' || echo 'FAIL')" \
    "$([ ${CBMC_C:-1} -eq 0 ] && echo 'PASS' || echo 'FAIL')"
printf "%-45s | %-12s | %-12s\n" "D1. enforce parse_id_len" \
    "$([ $ESBMC_D1 -eq 0 ] && echo 'PASS' || echo 'FAIL')" \
    "$([ ${CBMC_D1:-1} -eq 0 ] && echo 'PASS' || echo 'FAIL')"
printf "%-45s | %-12s | %-12s\n" "D2. enforce parse_x509_cert" \
    "$([ $ESBMC_D2 -eq 0 ] && echo 'PASS' || echo 'FAIL')" \
    "$([ ${CBMC_D2:-1} -eq 0 ] && echo 'PASS' || echo 'FAIL')"
printf "%-45s | %-12s | %-12s\n" "D3. replace leaves + enforce parent" \
    "$([ $ESBMC_D3 -eq 0 ] && echo 'PASS' || echo 'FAIL')" \
    "$([ ${CBMC_D3:-1} -eq 0 ] && echo 'PASS' || echo 'FAIL')"
printf "%-45s | %-12s | %-12s\n" "D4. replace all + verify main" \
    "$([ $ESBMC_D4 -eq 0 ] && echo 'PASS' || echo 'FAIL')" \
    "$([ ${CBMC_D4:-1} -eq 0 ] && echo 'PASS' || echo 'FAIL')"

echo ""
echo "================================================================"
echo "Log files located at /tmp/esbmc_*.log and /tmp/cbmc_*.log"
echo "================================================================"

# No cleanup: goto-cc -o overwrites, and x509.goto / d4_*.goto are tracked.
