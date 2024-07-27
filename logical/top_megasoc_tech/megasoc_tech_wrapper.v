//-----------------------------------------------------------------------------
// MegaSoC Tech Wrapper
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Contributors
//
// Daniel Newbrook (d.newbrook@soton.ac.uk)
// 
// Copyright � 2021-4, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Modules instantiated:

`timescale 1ns/1ps

module megasoc_tech_wrapper(
    input  wire           SYS_CLK,
    input  wire           SYS_CLKEN,
    input  wire           SYS_RESETn,

    // Millisoc system AXI Manager
    output wire [1:0]     AXI_SYS_EXP_awid,
    output wire [31:0]    AXI_SYS_EXP_awaddr,
    output wire [7:0]     AXI_SYS_EXP_awlen,
    output wire [2:0]     AXI_SYS_EXP_awsize,
    output wire [1:0]     AXI_SYS_EXP_awburst,
    output wire           AXI_SYS_EXP_awlock,
    output wire [3:0]     AXI_SYS_EXP_awcache,
    output wire [2:0]     AXI_SYS_EXP_awprot,
    output wire           AXI_SYS_EXP_awvalid,
    input wire            AXI_SYS_EXP_awready,
    output wire [63:0]    AXI_SYS_EXP_wdata,
    output wire [7:0]     AXI_SYS_EXP_wstrb,
    output wire           AXI_SYS_EXP_wlast,
    output wire           AXI_SYS_EXP_wvalid,
    input wire            AXI_SYS_EXP_wready,
    input wire  [1:0]     AXI_SYS_EXP_bid,
    input wire  [1:0]     AXI_SYS_EXP_bresp,
    input wire            AXI_SYS_EXP_bvalid,
    output wire           AXI_SYS_EXP_bready,
    output wire [1:0]     AXI_SYS_EXP_arid,
    output wire [31:0]    AXI_SYS_EXP_araddr,
    output wire [7:0]     AXI_SYS_EXP_arlen,
    output wire [2:0]     AXI_SYS_EXP_arsize,
    output wire [1:0]     AXI_SYS_EXP_arburst,
    output wire           AXI_SYS_EXP_arlock,
    output wire [3:0]     AXI_SYS_EXP_arcache,
    output wire [2:0]     AXI_SYS_EXP_arprot,
    output wire           AXI_SYS_EXP_arvalid,
    input wire            AXI_SYS_EXP_arready,
    input wire  [1:0]     AXI_SYS_EXP_rid,
    input wire  [63:0]    AXI_SYS_EXP_rdata,
    input wire  [1:0]     AXI_SYS_EXP_rresp,
    input wire            AXI_SYS_EXP_rlast,
    input wire            AXI_SYS_EXP_rvalid,
    output wire           AXI_SYS_EXP_rready,
    

    // Millisoc system AXI Subordinate
    input wire         AXI_EXP_SYS_awid,
    input wire  [31:0] AXI_EXP_SYS_awaddr,
    input wire  [7:0]  AXI_EXP_SYS_awlen,
    input wire  [2:0]  AXI_EXP_SYS_awsize,
    input wire  [1:0]  AXI_EXP_SYS_awburst,
    input wire         AXI_EXP_SYS_awlock,
    input wire  [3:0]  AXI_EXP_SYS_awcache,
    input wire  [2:0]  AXI_EXP_SYS_awprot,
    input wire         AXI_EXP_SYS_awvalid,
    output wire        AXI_EXP_SYS_awready,
    input wire  [63:0] AXI_EXP_SYS_wdata,
    input wire  [7:0]  AXI_EXP_SYS_wstrb,
    input wire         AXI_EXP_SYS_wlast,
    input wire         AXI_EXP_SYS_wvalid,
    output wire        AXI_EXP_SYS_wready,
    output wire        AXI_EXP_SYS_bid,
    output wire [1:0]  AXI_EXP_SYS_bresp,
    output wire        AXI_EXP_SYS_bvalid,
    input wire         AXI_EXP_SYS_bready,
    input wire         AXI_EXP_SYS_arid,
    input wire  [31:0] AXI_EXP_SYS_araddr,
    input wire  [7:0]  AXI_EXP_SYS_arlen,
    input wire  [2:0]  AXI_EXP_SYS_arsize,
    input wire  [1:0]  AXI_EXP_SYS_arburst,
    input wire         AXI_EXP_SYS_arlock,
    input wire  [3:0]  AXI_EXP_SYS_arcache,
    input wire  [2:0]  AXI_EXP_SYS_arprot,
    input wire         AXI_EXP_SYS_arvalid,
    output wire        AXI_EXP_SYS_arready,
    output wire        AXI_EXP_SYS_rid,
    output wire [63:0] AXI_EXP_SYS_rdata,
    output wire [1:0]  AXI_EXP_SYS_rresp,
    output wire        AXI_EXP_SYS_rlast,
    output wire        AXI_EXP_SYS_rvalid,
    input wire         AXI_EXP_SYS_rready

);

endmodule