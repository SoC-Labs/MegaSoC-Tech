#!/bin/bash
# Unit test for megasoc_axi_async_bridge: compile once, run a clock-ratio matrix.
# Usage: ./run_bridge_tb.sh [n_txn]     (needs VCS on PATH)
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
L=$HERE/../../..                      # megasoc_tech/logical
N=${1:-300}
OUT=${BRIDGE_TB_OUT:-$HERE/out}
mkdir -p "$OUT" && cd "$OUT"
vcs -full64 -sverilog -timescale=1ns/1ps -l compile.log \
    $L/interfaces/axi4_if.sv \
    $L/DRAM/lpddr4/megasoc_async_fifo.sv \
    $L/DRAM/lpddr4/megasoc_axi_async_bridge.sv \
    $HERE/megasoc_axi_async_bridge_tb.sv -o simv > /dev/null
#        s_half  m_half  seed   (half periods in ns)
cases=( "10.0   1.25    1"     # MegaSoC case: 50 MHz fabric, 400 MHz controller
        "1.25   10.0    2"     # reverse: fabric 8x faster
        "5.0    4.9     3"     # near-equal, slowly drifting
        "3.5    1.65    4"     # non-integer ratio
        "1.0    1.0     5"     # same frequency, different phase
        "10.0   1.25    6"     # MegaSoC case, another seed
        "2.2    7.3     7" )
fail=0
for c in "${cases[@]}"; do
    set -- $c
    r=$(./simv +S_HALF=$1 +M_HALF=$2 +SEED=$3 +ntb_random_seed=$3 +N_TXN=$N -l run_$3.log | grep -E "BRIDGE_TB: (PASS|FAIL)" || echo "BRIDGE_TB: FAIL (no result)")
    chk=$(grep "BRIDGE_TB: checked" run_$3.log | sed 's/BRIDGE_TB: //')
    printf "s %-5s m %-5s seed %s : %s  [%s]\n" "$(echo "2*$1"|bc)ns" "$(echo "2*$2"|bc)ns" "$3" "$r" "$chk"
    case "$r" in *PASS*) ;; *) fail=1 ;; esac
done
[ $fail -eq 0 ] && echo "ALL BRIDGE CASES PASS" || { echo "SOME BRIDGE CASES FAILED (see $OUT/run_*.log)"; exit 1; }
