//-----------------------------------------------------------------------------
// Megasoc SRAM Wrapper
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Contributors
//
// Daniel Newbrook (d.newbrook@soton.ac.uk)
// 
// Copyright � 2021-4, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Modules instantiated:
//  sie300_axi5_sram_ctrl_expansion_subsystem
//  SRAM

module SRAM_wrapper#(
    parameter ID_W=8)(
    input  wire             ACLK,
    input  wire             ARESETn,

    axi4.subordinate        SRAM_AXI,

    input  wire             AWAKEUP,

    qchannel.subordinate    SRAM_qchan_q,
    qchannel.subordinate    SRAM_qchan_p,

    input  wire             cfg_gate_resp
);


wire [19:0]     memaddr;
wire [63:0]     memd;
wire [63:0]     memq;
wire            memcen;
wire [7:0]      memwen;

sie300_axi5_sram_ctrl_1 u_SMC(
    .aclk(ACLK),
    .aresetn(ARESETn),

    .awvalid_s(SRAM_AXI.AWVALID),
    .awready_s(SRAM_AXI.AWREADY),
    .awid_s(SRAM_AXI.AWID),
    .awaddr_s(SRAM_AXI.AWADDR[19:0]),
    .awlen_s(SRAM_AXI.AWLEN),
    .awsize_s(SRAM_AXI.AWSIZE),
    .awburst_s(SRAM_AXI.AWBURST),
    .awlock_s(SRAM_AXI.AWLOCK),
    .awprot_s(SRAM_AXI.AWPROT),
    .awqos_s(4'h0),
    .wvalid_s(SRAM_AXI.WVALID),
    .wready_s(SRAM_AXI.WREADY),
    .wdata_s(SRAM_AXI.WDATA),
    .wstrb_s(SRAM_AXI.WSTRB),
    .wlast_s(SRAM_AXI.WLAST),
    .wpoison_s(1'b0),
    .bvalid_s(SRAM_AXI.BVALID),
    .bready_s(SRAM_AXI.BREADY),
    .bid_s(SRAM_AXI.BID),
    .bresp_s(SRAM_AXI.BRESP),
    .arvalid_s(SRAM_AXI.ARVALID),
    .arready_s(SRAM_AXI.ARREADY),
    .arid_s(SRAM_AXI.ARID),
    .araddr_s(SRAM_AXI.ARADDR[19:0]),
    .arlen_s(SRAM_AXI.ARLEN),
    .arsize_s(SRAM_AXI.ARSIZE),
    .arburst_s(SRAM_AXI.ARBURST),
    .arlock_s(SRAM_AXI.ARLOCK),
    .arprot_s(SRAM_AXI.ARPROT),
    .arqos_s(4'h0),
    .rvalid_s(SRAM_AXI.RVALID),
    .rready_s(SRAM_AXI.RREADY),
    .rid_s(SRAM_AXI.RID),
    .rdata_s(SRAM_AXI.RDATA),
    .rresp_s(SRAM_AXI.RRESP),
    .rlast_s(SRAM_AXI.RLAST),
    .rpoison_s(),

    .awakeup_s(AWAKEUP),

    .clk_qreqn(SRAM_qchan_q.qreqn),
    .clk_qacceptn(SRAM_qchan_q.qacceptn),
    .clk_qdeny(SRAM_qchan_q.qdeny),
    .clk_qactive(SRAM_qchan_q.qactive),

    .pwr_qreqn(SRAM_qchan_p.qreqn),
    .pwr_qacceptn(SRAM_qchan_p.qacceptn),
    .pwr_qdeny(SRAM_qchan_p.qdeny),
    .pwr_qactive(SRAM_qchan_p.qactive),

    .ext_gt_qreqn(SRAM_qchan_p.qreqn),
    .ext_gt_qacceptn(),
    .cfg_gate_resp(cfg_gate_resp),

    .memaddr(memaddr),
    .memd(memd),
    .memq(memq),
    .memcen(memcen),
    .memwen(memwen)
);

SRAM u_SRAM(
    .clk(ACLK),
    .memaddr(memaddr),
    .memd(memd),
    .memq(memq),
    .memcen(memcen),
    .memwen(memwen)
);


endmodule