//------------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2004-2014 ARM Limited.
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
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Abstract : Caches and TLB RAMs wrapper
//
//-----------------------------------------------------------------------------
//

`include "cortexa53params.v"

module ca53_caches_tlb_rams `CA53_L1_RAM_PARAM_DECL
(
  input wire                            clk,
  input wire                            DFTSE,

  output wire [2:0]                     ic_size_o,
  output wire [2:0]                     dc_size_o,

  // L1 Data Cache Data RAMs
  // 8 banks (byte enable without ECC, global enable with ECC)
  input wire [7:0]                      dc_dataram_en_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb0_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb1_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb2_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb3_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb4_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb5_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb6_i,
  input wire [`CA53_DDATA_WEN_W-1:0]    dc_dataram_strb7_i,
  input wire                            dc_dataram_wr_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata0_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata1_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata2_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata3_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata4_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata5_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata6_i,
  input wire [`CA53_DDATA_RAM_W-1:0]    dc_dataram_wdata7_i,
  input wire [10:0]                     dc_dataram_addr0_i,
  input wire [10:0]                     dc_dataram_addr1_i,
  input wire [10:0]                     dc_dataram_addr2_i,
  input wire [10:0]                     dc_dataram_addr3_i,
  input wire [10:0]                     dc_dataram_addr4_i,
  input wire [10:0]                     dc_dataram_addr5_i,
  input wire [10:0]                     dc_dataram_addr6_i,
  input wire [10:0]                     dc_dataram_addr7_i,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata0_o,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata1_o,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata2_o,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata3_o,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata4_o,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata5_o,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata6_o,
  output wire [`CA53_DDATA_RAM_W-1:0]   dc_dataram_rdata7_o,

  // tag rams for DCACHE
  // 4 banks of 26-bit rams (with global enable)
  input wire [3:0]                      dc_tagram_en_i,
  input wire                            dc_tagram_wr_i,
  input wire [`CA53_DTAG_RAM_W-1:0]     dc_tagram_wdata_i,
  input wire [7:0]                      dc_tagram_addr_i,
  output wire [`CA53_DTAG_RAM_W-1:0]    dc_tagram_rdata0_o,
  output wire [`CA53_DTAG_RAM_W-1:0]    dc_tagram_rdata1_o,
  output wire [`CA53_DTAG_RAM_W-1:0]    dc_tagram_rdata2_o,
  output wire [`CA53_DTAG_RAM_W-1:0]    dc_tagram_rdata3_o,

  // dirty ram for DCACHE
  // 1 bank of 12-bit ram (with bit enable)
  input wire                            dc_dirtyram_en_i,
  input wire [`CA53_DDIRTY_RAM_W-1:0]   dc_dirtyram_strb_i,
  input wire                            dc_dirtyram_wr_i,
  input wire [`CA53_DDIRTY_RAM_W-1:0]   dc_dirtyram_wdata_i,
  input wire [8:0]                      dc_dirtyram_addr_i,
  output wire [`CA53_DDIRTY_RAM_W-1:0]  dc_dirtyram_rdata_o,

  // data rams for I-CACHE
  // 2 banks of 72-bit rams (with sub-data (9-bits) enable)
  input wire [3:0]                      ic_dataram_en_i,
  input wire                            ic_dataram_wr_i,
  input wire [11:0]                     ic_dataram_addr0_i,
  input wire [11:0]                     ic_dataram_addr1_i,
  input wire [`CA53_IDATA_WEN_W-1:0]    ic_dataram_strb0_i,
  input wire [`CA53_IDATA_WEN_W-1:0]    ic_dataram_strb1_i,
  input wire [`CA53_IDATA_RAM_W-1:0]    ic_dataram_wdata0_i,
  input wire [`CA53_IDATA_RAM_W-1:0]    ic_dataram_wdata1_i,
  output wire [`CA53_IDATA_RAM_W-1:0]   ic_dataram_rdata0_o,
  output wire [`CA53_IDATA_RAM_W-1:0]   ic_dataram_rdata1_o,
  // tag rams for Instruction Cache (via the ICU)
  // 2 banks of 24-bit rams (with global enable)
  input wire [1:0]                      ic_tagram_en_i,
  input wire                            ic_tagram_wr_i,
  input wire [`CA53_ITAG_RAM_W-1:0]     ic_tagram_wdata_i,
  input wire [8:0]                      ic_tagram_addr_i,
  output wire [`CA53_ITAG_RAM_W-1:0]    ic_tagram_rdata0_o,
  output wire [`CA53_ITAG_RAM_W-1:0]    ic_tagram_rdata1_o,

  // RAM interface for the TLB

  // 2 banks of 86-bits rams
  input wire [3:0]                      tlb_ram_en_i,
  input wire                            tlb_ram_wr_i,
  input wire [`CA53_TLB_RAM_W-1:0]      tlb_ram_wdata_i,
  input wire [`CA53_TLB_RAM_ADDR_W-1:0] tlb_ram_addr_i,
  output wire [`CA53_TLB_RAM_W-1:0]     tlb_ram_rdata0_o,
  output wire [`CA53_TLB_RAM_W-1:0]     tlb_ram_rdata1_o,
  output wire [`CA53_TLB_RAM_W-1:0]     tlb_ram_rdata2_o,
  output wire [`CA53_TLB_RAM_W-1:0]     tlb_ram_rdata3_o,

  // BTAC RAM Interface
  // 2 banks (global enable)
  input  wire                               btac_stg0_ram_en_i,
  input  wire                               btac_stg0_ram_wr_i,
  input  wire [(`CA53_BTAC_RAM_S0D_W-1):0]  btac_stg0_ram_wdata_i,
  input  wire [(`CA53_BTAC_RAM_ADDR_W-1):0] btac_stg0_ram_addr_i,
  input  wire                               btac_stg1_ram_en_i,
  input  wire                               btac_stg1_ram_wr_i,
  input  wire [(`CA53_BTAC_RAM_S1D_W-1):0]  btac_stg1_ram_wdata_i,
  input  wire [(`CA53_BTAC_RAM_ADDR_W-1):0] btac_stg1_ram_addr_i,
  output wire [(`CA53_BTAC_RAM_S0D_W-1):0]  btac_stg0_ram_rdata_o,
  output wire [(`CA53_BTAC_RAM_S1D_W-1):0]  btac_stg1_ram_rdata_o
  );

  // ---------------------------------------------------
  // Main
  // ---------------------------------------------------

  // Local size signals declaration which can be used to force a value
  // for dynamic verification and it can be used as a mask to set
  // generic RAM modules
  wire [2:0]                     i_size_mask;
  wire [2:0]                     d_size_mask;

  // `ifdef part is for DSM geneartion, `else part is for all other
  // cases. For DSM generation, cache sizes are passed through defines,
  // declared in a separate file and included here.
  `ifdef L1_CACHE_SMASK_DECLS
    `L1_CACHE_SMASK_DECLS
  `endif

  `ifdef L1_ICACHE_SMASK
     assign i_size_mask = `L1_ICACHE_SMASK;
  `else
     assign i_size_mask = L1_ICACHE_SIZE;
  `endif

  `ifdef L1_DCACHE_SMASK
     assign d_size_mask = `L1_DCACHE_SMASK;
  `else
     assign d_size_mask = L1_DCACHE_SIZE;
  `endif

  assign ic_size_o = i_size_mask;
  assign dc_size_o = d_size_mask;

  // -------------------
  // RAMs INSTANTIATION
  // -------------------

  // I Data RAM

  // NB Each bit of ic_dataram_strb_i[`CA53_IDATA_WEN_W-1:0]
  // represents `CA53_IDATA_RAM_W/`CA53_IDATA_WEN_W bits of the
  // write data (9 bits of 72 bits)

  L1_idata u_idata_bank0_h1 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr0_i[10:0]),
                                        .Q         (ic_dataram_rdata0_o[79:60]),
                                        .D         (ic_dataram_wdata0_i[79:60]),
                                        .CEN       (~ic_dataram_en_i[1]),
                                        .WEN       ({20{~(ic_dataram_strb0_i[3])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)
                                        );

  L1_idata u_idata_bank0_h0 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr0_i[10:0]),
                                        .Q         (ic_dataram_rdata0_o[59:40]),
                                        .D         (ic_dataram_wdata0_i[59:40]),
                                        .CEN       (~ic_dataram_en_i[1]),
                                        .WEN       ({20{~(ic_dataram_strb0_i[2])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)
                                        );


  L1_idata u_idata_bank0_l1 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr0_i[10:0]),
                                        .Q         (ic_dataram_rdata0_o[39:20]),
                                        .D         (ic_dataram_wdata0_i[39:20]),
                                        .CEN       (~ic_dataram_en_i[0]),
                                        .WEN       ({20{~(ic_dataram_strb0_i[1])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)

                                        );

  L1_idata u_idata_bank0_l0 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr0_i[10:0]),
                                        .Q         (ic_dataram_rdata0_o[19:0]),
                                        .D         (ic_dataram_wdata0_i[19:0]),
                                        .CEN       (~ic_dataram_en_i[0]),
                                        .WEN       ({20{~(ic_dataram_strb0_i[0])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)

                                        );

  L1_idata u_idata_bank1_h1 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr1_i[10:0]),
                                        .Q         (ic_dataram_rdata1_o[79:60]),
                                        .D         (ic_dataram_wdata1_i[79:60]),
                                        .CEN       (~ic_dataram_en_i[3]),
                                        .WEN       ({20{~(ic_dataram_strb1_i[3])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)

                                        );
  L1_idata u_idata_bank1_h0 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr1_i[10:0]),
                                        .Q         (ic_dataram_rdata1_o[59:40]),
                                        .D         (ic_dataram_wdata1_i[59:40]),
                                        .CEN       (~ic_dataram_en_i[3]),
                                        .WEN       ({20{~(ic_dataram_strb1_i[2])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)

                                        );

  L1_idata u_idata_bank1_l1 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr1_i[10:0]),
                                        .Q         (ic_dataram_rdata1_o[39:20]),
                                        .D         (ic_dataram_wdata1_i[39:20]),
                                        .CEN       (~ic_dataram_en_i[2]),
                                        .WEN       ({20{~(ic_dataram_strb1_i[1])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)

                                        );
  L1_idata u_idata_bank1_l0 (
                                        .CLK       (clk),
                                        .A         (ic_dataram_addr1_i[10:0]),
                                        .Q         (ic_dataram_rdata1_o[19:0]),
                                        .D         (ic_dataram_wdata1_i[19:0]),
                                        .CEN       (~ic_dataram_en_i[2]),
                                        .WEN       ({20{~(ic_dataram_strb1_i[0])}}),
                                        .EMA       (3'b011),
                                        .EMAW      (2'b01),
                                        .EMAS      (1'b0),
                                        .STOV      (1'b0),
                                        .GWEN      (~ic_dataram_wr_i),
                                        .GWENY     (),
                                        .TGWEN     (1'b1),
                                        .TA        (11'd0),
                                        .TD        (20'd0),
                                        .SO        (),
                                        .CENY      (),
                                        .WENY      (),
                                        .AY        (),
                                        .TEN       (1'b1),
                                        .TCEN      (1'b1),
                                        .TWEN      (20'hFFFFF),
                                        .SI        (2'b00),
                                        .SE        (DFTSE),
                                        .DFTRAMBYP (1'b0),
                                        .RET1N     (1'b1),
                                        .RET2N     (1'b1),
                                        .PGEN      (1'b0)

                                        );
  // I Tag RAM
  // Reset the Tag RAM memory to n`b1111... to make
  // sure the most significant bits are 2'b11
  // indicating an non-valid tag.


  L1_itag u_itag_ram0 (
                                   .CLK       (clk),
                                   .A         (ic_tagram_addr_i[7:0]),
                                   .Q         (ic_tagram_rdata0_o[`CA53_ITAG_RAM_W-1:0]), //connection size 31
                                   .D         (ic_tagram_wdata_i[`CA53_ITAG_RAM_W-1:0]), //connection size 31
                                   .CEN       (~ic_tagram_en_i[0]),
                                   .GWEN      (~ic_tagram_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (31'b0),
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

  L1_itag u_itag_ram1 (
                                   .CLK       (clk),
                                   .A         (ic_tagram_addr_i[7:0]),
                                   .Q         (ic_tagram_rdata1_o[`CA53_ITAG_RAM_W-1:0]),
                                   .D         (ic_tagram_wdata_i[`CA53_ITAG_RAM_W-1:0]),
                                   .CEN       (~ic_tagram_en_i[1]),
                                   .GWEN      (~ic_tagram_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (8'b0),
                                   .TD        (31'b0),
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

  // D Data Ram

  // NB Each bit of dc_dataram_strb_i[`CA53_DDATA_WEN_W-1:0]
  // represents `CA53_DDATA_RAM_W/`CA53_DDATA_WEN_W bits of the
  // write data (8 bits of 32 bits)


  wire [31:0] dc_dataram_wen0_i;
  wire [31:0] dc_dataram_wen1_i;
  wire [31:0] dc_dataram_wen2_i;
  wire [31:0] dc_dataram_wen3_i;
  wire [31:0] dc_dataram_wen4_i;
  wire [31:0] dc_dataram_wen5_i;
  wire [31:0] dc_dataram_wen6_i;
  wire [31:0] dc_dataram_wen7_i;

  assign dc_dataram_wen0_i = {{8{dc_dataram_strb0_i[3]}}, {8{dc_dataram_strb0_i[2]}}, {8{dc_dataram_strb0_i[1]}}, {8{dc_dataram_strb0_i[0]}}};
  assign dc_dataram_wen1_i = {{8{dc_dataram_strb1_i[3]}}, {8{dc_dataram_strb1_i[2]}}, {8{dc_dataram_strb1_i[1]}}, {8{dc_dataram_strb1_i[0]}}};
  assign dc_dataram_wen2_i = {{8{dc_dataram_strb2_i[3]}}, {8{dc_dataram_strb2_i[2]}}, {8{dc_dataram_strb2_i[1]}}, {8{dc_dataram_strb2_i[0]}}};
  assign dc_dataram_wen3_i = {{8{dc_dataram_strb3_i[3]}}, {8{dc_dataram_strb3_i[2]}}, {8{dc_dataram_strb3_i[1]}}, {8{dc_dataram_strb3_i[0]}}};
  assign dc_dataram_wen4_i = {{8{dc_dataram_strb4_i[3]}}, {8{dc_dataram_strb4_i[2]}}, {8{dc_dataram_strb4_i[1]}}, {8{dc_dataram_strb4_i[0]}}};
  assign dc_dataram_wen5_i = {{8{dc_dataram_strb5_i[3]}}, {8{dc_dataram_strb5_i[2]}}, {8{dc_dataram_strb5_i[1]}}, {8{dc_dataram_strb5_i[0]}}};
  assign dc_dataram_wen6_i = {{8{dc_dataram_strb6_i[3]}}, {8{dc_dataram_strb6_i[2]}}, {8{dc_dataram_strb6_i[1]}}, {8{dc_dataram_strb6_i[0]}}};
  assign dc_dataram_wen7_i = {{8{dc_dataram_strb7_i[3]}}, {8{dc_dataram_strb7_i[2]}}, {8{dc_dataram_strb7_i[1]}}, {8{dc_dataram_strb7_i[0]}}};


  L1_ddata u_ddata_bank0 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr0_i[9:0]),
                                      .Q         (dc_dataram_rdata0_o[`CA53_DDATA_RAM_W-1:0]), //connection size 32
                                      .D         (dc_dataram_wdata0_i[`CA53_DDATA_RAM_W-1:0]), //connection size 32
                                      .CEN       (~dc_dataram_en_i[0]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen0_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'd0),
                                      .TD        (32'd0),
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

  L1_ddata u_ddata_bank1 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr1_i[9:0]),
                                      .Q         (dc_dataram_rdata1_o[`CA53_DDATA_RAM_W-1:0]),
                                      .D         (dc_dataram_wdata1_i[`CA53_DDATA_RAM_W-1:0]),
                                      .CEN       (~dc_dataram_en_i[1]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen1_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'b0),
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

  L1_ddata u_ddata_bank2 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr2_i[9:0]),
                                      .Q         (dc_dataram_rdata2_o[`CA53_DDATA_RAM_W-1:0]),
                                      .D         (dc_dataram_wdata2_i[`CA53_DDATA_RAM_W-1:0]),
                                      .CEN       (~dc_dataram_en_i[2]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen2_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'b0),
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

  L1_ddata u_ddata_bank3 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr3_i[9:0]),
                                      .Q         (dc_dataram_rdata3_o[`CA53_DDATA_RAM_W-1:0]),
                                      .D         (dc_dataram_wdata3_i[`CA53_DDATA_RAM_W-1:0]),
                                      .CEN       (~dc_dataram_en_i[3]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen3_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'b0),
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

  L1_ddata u_ddata_bank4 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr4_i[9:0]),
                                      .Q         (dc_dataram_rdata4_o[`CA53_DDATA_RAM_W-1:0]),
                                      .D         (dc_dataram_wdata4_i[`CA53_DDATA_RAM_W-1:0]),
                                      .CEN       (~dc_dataram_en_i[4]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen4_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'b0),
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

  L1_ddata u_ddata_bank5 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr5_i[9:0]),
                                      .Q         (dc_dataram_rdata5_o[`CA53_DDATA_RAM_W-1:0]),
                                      .D         (dc_dataram_wdata5_i[`CA53_DDATA_RAM_W-1:0]),
                                      .CEN       (~dc_dataram_en_i[5]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen5_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'b0),
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

  L1_ddata u_ddata_bank6 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr6_i[9:0]),
                                      .Q         (dc_dataram_rdata6_o[`CA53_DDATA_RAM_W-1:0]),
                                      .D         (dc_dataram_wdata6_i[`CA53_DDATA_RAM_W-1:0]),
                                      .CEN       (~dc_dataram_en_i[6]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen6_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'b0),
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

  L1_ddata u_ddata_bank7 (
                                      .CLK       (clk),
                                      .A         (dc_dataram_addr7_i[9:0]),
                                      .Q         (dc_dataram_rdata7_o[`CA53_DDATA_RAM_W-1:0]),
                                      .D         (dc_dataram_wdata7_i[`CA53_DDATA_RAM_W-1:0]),
                                      .CEN       (~dc_dataram_en_i[7]),
                                      .GWEN      ({~(dc_dataram_wr_i)}),
                                      .WEN       (~dc_dataram_wen7_i[31:0]),
                                      .EMA       (3'b111),
                                      .EMAW      (2'b11),
                                      .EMAS      (1'b1),
                                      .STOV      (1'b0),
                                      .TA        (10'b0),
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


  // D Tag Ram

  //Lowest 2 bits of dc_tagram_rdata0_o are unused due to chosen cache size
  assign dc_tagram_rdata0_o[1:0] = 2'b0;
  assign dc_tagram_rdata1_o[1:0] = 2'b0;
  assign dc_tagram_rdata2_o[1:0] = 2'b0;
  assign dc_tagram_rdata3_o[1:0] = 2'b0;
  wire [1:0] u_dtag_bank0_UNCONNECTED;
  wire [1:0] u_dtag_bank1_UNCONNECTED;
  wire [1:0] u_dtag_bank2_UNCONNECTED;
  wire [1:0] u_dtag_bank3_UNCONNECTED;      

  L1_dtag u_dtag_bank0 (
                                   .CLK       (clk),
                                   .A         (dc_tagram_addr_i[6:0]),
                                   .Q         ({u_dtag_bank0_UNCONNECTED,dc_tagram_rdata0_o[31:2]}), //connection size 32
                                   .D         ({2'b00,dc_tagram_wdata_i[31:2]}), //connection size 32
                                   .CEN       (~dc_tagram_en_i[0]),
                                   .GWEN       (~dc_tagram_wr_i),
                                   .EMA       (3'b111),
                                   .EMAW      (2'b11),
                                   .EMAS      (1'b1),
                                   .STOV      (1'b0),
                                   .TA        (7'b0),
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

  L1_dtag u_dtag_bank1 (
                                   .CLK       (clk),
                                   .A         (dc_tagram_addr_i[6:0]),
                                   .Q         ({u_dtag_bank1_UNCONNECTED,dc_tagram_rdata1_o[31:2]}),
                                   .D         ({2'b00,dc_tagram_wdata_i[31:2]}),
                                   .CEN       (~dc_tagram_en_i[1]),
                                   .GWEN       (~dc_tagram_wr_i),
                                        .EMA       (3'b111),
                                        .EMAW      (2'b11),
                                        .EMAS      (1'b1),
                                        .STOV      (1'b0),
                                   .TA        (7'b0),
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

  L1_dtag u_dtag_bank2 (
                                   .CLK       (clk),
                                   .A         (dc_tagram_addr_i[6:0]),
                                   .Q         ({u_dtag_bank2_UNCONNECTED,dc_tagram_rdata2_o[31:2]}),
                                   .D         ({2'b00,dc_tagram_wdata_i[31:2]}),
                                   .CEN       (~dc_tagram_en_i[2]),
                                   .GWEN       (~dc_tagram_wr_i),
                                        .EMA       (3'b111),
                                        .EMAW      (2'b11),
                                        .EMAS      (1'b1),
                                        .STOV      (1'b0),
                                   .TA        (7'b0),
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

  L1_dtag u_dtag_bank3 (
                                   .CLK       (clk),
                                   .A         (dc_tagram_addr_i[6:0]),
                                   .Q         ({u_dtag_bank3_UNCONNECTED,dc_tagram_rdata3_o[31:2]}),
                                   .D         ({2'b00,dc_tagram_wdata_i[31:2]}),
                                   .CEN       (~dc_tagram_en_i[3]),
                                   .GWEN       (~dc_tagram_wr_i),
                                        .EMA       (3'b111),
                                        .EMAW      (2'b11),
                                        .EMAS      (1'b1),
                                        .STOV      (1'b0),
                                   .TA        (7'b0),
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

  // D Dirty Ram


  L1_ddirty u_ddirty_ram (
                                     .CLK       (clk),
                                     .A         (dc_dirtyram_addr_i[7:0]),
                                     .Q         (dc_dirtyram_rdata_o[7:0]),
                                     .D         (dc_dirtyram_wdata_i[7:0]),
                                     .CEN       (~dc_dirtyram_en_i),
                                     .WEN       (~(dc_dirtyram_strb_i[7:0])),
                                        .EMA       (3'b111),
                                        .EMAW      (2'b11),
                                        .EMAS      (1'b1),
                                        .STOV      (1'b0),
                                     .GWEN      (~dc_dirtyram_wr_i),
                                     .GWENY     (),
                                     .TGWEN     (1'b1),
                                     .TA        (8'd0),
                                     .TD        (8'd0),
                                     .SO        (),
                                     .CENY      (),
                                     .WENY      (),
                                     .AY        (),
                                     .TEN       (1'b1),
                                     .TCEN      (1'b1),
                                     .TWEN      (8'hFF),
                                     .SI        (2'b00),
                                     .SE        (DFTSE),
                                     .DFTRAMBYP (1'b0),
                                     .RET1N     (1'b1),
                                     .RET2N     (1'b1),
                                     .PGEN      (1'b0)
                                     );

  // TLB Ram


  // Ram compiler can only generate even number of bits for these memory instances, 118th bit is unused
  wire [3:0] u_tlb_bank_i_UNCONNECTED;
  wire [3:0] u_tlb_bank_o_UNCONNECTED;

  assign u_tlb_bank_i_UNCONNECTED = 4'b0;

  L1_tlb u_tlb_bank0 (
                                  .CLK       (clk),
                                  .A         (tlb_ram_addr_i),
                                  .Q         (tlb_ram_rdata0_o[`CA53_TLB_RAM_W-1:0]),
                                  .D         (tlb_ram_wdata_i[`CA53_TLB_RAM_W-1:0]),
                                  .CEN       (~tlb_ram_en_i[0]),
                                  .GWEN       (~tlb_ram_wr_i),
                                  .EMA       (3'b111),
                                  .EMAW      (2'b11),
                                  .EMAS      (1'b1),
                                  .STOV      (1'b0),
                                  .TA        (8'b0),
                                  .TD        (114'b0),
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

  L1_tlb u_tlb_bank1 (
                                  .CLK       (clk),
                                  .A         (tlb_ram_addr_i),
                                  .Q         (tlb_ram_rdata1_o[`CA53_TLB_RAM_W-1:0]),
                                  .D         (tlb_ram_wdata_i[`CA53_TLB_RAM_W-1:0]),
                                  .CEN       (~tlb_ram_en_i[1]),
                                  .GWEN       (~tlb_ram_wr_i),
                                  .EMA       (3'b111),
                                  .EMAW      (2'b11),
                                  .EMAS      (1'b1),
                                  .STOV      (1'b0),
                                  .TA        (8'b0),
                                  .TD        (114'b0),
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

  L1_tlb u_tlb_bank2 (
                                  .CLK       (clk),
                                  .A         (tlb_ram_addr_i),
                                  .Q         (tlb_ram_rdata2_o[`CA53_TLB_RAM_W-1:0]),
                                  .D         (tlb_ram_wdata_i[`CA53_TLB_RAM_W-1:0]),
                                  .CEN       (~tlb_ram_en_i[2]),
                                  .GWEN       (~tlb_ram_wr_i),
                                  .EMA       (3'b111),
                                  .EMAW      (2'b11),
                                  .EMAS      (1'b1),
                                  .STOV      (1'b0),
                                  .TA        (8'b0),
                                  .TD        (114'b0),
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
  L1_tlb u_tlb_bank3 (
                                  .CLK       (clk),
                                  .A         (tlb_ram_addr_i),
                                  .Q         (tlb_ram_rdata3_o[`CA53_TLB_RAM_W-1:0]),
                                  .D         (tlb_ram_wdata_i[`CA53_TLB_RAM_W-1:0]),
                                  .CEN       (~tlb_ram_en_i[3]),
                                  .GWEN       (~tlb_ram_wr_i),
                                  .EMA       (3'b111),
                                  .EMAW      (2'b11),
                                  .EMAS      (1'b1),
                                  .STOV      (1'b0),
                                  .TA        (8'b0),
                                  .TD        (114'b0),
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

  L1_btac1 u_btac_stg0 (
                                     .CLK       (clk),
                                     .A         (btac_stg0_ram_addr_i),
                                     .Q         (btac_stg0_ram_rdata_o),
                                     .D         (btac_stg0_ram_wdata_i),
                                     .CEN       (~btac_stg0_ram_en_i),
                                     .GWEN       (~btac_stg0_ram_wr_i),
                                     .EMA       (3'b111),
                                     .EMAW      (2'b11),
                                     .EMAS      (1'b1),
                                     .STOV      (1'b0),
                                     .TA        (7'b0),
                                     .TD        (50'b0),
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

  L1_btac2 u_btac_stg1 (
                                     .CLK       (clk),
                                     .A         (btac_stg1_ram_addr_i),
                                     .Q         (btac_stg1_ram_rdata_o),
                                     .D         (btac_stg1_ram_wdata_i),
                                     .CEN       (~btac_stg1_ram_en_i),
                                     .GWEN       (~btac_stg1_ram_wr_i),
                                     .EMA       (3'b111),
                                     .EMAW      (2'b11),
                                     .EMAS      (1'b1),
                                     .STOV      (1'b0),
                                     .TA        (7'b0),
                                     .TD        (59'b0),
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

endmodule

/*ARMAUTO_UNDEF*/
`define CA53_UNDEFINE
`include "cortexa53params.v"
`undef CA53_UNDEFINE
/*END*/
