//-----------------------------------------------------------------------------
// MegaSoC DMA-350 wrapper
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Connects the generated Arm CoreLink DMA-350 (configuration
// socrates/DMA350/config/cfg_dma_megasoc.yaml, generated into
// logical/dma350/ by `make build_dma350`) to the NIC-400:
//   APB4 target  <- NIC-400 DMA_S (64 KB window at 0x0103_0000)
//   AXI4 manager -> NIC-400 DMA_M (64-bit data, 44-bit address, 2-bit ID)
//
// Configuration in use (memory-to-memory only):
//   4 channels, 64-bit data, 44-bit address, no trigger inputs or outputs,
//   no streams, no GPO, no second AXI manager, no TrustZone support.
//   Without TrustZone support every channel is Non-secure.
//
// Port names follow the public DMA-350 TRM (102482_0000_03, Appendix A
// "Signal descriptions") and the SoC Labs SLDMA-350-Tech wrapper
// (sldma350_axi.v). Ports the configuration does not generate are not
// listed here.
//
// Integration choices:
//   - Clock/reset: the NIC DMA_M/DMA_S side (SYS_CLK, SYS_RESETn_int in
//     megasoc_tech_wrapper). APB and AXI clock enables tied high.
//   - Register window: the DMA decodes paddr[12:0] (8 KB). The NIC window is
//     64 KB, so psel is only passed on in the first 8 KB. The rest of the
//     window completes at once with read data 0 and no error, rather than
//     aliasing the DMA registers 8 times.
//   - AXI ID: the DMA drives one ID per channel. The NIC port is 2 bits wide
//     (enough for 4 channels). If the generated ID width differs, VCS reports
//     a port-width lint (PCWM) on u_dma350 awid_m0/arid_m0/rid_m0/bid_m0.
//   - AXI5 sideband outputs the NIC port has no wires for (awakeup, axqos,
//     axinner, axdomain, arcmdlink) are left open. rpoison is tied to 0.
//     The DMA never issues exclusive or locked transfers: AxLOCK = 0.
//   - Low power: Q-channel never requests quiescence (clk_qreqn = 1); the
//     P-channel never requests a change and reports ON (preq = 0,
//     pstate = ON). pwakeup follows psel. Hookup to the PCK-600 is later work.
//   - Stop/pause/CTI: not used (requests tied 0, status outputs open).
//   - Automatic boot: disabled (boot_en = 0).
//   - Interrupts: irq_channel[3:0] (one per channel) and irq_comb_nonsec,
//     level, clk domain.
//-----------------------------------------------------------------------------

module megasoc_dma350_wrapper #(
    parameter ADDR_W   = 44,
    parameter DATA_W   = 64,
    parameter ID_W     = 2,
    parameter NUM_CH   = 4
)(
    input  wire                     clk,
    input  wire                     resetn,

    // APB4 target (NIC-400 DMA_S)
    input  wire                     psel,
    input  wire                     penable,
    input  wire [31:0]              paddr,
    input  wire                     pwrite,
    input  wire [31:0]              pwdata,
    input  wire [3:0]               pstrb,
    input  wire [2:0]               pprot,
    output wire [31:0]              prdata,
    output wire                     pready,
    output wire                     pslverr,

    // AXI4 manager (NIC-400 DMA_M)
    output wire [ID_W-1:0]          awid,
    output wire [ADDR_W-1:0]        awaddr,
    output wire [7:0]               awlen,
    output wire [2:0]               awsize,
    output wire [1:0]               awburst,
    output wire                     awlock,
    output wire [3:0]               awcache,
    output wire [2:0]               awprot,
    output wire                     awvalid,
    input  wire                     awready,

    output wire [DATA_W-1:0]        wdata,
    output wire [(DATA_W/8)-1:0]    wstrb,
    output wire                     wlast,
    output wire                     wvalid,
    input  wire                     wready,

    input  wire [ID_W-1:0]          bid,
    input  wire [1:0]               bresp,
    input  wire                     bvalid,
    output wire                     bready,

    output wire [ID_W-1:0]          arid,
    output wire [ADDR_W-1:0]        araddr,
    output wire [7:0]               arlen,
    output wire [2:0]               arsize,
    output wire [1:0]               arburst,
    output wire                     arlock,
    output wire [3:0]               arcache,
    output wire [2:0]               arprot,
    output wire                     arvalid,
    input  wire                     arready,

    input  wire [ID_W-1:0]          rid,
    input  wire [DATA_W-1:0]        rdata,
    input  wire [1:0]               rresp,
    input  wire                     rlast,
    input  wire                     rvalid,
    output wire                     rready,

    // Interrupts (level)
    output wire [NUM_CH-1:0]        irq_channel,
    output wire                     irq_comb_nonsec
);

