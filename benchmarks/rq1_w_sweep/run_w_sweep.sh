#!/usr/bin/env bash
# RQ1, Exp 1a: VCC count and time against the number of sequential writes W.
#
# The program is the one the paper defines: G=5 globals, one assigns target
# (g0), one ensures clause, and W writes to g0. ESBMC's count stays at
# (G-M)+E = 5; CBMC's DFCC count is 20+W+E.
#
# Written on 2026-10-13 to regenerate those programs. The driver that wrote
# results/rq1_w_sweep.csv and results/rq1_cbmc.csv on 2026-09-03 was not kept;
# this script reproduces its column layout.
#
#   ./run_w_sweep.sh [out-dir]      REPS=5 by default
set -u
ESBMC=${ESBMC:-esbmc}
OUT=${1:-.}; REPS=${REPS:-5}
DIR="$(cd "$(dirname "$0")" && pwd)"; GEN="$DIR/generated"; mkdir -p "$GEN" "$OUT"
E_CSV="$OUT/rq1_w_sweep.csv"; C_CSV="$OUT/rq1_cbmc.csv"
echo "tool,W,rep,vccs,wall_ms,result" > "$E_CSV"
echo "tool,W,rep,props,wall_ms,result" > "$C_CSV"

gen(){ # W tool -> file
  local W=$1 tool=$2 f="$GEN/w_${1}_$2.c" i
  {
    echo "int g0 = 0, g1 = 0, g2 = 0, g3 = 0, g4 = 0;"
    if [ "$tool" = esbmc ]; then
      echo "void foo(void)"; echo "{"
      echo "    __ESBMC_assigns(g0);"; echo "    __ESBMC_ensures(g0 == $W);"
    else
      echo "void foo(void)"; echo "__CPROVER_assigns(g0)"; echo "__CPROVER_ensures(g0 == $W)"; echo "{"
    fi
    for i in $(seq 1 "$W"); do echo "    g0 = $i;"; done
    echo "}"
    [ "$tool" = cbmc ] && echo "int main(void) { foo(); return 0; }"
  } > "$f"
  echo "$f"
}
ms(){ date +%s%3N; }
verdict(){ grep -oE "VERIFICATION (SUCCESSFUL|FAILED)" | head -1; }

for rep in $(seq 1 "$REPS"); do
  for W in 1 10 50 100 200 500; do
    f=$(gen "$W" esbmc); s=$(ms)
    out=$("$ESBMC" "$f" --function foo --enforce-contract foo 2>&1); e=$(ms)
    echo "esbmc,$W,$rep,$(echo "$out" | grep -oP 'Generated \K[0-9]+(?= VCC)'),$((e-s)),$(echo "$out" | verdict)" | tee -a "$E_CSV"
    f=$(gen "$W" cbmc); s=$(ms)
    goto-cc "$f" -o "$GEN/w.goto" && goto-instrument "$GEN/w.goto" "$GEN/w_dfcc.goto" --dfcc main --enforce-contract foo >/dev/null
    out=$(cbmc "$GEN/w_dfcc.goto" 2>&1); e=$(ms)
    echo "cbmc,$W,$rep,$(echo "$out" | grep -oP 'of \K[0-9]+(?= failed)' | head -1),$((e-s)),$(echo "$out" | verdict)" | tee -a "$C_CSV"
  done
done
