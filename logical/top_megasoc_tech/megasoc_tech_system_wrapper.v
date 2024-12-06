//-----------------------------------------------------------------------------
// MegaSoC Tech System Wrapper
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Contributors
//
// Daniel Newbrook (d.newbrook@soton.ac.uk)
// 
// Copyright � 2021-4, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Purpose:
//  Top level wrapper for the megasoc "System" subsystem. This subsystem includes
//  the filesystem and similar peripherals (ethernet/PCIe). Uses DMA to move from
//  these peripherals to DRAM
//-----------------------------------------------------------------------------
// Modules instantiated:
//  - ada_top_sldma350_megasoc
//  - nic400_megasoc_system
//-----------------------------------------------------------------------------
// To Do
//  - Everything

module megasoc_tech_system_wrapper(
    input  wire             CLK,
    input  wire             RESETn,

    input  wire             DMA350_PWAKEUP,
    input  wire             DMA350_PDEBUG,
    input  wire             DMA350_PSEL,
    input  wire             DMA350_PENABLE,
    input  wire [2:0]       DMA350_PPROT,
    input  wire             DMA350_PWRITE,
    input  wire [12:0]      DMA350_PADDR,
    input  wire [31:0]      DMA350_PWDATA,
    input  wire [3:0]       DMA350_PSTRB,
    output wire             DMA350_PREADY,
    output wire             DMA350_PSLVERR,
    output wire [31:0]      DMA350_PRDATA,

    output wire             DMA350_AWAKEUP_M0,
    output wire             DMA350_AWVALID_M0,
    output wire [44-1:0]    DMA350_AWADDR_M0,
    output wire [1:0]       DMA350_AWBURST_M0,
    output wire [2-1:0]     DMA350_AWID_M0,
    output wire [7:0]       DMA350_AWLEN_M0,
    output wire [2:0]       DMA350_AWSIZE_M0,
    output wire [3:0]       DMA350_AWQOS_M0,
    output wire [2:0]       DMA350_AWPROT_M0,
    input  wire             DMA350_AWREADY_M0,
    output wire [3:0]       DMA350_AWCACHE_M0,
    output wire [3:0]       DMA350_AWINNER_M0,
    output wire [1:0]       DMA350_AWDOMAIN_M0,

    output wire             DMA350_ARVALID_M0,
    output wire [44-1:0]    DMA350_ARADDR_M0,
    output wire [1:0]       DMA350_ARBURST_M0,
    output wire [2-1:0]     DMA350_ARID_M0,
    output wire [7:0]       DMA350_ARLEN_M0,
    output wire [2:0]       DMA350_ARSIZE_M0,
    output wire [3:0]       DMA350_ARQOS_M0,
    output wire [2:0]       DMA350_ARPROT_M0,
    input  wire             DMA350_ARREADY_M0,
    output wire [3:0]       DMA350_ARCACHE_M0,
    output wire [3:0]       DMA350_ARINNER_M0,
    output wire [1:0]       DMA350_ARDOMAIN_M0,
    output wire             DMA350_ARCMDLINK_M0,

    output wire             DMA350_WVALID_M0,
    output wire             DMA350_WLAST_M0,
    output wire [16-1:0]    DMA350_WSTRB_M0,
    output wire [128-1:0]   DMA350_WDATA_M0,
    input  wire             DMA350_WREADY_M0,

    input  wire             DMA350_RVALID_M0,
    input  wire  [2-1:0]    DMA350_RID_M0,
    input  wire             DMA350_RLAST_M0,
    input  wire  [128-1:0]  DMA350_RDATA_M0,
    input  wire  [2-1:0]    DMA350_RPOISON_M0,
    input  wire  [1:0]      DMA350_RRESP_M0,
    output wire             DMA350_RREADY_M0,

    input  wire             DMA350_BVALID_M0,
    input  wire  [2-1:0]    DMA350_BID_M0,
    input  wire  [1:0]      DMA350_BRESP_M0,
    output wire             DMA350_BREADY_M0,

    output wire [4-1:0]     DMA350_irq_channel,
    output wire             DMA350_irq_comb_nonsec


);

