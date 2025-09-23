
module megasoc_dram_wrapper #(
    parameter ID_W=6
    )(
    input  wire             ACLK,
    input  wire             ARESETn,

    axi4.subordinate        DRAM_AXI,

    // DDR4 signals
    output wire             DDR4_CK_T,
    output wire             DDR4_CK_C,

    output wire [16:0]      DDR4_ADR,
    output wire [1:0]       DDR4_BA,
    output wire [1:0]       DDR4_BG,

    output wire             DDR4_ACT_n,
    output wire [1:0]       DDR4_CKE,
    output wire [1:0]       DDR4_CS_N,
    output wire [1:0]       DDR4_ODT,
    output wire             DDR_PARITY,

    inout  wire [63:0]      DDR4_DQ,
    inout  wire [7:0]       DDR4_DM_DBI_N,
    inout  wire [7:0]       DDR4_DQS_T,
    inout  wire [7:0]       DDR4_DQS_C,

    output wire             DDR4_RESET_N,

    input  wire             DDR_nALERT,
    input  wire             DDR_nEVENT
);




endmodule 
