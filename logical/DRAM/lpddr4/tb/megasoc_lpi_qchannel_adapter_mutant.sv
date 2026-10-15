// Mutation check for megasoc_lpi_qchannel_adapter_tb: the plain renaming the
// subsystem wrapper uses (QACCEPTn = CSYSACK, QDENY = 0). The testbench must
// FAIL on this; run_lpiq_tb.sh checks that it does.
module megasoc_lpi_qchannel_adapter (
    input  wire          clk,
    input  wire          resetn,
    qchannel.subordinate CTRL,
    qchannel.master      DEV
);
assign DEV.qreqn     = CTRL.qreqn;
assign CTRL.qacceptn = DEV.qacceptn;
assign CTRL.qdeny    = 1'b0;
assign CTRL.qactive  = DEV.qactive;
endmodule