// ---------------------------------------------------------------------------
// Register window: DMA registers in the first 8 KB of the 64 KB NIC window
// ---------------------------------------------------------------------------
wire        dma_win   = (paddr[15:13] == 3'b000);
wire        dma_psel  = psel & dma_win;
wire [31:0] dma_prdata;
wire        dma_pready;
wire        dma_pslverr;

assign prdata  = dma_win ? dma_prdata  : 32'h0;
assign pready  = dma_win ? dma_pready  : 1'b1;
assign pslverr = dma_win ? dma_pslverr : 1'b0;

assign awlock = 1'b0;
assign arlock = 1'b0;

// ---------------------------------------------------------------------------
// DMA-350 (generated, Arm IP)
// ---------------------------------------------------------------------------
ada_top_sldma350_megasoc u_dma350 (
    // Clock and reset
    .clk                    (clk),
    .resetn                 (resetn),
    .aclken_m0              (1'b1),
    .pclken                 (1'b1),

    // Q-channel (clock): never request quiescence
    .clk_qreqn              (1'b1),
    .clk_qacceptn           (),
    .clk_qdeny              (),
    .clk_qactive            (),

    // P-channel (power): no requests, state ON
    .preq                   (1'b0),
    .pstate                 (4'b1000),
    .paccept                (),
    .pdeny                  (),
    .pactive                (),

    // APB4 target
    .pwakeup                (psel),
    .pdebug                 (1'b0),
    .psel                   (dma_psel),
    .penable                (penable),
    .pprot                  (pprot),
    .pwrite                 (pwrite),
    .paddr                  (paddr[12:0]),
    .pwdata                 (pwdata),
    .pstrb                  (pstrb),
    .pready                 (dma_pready),
    .pslverr                (dma_pslverr),
    .prdata                 (dma_prdata),

    // AXI5 manager M0: write address
    .awakeup_m0             (),
    .awvalid_m0             (awvalid),
    .awready_m0             (awready),
    .awaddr_m0              (awaddr),
    .awburst_m0             (awburst),
    .awid_m0                (awid),
    .awlen_m0               (awlen),
    .awsize_m0              (awsize),
    .awqos_m0               (),
    .awprot_m0              (awprot),
    .awcache_m0             (awcache),
    .awdomain_m0            (),
    .awinner_m0             (),

    // read address
    .arvalid_m0             (arvalid),
    .arready_m0             (arready),
    .araddr_m0              (araddr),
    .arburst_m0             (arburst),
    .arid_m0                (arid),
    .arlen_m0               (arlen),
    .arsize_m0              (arsize),
    .arqos_m0               (),
    .arprot_m0              (arprot),
    .arcache_m0             (arcache),
    .ardomain_m0            (),
    .arinner_m0             (),
    .arcmdlink_m0           (),

    // write data
    .wvalid_m0              (wvalid),
    .wready_m0              (wready),
    .wlast_m0               (wlast),
    .wstrb_m0               (wstrb),
    .wdata_m0               (wdata),

    // read data
    .rvalid_m0              (rvalid),
    .rready_m0              (rready),
    .rid_m0                 (rid),
    .rlast_m0               (rlast),
    .rdata_m0               (rdata),
    .rpoison_m0             ('0),
    .rresp_m0               (rresp),

    // write response
    .bvalid_m0              (bvalid),
    .bready_m0              (bready),
    .bid_m0                 (bid),
    .bresp_m0               (bresp),

    // Interrupts
    .irq_channel            (irq_channel),
    .irq_comb_nonsec        (irq_comb_nonsec),

    // Stop / pause / CTI: unused
    .allch_stop_req_nonsec  (1'b0),
    .allch_stop_ack_nonsec  (),
    .allch_pause_req_nonsec (1'b0),
    .allch_pause_ack_nonsec (),
    .halt_req               (1'b0),
    .restart_req            (1'b0),
    .halted                 (),

    // Channel status: unused
    .ch_enabled             (),
    .ch_err                 (),
    .ch_stopped             (),
    .ch_paused              (),
    .ch_priv                (),

    // Automatic boot: disabled
    .boot_en                (1'b0),
    .boot_addr              ('0),
    .boot_memattr           (8'h00),
    .boot_shareattr         (2'b00)
);

endmodule
