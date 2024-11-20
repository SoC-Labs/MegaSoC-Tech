
module megasoc_dram_wrapper #(
    parameter ID_W=6
    )(
    input  wire             ACLK,
    input  wire             ARESETn,

    input  wire [ID_W-1:0]  AWID,
    input  wire [31:0]      AWADDR,
    input  wire [7:0]       AWLEN,
    input  wire [2:0]       AWSIZE,
    input  wire [1:0]       AWBURST,
    input  wire             AWLOCK,
    input  wire [3:0]       AWCACHE,
    input  wire [2:0]       AWPROT,
    input  wire             AWVALID,
    output wire             AWREADY,

    input  wire [63:0]      WDATA,
    input  wire [7:0]       WSTRB,
    input  wire             WLAST,
    input  wire             WVALID,
    output wire             WREADY,

    output wire [ID_W-1:0]  BID,
    output wire [1:0]       BRESP,
    output wire             BVALID,
    input  wire             BREADY,
    
    input  wire [ID_W-1:0]  ARID,
    input  wire [31:0]      ARADDR,
    input  wire [7:0]       ARLEN,
    input  wire [2:0]       ARSIZE,
    input  wire [1:0]       ARBURST,
    input  wire             ARLOCK,
    input  wire [3:0]       ARCACHE,
    input  wire [2:0]       ARPROT,
    input  wire             ARVALID,
    output wire             ARREADY,
    
    output wire [ID_W-1:0]  RID,
    output wire [63:0]      RDATA,
    output wire [1:0]       RRESP,
    output wire             RLAST,
    output wire             RVALID,
    input  wire             RREADY,

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

    input  wire [63:0]      DDR4_DQ_I,
    output wire [63:0]      DDR4_DQ_O,
    output wire             DDR4_DQ_E,
    input  wire [7:0]       DDR4_DM_DBI_N_I,
    output wire [7:0]       DDR4_DM_DBI_N_O,
    output wire             DDR4_DM_DBI_E,
    input  wire [7:0]       DDR4_DQS_T_I,
    output wire [7:0]       DDR4_DQS_T_O,
    output wire             DDR4_DQS_T_E,
    input  wire [7:0]       DDR4_DQS_C_I,
    output wire [7:0]       DDR4_DQS_C_O,
    output wire             DDR4_DQS_C_E,

    output wire             DDR4_RESET_N,

    input  wire             DDR_nALERT,
    input  wire             DDR_nEVENT
);




endmodule 