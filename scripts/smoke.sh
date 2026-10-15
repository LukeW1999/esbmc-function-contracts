#!/usr/bin/env bash
# The three commands the paper prints, on the example it prints them for.
# Needs `esbmc` on PATH at the measured commit (see README for why releases fail).
set -u
cd "$(dirname "$0")/../examples"
fail=0
run() {
  want=$1; shift
  printf '$ esbmc %s\n' "$*"
  got=$(esbmc "$@" 2>&1)
  echo "$got" | grep -E 'Generated [0-9]+ VCC|properties failed|VERIFICATION'
  echo "$got" | grep -q "VERIFICATION $want" || { echo "  EXPECTED VERIFICATION $want"; fail=1; }
  echo
}
run SUCCESSFUL prog.c --enforce-contract compute --function compute --unwind 9
run FAILED     bad.c  --enforce-contract compute --function compute --unwind 9
run SUCCESSFUL prog.c --replace-call-with-contract compute --unwind 9
[ $fail -eq 0 ] && echo "smoke: ok" || { echo "smoke: FAILED"; exit 1; }
