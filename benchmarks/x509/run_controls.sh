#!/bin/bash
#
# run_controls.sh — negative controls for the X.509 propagation experiment.
#
# run_experiment3.sh reports seven scenarios, all passing. All-passing is also
# what a vacuous experiment looks like, and that is not hypothetical here: the
# ESBMC subject carried `*data_len <= 125` where the body can produce 127, and
# `parse_x509_cert` carried no `ensures` at all, so scenario D2 was enforcing
# requires and assigns against nothing. Both survived because nothing ever
# checked that a passing scenario could fail.
#
# Each scenario below gets one mutation the configuration should reject. A
# scenario whose control does not fire is not evidence of anything.
#
# Usage: ./run_controls.sh [path-to-esbmc]

set -u
ESBMC="${1:-${ESBMC:-esbmc}}"
DIR="$(cd "$(dirname "$0")" && pwd)"
SRC="$DIR/x509_esbmc.c"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

R=--replace-call-with-contract

mutate() { # <find> <replace> <outfile>
  python3 - "$1" "$2" "$WORK/$3" "$SRC" <<'PY' || { echo "could not write mutant $3" >&2; exit 1; }
import sys
find, repl, out, src = sys.argv[1:5]
s = open(src).read()
assert find in s, "control pattern not found, the subject has moved: " + find
open(out, "w").write(s.replace(find, repl, 1))
PY
}

check() { # <label> <description> <mutant> <esbmc args...>
  printf "  %-4s %-32s " "$1" "$2"
  local m="$3"; shift 3
  local out; out=$("$ESBMC" "$WORK/$m" "$@" --unwind 5 2>&1)
  case "$out" in
    *"VERIFICATION FAILED"*)     echo "FAIL  ok" ;;
    *"VERIFICATION SUCCESSFUL"*) echo "PASS  <-- control did NOT fire"; FAILED=1 ;;
    *)                           echo "ERROR <-- no verdict, so no control"; FAILED=1 ;;
  esac
}

mutate "__ESBMC_assert(ret <= 1"  "__ESBMC_assert(ret <= 0"  m_ret.c
mutate "__ESBMC_assert(eaten > 0" "__ESBMC_assert(eaten > 1" m_eaten.c
mutate "(*hdr_len == 2)"          "(*hdr_len == 3)"          m_hdr.c
mutate "(*eaten <= len)"          "(*eaten < len)"           m_elen.c
mutate "__ESBMC_return_value == 0 || __ESBMC_return_value == -1);" \
       "__ESBMC_return_value == 0);"                         m_rv.c

FAILED=0
echo "Negative controls (every line must read FAIL):"

# A and B run parse_x509_cert_relaxed's real body, so the mutation has to be
# one that body can violate. C and D4 abstract it, so its contract's weaker
# guarantee (eaten > 0, not eaten >= 2) is what the harness assertion probes.
check A  "assert ret<=1 -> <=0"  m_ret.c
check B  "assert ret<=1 -> <=0"  m_ret.c   $R parse_id_len $R parse_x509_cert
check C  "assert eaten>0 -> >1"  m_eaten.c $R '*'
check D1 "hdr_len==2 -> ==3"     m_hdr.c   --enforce-contract parse_id_len \
                                           --function parse_id_len
check D2 "RV in {0,-1} -> ==0"   m_rv.c    --enforce-contract parse_x509_cert \
                                           --function parse_x509_cert
check D3 "eaten<=len -> <len"    m_elen.c  $R parse_id_len $R parse_x509_cert \
                                           --enforce-contract parse_x509_cert_relaxed \
                                           --function parse_x509_cert_relaxed
check D4 "assert eaten>0 -> >1"  m_eaten.c $R parse_id_len $R parse_x509_cert \
                                           $R parse_x509_cert_relaxed

echo
[ $FAILED -eq 0 ] && echo "All seven controls fired." \
                  || echo "A control did not fire; the matching scenario proves nothing."
exit $FAILED
