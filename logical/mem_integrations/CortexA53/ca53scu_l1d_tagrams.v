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
//  Wrapper for SCU duplicate tag RAMs
//-----------------------------------------------------------------------------
//
`include "ca53scu_defs.v"
`include "cortexa53params.v"

module ca53scu_l1d_tagrams `CA53_L1D_RAM_PARAM_DECL (
  input  wire                                   clk,
  input  wire                                   DFTSE,
  input  wire [`CA53_L1DC_SIZE_W-1:0]           l1_dc_size_i,
  input  wire                                   l1d_tagram_clken_i,
  input  wire [`CA53_SCU_L1D_ASSOC-1:0]         l1d_tagram_cpu0_en_i,
  input  wire                                   l1d_tagram_cpu0_wr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0] l1d_tagram_cpu0_addr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu0_wdata_i,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu0_way0_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu0_way1_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu0_way2_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu0_way3_rdata_o,
  input  wire [`CA53_SCU_L1D_ASSOC-1:0]         l1d_tagram_cpu1_en_i,
  input  wire                                   l1d_tagram_cpu1_wr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0] l1d_tagram_cpu1_addr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu1_wdata_i,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu1_way0_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu1_way1_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu1_way2_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu1_way3_rdata_o,
  input  wire [`CA53_SCU_L1D_ASSOC-1:0]         l1d_tagram_cpu2_en_i,
  input  wire                                   l1d_tagram_cpu2_wr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0] l1d_tagram_cpu2_addr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu2_wdata_i,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu2_way0_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu2_way1_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu2_way2_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu2_way3_rdata_o,
  input  wire [`CA53_SCU_L1D_ASSOC-1:0]         l1d_tagram_cpu3_en_i,
  input  wire                                   l1d_tagram_cpu3_wr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0] l1d_tagram_cpu3_addr_i,
  input  wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu3_wdata_i,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu3_way0_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu3_way1_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu3_way2_rdata_o,
  output wire [`CA53_SCU_L1D_TAGRAM_DATA_W-1:0] l1d_tagram_cpu3_way3_rdata_o
);

  //-----------------------------------------------------------------------------
  //  Declarations
  //-----------------------------------------------------------------------------

  wire [NUM_CPUS-1:0]                            clk_tagram;
  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0]         cpu0_addr;
  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0]         cpu1_addr;
  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0]         cpu2_addr;
  wire [`CA53_SCU_L1D_TAGRAM_ADDR_W-1:0]         cpu3_addr;

  genvar i;

  //-----------------------------------------------------------------------------
  //  Main code
  //-----------------------------------------------------------------------------

  // Intermediate clock gates, to reduce idle RAM power. If the
  // tag RAMs are physically located close together then a single
  // clock gate could be shared between all tag RAMs.

  generate for (i = 0; i < NUM_CPUS; i = i + 1) begin : g_clk_gate

    ca53_cell_inter_clkgate u_tag_clkgate (
      .clk_i         (clk),
      .clk_enable_i  (l1d_tagram_clken_i),
      .clk_senable_i (DFTSE),
      .clk_gated_o   (clk_tagram[i])
    );

  end endgenerate

  // L1 duplicate tag RAMs
  // These RAM instances should be the same as the data tag RAMs for the
  // corresponding cores:
  // L1 dcache size | RAM instance size (per way)
  // ---------------+----------------------------
  //            8KB |  32 words x 32 bits
  //           16KB |  64 words x 31 bits
  //           32KB | 128 words x 30 bits
  //           64KB | 256 words x 29 bits
  //
  // The address lines of the RAMs should always be connected to the low-order
  // bits of the address bus - any unused bits should be left unconnected
  //
  // The data lines of the RAMs should always be connected to the high-order
  // bits of the data bus - any unused bits on the wdata bus should be left
  // unconnected, and any unused low-order bits on the rdata busses should be
  // tied low

  assign cpu0_addr = l1d_tagram_cpu0_addr_i & {l1_dc_size_i, {(`CA53_SCU_L1D_TAGRAM_ADDR_W-`CA53_L1DC_SIZE_W){1'b1}}};

  generate if (NUM_CPUS >= 1) begin : g_l1d_cpu0_rams
//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu0_way0 (
//      .clk             (clk),
//      .addr_i          (cpu0_addr),
//      .rd_o            (l1d_tagram_cpu0_way0_rdata_o),
//      .wd_i            (l1d_tagram_cpu0_wdata_i),
//      .cs_i            (l1d_tagram_cpu0_en_i[0]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu0_wr_i}})
//    );
//
//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu0_way1 (
//      .clk             (clk),
//      .addr_i          (cpu0_addr),
//      .rd_o            (l1d_tagram_cpu0_way1_rdata_o),
//      .wd_i            (l1d_tagram_cpu0_wdata_i),
//      .cs_i            (l1d_tagram_cpu0_en_i[1]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu0_wr_i}})
//    );
//
//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu0_way2 (
//      .clk             (clk),
//      .addr_i          (cpu0_addr),
//      .rd_o            (l1d_tagram_cpu0_way2_rdata_o),
//      .wd_i            (l1d_tagram_cpu0_wdata_i),
//      .cs_i            (l1d_tagram_cpu0_en_i[2]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu0_wr_i}})
//    );
//
//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu0_way3 (
//      .clk             (clk),
//      .addr_i          (cpu0_addr),
//      .rd_o            (l1d_tagram_cpu0_way3_rdata_o),
//      .wd_i            (l1d_tagram_cpu0_wdata_i),
//      .cs_i            (l1d_tagram_cpu0_en_i[3]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu0_wr_i}})
//    );

//Lowest 2 bits of l1d_tagram_cpu0_way0_rdata_o are unused due to chosen cache size
//assign l1d_tagram_cpu0_way0_rdata_o[0] = 1'b0;
//assign l1d_tagram_cpu0_way1_rdata_o[0] = 1'b0;
//assign l1d_tagram_cpu0_way2_rdata_o[0] = 1'b0;
//assign l1d_tagram_cpu0_way3_rdata_o[0] = 1'b0;

          L1_tag u_l1d_tagram_cpu0_way0 (
                                   .CLK       (clk_tagram[0]),
                                   .A         (cpu0_addr[6:0]),
                                   .Q         (l1d_tagram_cpu0_way0_rdata_o[32:0]),
                                   .D         (l1d_tagram_cpu0_wdata_i[32:0]),
                                   .CEN       (~l1d_tagram_cpu0_en_i[0]),
                                   .GWEN       (~l1d_tagram_cpu0_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (7'd0),
                                   .TD        (33'd0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu0_way1 (
                                   .CLK       (clk_tagram[0]),
                                   .A         (cpu0_addr[6:0]),
                                   .Q         (l1d_tagram_cpu0_way1_rdata_o[32:0]),
                                   .D         (l1d_tagram_cpu0_wdata_i[32:0]),
                                   .CEN       (~l1d_tagram_cpu0_en_i[1]),
                                   .GWEN       (~l1d_tagram_cpu0_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (7'd0),
                                   .TD        (33'd0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu0_way2 (
                                   .CLK       (clk_tagram[0]),
                                   .A         (cpu0_addr[6:0]),
                                   .Q         (l1d_tagram_cpu0_way2_rdata_o[32:0]),
                                   .D         (l1d_tagram_cpu0_wdata_i[32:0]),
                                   .CEN       (~l1d_tagram_cpu0_en_i[2]),
                                   .GWEN       (~l1d_tagram_cpu0_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (7'd0),
                                   .TD        (33'd0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu0_way3 (
                                   .CLK       (clk_tagram[0]),
                                   .A         (cpu0_addr[6:0]),
                                   .Q         (l1d_tagram_cpu0_way3_rdata_o[32:0]),
                                   .D         (l1d_tagram_cpu0_wdata_i[32:0]),
                                   .CEN       (~l1d_tagram_cpu0_en_i[3]),
                                   .GWEN       (~l1d_tagram_cpu0_wr_i),
                                   .EMA       (3'b011),
                                   .EMAW      (2'b01),
                                   .EMAS      (1'b0),
                                   .STOV      (1'b0),
                                   .TA        (7'd0),
                                   .TD        (33'd0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  end endgenerate

  generate if (NUM_CPUS>=2) begin : g_l1d_cpu1_rams
    assign cpu1_addr = l1d_tagram_cpu1_addr_i & {l1_dc_size_i, {(`CA53_SCU_L1D_TAGRAM_ADDR_W-`CA53_L1DC_SIZE_W){1'b1}}};

//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu1_way0 (
//      .clk             (clk),
//      .addr_i          (cpu1_addr),
//      .rd_o            (l1d_tagram_cpu1_way0_rdata_o),
//      .wd_i            (l1d_tagram_cpu1_wdata_i),
//      .cs_i            (l1d_tagram_cpu1_en_i[0]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu1_wr_i}})
//    );
//
//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu1_way1 (
//      .clk             (clk),
//      .addr_i          (cpu1_addr),
//      .rd_o            (l1d_tagram_cpu1_way1_rdata_o),
//      .wd_i            (l1d_tagram_cpu1_wdata_i),
//      .cs_i            (l1d_tagram_cpu1_en_i[1]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu1_wr_i}})
//    );
//
//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu1_way2 (
//      .clk             (clk),
//      .addr_i          (cpu1_addr),
//      .rd_o            (l1d_tagram_cpu1_way2_rdata_o),
//      .wd_i            (l1d_tagram_cpu1_wdata_i),
//      .cs_i            (l1d_tagram_cpu1_en_i[2]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu1_wr_i}})
//    );
//
//    ca53_bit_ram #(
//      .addr_bits  (`CA53_SCU_L1D_TAGRAM_ADDR_W),
//      .data_bits  (`CA53_SCU_L1D_TAGRAM_DATA_W),
//      .reset      (0),
//      .reset_high (0)
//    ) u_l1d_tagram_cpu1_way3 (
//      .clk             (clk),
//      .addr_i          (cpu1_addr),
//      .rd_o            (l1d_tagram_cpu1_way3_rdata_o),
//      .wd_i            (l1d_tagram_cpu1_wdata_i),
//      .cs_i            (l1d_tagram_cpu1_en_i[3]),
//      .we_i            ({`CA53_SCU_L1D_TAGRAM_DATA_W{l1d_tagram_cpu1_wr_i}})
//    );

//Lowest 2 bits of l1d_tagram_cpu0_way0_rdata_o are unused due to chosen cache size
assign l1d_tagram_cpu1_way0_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu1_way1_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu1_way2_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu1_way3_rdata_o[1:0] = 2'b00;

          L1_tag u_l1d_tagram_cpu1_way0 (
                                   .CLK       (clk_tagram[1]),
                                   .A         (cpu1_addr[6:0]),
                                   .Q         (l1d_tagram_cpu1_way0_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu1_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu1_en_i[0]),
                                   .GWEN       (~l1d_tagram_cpu1_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu1_way1 (
                                   .CLK       (clk_tagram[1]),
                                   .A         (cpu1_addr[6:0]),
                                   .Q         (l1d_tagram_cpu1_way1_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu1_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu1_en_i[1]),
                                   .GWEN       (~l1d_tagram_cpu1_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu1_way2 (
                                   .CLK       (clk_tagram[1]),
                                   .A         (cpu1_addr[6:0]),
                                   .Q         (l1d_tagram_cpu1_way2_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu1_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu1_en_i[2]),
                                   .GWEN       (~l1d_tagram_cpu1_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu1_way3 (
                                   .CLK       (clk_tagram[1]),
                                   .A         (cpu1_addr[6:0]),
                                   .Q         (l1d_tagram_cpu1_way3_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu1_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu1_en_i[3]),
                                   .GWEN       (~l1d_tagram_cpu1_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );


  end else begin
    assign l1d_tagram_cpu1_way0_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu1_way1_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu1_way2_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu1_way3_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
  end endgenerate

  generate if (NUM_CPUS>=3) begin : g_l1d_cpu2_rams
    assign cpu2_addr = l1d_tagram_cpu2_addr_i & {l1_dc_size_i, {(`CA53_SCU_L1D_TAGRAM_ADDR_W-`CA53_L1DC_SIZE_W){1'b1}}};

//Lowest 2 bits of l1d_tagram_cpu0_way0_rdata_o are unused due to chosen cache size
assign l1d_tagram_cpu2_way0_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu2_way1_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu2_way2_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu2_way3_rdata_o[1:0] = 2'b00;


          L1_tag u_l1d_tagram_cpu2_way0 (
                                   .CLK       (clk_tagram[2]),
                                   .A         (cpu2_addr[6:0]),
                                   .Q         (l1d_tagram_cpu2_way0_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu2_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu2_en_i[0]),
                                   .GWEN       (~l1d_tagram_cpu2_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu2_way1 (
                                   .CLK       (clk_tagram[2]),
                                   .A         (cpu2_addr[6:0]),
                                   .Q         (l1d_tagram_cpu2_way1_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu2_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu2_en_i[1]),
                                   .GWEN       (~l1d_tagram_cpu2_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu2_way2 (
                                   .CLK       (clk_tagram[2]),
                                   .A         (cpu2_addr[6:0]),
                                   .Q         (l1d_tagram_cpu2_way2_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu2_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu2_en_i[2]),
                                   .GWEN       (~l1d_tagram_cpu2_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu2_way3 (
                                   .CLK       (clk_tagram[2]),
                                   .A         (cpu2_addr[6:0]),
                                   .Q         (l1d_tagram_cpu2_way3_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu2_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu2_en_i[3]),
                                   .GWEN       (~l1d_tagram_cpu2_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );



  end else begin
    assign l1d_tagram_cpu2_way0_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu2_way1_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu2_way2_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu2_way3_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
  end endgenerate

  generate if (NUM_CPUS>=4) begin : g_l1d_cpu3_rams
    assign cpu3_addr = l1d_tagram_cpu3_addr_i & {l1_dc_size_i, {(`CA53_SCU_L1D_TAGRAM_ADDR_W-`CA53_L1DC_SIZE_W){1'b1}}};

//Lowest 2 bits of l1d_tagram_cpu0_way0_rdata_o are unused due to chosen cache size
assign l1d_tagram_cpu3_way0_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu3_way1_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu3_way2_rdata_o[1:0] = 2'b00;
assign l1d_tagram_cpu3_way3_rdata_o[1:0] = 2'b00;

          L1_tag u_l1d_tagram_cpu3_way0 (
                                   .CLK       (clk_tagram[3]),
                                   .A         (cpu3_addr[6:0]),
                                   .Q         (l1d_tagram_cpu3_way0_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu3_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu3_en_i[0]),
                                   .GWEN       (~l1d_tagram_cpu3_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu3_way1 (
                                   .CLK       (clk_tagram[3]),
                                   .A         (cpu3_addr[6:0]),
                                   .Q         (l1d_tagram_cpu3_way1_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu3_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu3_en_i[1]),
                                   .GWEN       (~l1d_tagram_cpu3_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu3_way2 (
                                   .CLK       (clk_tagram[3]),
                                   .A         (cpu3_addr[6:0]),
                                   .Q         (l1d_tagram_cpu3_way2_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu3_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu3_en_i[2]),
                                   .GWEN       (~l1d_tagram_cpu3_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );
          L1_tag u_l1d_tagram_cpu3_way3 (
                                   .CLK       (clk_tagram[3]),
                                   .A         (cpu3_addr[6:0]),
                                   .Q         (l1d_tagram_cpu3_way3_rdata_o[32:2]),
                                   .D         (l1d_tagram_cpu3_wdata_i[32:2]),
                                   .CEN       (~l1d_tagram_cpu3_en_i[3]),
                                   .GWEN       (~l1d_tagram_cpu3_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (6'b0),
                                   .TD        (31'b0),
                                   .SO        (),
                                   .CENY      (),
                                   .AY        (),
                                   .TEN       (1'b1),
                                   .TCEN      (1'b1),
                                   .TGWEN     (1'b1),
                                   .SI        (2'b00),
                                   .SE        (DFTSE),
                                   .DFTRAMBYP (1'b0),
                                   .RET1N     (1'b1),
                                   .RET2N     (1'b1),
                                   .PGEN      (1'b0)
                                   );

  end else begin
    assign l1d_tagram_cpu3_way0_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu3_way1_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu3_way2_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
    assign l1d_tagram_cpu3_way3_rdata_o = {`CA53_SCU_L1D_TAGRAM_DATA_W{1'b0}};
  end endgenerate

endmodule

/*ARMAUTO_UNDEF*/
`define CA53_UNDEFINE
`include "cortexa53params.v"
`include "ca53scu_defs.v"
`undef CA53_UNDEFINE
/*END*/
