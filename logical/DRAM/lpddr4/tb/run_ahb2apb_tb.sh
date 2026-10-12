#!/bin/bash
# Unit test for megasoc_ahb_to_apb_async: compile once, run a clock-ratio matrix.
# Usage: ./run_ahb2apb_tb.sh [n_txn]     (needs VCS on PATH)
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
L=$HERE/../../..                      # megasoc_tech/logical
N=${1:-2000}
OUT=${AHB2APB_TB_OUT:-$HERE/out/ahb2apb}
mkdir -p "$OUT" && cd "$OUT"
vcs -full64 -sverilog -timescale=1ns/1ps -l compile.log \
    $L/interfaces/ahb_if.sv $L/interfaces/apb3_if.sv $L/interfaces/apb4_if.sv \
    $L/DRAM/lpddr4/megasoc_ahb_to_apb_async.sv \
    $HERE/megasoc_ahb_to_apb_async_tb.sv -o simv > /dev/null
#        h_half  p_half  seed   (half periods in ns)
cases=( "10.0   1.25    1"     # MegaSoC case: 50 MHz AHB, 400 MHz APB
        "1.25   10.0    2"     # reverse
        "5.0    4.9     3"     # near-equal, drifting
        "3.5    1.65    4"     # non-integer ratio
        "1.0    1.0     5"     # same frequency, different phase
        "10.0   1.25    6" )   # MegaSoC case, another seed
fail=0
for c in "${cases[@]}"; do
    set -- $c
    r=$(./simv +H_HALF=$1 +P_HALF=$2 +SEED=$3 +ntb_random_seed=$3 +N_TXN=$N -l run_$3.log | grep -E "AHB2APB_TB: (PASS|FAIL)" || echo "AHB2APB_TB: FAIL (no result)")
    chk=$(grep -E "AHB2APB_TB: [0-9]+ transfers" run_$3.log | sed 's/AHB2APB_TB: //')
    printf "hclk %-6s pclk %-6s seed %s : %s  [%s]\n" "$(echo "2*$1"|bc)ns" "$(echo "2*$2"|bc)ns" "$3" "$r" "$chk"
    case "$r" in *PASS*) ;; *) fail=1 ;; esac
done
[ $fail -eq 0 ] && echo "ALL AHB2APB CASES PASS" || { echo "SOME AHB2APB CASES FAILED (see $OUT/run_*.log)"; exit 1; }