// DMA AXI Manager 1
wire             DMA350_AWAKEUP_M1;
wire             DMA350_AWVALID_M1;
wire [44-1:0]    DMA350_AWADDR_M1;
wire [1:0]       DMA350_AWBURST_M1;
wire [2-1:0]     DMA350_AWID_M1;
wire [7:0]       DMA350_AWLEN_M1;
wire [2:0]       DMA350_AWSIZE_M1;
wire [3:0]       DMA350_AWQOS_M1;
wire [2:0]       DMA350_AWPROT_M1;
wire             DMA350_AWREADY_M1;
wire [3:0]       DMA350_AWCACHE_M1;
wire [3:0]       DMA350_AWINNER_M1;
wire [1:0]       DMA350_AWDOMAIN_M1;
wire             DMA350_ARVALID_M1;
wire [44-1:0]    DMA350_ARADDR_M1;
wire [1:0]       DMA350_ARBURST_M1;
wire [2-1:0]     DMA350_ARID_M1;
wire [7:0]       DMA350_ARLEN_M1;
wire [2:0]       DMA350_ARSIZE_M1;
wire [3:0]       DMA350_ARQOS_M1;
wire [2:0]       DMA350_ARPROT_M1;
wire             DMA350_ARREADY_M1;
wire [3:0]       DMA350_ARCACHE_M1;
wire [3:0]       DMA350_ARINNER_M1;
wire [1:0]       DMA350_ARDOMAIN_M1;
wire             DMA350_ARCMDLINK_M1;
wire             DMA350_WVALID_M1;
wire             DMA350_WLAST_M1;
wire [16-1:0]    DMA350_WSTRB_M1;
wire [128-1:0]   DMA350_WDATA_M1;
wire             DMA350_WREADY_M1;
wire             DMA350_RVALID_M1;
wire  [2-1:0]    DMA350_RID_M1;
wire             DMA350_RLAST_M1;
wire  [128-1:0]  DMA350_RDATA_M1;
wire  [2-1:0]    DMA350_RPOISON_M1;
wire  [1:0]      DMA350_RRESP_M1;
wire             DMA350_RREADY_M1;
wire             DMA350_BVALID_M1;
wire  [2-1:0]    DMA350_BID_M1;
wire  [1:0]      DMA350_BRESP_M1;
wire             DMA350_BREADY_M1;

// SRAM AXI Signals
wire [1:0]      AWID_SYS_SRAM;
wire [31:0]     AWADDR_SYS_SRAM;
wire [7:0]      AWLEN_SYS_SRAM;
wire [2:0]      AWSIZE_SYS_SRAM;
wire [1:0]      AWBURST_SYS_SRAM;
wire            AWLOCK_SYS_SRAM;
wire [3:0]      AWCACHE_SYS_SRAM;
wire [2:0]      AWPROT_SYS_SRAM;
wire            AWVALID_SYS_SRAM;
wire            AWREADY_SYS_SRAM;
wire [63:0]     WDATA_SYS_SRAM;
wire [7:0]      WSTRB_SYS_SRAM;
wire            WLAST_SYS_SRAM;
wire            WVALID_SYS_SRAM;
wire            WREADY_SYS_SRAM;
wire [1:0]      BID_SYS_SRAM;
wire [1:0]      BRESP_SYS_SRAM;
wire            BVALID_SYS_SRAM;
wire            BREADY_SYS_SRAM;
wire [1:0]      ARID_SYS_SRAM;
wire [31:0]     ARADDR_SYS_SRAM;
wire [7:0]      ARLEN_SYS_SRAM;
wire [2:0]      ARSIZE_SYS_SRAM;
wire [1:0]      ARBURST_SYS_SRAM;
wire            ARLOCK_SYS_SRAM;
wire [3:0]      ARCACHE_SYS_SRAM;
wire [2:0]      ARPROT_SYS_SRAM;
wire            ARVALID_SYS_SRAM;
wire            ARREADY_SYS_SRAM;
wire [1:0]      RID_SYS_SRAM;
wire [63:0]     RDATA_SYS_SRAM;
wire [1:0]      RRESP_SYS_SRAM;
wire            RLAST_SYS_SRAM;
wire            RVALID_SYS_SRAM;
wire            RREADY_SYS_SRAM;


