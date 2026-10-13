#!/usr/bin/env bash
# For each mutant: Direct (monolithic) verdict vs Modular (Enforce then Replace).
# Modular verdict = FAIL if EITHER Enforce or Replace fails, else PASS.
set -u
E=${ESBMC:-esbmc}
SRC=ce_agree.c
N=8; K=2; U=9
echo "mutant,direct,enforce,replace,modular"
verdict(){ # echoes PASS/FAIL/ERR from an esbmc invocation
  local out; out=$(timeout 120 $E "$@" 2>&1)
  if   echo "$out"|grep -q "VERIFICATION SUCCESSFUL"; then echo PASS
  elif echo "$out"|grep -q "VERIFICATION FAILED";    then echo FAIL
  else echo ERR; fi
}
for M in 0 1 2 3 4 5; do
  d=$(verdict $SRC -DHARNESS -DMUT=$M -DN=$N -DK=$K --unwind $U)
  e=$(verdict $SRC -DCONTRACTS -DMUT=$M -DN=$N -DK=$K --enforce-contract compute --function compute --unwind $U)
  r=$(verdict $SRC -DHARNESS -DCONTRACTS -DMUT=$M -DN=$N -DK=$K --replace-call-with-contract compute --unwind $U)
  if [ "$e" = FAIL ] || [ "$r" = FAIL ]; then mod=FAIL; else mod=PASS; fi
  echo "M$M,$d,$e,$r,$mod"
done
