#!/usr/bin/env bash
#
# run_controls.sh — negative controls for the ML-KEM poly_reduce study.
#
# run_esbmc.sh reports 28 cells, every one VERIFICATION SUCCESSFUL. That is
# also what a vacuous experiment looks like, and it is not hypothetical: the
# X.509 study in this repository carried a false postcondition and a
# contract-free scenario for three months, both hidden behind an all-passing
# table.
#
# Four mutations, one per thing the study claims:
#   Enforce  — the leaf really is checked against its own postcondition
#   Replace  — the caller's goal really is checked
#   Replace  — the caller's proof really goes through the stub, not the body
#   Direct   — the monolithic run really checks the same goal
#
# The third is the one that catches a silently degraded run: if
# --replace-call-with-contract quietly does nothing, the body is inlined, the
# goal still holds, and the cell still passes. Weakening the stub separates
# those two worlds.
#
# Usage: ./run_controls.sh [path-to-esbmc]

set -u
ESBMC="${1:-${ESBMC:-esbmc}}"
DIR="$(cd "$(dirname "$0")" && pwd)"
SRC="$DIR/poly_reduce.c"
N=16
K=3
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

mutate() { # <find> <replace> <outfile>
  python3 - "$1" "$2" "$WORK/$3" "$SRC" <<'PY' || { echo "could not write mutant $3" >&2; exit 1; }
import sys
find, repl, out, src = sys.argv[1:5]
s = open(src).read()
assert find in s, "control pattern not found, the subject has moved: " + find
open(out, "w").write(s.replace(find, repl, 1))
PY
}

check() { # <description> <mutant> <esbmc args...>
  printf "  %-40s " "$1"
  local m="$2"; shift 2
  local out; out=$("$ESBMC" "$WORK/$m" -DCONTRACTS -DMLKEM_N=$N -DMLKEM_K=$K "$@" \
    --unwind $((N + 1)) 2>&1)
  case "$out" in
    *"VERIFICATION FAILED"*)     echo "FAIL  ok" ;;
    *"VERIFICATION SUCCESSFUL"*) echo "PASS  <-- control did NOT fire"; FAILED=1 ;;
    *)                           echo "ERROR <-- no verdict, so no control"; FAILED=1 ;;
  esac
}

POLY="(r->coeffs[i] >= 0 && r->coeffs[i] < MLKEM_Q)"
VEC="(r->vec[j / MLKEM_N].coeffs[j % MLKEM_N] >= 0 &&"

# Tightening a postcondition past what the body establishes: the run that
# checks it must reject it. Weakening the leaf's postcondition instead leaves
# the caller's goal underivable from the stub.
mutate "$POLY" "(r->coeffs[i] >= 1 && r->coeffs[i] < MLKEM_Q)"      m_leaf.c
mutate "$POLY" "(r->coeffs[i] > -MLKEM_Q && r->coeffs[i] < MLKEM_Q)" m_weakstub.c
mutate "$VEC"  "(r->vec[j / MLKEM_N].coeffs[j % MLKEM_N] >= 1 &&"    m_goal.c

FAILED=0
echo "Negative controls at N=$N, K=$K (every line must read FAIL):"

check "Enforce: leaf bound >=0 -> >=1"  m_leaf.c \
      --enforce-contract mlk_poly_reduce --function mlk_poly_reduce
check "Replace: caller goal >=0 -> >=1" m_goal.c \
      --enforce-contract mlk_polyvec_reduce --function mlk_polyvec_reduce \
      --replace-call-with-contract mlk_poly_reduce
check "Replace: leaf stub weakened"     m_weakstub.c \
      --enforce-contract mlk_polyvec_reduce --function mlk_polyvec_reduce \
      --replace-call-with-contract mlk_poly_reduce
check "Direct: caller goal >=0 -> >=1"  m_goal.c \
      --enforce-contract mlk_polyvec_reduce --function mlk_polyvec_reduce

echo
[ $FAILED -eq 0 ] && echo "All four controls fired." \
                  || echo "A control did not fire; the matching column proves nothing."
exit $FAILED