ada_top_sldma350_megasoc u_megasoc_dma350(
    .clk(CLK),
    .resetn(RESETn),
    .aclken_m0(1'b1),
    .aclken_m1(1'b1),
    .pclken(1'b1),

    .clk_qreqn(1'b1),
    .clk_qacceptn(),
    .clk_qdeny(),
    .clk_qactive(),

    .preq(1'b1),
    .pstate(4'b1000),
    .paccept(),
    .pdeny(),
    .pactive(),

    .pwakeup        (DMA350_PWAKEUP),
    .pdebug         (DMA350_PDEBUG),
    .psel           (DMA350_PSEL),
    .penable        (DMA350_PENABLE),
    .pprot          (DMA350_PPROT),
    .pwrite         (DMA350_PWRITE),
    .paddr          (DMA350_PADDR),
    .pwdata         (DMA350_PWDATA),
    .pstrb          (DMA350_PSTRB),
    .pready         (DMA350_PREADY),
    .pslverr        (DMA350_PSLVERR),
    .prdata         (DMA350_PRDATA),

    .awakeup_m0     (DMA350_AWAKEUP_M0),
    .awvalid_m0     (DMA350_AWVALID_M0),
    .awaddr_m0      (DMA350_AWADDR_M0),
    .awburst_m0     (DMA350_AWBURST_M0),
    .awid_m0        (DMA350_AWID_M0),
    .awlen_m0       (DMA350_AWLEN_M0),
    .awsize_m0      (DMA350_AWSIZE_M0),
    .awqos_m0       (DMA350_AWQOS_M0),
    .awprot_m0      (DMA350_AWPROT_M0),
    .awready_m0     (DMA350_AWREADY_M0),
    .awcache_m0     (DMA350_AWCACHE_M0),
    .awinner_m0     (DMA350_AWINNER_M0),
    .awdomain_m0    (DMA350_AWDOMAIN_M0),
    .arvalid_m0     (DMA350_ARVALID_M0),
    .araddr_m0      (DMA350_ARADDR_M0),
    .arburst_m0     (DMA350_ARBURST_M0),
    .arid_m0        (DMA350_ARID_M0),
    .arlen_m0       (DMA350_ARLEN_M0),
    .arsize_m0      (DMA350_ARSIZE_M0),
    .arqos_m0       (DMA350_ARQOS_M0),
    .arprot_m0      (DMA350_ARPROT_M0),
    .arready_m0     (DMA350_ARREADY_M0),
    .arcache_m0     (DMA350_ARCACHE_M0),
    .arinner_m0     (DMA350_ARINNER_M0),
    .ardomain_m0    (DMA350_ARDOMAIN_M0),
    .arcmdlink_m0   (DMA350_ARCMDLINK_M0),
    .wvalid_m0      (DMA350_WVALID_M0),
    .wlast_m0       (DMA350_WLAST_M0),
    .wstrb_m0       (DMA350_WSTRB_M0),
    .wdata_m0       (DMA350_WDATA_M0),
    .wready_m0      (DMA350_WREADY_M0),
    .rvalid_m0      (DMA350_RVALID_M0),
    .rid_m0         (DMA350_RID_M0),
    .rlast_m0       (DMA350_RLAST_M0),
    .rdata_m0       (DMA350_RDATA_M0),
    .rpoison_m0     (DMA350_RPOISON_M0),
    .rresp_m0       (DMA350_RRESP_M0),
    .rready_m0      (DMA350_RREADY_M0),
    .bvalid_m0      (DMA350_BVALID_M0),
    .bid_m0         (DMA350_BID_M0),
    .bresp_m0       (DMA350_BRESP_M0),
    .bready_m0      (DMA350_BREADY_M0),

    .awakeup_m1     (DMA350_AWAKEUP_M1),
    .awvalid_m1     (DMA350_AWVALID_M1),
    .awaddr_m1      (DMA350_AWADDR_M1),
    .awburst_m1     (DMA350_AWBURST_M1),
    .awid_m1        (DMA350_AWID_M1),
    .awlen_m1       (DMA350_AWLEN_M1),
    .awsize_m1      (DMA350_AWSIZE_M1),
    .awqos_m1       (DMA350_AWQOS_M1),
    .awprot_m1      (DMA350_AWPROT_M1),
    .awready_m1     (DMA350_AWREADY_M1),
    .awcache_m1     (DMA350_AWCACHE_M1),
    .awinner_m1     (DMA350_AWINNER_M1),
    .awdomain_m1    (DMA350_AWDOMAIN_M1),
    .arvalid_m1     (DMA350_ARVALID_M1),
    .araddr_m1      (DMA350_ARADDR_M1),
    .arburst_m1     (DMA350_ARBURST_M1),
    .arid_m1        (DMA350_ARID_M1),
    .arlen_m1       (DMA350_ARLEN_M1),
    .arsize_m1      (DMA350_ARSIZE_M1),
    .arqos_m1       (DMA350_ARQOS_M1),
    .arprot_m1      (DMA350_ARPROT_M1),
    .arready_m1     (DMA350_ARREADY_M1),
    .arcache_m1     (DMA350_ARCACHE_M1),
    .arinner_m1     (DMA350_ARINNER_M1),
    .ardomain_m1    (DMA350_ARDOMAIN_M1),
    .arcmdlink_m1   (DMA350_ARCMDLINK_M1),
    .wvalid_m1      (DMA350_WVALID_M1),
    .wlast_m1       (DMA350_WLAST_M1),
    .wstrb_m1       (DMA350_WSTRB_M1),
    .wdata_m1       (DMA350_WDATA_M1),
    .wready_m1      (DMA350_WREADY_M1),
    .rvalid_m1      (DMA350_RVALID_M1),
    .rid_m1         (DMA350_RID_M1),
    .rlast_m1       (DMA350_RLAST_M1),
    .rdata_m1       (DMA350_RDATA_M1),
    .rpoison_m1     (DMA350_RPOISON_M1),
    .rresp_m1       (DMA350_RRESP_M1),
    .rready_m1      (DMA350_RREADY_M1),
    .bvalid_m1      (DMA350_BVALID_M1),
    .bid_m1         (DMA350_BID_M1),
    .bresp_m1       (DMA350_BRESP_M1),
    .bready_m1      (DMA350_BREADY_M1),

    .trig_in_0_req(),
    .trig_in_0_req_type(),
    .trig_in_0_ack(),
    .trig_in_0_ack_type(),
    .trig_in_1_req(),
    .trig_in_1_req_type(),
    .trig_in_1_ack(),
    .trig_in_1_ack_type(),
    .trig_out_0_req(),
    .trig_out_0_ack(),
    .trig_out_1_req(),
    .trig_out_1_ack(),

    .irq_channel(DMA350_irq_channel),
    .irq_comb_nonsec(DMA350_irq_comb_nonsec),

    .str_out_0_tvalid(),
    .str_out_0_tready(),
    .str_out_0_tdata(),
    .str_out_0_tstrb(),
    .str_out_0_tlast(),
    .str_in_0_tvalid(),
    .str_in_0_tready(),
    .str_in_0_tdata(),
    .str_in_0_tstrb(),
    .str_in_0_tlast(),
    .str_in_0_flush(),
    .str_out_1_tvalid(),
    .str_out_1_tready(),
    .str_out_1_tdata(),
    .str_out_1_tstrb(),
    .str_out_1_tlast(),
    .str_in_1_tvalid(),
    .str_in_1_tready(),
    .str_in_1_tdata(),
    .str_in_1_tstrb(),
    .str_in_1_tlast(),
    .str_in_1_flush(),

    .gpo_ch_0(),
    .gpo_ch_1(),

    .allch_stop_req_nonsec(1'b0),
    .allch_stop_ack_nonsec(),
    .allch_pause_req_nonsec(1'b0),
    .allch_pause_ack_nonsec(),

    .ch_enabled(),
    .ch_err(),
    .ch_stopped(),
    .ch_paused(),
    .ch_priv(),

    .halt_req(1'b0),
    .restart_req(1'b0),
    .halted(),
    .boot_en(1'b0),
    .boot_addr({42{1'b0}}),
    .boot_memattr({8{1'b0}}),
    .boot_shareattr({2{1'b0}})
);


nic400_megasoc_system u_nic400_megasoc_system(
    .AWID_SYS_SRAM(AWID_SYS_SRAM),
    .AWADDR_SYS_SRAM(AWADDR_SYS_SRAM),
    .AWLEN_SYS_SRAM(AWLEN_SYS_SRAM),
    .AWSIZE_SYS_SRAM(AWSIZE_SYS_SRAM),
    .AWBURST_SYS_SRAM(AWBURST_SYS_SRAM),
    .AWLOCK_SYS_SRAM(AWLOCK_SYS_SRAM),
    .AWCACHE_SYS_SRAM(AWCACHE_SYS_SRAM),
    .AWPROT_SYS_SRAM(AWPROT_SYS_SRAM),
    .AWVALID_SYS_SRAM(AWVALID_SYS_SRAM),
    .AWREADY_SYS_SRAM(AWREADY_SYS_SRAM),
    .WDATA_SYS_SRAM(WDATA_SYS_SRAM),
    .WSTRB_SYS_SRAM(WSTRB_SYS_SRAM),
    .WLAST_SYS_SRAM(WLAST_SYS_SRAM),
    .WVALID_SYS_SRAM(WVALID_SYS_SRAM),
    .WREADY_SYS_SRAM(WREADY_SYS_SRAM),
    .BID_SYS_SRAM(BID_SYS_SRAM),
    .BRESP_SYS_SRAM(BRESP_SYS_SRAM),
    .BVALID_SYS_SRAM(BVALID_SYS_SRAM),
    .BREADY_SYS_SRAM(BREADY_SYS_SRAM),
    .ARID_SYS_SRAM(ARID_SYS_SRAM),
    .ARADDR_SYS_SRAM(ARADDR_SYS_SRAM),
    .ARLEN_SYS_SRAM(ARLEN_SYS_SRAM),
    .ARSIZE_SYS_SRAM(ARSIZE_SYS_SRAM),
    .ARBURST_SYS_SRAM(ARBURST_SYS_SRAM),
    .ARLOCK_SYS_SRAM(ARLOCK_SYS_SRAM),
    .ARCACHE_SYS_SRAM(ARCACHE_SYS_SRAM),
    .ARPROT_SYS_SRAM(ARPROT_SYS_SRAM),
    .ARVALID_SYS_SRAM(ARVALID_SYS_SRAM),
    .ARREADY_SYS_SRAM(ARREADY_SYS_SRAM),
    .RID_SYS_SRAM(RID_SYS_SRAM),
    .RDATA_SYS_SRAM(RDATA_SYS_SRAM),
    .RRESP_SYS_SRAM(RRESP_SYS_SRAM),
    .RLAST_SYS_SRAM(RLAST_SYS_SRAM),
    .RVALID_SYS_SRAM(RVALID_SYS_SRAM),
    .RREADY_SYS_SRAM(RREADY_SYS_SRAM),

    .AWID_DMA(DMA350_AWID_M1),
    .AWADDR_DMA(DMA350_AWADDR_M1),
    .AWLEN_DMA(DMA350_AWLEN_M1),
    .AWSIZE_DMA(DMA350_AWSIZE_M1),
    .AWBURST_DMA(DMA350_AWBURST_M1),
    .AWLOCK_DMA(DMA350_AWLOCK_M1),
    .AWCACHE_DMA(DMA350_AWCACHE_M1),
    .AWPROT_DMA(DMA350_AWPROT_M1),
    .AWVALID_DMA(DMA350_AWVALID_M1),
    .AWREADY_DMA(DMA350_AWREADY_M1),
    .WDATA_DMA(DMA350_WDATA_M1),
    .WSTRB_DMA(DMA350_WSTRB_M1),
    .WLAST_DMA(DMA350_WLAST_M1),
    .WVALID_DMA(DMA350_WVALID_M1),
    .WREADY_DMA(DMA350_WREADY_M1),
    .BID_DMA(DMA350_BID_M1),
    .BRESP_DMA(DMA350_BRESP_M1),
    .BVALID_DMA(DMA350_BVALID_M1),
    .BREADY_DMA(DMA350_BREADY_M1),
    .ARID_DMA(DMA350_ARID_M1),
    .ARADDR_DMA(DMA350_ARADDR_M1),
    .ARLEN_DMA(DMA350_ARLEN_M1),
    .ARSIZE_DMA(DMA350_ARSIZE_M1),
    .ARBURST_DMA(DMA350_ARBURST_M1),
    .ARLOCK_DMA(DMA350_ARLOCK_M1),
    .ARCACHE_DMA(DMA350_ARCACHE_M1),
    .ARPROT_DMA(DMA350_ARPROT_M1),
    .ARVALID_DMA(DMA350_ARVALID_M1),
    .ARREADY_DMA(DMA350_ARREADY_M1),
    .RID_DMA(DMA350_RID_M1),
    .RDATA_DMA(DMA350_RDATA_M1),
    .RRESP_DMA(DMA350_RRESP_M1),
    .RLAST_DMA(DMA350_RLAST_M1),
    .RVALID_DMA(DMA350_RVALID_M1),
    .RREADY_DMA(DMA350_RREADY_M1),

    .clk0clk(CLK),
    .clk0resetn(RESETn)
);

SYS_SRAM_wrapper #(
    .ID_W(2)
) u_SYS_SRAM_wrapper(
    .ACLK(CLK),
    .ARESETn(RESETn),
    .AWVALID(AWVALID_SYS_SRAM),
    .AWREADY(AWREADY_SYS_SRAM),
    .AWID(AWID_SYS_SRAM),
    .AWADDR(AWADDR_SYS_SRAM),
    .AWLEN(AWLEN_SYS_SRAM),
    .AWSIZE(AWSIZE_SYS_SRAM),
    .AWBURST(AWBURST_SYS_SRAM),
    .AWLOCK(AWLOCK_SYS_SRAM),
    .AWPROT(AWPROT_SYS_SRAM),
    .AWQOS(4'h0),
    .WVALID(WVALID_SYS_SRAM),
    .WREADY(WREADY_SYS_SRAM),
    .WDATA(WDATA_SYS_SRAM),
    .WSTRB(WSTRB_SYS_SRAM),
    .WLAST(WLAST_SYS_SRAM),
    .WPOISON(1'b0),
    .BVALID(BVALID_SYS_SRAM),
    .BREADY(BREADY_SYS_SRAM),
    .BID(BID_SYS_SRAM),
    .BRESP(BRESP_SYS_SRAM),
    .ARVALID(ARVALID_SYS_SRAM),
    .ARREADY(ARREADY_SYS_SRAM),
    .ARID(ARID_SYS_SRAM),
    .ARADDR(ARADDR_SYS_SRAM),
    .ARLEN(ARLEN_SYS_SRAM),
    .ARSIZE(ARSIZE_SYS_SRAM),
    .ARBURST(ARBURST_SYS_SRAM),
    .ARLOCK(ARLOCK_SYS_SRAM),
    .ARPROT(ARPROT_SYS_SRAM),
    .ARQOS(4'h0),
    .RVALID(RVALID_SYS_SRAM),
    .RREADY(RREADY_SYS_SRAM),
    .RID(RID_SYS_SRAM),
    .RDATA(RDATA_SYS_SRAM),
    .RRESP(RRESP_SYS_SRAM),
    .RLAST(RLAST_SYS_SRAM),
    .RPOISON(),
    .AWAKEUP(1'b1),
    .clk_qreqn(1'b1),
    .clk_qacceptn(),
    .clk_qdeny(),
    .clk_qactive(),
    .pwr_qreqn(1'b1),
    .pwr_qacceptn(),
    .pwr_qdeny(),
    .pwr_qactive(),
    .ext_gt_qreqn(1'b1),
    .ext_gt_qacceptn(),
    .cfg_gate_resp(1'b0)
);

endmodule