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
    input  wire             clk_qreqn,
    output wire             clk_qacceptn,
    output wire             clk_qdeny,
    output wire             clk_qactive,

    input  wire             pwr_qreqn,
    output wire             pwr_qacceptn,
    output wire             pwr_qdeny,
    output wire             pwr_qactive,

    input  wire             ext_gt_qreqn,
    output wire             ext_gt_qacceptn,
    input  wire             cfg_gate_resp
);


wire [19:0]     memaddr;
wire [63:0]    memd;
wire [63:0]    memq;
wire            memcen;
wire [7:0]     memwen;

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
    .clk_qreqn(clk_qreqn),
    .clk_qacceptn(clk_qacceptn),
    .clk_qdeny(clk_qdeny),
    .clk_qactive(clk_qactive),
    .pwr_qreqn(pwr_qreqn),
    .pwr_qacceptn(pwr_qacceptn),
    .pwr_qdeny(pwr_qdeny),
    .pwr_qactive(pwr_qactive),
    .ext_gt_qreqn(ext_gt_qreqn),
    .ext_gt_qacceptn(ext_gt_qacceptn),
    .cfg_gate_resp(cfg_gate_resp),
    .memaddr(memaddr),
    .memd(memd),
    .memq(memq),
    .memcen(memcen),
    .memwen(memwen)
);

cmsdk_fpga_sram #(.AW(19)) u_fpga_sram_0(
    .CLK(ACLK),
    .ADDR(memaddr),
    .WDATA(memd[31:0]),
    .WREN(memwen[3:0]),
    .CS(memcen),
    .RDATA(memq[31:0])
);

cmsdk_fpga_sram #(.AW(19)) u_fpga_sram_1(
    .CLK(ACLK),
    .ADDR(memaddr),
    .WDATA(memd[63:32]),
    .WREN(memwen[7:4]),
    .CS(memcen),
    .RDATA(memq[63:32])
);

endmodule