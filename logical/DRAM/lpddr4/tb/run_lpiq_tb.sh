#!/bin/bash
# Unit test for megasoc_lpi_qchannel_adapter: clock-ratio matrix, a reset-in-
# Q_STOPPED case, and a mutation check (the plain wrapper mapping must FAIL).
# Usage: ./run_lpiq_tb.sh [n_req]     (needs VCS on PATH)
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
L=$HERE/../../..                      # megasoc_tech/logical
N=${1:-300}
OUT=${LPIQ_TB_OUT:-$HERE/out/lpiq}
mkdir -p "$OUT" && cd "$OUT"
vcs -full64 -sverilog -timescale=1ns/1ps -l compile.log \
    $L/interfaces/qchannel_if.sv $L/DRAM/lpddr4/megasoc_lpi_qchannel_adapter.sv \
    $HERE/megasoc_lpi_qchannel_adapter_tb.sv -o simv > /dev/null
vcs -full64 -sverilog -timescale=1ns/1ps -l compile_mutant.log \
    $L/interfaces/qchannel_if.sv $HERE/megasoc_lpi_qchannel_adapter_mutant.sv \
    $HERE/megasoc_lpi_qchannel_adapter_tb.sv -o simv_mutant > /dev/null
#        p_half  d_half  seed  extra        (half periods in ns)
cases=( "5.0     1.25    1     "            # MegaSoC: 100 MHz power controller, 400 MHz DDR
        "5.0     1.25    2     +DENY_PCT=0" # never refused
        "5.0     1.25    3     +DENY_PCT=90"
        "1.25    5.0     4     "            # reverse ratio
        "3.3     3.1     5     "            # near-equal, drifting
        "5.0     1.25    6     +RESET_STOPPED"
        "1.25    5.0     7     +RESET_STOPPED" )
fail=0
run() {  # simv label args...
    local sim=$1 tag=$2; shift 2
    ./$sim "$@" +N_REQ=$N -l run_$tag.log > /dev/null 2>&1 || true
    grep -E "LPIQ_TB: (PASS|FAIL)" run_$tag.log || echo "LPIQ_TB: FAIL (no result)"
}
for c in "${cases[@]}"; do
    set -- $c
    r=$(run simv $3 +P_HALF=$1 +D_HALF=$2 +SEED=$3 +ntb_random_seed=$3 $4)
    s=$(grep -E "LPIQ_TB: [0-9]+ requests" run_$3.log | sed 's/LPIQ_TB: //')
    printf "pclk %-6s dclk %-6s seed %s %-15s: %s  [%s]\n" "$(echo "2*$1"|bc)ns" "$(echo "2*$2"|bc)ns" "$3" "$4" "$r" "$s"
    case "$r" in *PASS*) ;; *) fail=1 ;; esac
done
m=$(run simv_mutant mutant +P_HALF=5.0 +D_HALF=1.25 +SEED=1 +ntb_random_seed=1)
case "$m" in
    *FAIL*) echo "mutation (plain wrapper mapping): detected  [$m]" ;;
    *)      echo "mutation (plain wrapper mapping): NOT detected  [$m]"; fail=1 ;;
esac
[ $fail -eq 0 ] && echo "ALL LPIQ CASES PASS" || { echo "SOME LPIQ CASES FAILED (see $OUT/run_*.log)"; exit 1; }
