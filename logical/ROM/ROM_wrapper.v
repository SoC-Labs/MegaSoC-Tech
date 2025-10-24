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

    input  wire             AWVALID,
    output wire             AWREADY,
    input  wire [ID_W-1:0]  AWID,
    input  wire [31:0]      AWADDR,
    input  wire [7:0]       AWLEN,
    input  wire [2:0]       AWSIZE,
    input  wire [1:0]       AWBURST,
    input  wire             AWLOCK,
    input  wire [2:0]       AWPROT,
    input  wire [3:0]       AWQOS,
    
    input  wire             WVALID,
    output wire             WREADY,
    input  wire [63:0]      WDATA,
    input  wire [7:0]       WSTRB,
    input  wire             WLAST,
    input  wire             WPOISON,

    output wire             BVALID,
    input  wire             BREADY,
    output wire [ID_W-1:0]  BID,
    output wire [1:0]       BRESP,
    
    input  wire             ARVALID,
    output wire             ARREADY,
    input  wire [ID_W-1:0]  ARID,
    input  wire [31:0]      ARADDR,
    input  wire [7:0]       ARLEN,
    input  wire [2:0]       ARSIZE,
    input  wire [1:0]       ARBURST,
    input  wire             ARLOCK,
    input  wire [2:0]       ARPROT,
    input  wire [3:0]       ARQOS,
    
    output wire             RVALID,
    input  wire             RREADY,
    output wire [ID_W-1:0]       RID,
    output wire [63:0]      RDATA,
    output wire [1:0]       RRESP,
    output wire             RLAST,
    output wire             RPOISON,
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


wire [19:0]    memaddr;
wire [64:0]    memd;
wire [64:0]    memq;
wire           memcen;
wire [7:0]     memwen;

sie300_axi5_sram_ctrl_1 u_SMC (
    .aclk(ACLK),
    .aresetn(ARESETn),
    .awvalid_s(AWVALID),
    .awready_s(AWREADY),
    .awid_s(AWID),
    .awaddr_s(AWADDR[19:0]),
    .awlen_s(AWLEN),
    .awsize_s(AWSIZE),
    .awburst_s(AWBURST),
    .awlock_s(AWLOCK),
    .awprot_s(AWPROT),
    .awqos_s(AWQOS),
    .wvalid_s(WVALID),
    .wready_s(WREADY),
    .wdata_s(WDATA),
    .wstrb_s(WSTRB),
    .wlast_s(WLAST),
    .wpoison_s(WPOISON),
    .bvalid_s(BVALID),
    .bready_s(BREADY),
    .bid_s(BID),
    .bresp_s(BRESP),
    .arvalid_s(ARVALID),
    .arready_s(ARREADY),
    .arid_s(ARID),
    .araddr_s(ARADDR[19:0]),
    .arlen_s(ARLEN),
    .arsize_s(ARSIZE),
    .arburst_s(ARBURST),
    .arlock_s(ARLOCK),
    .arprot_s(ARPROT),
    .arqos_s(ARQOS),
    .rvalid_s(RVALID),
    .rready_s(RREADY),
    .rid_s(RID),
    .rdata_s(RDATA),
    .rresp_s(RRESP),
    .rlast_s(RLAST),
    .rpoison_s(RPOISON),
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

cmsdk_fpga_sram #(.AW(15)) u_fpga_sram_0(
    .CLK(ACLK),
    .ADDR(memaddr),
    .WDATA(memd[31:0]),
    .WREN(memwen[3:0]),
    .CS(memcen),
    .RDATA(memq[31:0])
);

cmsdk_fpga_sram #(.AW(15)) u_fpga_sram_1(
    .CLK(ACLK),
    .ADDR(memaddr),
    .WDATA(memd[63:32]),
    .WREN(memwen[7:4]),
    .CS(memcen),
    .RDATA(memq[63:32])
);


endmodule
