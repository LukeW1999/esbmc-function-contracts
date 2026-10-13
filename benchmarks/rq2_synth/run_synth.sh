#!/usr/bin/env bash
# RQ2 synthetic sweep over N globals and K call sites:
# Direct uses a contract-free build; Enforce/Replace use -DCONTRACTS.
# compute() exercises assigns(g_processed).
set -u
ESBMC=${ESBMC:-esbmc}
SRC=synth_compute.c
TO=120
NS="${NS:-5 10 20 50}"
KS="${KS:-1 3 5 10}"
OUT=results_synth_esbmc.csv
echo "mode,N,K,result,bmc_time_s,wall_s" > "$OUT"

run(){ # mode N K  -- rest: esbmc args
  local mode=$1 N=$2 K=$3; shift 3
  local s e wall out rc t res
  s=$(date +%s.%N); out=$(timeout $TO $ESBMC "$@" 2>&1); rc=$?; e=$(date +%s.%N)
  wall=$(awk "BEGIN{printf \"%.2f\", $e-$s}")
  if [ $rc -eq 124 ]; then res=TIMEOUT; t=">$TO"; else
     res=$(echo "$out"|grep -oE "VERIFICATION (SUCCESSFUL|FAILED)"|head -1)
     t=$(echo "$out"|grep -oE "BMC program time: [0-9.]+s"|grep -oE "[0-9.]+"); fi
  echo "$mode,$N,$K,${res:-ERR},${t:-NA},$wall" | tee -a "$OUT"
}

for N in $NS; do
  run Enforce "$N" "-" "$SRC" -DCONTRACTS -DN=$N -DK=1 --enforce-contract compute --function compute --unwind $((N+1))
  for K in $KS; do
    U=$(( (N>K?N:K) + 1 ))            # cover inner loop (N) and call loop (K)
    run Direct  "$N" "$K" "$SRC" -DHARNESS -DN=$N -DK=$K --unwind $U
    run Replace "$N" "$K" "$SRC" -DHARNESS -DCONTRACTS -DN=$N -DK=$K --replace-call-with-contract compute --unwind $U
  done
done
echo "DONE -> $OUT"
