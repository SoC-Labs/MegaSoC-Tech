//-----------------------------------------------------------------------------
// Expansion Subsystem SRAM Wrapper
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

module ROM_wrapper (
    input  wire             ACLK,
    input  wire             ARESETn,

    axi4.subordinate        ROM_AXI,

    input  wire             AWAKEUP,

    qchannel.subordinate    ROM_qchan_q,
    qchannel.subordinate    ROM_qchan_p,

    input  wire             cfg_gate_resp
);


wire [19:0]    memaddr;
wire [64:0]    memd;
wire [64:0]    memq;
wire           memcen;
wire [7:0]     memwen;

sie300_axi5_sram_ctrl_1 u_SMC(
    .aclk(ACLK),
    .aresetn(ARESETn),

    .awvalid_s(ROM_AXI.AWVALID),
    .awready_s(ROM_AXI.AWREADY),
    .awid_s(ROM_AXI.AWID),
    .awaddr_s(ROM_AXI.AWADDR[19:0]),
    .awlen_s(ROM_AXI.AWLEN),
    .awsize_s(ROM_AXI.AWSIZE),
    .awburst_s(ROM_AXI.AWBURST),
    .awlock_s(ROM_AXI.AWLOCK),
    .awprot_s(ROM_AXI.AWPROT),
    .awqos_s(4'h0),
    .wvalid_s(ROM_AXI.WVALID),
    .wready_s(ROM_AXI.WREADY),
    .wdata_s(ROM_AXI.WDATA),
    .wstrb_s(ROM_AXI.WSTRB),
    .wlast_s(ROM_AXI.WLAST),
    .wpoison_s(1'b0),
    .bvalid_s(ROM_AXI.BVALID),
    .bready_s(ROM_AXI.BREADY),
    .bid_s(ROM_AXI.BID),
    .bresp_s(ROM_AXI.BRESP),
    .arvalid_s(ROM_AXI.ARVALID),
    .arready_s(ROM_AXI.ARREADY),
    .arid_s(ROM_AXI.ARID),
    .araddr_s(ROM_AXI.ARADDR[19:0]),
    .arlen_s(ROM_AXI.ARLEN),
    .arsize_s(ROM_AXI.ARSIZE),
    .arburst_s(ROM_AXI.ARBURST),
    .arlock_s(ROM_AXI.ARLOCK),
    .arprot_s(ROM_AXI.ARPROT),
    .arqos_s(4'h0),
    .rvalid_s(ROM_AXI.RVALID),
    .rready_s(ROM_AXI.RREADY),
    .rid_s(ROM_AXI.RID),
    .rdata_s(ROM_AXI.RDATA),
    .rresp_s(ROM_AXI.RRESP),
    .rlast_s(ROM_AXI.RLAST),
    .rpoison_s(),

    .awakeup_s(AWAKEUP),
    .clk_qreqn(ROM_qchan_q.qreqn),
    .clk_qacceptn(ROM_qchan_q.qacceptn),
    .clk_qdeny(ROM_qchan_q.qdeny),
    .clk_qactive(ROM_qchan_q.qactive),
    .pwr_qreqn(ROM_qchan_p.qreqn),
    .pwr_qacceptn(ROM_qchan_p.qacceptn),
    .pwr_qdeny(ROM_qchan_p.qdeny),
    .pwr_qactive(ROM_qchan_p.qactive),
    .ext_gt_qreqn(ROM_qchan_p.qreqn),
    .ext_gt_qacceptn(),
    .cfg_gate_resp(cfg_gate_resp),

    .memaddr(memaddr),
    .memd(),
    .memq(memq),
    .memcen(memcen),
    .memwen(memwen)
);

bootrom u_ROM (
    .CLK(ACLK),
    .W_ADDR(memaddr[19:3]),
    .RDATA(memq),
    .EN(~memcen)
    );


endmodule