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
//  Wrapper for L2 tag RAMs
//-----------------------------------------------------------------------------
//
`include "ca53scu_defs.v"
`include "cortexa53params.v"

module ca53_l2_tagrams `CA53_L2_RAM_PARAM_DECL
(
  input  wire                                     clk,
  input  wire                                     DFTSE,
  output wire [`CA53_L2_SIZE_W-1:0]               l2_size_o,
  input  wire                                     l2_tagram_clken_i,
  input  wire [`CA53_SCU_L2_ASSOC-1:0]            l2_tagram_en_i,
  input  wire                                     l2_tagram_wr_i,
  input  wire [`CA53_SCU_L2_TAGRAM_ADDR_W-1:0]    l2_tagram_addr_i,
  input  wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_wdata_i,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way0_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way1_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way2_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way3_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way4_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way5_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way6_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way7_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way8_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way9_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way10_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way11_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way12_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way13_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way14_rdata_o,
  output wire [`CA53_SCU_L2_TAGRAM_DATA_W-1:0]    l2_tagram_way15_rdata_o,
  input  wire                                     l2_victimram_no_acc_next_cycle_i,
  input  wire                                     l2_victimram_clken_i,
  input  wire                                     l2_victimram_en_i,
  input  wire                                     l2_victimram_wr_i,
  input  wire [`CA53_SCU_L2_VICTIMRAM_STRB_W-1:0] l2_victimram_strb_i,
  input  wire [`CA53_SCU_L2_VICTIMRAM_ADDR_W-1:0] l2_victimram_addr_i,
  input  wire [`CA53_SCU_L2_VICTIMRAM_DATA_W-1:0] l2_victimram_wdata_i,
  output wire [`CA53_SCU_L2_VICTIMRAM_DATA_W-1:0] l2_victimram_rdata_o
);

  //-----------------------------------------------------------------------------
  //  Declarations
  //-----------------------------------------------------------------------------

  wire [`CA53_SCU_L2_ASSOC-1:0]                 clk_tagram;
  wire                                          clk_victimram;
  wire [`CA53_L2_SIZE_W-1:0]                    l2_size;
  wire [31:0]                                   l2_victimram_bit_strb;

  genvar i;

  //-----------------------------------------------------------------------------
  //  Main code
  //-----------------------------------------------------------------------------

  // Intermediate clock gates, to reduce idle RAM power. If the
  // tag RAMs are physically located close together then a single
  // clock gate could be shared between all tag RAMs.

  generate for (i = 0; i < `CA53_SCU_L2_ASSOC; i = i + 1) begin : g_clk_gate

    ca53_cell_inter_clkgate u_tag_clkgate (
      .clk_i         (clk),
      .clk_enable_i  (l2_tagram_clken_i),
      .clk_senable_i (DFTSE),
      .clk_gated_o   (clk_tagram[i])
    );

  end endgenerate

  ca53_cell_inter_clkgate u_victim_clkgate (
    .clk_i         (clk),
    .clk_enable_i  (l2_victimram_clken_i),
    .clk_senable_i (DFTSE),
    .clk_gated_o   (clk_victimram)
  );

  // L2 tag RAMs
  // These RAM instances should be sized according to the L2 cache size:
  //
  // L2 cache size | RAM instance size (per way)
  // ---------------+----------------------------
  //         128KB |  128 words x 33 bits        |  128 words x 40 bits
  //         256KB |  256 words x 32 bits        |  256 words x 39 bits
  //         512KB |  512 words x 31 bits        |  512 words x 38 bits
  //        1024KB | 1024 words x 30 bits        | 1024 words x 37 bits
  //        2048KB | 2048 words x 29 bits        | 2048 words x 36 bits
  //
  // The address lines of the RAMs should always be connected to the low-order
  // bits of the address bus - any unused bits should be left unconnected
  //
  // The data lines of the RAMs should always be connected to the high-order
  // bits of the data bus - any unused bits on the wdata bus should be left
  // unconnected, and any unused low-order bits on the rdata busses should be
  // tied low


  assign l2_size = L2_CACHE_SIZE;


//  ca53_bit_ram #(
//    .addr_bits  (`CA53_SCU_L2_TAGRAM_ADDR_W),
//    .data_bits  (`CA53_SCU_L2_TAGRAM_DATA_W),
//    .reset      (0),
//    .reset_high (0)
//  ) u_l2_tagram_way0 (
//    .clk             (clk),
//    .addr_i          (l2_tagram_addr_masked),
//    .rd_o            (l2_tagram_way0_rdata_o),
//    .wd_i            (l2_tagram_wdata_i),
//    .cs_i            (l2_tagram_en_i[0]),
//    .we_i            ({`CA53_SCU_L2_TAGRAM_DATA_W{l2_tagram_wr_i}})
//  );

