//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2008-2014 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2014-07-02 16:52:21 +0100 (Wed, 02 Jul 2014) $
//
//      Revision            : $Revision: 283836 $
//
//      Release Information : CORTEXA53-r0p4-00rel0
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description:
//  Wrapper for L2 cache data RAMs
//-----------------------------------------------------------------------------
//
`include "ca53scu_defs.v"
`include "cortexa53params.v"

module ca53_l2_datarams `CA53_L2_RAM_PARAM_DECL (
  input  wire                                   clk,
  input  wire                                   DFTSE,
  input  wire [`CA53_L2_SIZE_W-1:0]             l2_size_i,
  input  wire                                   l2_dataram_no_acc_next_cycle_i,
  input  wire [`CA53_SCU_L2_DATARAM_EN_W-1:0]   l2_dataram_clken_i,
  input  wire [`CA53_SCU_L2_DATARAM_EN_W-1:0]   l2_dataram_en_i,
  input  wire                                   l2_dataram_wr_i,
  input  wire [`CA53_SCU_L2_DATARAM_ADDR_W-1:0] l2_dataram_addr_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata0_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata1_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata2_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata3_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata4_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata5_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata6_i,
  input  wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_wdata7_i,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata0_o,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata1_o,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata2_o,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata3_o,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata4_o,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata5_o,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata6_o,
  output wire [`CA53_SCU_L2_DATARAM_DATA_W-1:0] l2_dataram_rdata7_o
);

  //-----------------------------------------------------------------------------
  //  Declarations
  //-----------------------------------------------------------------------------

  wire [`CA53_SCU_L2_DATARAM_EN_W-1:0]           clk_bank;

  localparam BYTES_PER_BANK = SCU_CACHE_PROTECTION ? 9 : 8;
  genvar i;
  //-----------------------------------------------------------------------------
  //  Main code
  //-----------------------------------------------------------------------------

  // Intermediate clock gates
  // There is a clock gate instantiated per RAM bank. These gates serve two
  // purposes:
  //  - To reduce idle RAM power
  //  - To allow the use of RAMs that cannot be clocked at the full core
  //    frequency

  generate for (i = 0; i < `CA53_SCU_L2_DATARAM_EN_W; i = i + 1) begin : g_clk_gate

    ca53_cell_inter_clkgate u_inter_clkgate (
      .clk_i         (clk),
      .clk_enable_i  (l2_dataram_clken_i[i]),
      .clk_senable_i (DFTSE),
      .clk_gated_o   (clk_bank[i])
    );
  end endgenerate

  // L2 data RAMs
  // These RAM instances should be sized according to the L2 cache size.
  // The data RAMs are logically organised as 8 banks, with
  // separate chip enables.
  //
  // L2 cache size | RAM instance size (per bank, | RAM instance size (per bank,
  //               | no cache protection)         | cache protection)
  // --------------+------------------------------+-----------------------------
  //         128KB |  2048 words x 64 bits        |  2048 words x 72 bits
  //         256KB |  4096 words x 64 bits        |  4096 words x 72 bits
  //         512KB |  8192 words x 64 bits        |  8192 words x 72 bits
  //        1024KB | 16384 words x 64 bits        | 16384 words x 72 bits
  //        2048KB | 32768 words x 64 bits        | 32768 words x 72 bits
  //
  // The address lines of the RAMs should always be connected to the low-order
  // bits of the address bus - any unused bits should be left unconnected
  //
  // The data RAMs are assumed to have two-cycle latency between the enable input
  // and the data outputs. The enable pin will never be high for two consecutive
  // cycles.

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_0_low (
        .CLK    (clk_bank[0]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata0_o[31:0]),
        .WDATA  (l2_dataram_wdata0_i[31:0]),
        .CS     (l2_dataram_en_i[0]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_0_high (
        .CLK    (clk_bank[0]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata0_o[63:32]),
        .WDATA  (l2_dataram_wdata0_i[63:32]),
        .CS     (l2_dataram_en_i[0]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_1_low (
        .CLK    (clk_bank[1]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata1_o[31:0]),
        .WDATA  (l2_dataram_wdata1_i[31:0]),
        .CS     (l2_dataram_en_i[1]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_1_high (
        .CLK    (clk_bank[1]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata1_o[63:32]),
        .WDATA  (l2_dataram_wdata1_i[63:32]),
        .CS     (l2_dataram_en_i[1]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_2_low (
        .CLK    (clk_bank[2]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata2_o[31:0]),
        .WDATA  (l2_dataram_wdata2_i[31:0]),
        .CS     (l2_dataram_en_i[2]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_2_high (
        .CLK    (clk_bank[2]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata2_o[63:32]),
        .WDATA  (l2_dataram_wdata2_i[63:32]),
        .CS     (l2_dataram_en_i[2]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_3_low (
        .CLK    (clk_bank[3]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata3_o[31:0]),
        .WDATA  (l2_dataram_wdata3_i[31:0]),
        .CS     (l2_dataram_en_i[3]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_3_high (
        .CLK    (clk_bank[3]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata3_o[63:32]),
        .WDATA  (l2_dataram_wdata3_i[63:32]),
        .CS     (l2_dataram_en_i[3]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_4_low (
        .CLK    (clk_bank[4]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata4_o[31:0]),
        .WDATA  (l2_dataram_wdata4_i[31:0]),
        .CS     (l2_dataram_en_i[4]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_4_high (
        .CLK    (clk_bank[4]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata4_o[63:32]),
        .WDATA  (l2_dataram_wdata4_i[63:32]),
        .CS     (l2_dataram_en_i[4]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_5_low (
        .CLK    (clk_bank[5]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata5_o[31:0]),
        .WDATA  (l2_dataram_wdata5_i[31:0]),
        .CS     (l2_dataram_en_i[5]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_5_high (
        .CLK    (clk_bank[5]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata5_o[63:32]),
        .WDATA  (l2_dataram_wdata5_i[63:32]),
        .CS     (l2_dataram_en_i[5]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_6_low (
        .CLK    (clk_bank[6]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata6_o[31:0]),
        .WDATA  (l2_dataram_wdata6_i[31:0]),
        .CS     (l2_dataram_en_i[6]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_6_high (
        .CLK    (clk_bank[6]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata6_o[63:32]),
        .WDATA  (l2_dataram_wdata6_i[63:32]),
        .CS     (l2_dataram_en_i[6]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_7_low (
        .CLK    (clk_bank[7]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata7_o[31:0]),
        .WDATA  (l2_dataram_wdata7_i[31:0]),
        .CS     (l2_dataram_en_i[7]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

cmsdk_fpga_sram #(.AW(12)) u_l2_dataram_7_high (
        .CLK    (clk_bank[7]),
        .ADDR   (l2_dataram_addr_i[11:2]),
        .RDATA  (l2_dataram_rdata7_o[63:32]),
        .WDATA  (l2_dataram_wdata7_i[63:32]),
        .CS     (l2_dataram_en_i[7]),
        .WREN   ({4{l2_dataram_wr_i}})
        );

endmodule

/*ARMAUTO_UNDEF*/
`define CA53_UNDEFINE
`include "cortexa53params.v"
`include "ca53scu_defs.v"
`undef CA53_UNDEFINE
/*END*/