//Lowest 3 bits of l2_tagram_way0_rdata_o are unused due to chosen cache size
  assign l2_tagram_way0_rdata_o[0] = 1'b0;
  assign l2_tagram_way1_rdata_o[0] = 1'b0;
  assign l2_tagram_way2_rdata_o[0] = 1'b0;
  assign l2_tagram_way3_rdata_o[0] = 1'b0;
  assign l2_tagram_way4_rdata_o[0] = 1'b0;
  assign l2_tagram_way5_rdata_o[0] = 1'b0;
  assign l2_tagram_way6_rdata_o[0] = 1'b0;
  assign l2_tagram_way7_rdata_o[0] = 1'b0;
  assign l2_tagram_way8_rdata_o[0] = 1'b0;
  assign l2_tagram_way9_rdata_o[0] = 1'b0;
  assign l2_tagram_way10_rdata_o[0] = 3'b0;
  assign l2_tagram_way11_rdata_o[0] = 3'b0;
  assign l2_tagram_way12_rdata_o[0] = 3'b0;
  assign l2_tagram_way13_rdata_o[0] = 3'b0;
  assign l2_tagram_way14_rdata_o[0] = 3'b0;
  assign l2_tagram_way15_rdata_o[0] = 3'b0;

  L2_tag u_l2_tagram_way0 (
                                   .CLK       (clk_tagram[0]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way0_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[0]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );

  L2_tag u_l2_tagram_way1 (
                                   .CLK       (clk_tagram[1]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way1_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[1]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  L2_tag u_l2_tagram_way2 (
                                   .CLK       (clk_tagram[2]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way2_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[2]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  L2_tag u_l2_tagram_way3 (
                                   .CLK       (clk_tagram[3]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way3_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[3]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  L2_tag u_l2_tagram_way4 (
                                   .CLK       (clk_tagram[4]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way4_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[4]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  L2_tag u_l2_tagram_way5 (
                                   .CLK       (clk_tagram[5]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way5_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[5]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  L2_tag u_l2_tagram_way6 (
                                   .CLK       (clk_tagram[6]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way6_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[6]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  L2_tag u_l2_tagram_way7 (
                                   .CLK       (clk_tagram[7]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way7_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[7]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way8 (
                                   .CLK       (clk_tagram[8]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way8_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[8]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way9 (
                                   .CLK       (clk_tagram[9]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way9_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[9]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way10 (
                                   .CLK       (clk_tagram[10]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way10_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[10]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way11 (
                                   .CLK       (clk_tagram[11]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way11_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[11]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way12 (
                                   .CLK       (clk_tagram[12]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way12_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[12]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way13 (
                                   .CLK       (clk_tagram[13]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way13_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[13]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way14 (
                                   .CLK       (clk_tagram[14]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way14_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[14]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
  L2_tag u_l2_tagram_way15 (
                                   .CLK       (clk_tagram[15]),
                                   .A         (l2_tagram_addr_i[7:0]),
                                   .Q         (l2_tagram_way15_rdata_o[32:1]),
                                   .D         (l2_tagram_wdata_i[32:1]),
                                   .CEN       (~l2_tagram_en_i[15]),
                                   .GWEN       (~l2_tagram_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .GWENY     (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN      (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
                                   
  assign l2_victimram_bit_strb = {{2{l2_victimram_strb_i[15]}},
                                  {2{l2_victimram_strb_i[14]}},
                                  {2{l2_victimram_strb_i[13]}},
                                  {2{l2_victimram_strb_i[12]}},
                                  {2{l2_victimram_strb_i[11]}},
                                  {2{l2_victimram_strb_i[10]}},
                                  {2{l2_victimram_strb_i[9]}},
                                  {2{l2_victimram_strb_i[8]}},
                                  {2{l2_victimram_strb_i[7]}},
                                  {2{l2_victimram_strb_i[6]}},
                                  {2{l2_victimram_strb_i[5]}},
                                  {2{l2_victimram_strb_i[4]}},
                                  {2{l2_victimram_strb_i[3]}},
                                  {2{l2_victimram_strb_i[2]}},
                                  {2{l2_victimram_strb_i[1]}},
                                  {2{l2_victimram_strb_i[0]}}};

  L2_vict u_l2_victimram (
                                   .CLK       (clk_victimram),
                                   .A         (l2_victimram_addr_i[7:0]),
                                   .Q         (l2_victimram_rdata_o[31:0]),
                                   .D         (l2_victimram_wdata_i[31:0]),
                                   .CEN       (~l2_victimram_en_i),
                                   .WEN       (~l2_victimram_bit_strb[31:0]),
                                   .GWEN      (~l2_victimram_wr_i),
                                   .GWENY     (),
                                   .TGWEN     (1'b1),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (32'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .WENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TWEN      (32'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );

  assign l2_size_o = l2_size;

endmodule

/*ARMAUTO_UNDEF*/
`define CA53_UNDEFINE
`include "cortexa53params.v"
`include "ca53scu_defs.v"
`undef CA53_UNDEFINE
/*END*/
