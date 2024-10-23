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
// Purpose:
//  Top level wrapper for the megasoc CPU subsystem.
//-----------------------------------------------------------------------------
// Modules instantiated:
//  megasoc_cpu_ss                  (u_megasoc_cpu_ss)
//  nic400_megasoc_main             (u_nic400_megasoc_main)
//  ROM_wrapper                     (u_ROM_wrapper)
//  sl_ahb_sram                     (u_sl_ahb_sram)
//  SRAM_wrapper                    (u_SRAM_wrapper)
//  megasoc_peripheral_subsystem    (u_megasoc_peripheral_subsystem)
//
//-----------------------------------------------------------------------------
// To Do
//  - Replace sl_ahb_sram with QSPI controller to use external flash
//  - Add DDR controller for DRAM port on NIC400


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
    input wire         AXI_EXP_SYS_rready,

    // QSPI Signals
    output wire         QSPI_SCLK,
    output wire         QSPI_nCS,
    output wire [3:0]   QSPI_IO_o,
    input  wire [3:0]   QSPI_IO_i,
    output wire [3:0]   QSPI_IO_e,

    // UART signals
    input  wire         UARTRXD,
    output wire         UARTTXD,
    output wire         UARTTXEN,

    // DAP-LITE external signals
    input  wire         nTRST,
    input  wire         SWCLKTCK,
    input  wire         SWDITMS,
    input  wire         TDI,
    output wire         TDO,
    output wire         nTDOEN,
    output wire         SWDO,
    output wire         SWDOEN
);


parameter ID_W=6;
parameter NUM_SPIS=480;

wire                CPU_AWREADYM;
wire                CPU_AWVALIDM;
wire  [  5: 0]      CPU_AWIDM;
wire  [ 43: 0]      CPU_AWADDRM;
wire  [  7: 0]      CPU_AWLENM;
wire  [  2: 0]      CPU_AWSIZEM;
wire  [  1: 0]      CPU_AWBURSTM;
wire                CPU_AWLOCKM;
wire  [  3: 0]      CPU_AWCACHEM;
wire  [  2: 0]      CPU_AWPROTM;
wire                CPU_WREADYM;
wire                CPU_WVALIDM;
wire  [  5: 0]      CPU_WIDM;
wire  [127: 0]      CPU_WDATAM;
wire  [ 15: 0]      CPU_WSTRBM;
wire                CPU_WLASTM;
wire                CPU_BREADYM;
wire                CPU_BVALIDM;
wire  [  4: 0]      CPU_BIDM;
wire  [  1: 0]      CPU_BRESPM;
wire                CPU_ARREADYM;
wire                CPU_ARVALIDM;
wire  [  5: 0]      CPU_ARIDM;
wire  [ 43: 0]      CPU_ARADDRM;
wire  [  7: 0]      CPU_ARLENM;
wire  [  2: 0]      CPU_ARSIZEM;
wire  [  1: 0]      CPU_ARBURSTM;
wire                CPU_ARLOCKM;
wire  [  3: 0]      CPU_ARCACHEM;
wire  [  2: 0]      CPU_ARPROTM;
wire                CPU_RREADYM;
wire                CPU_RVALIDM;
wire  [  5: 0]      CPU_RIDM;
wire  [127: 0]      CPU_RDATAM;
wire  [  1: 0]      CPU_RRESPM;
wire                CPU_RLASTM;

wire [ID_W-1:0] GIC_ARID;
wire                [14:0] GIC_ARADDR;
wire                 [7:0] GIC_ARLEN;
wire                 [2:0] GIC_ARSIZE;
wire                 [1:0] GIC_ARBURST;
wire                 [2:0] GIC_ARPROT;
wire                 [2:0] GIC_ARUSER;
wire                       GIC_ARVALID;
wire                       GIC_ARREADY;
wire [ID_W-1:0] GIC_RID;
wire                [31:0] GIC_RDATA;
wire                       GIC_RLAST;
wire                 [1:0] GIC_RRESP;
wire                       GIC_RVALID;
wire                       GIC_RREADY;
wire [ID_W-1:0] GIC_AWID;
wire                [14:0] GIC_AWADDR;
wire                 [7:0] GIC_AWLEN;
wire                 [2:0] GIC_AWSIZE;
wire                 [1:0] GIC_AWBURST;
wire                 [2:0] GIC_AWPROT;
wire                 [2:0] GIC_AWUSER;
wire                       GIC_AWVALID;
wire                       GIC_AWREADY;
wire                [31:0] GIC_WDATA;
wire                 [3:0] GIC_WSTRB;
wire                       GIC_WVALID;
wire                       GIC_WREADY;
wire [ID_W-1:0] GIC_BID;
wire                 [1:0] GIC_BRESP;
wire                       GIC_BVALID;
wire                       GIC_BREADY;

assign GIC_ARUSER=3'h0;
assign GIC_AWUSER=3'h0;


wire [ID_W-1:0]     AWID_DRAM;
wire [31:0]         AWADDR_DRAM;
wire [7:0]          AWLEN_DRAM;
wire [2:0]          AWSIZE_DRAM;
wire [1:0]          AWBURST_DRAM;
wire                AWLOCK_DRAM;
wire [3:0]          AWCACHE_DRAM;
wire [2:0]          AWPROT_DRAM;
wire                AWVALID_DRAM;
wire                AWREADY_DRAM;
wire [63:0]         WDATA_DRAM;
wire [7:0]          WSTRB_DRAM;
wire                WLAST_DRAM;
wire                WVALID_DRAM;
wire                WREADY_DRAM;
wire [ID_W-1:0]     BID_DRAM;
wire [1:0]          BRESP_DRAM;
wire                BVALID_DRAM;
wire                BREADY_DRAM;
wire [ID_W-1:0]     ARID_DRAM;
wire [31:0]         ARADDR_DRAM;
wire [7:0]          ARLEN_DRAM;
wire [2:0]          ARSIZE_DRAM;
wire [1:0]          ARBURST_DRAM;
wire                ARLOCK_DRAM;
wire [3:0]          ARCACHE_DRAM;
wire [2:0]          ARPROT_DRAM;
wire                ARVALID_DRAM;
wire                ARREADY_DRAM;
wire [ID_W-1:0]     RID_DRAM;
wire [63:0]         RDATA_DRAM;
wire [1:0]          RRESP_DRAM;
wire                RLAST_DRAM;
wire                RVALID_DRAM;
wire                RREADY_DRAM;

wire [31:0]         HADDR_FLASH;
wire [1:0]          HTRANS_FLASH;
wire                HWRITE_FLASH;
wire [2:0]          HSIZE_FLASH;
wire [2:0]          HBURST_FLASH;
wire [3:0]          HPROT_FLASH;
wire [31:0]         HWDATA_FLASH;
wire [31:0]         HRDATA_FLASH;
wire                HREADY_FLASH;
wire                HRESP_FLASH;

wire [31:0]         PADDR_PERIPHERAL;
wire [31:0]         PWDATA_PERIPHERAL;
wire                PWRITE_PERIPHERAL;
wire                PENABLE_PERIPHERAL;
wire                PSELx_PERIPHERAL;
wire [31:0]         PRDATA_PERIPHERAL;
wire                PSLVERR_PERIPHERAL;
wire                PREADY_PERIPHERAL;

wire [ID_W-1:0]     AWID_RAM;
wire [31:0]         AWADDR_RAM;
wire [7:0]          AWLEN_RAM;
wire [2:0]          AWSIZE_RAM;
wire [1:0]          AWBURST_RAM;
wire                AWLOCK_RAM;
wire [3:0]          AWCACHE_RAM;
wire [2:0]          AWPROT_RAM;
wire                AWVALID_RAM;
wire                AWREADY_RAM;
wire [63:0]         WDATA_RAM;
wire [7:0]          WSTRB_RAM;
wire                WLAST_RAM;
wire                WVALID_RAM;
wire                WREADY_RAM;
wire [ID_W-1:0]     BID_RAM;
wire [1:0]          BRESP_RAM;
wire                BVALID_RAM;
wire                BREADY_RAM;
wire [ID_W-1:0]     ARID_RAM;
wire [31:0]         ARADDR_RAM;
wire [7:0]          ARLEN_RAM;
wire [2:0]          ARSIZE_RAM;
wire [1:0]          ARBURST_RAM;
wire                ARLOCK_RAM;
wire [3:0]          ARCACHE_RAM;
wire [2:0]          ARPROT_RAM;
wire                ARVALID_RAM;
wire                ARREADY_RAM;
wire [ID_W-1:0]     RID_RAM;
wire [63:0]         RDATA_RAM;
wire [1:0]          RRESP_RAM;
wire                RLAST_RAM;
wire                RVALID_RAM;
wire                RREADY_RAM;

wire [ID_W-1:0]     AWID_ROM;
wire [31:0]         AWADDR_ROM;
wire [7:0]          AWLEN_ROM;
wire [2:0]          AWSIZE_ROM;
wire [1:0]          AWBURST_ROM;
wire                AWLOCK_ROM;
wire [3:0]          AWCACHE_ROM;
wire [2:0]          AWPROT_ROM;
wire                AWVALID_ROM;
wire                AWREADY_ROM;
wire [63:0]         WDATA_ROM;
wire [7:0]          WSTRB_ROM;
wire                WLAST_ROM;
wire                WVALID_ROM;
wire                WREADY_ROM;
wire [ID_W-1:0]     BID_ROM;
wire [1:0]          BRESP_ROM;
wire                BVALID_ROM;
wire                BREADY_ROM;
wire [ID_W-1:0]     ARID_ROM;
wire [31:0]         ARADDR_ROM;
wire [7:0]          ARLEN_ROM;
wire [2:0]          ARSIZE_ROM;
wire [1:0]          ARBURST_ROM;
wire                ARLOCK_ROM;
wire [3:0]          ARCACHE_ROM;
wire [2:0]          ARPROT_ROM;
wire                ARVALID_ROM;
wire                ARREADY_ROM;
wire [ID_W-1:0]     RID_ROM;
wire [63:0]         RDATA_ROM;
wire [1:0]          RRESP_ROM;
wire                RLAST_ROM;
wire                RVALID_ROM;
wire                RREADY_ROM;

wire [31:0]         PADDR_FLASH_CTRL;
wire [31:0]         PWDATA_FLASH_CTRL;
wire                PWRITE_FLASH_CTRL;
wire [2:0]          PPROT_FLASH_CTRL;
wire [3:0]          PSTRB_FLASH_CTRL;
wire                PENABLE_FLASH_CTRL;
wire                PSELx_FLASH_CTRL;
wire [31:0]         PRDATA_FLASH_CTRL;
wire                PSLVERR_FLASH_CTRL;
wire                PREADY_FLASH_CTRL;

wire                CPU_nPRESETDBG;
wire                CPU_PCLKENDBG;
wire                CPU_PSELDBG;
wire  [ 31: 0]      CPU_PADDRDBG;
wire                CPU_PADDRDBG31;
wire                CPU_PENABLEDBG;
wire                CPU_PWRITEDBG;
wire  [ 31: 0]      CPU_PWDATADBG;
wire  [ 31: 0]      CPU_PRDATADBG;
wire                CPU_PREADYDBG;
wire                CPU_PSLVERRDBG;

assign CPU_nPRESETDBG = SYS_RESETn;
assign CPU_PCLKENDBG = 1'b1;
assign CPU_PADDRDBG31 = 1'b0;

wire [(NUM_SPIS-1):0]   CPU_IRQS;
wire [5:0]              PERI_IRQS;

assign CPU_IRQS={{(NUM_SPIS-38){1'b0}}, PERI_IRQS};

megasoc_cpu_ss #(
    .NUM_GICRID_BITS(ID_W),
    .NUM_GICWID_BITS(ID_W),
    .NUM_SPIS(NUM_SPIS)
    ) u_megasoc_cpu_ss(
    .CPU_CLK(SYS_CLK),
    .RESETn(SYS_RESETn),

    .ACLKENM(1'b1),
    .ACINACTM(),
    .RDMEMATTR(),
    .WRMEMATTR(),

    .AWREADYM(CPU_AWREADYM),
    .AWVALIDM(CPU_AWVALIDM),
    .AWIDM(CPU_AWIDM),
    .AWADDRM(CPU_AWADDRM),
    .AWLENM(CPU_AWLENM),
    .AWSIZEM(CPU_AWSIZEM),
    .AWBURSTM(CPU_AWBURSTM),
    .AWLOCKM(CPU_AWLOCKM),
    .AWCACHEM(CPU_AWCACHEM),
    .AWPROTM(CPU_AWPROTM),

    .WREADYM(CPU_WREADYM),
    .WVALIDM(CPU_WVALIDM),
    .WIDM(CPU_WIDM),
    .WDATAM(CPU_WDATAM),
    .WSTRBM(CPU_WSTRBM),
    .WLASTM(CPU_WLASTM),

    .BREADYM(CPU_BREADYM),
    .BVALIDM(CPU_BVALIDM),
    .BIDM(CPU_BIDM),
    .BRESPM(CPU_BRESPM),    

    .ARREADYM(CPU_ARREADYM),
    .ARVALIDM(CPU_ARVALIDM),
    .ARIDM(CPU_ARIDM),
    .ARADDRM(CPU_ARADDRM),
    .ARLENM(CPU_ARLENM),
    .ARSIZEM(CPU_ARSIZEM),
    .ARBURSTM(CPU_ARBURSTM),
    .ARLOCKM(CPU_ARLOCKM),
    .ARCACHEM(CPU_ARCACHEM),
    .ARPROTM(CPU_ARPROTM),

    .RREADYM(CPU_RREADYM),
    .RVALIDM(CPU_RVALIDM),
    .RIDM(CPU_RIDM),
    .RDATAM(CPU_RDATAM),
    .RRESPM(CPU_RRESPM),
    .RLASTM(CPU_RLASTM),

    .nPRESETDBG(CPU_nPRESETDBG),
    .PCLKENDBG(CPU_PCLKENDBG),
    .PSELDBG(CPU_PSELDBG),
    .PADDRDBG(CPU_PADDRDBG[30:2]),
    .PENABLEDBG(CPU_PENABLEDBG),
    .PWRITEDBG(CPU_PWRITEDBG),
    .PWDATADBG(CPU_PWDATADBG),
    .PRDATADBG(CPU_PRDATADBG),
    .PREADYDBG(CPU_PREADYDBG),
    .PSLVERRDBG(CPU_PSLVERRDBG),

    .nTRST(nTRST),
    .SWCLKTCK(SWCLKTCK),
    .SWDITMS(SWDITMS),
    .TDI(TDI),
    .TDO(TDO),
    .nTDOEN(nTDOEN),
    .SWDO(SWDO),
    .SWDOEN(SWDOEN),

    .GIC_ARID(GIC_ARID),
    .GIC_ARADDR(GIC_ARADDR),
    .GIC_ARLEN(GIC_ARLEN),
    .GIC_ARSIZE(GIC_ARSIZE),
    .GIC_ARBURST(GIC_ARBURST),
    .GIC_ARPROT(GIC_ARPROT),
    .GIC_ARUSER(GIC_ARUSER),
    .GIC_ARVALID(GIC_ARVALID),
    .GIC_ARREADY(GIC_ARREADY),

    .GIC_RID(GIC_RID),
    .GIC_RDATA(GIC_RDATA),
    .GIC_RLAST(GIC_RLAST),
    .GIC_RRESP(GIC_RRESP),
    .GIC_RVALID(GIC_RVALID),
    .GIC_RREADY(GIC_RREADY),

    .GIC_AWID(GIC_AWID),
    .GIC_AWADDR(GIC_AWADDR),
    .GIC_AWLEN(GIC_AWLEN),
    .GIC_AWSIZE(GIC_AWSIZE),
    .GIC_AWBURST(GIC_AWBURST),
    .GIC_AWPROT(GIC_AWPROT),
    .GIC_AWUSER(GIC_AWUSER),
    .GIC_AWVALID(GIC_AWVALID),
    .GIC_AWREADY(GIC_AWREADY),

    .GIC_WDATA(GIC_WDATA),
    .GIC_WSTRB(GIC_WSTRB),
    .GIC_WVALID(GIC_WVALID),
    .GIC_WREADY(GIC_WREADY),

    .GIC_BID(GIC_BID),
    .GIC_BRESP(GIC_BRESP),
    .GIC_BVALID(GIC_BVALID),
    .GIC_BREADY(GIC_BREADY),

    .IRQs(CPU_IRQS)
);


nic400_megasoc_main u_nic400_megasoc_main(
    .AWID_DRAM(),
    .AWADDR_DRAM(),
    .AWLEN_DRAM(),
    .AWSIZE_DRAM(),
    .AWBURST_DRAM(),
    .AWLOCK_DRAM(),
    .AWCACHE_DRAM(),
    .AWPROT_DRAM(),
    .AWVALID_DRAM(),
    .AWREADY_DRAM(),
    .WDATA_DRAM(),
    .WSTRB_DRAM(),
    .WLAST_DRAM(),
    .WVALID_DRAM(),
    .WREADY_DRAM(),
    .BID_DRAM(),
    .BRESP_DRAM(),
    .BVALID_DRAM(),
    .BREADY_DRAM(),
    .ARID_DRAM(),
    .ARADDR_DRAM(),
    .ARLEN_DRAM(),
    .ARSIZE_DRAM(),
    .ARBURST_DRAM(),
    .ARLOCK_DRAM(),
    .ARCACHE_DRAM(),
    .ARPROT_DRAM(),
    .ARVALID_DRAM(),
    .ARREADY_DRAM(),
    .RID_DRAM(),
    .RDATA_DRAM(),
    .RRESP_DRAM(),
    .RLAST_DRAM(),
    .RVALID_DRAM(),
    .RREADY_DRAM(),

    .HADDR_FLASH(HADDR_FLASH),
    .HTRANS_FLASH(HTRANS_FLASH),
    .HWRITE_FLASH(HWRITE_FLASH),
    .HSIZE_FLASH(HSIZE_FLASH),
    .HBURST_FLASH(HBURST_FLASH),
    .HPROT_FLASH(HPROT_FLASH),
    .HWDATA_FLASH(HWDATA_FLASH),
    .HRDATA_FLASH(HRDATA_FLASH),
    .HREADY_FLASH(HREADY_FLASH),
    .HRESP_FLASH(HRESP_FLASH),

    .AWID_GIC(GIC_AWID),
    .AWADDR_GIC(GIC_AWADDR),
    .AWLEN_GIC(GIC_AWLEN),
    .AWSIZE_GIC(GIC_AWSIZE),
    .AWBURST_GIC(GIC_AWBURST),
    .AWLOCK_GIC(GIC_AWLOCK),
    .AWCACHE_GIC(GIC_AWCACHE),
    .AWPROT_GIC(GIC_AWPROT),
    .AWVALID_GIC(GIC_AWVALID),
    .AWREADY_GIC(GIC_AWREADY),
    .WDATA_GIC(GIC_WDATA),
    .WSTRB_GIC(GIC_WSTRB),
    .WLAST_GIC(GIC_WLAST),
    .WVALID_GIC(GIC_WVALID),
    .WREADY_GIC(GIC_WREADY),
    .BID_GIC(GIC_BID),
    .BRESP_GIC(GIC_BRESP),
    .BVALID_GIC(GIC_BVALID),
    .BREADY_GIC(GIC_BREADY),
    .ARID_GIC(GIC_ARID),
    .ARADDR_GIC(GIC_ARADDR),
    .ARLEN_GIC(GIC_ARLEN),
    .ARSIZE_GIC(GIC_ARSIZE),
    .ARBURST_GIC(GIC_ARBURST),
    .ARLOCK_GIC(GIC_ARLOCK),
    .ARCACHE_GIC(GIC_ARCACHE),
    .ARPROT_GIC(GIC_ARPROT),
    .ARVALID_GIC(GIC_ARVALID),
    .ARREADY_GIC(GIC_ARREADY),
    .RID_GIC(GIC_RID),
    .RDATA_GIC(GIC_RDATA),
    .RRESP_GIC(GIC_RRESP),
    .RLAST_GIC(GIC_RLAST),
    .RVALID_GIC(GIC_RVALID),
    .RREADY_GIC(GIC_RREADY),

    .PADDR_PERIPHERAL(PADDR_PERIPHERAL),
    .PWDATA_PERIPHERAL(PWDATA_PERIPHERAL),
    .PWRITE_PERIPHERAL(PWRITE_PERIPHERAL),
    .PENABLE_PERIPHERAL(PENABLE_PERIPHERAL),
    .PSELx_PERIPHERAL(PSELx_PERIPHERAL),
    .PRDATA_PERIPHERAL(PRDATA_PERIPHERAL),
    .PSLVERR_PERIPHERAL(PSLVERR_PERIPHERAL),
    .PREADY_PERIPHERAL(PREADY_PERIPHERAL),


    .AWID_RAM(AWID_RAM),
    .AWADDR_RAM(AWADDR_RAM),
    .AWLEN_RAM(AWLEN_RAM),
    .AWSIZE_RAM(AWSIZE_RAM),
    .AWBURST_RAM(AWBURST_RAM),
    .AWLOCK_RAM(AWLOCK_RAM),
    .AWCACHE_RAM(AWCACHE_RAM),
    .AWPROT_RAM(AWPROT_RAM),
    .AWVALID_RAM(AWVALID_RAM),
    .AWREADY_RAM(AWREADY_RAM),
    .WDATA_RAM(WDATA_RAM),
    .WSTRB_RAM(WSTRB_RAM),
    .WLAST_RAM(WLAST_RAM),
    .WVALID_RAM(WVALID_RAM),
    .WREADY_RAM(WREADY_RAM),
    .BID_RAM(BID_RAM),
    .BRESP_RAM(BRESP_RAM),
    .BVALID_RAM(BVALID_RAM),
    .BREADY_RAM(BREADY_RAM),
    .ARID_RAM(ARID_RAM),
    .ARADDR_RAM(ARADDR_RAM),
    .ARLEN_RAM(ARLEN_RAM),
    .ARSIZE_RAM(ARSIZE_RAM),
    .ARBURST_RAM(ARBURST_RAM),
    .ARLOCK_RAM(ARLOCK_RAM),
    .ARCACHE_RAM(ARCACHE_RAM),
    .ARPROT_RAM(ARPROT_RAM),
    .ARVALID_RAM(ARVALID_RAM),
    .ARREADY_RAM(ARREADY_RAM),
    .RID_RAM(RID_RAM),
    .RDATA_RAM(RDATA_RAM),
    .RRESP_RAM(RRESP_RAM),
    .RLAST_RAM(RLAST_RAM),
    .RVALID_RAM(RVALID_RAM),
    .RREADY_RAM(RREADY_RAM),

    .AWID_ROM(AWID_ROM),
    .AWADDR_ROM(AWADDR_ROM),
    .AWLEN_ROM(AWLEN_ROM),
    .AWSIZE_ROM(AWSIZE_ROM),
    .AWBURST_ROM(AWBURST_ROM),
    .AWLOCK_ROM(AWLOCK_ROM),
    .AWCACHE_ROM(AWCACHE_ROM),
    .AWPROT_ROM(AWPROT_ROM),
    .AWVALID_ROM(AWVALID_ROM),
    .AWREADY_ROM(AWREADY_ROM),
    .WDATA_ROM(WDATA_ROM),
    .WSTRB_ROM(WSTRB_ROM),
    .WLAST_ROM(WLAST_ROM),
    .WVALID_ROM(WVALID_ROM),
    .WREADY_ROM(WREADY_ROM),
    .BID_ROM(BID_ROM),
    .BRESP_ROM(BRESP_ROM),
    .BVALID_ROM(BVALID_ROM),
    .BREADY_ROM(BREADY_ROM),
    .ARID_ROM(ARID_ROM),
    .ARADDR_ROM(ARADDR_ROM),
    .ARLEN_ROM(ARLEN_ROM),
    .ARSIZE_ROM(ARSIZE_ROM),
    .ARBURST_ROM(ARBURST_ROM),
    .ARLOCK_ROM(ARLOCK_ROM),
    .ARCACHE_ROM(ARCACHE_ROM),
    .ARPROT_ROM(ARPROT_ROM),
    .ARVALID_ROM(ARVALID_ROM),
    .ARREADY_ROM(ARREADY_ROM),
    .RID_ROM(RID_ROM),
    .RDATA_ROM(RDATA_ROM),
    .RRESP_ROM(RRESP_ROM),
    .RLAST_ROM(RLAST_ROM),
    .RVALID_ROM(RVALID_ROM),
    .RREADY_ROM(RREADY_ROM),

    .PADDR_DEBUG(CPU_PADDRDBG),
    .PWDATA_DEBUG(CPU_PWDATADBG),
    .PWRITE_DEBUG(CPU_PWRITEDBG),
    .PENABLE_DEBUG(CPU_PENABLEDBG),
    .PSELx_DEBUG(CPU_PSELDBG),
    .PRDATA_DEBUG(CPU_PRDATADBG),
    .PSLVERR_DEBUG(CPU_PSLVERRDBG),
    .PREADY_DEBUG(CPU_PREADYDBG),


    .PADDR_FLASH_CTRL(PADDR_FLASH_CTRL),
    .PWDATA_FLASH_CTRL(PWDATA_FLASH_CTRL),
    .PWRITE_FLASH_CTRL(PWRITE_FLASH_CTRL),
    .PPROT_FLASH_CTRL(PPROT_FLASH_CTRL),
    .PSTRB_FLASH_CTRL(PSTRB_FLASH_CTRL),
    .PENABLE_FLASH_CTRL(PENABLE_FLASH_CTRL),
    .PSELx_FLASH_CTRL(PSELx_FLASH_CTRL),
    .PRDATA_FLASH_CTRL(PRDATA_FLASH_CTRL),
    .PSLVERR_FLASH_CTRL(PSLVERR_FLASH_CTRL),
    .PREADY_FLASH_CTRL(PREADY_FLASH_CTRL),

    .AWID_A53(CPU_AWIDM),
    .AWADDR_A53(CPU_AWADDRM),
    .AWLEN_A53(CPU_AWLENM),
    .AWSIZE_A53(CPU_AWSIZEM),
    .AWBURST_A53(CPU_AWBURSTM),
    .AWLOCK_A53(CPU_AWLOCKM),
    .AWCACHE_A53(CPU_AWCACHEM),
    .AWPROT_A53(CPU_AWPROTM),
    .AWVALID_A53(CPU_AWVALIDM),
    .AWREADY_A53(CPU_AWREADYM),
    .WDATA_A53(CPU_WDATAM),
    .WSTRB_A53(CPU_WSTRBM),
    .WLAST_A53(CPU_WLASTM),
    .WVALID_A53(CPU_WVALIDM),
    .WREADY_A53(CPU_WREADYM),
    .BID_A53(CPU_BIDM),
    .BRESP_A53(CPU_BRESPM),
    .BVALID_A53(CPU_BVALIDM),
    .BREADY_A53(CPU_BREADYM),
    .ARID_A53(CPU_ARIDM),
    .ARADDR_A53(CPU_ARADDRM),
    .ARLEN_A53(CPU_ARLENM),
    .ARSIZE_A53(CPU_ARSIZEM),
    .ARBURST_A53(CPU_ARBURSTM),
    .ARLOCK_A53(CPU_ARLOCKM),
    .ARCACHE_A53(CPU_ARCACHEM),
    .ARPROT_A53(CPU_ARPROTM),
    .ARVALID_A53(CPU_ARVALIDM),
    .ARREADY_A53(CPU_ARREADYM),
    .RID_A53(CPU_RIDM),
    .RDATA_A53(CPU_RDATAM),
    .RRESP_A53(CPU_RRESPM),
    .RLAST_A53(CPU_RLASTM),
    .RVALID_A53(CPU_RVALIDM),
    .RREADY_A53(CPU_RREADYM),

    .clk0clk(SYS_CLK),
    .clk0clken(SYS_CLKEN),
    .clk0resetn(SYS_RESETn)

);

ROM_wrapper u_ROM_wrapper(
    .ACLK(SYS_CLK),
    .ARESETn(SYS_RESETn),
    .AWVALID(AWVALID_ROM),
    .AWREADY(AWREADY_ROM),
    .AWID(AWID_ROM),
    .AWADDR(AWADDR_ROM),
    .AWLEN(AWLEN_ROM),
    .AWSIZE(AWSIZE_ROM),
    .AWBURST(AWBURST_ROM),
    .AWLOCK(AWLOCK_ROM),
    .AWPROT(AWPROT_ROM),
    .AWQOS(4'h0),
    .WVALID(WVALID_ROM),
    .WREADY(WREADY_ROM),
    .WDATA(WDATA_ROM),
    .WSTRB(WSTRB_ROM),
    .WLAST(WLAST_ROM),
    .WPOISON(1'b0),
    .BVALID(BVALID_ROM),
    .BREADY(BREADY_ROM),
    .BID(BID_ROM),
    .BRESP(BRESP_ROM),
    .ARVALID(ARVALID_ROM),
    .ARREADY(ARREADY_ROM),
    .ARID(ARID_ROM),
    .ARADDR(ARADDR_ROM),
    .ARLEN(ARLEN_ROM),
    .ARSIZE(ARSIZE_ROM),
    .ARBURST(ARBURST_ROM),
    .ARLOCK(ARLOCK_ROM),
    .ARPROT(ARPROT_ROM),
    .ARQOS(4'h0),
    .RVALID(RVALID_ROM),
    .RREADY(RREADY_ROM),
    .RID(RID_ROM),
    .RDATA(RDATA_ROM),
    .RRESP(RRESP_ROM),
    .RLAST(RLAST_ROM),
    .RPOISON(),
    .AWAKEUP(1'b1),
    .clk_qreqn(SYS_CLKEN),
    .clk_qacceptn(),
    .clk_qdeny(),
    .clk_qactive(),
    .pwr_qreqn(SYS_CLKEN),
    .pwr_qacceptn(),
    .pwr_qdeny(),
    .pwr_qactive(),
    .ext_gt_qreqn(1'b1),
    .ext_gt_qacceptn(),
    .cfg_gate_resp(1'b0)
);

// top_ahb_qspi #(.DATA_W(32)) u_sl_ahb_qspi(
//     .HCLK(SYS_CLK),
//     .HRESETn(SYS_RESETn),
//     .PCLK(SYS_CLK),
//     .PRESETn(SYS_RESETn),
//     .HADDR(HADDR_FLASH),
//     .HTRANS(HTRANS_FLASH),
//     .HWRITE(HWRITE_FLASH),
//     .HSIZE(HSIZE_FLASH),
//     .HBURST(HBURST_FLASH),
//     .HPROT(HPROT_FLASH),
//     .HWDATA(HWDATA_FLASH),
//     .HSELx(1'b1),
//     .HRDATA(HRDATA_FLASH),
//     .HREADY(1'b1),
//     .HREADYOUT(HREADY_FLASH),
//     .HRESP(HRESP_FLASH),
//     .PADDR(PADDR_FLASH_CTRL),
//     .PPROT(PPROT_FLASH_CTRL),
//     .PSEL(PSELx_FLASH_CTRL),
//     .PENABLE(PENABLE_FLASH_CTRL),
//     .PWRITE(PWRITE_FLASH_CTRL),
//     .PWDATA(PWDATA_FLASH_CTRL),
//     .PSTRB(PSTRB_FLASH_CTRL),
//     .PRDATA(PRDATA_FLASH_CTRL),
//     .PREADY(PREADY_FLASH_CTRL),
//     .PSLVERR(PSLVERR_FLASH_CTRL),   
//     .QSPI_SCLK(QSPI_SCLK),
//     .QSPI_nCS(QSPI_nCS),
//     .QSPI_IO_o(QSPI_IO_o),
//     .QSPI_IO_i(QSPI_IO_i),
//     .QSPI_IO_e(QSPI_IO_e)
// );

sl_ahb_sram u_sl_ahb_sram(
    .HCLK(SYS_CLK),
    .HRESETn(SYS_RESETn),
    .HSEL(1'b1),
    .HREADY(1'b1),
    .HTRANS(HTRANS_FLASH),
    .HSIZE(HSIZE_FLASH),
    .HWRITE(HWRITE_FLASH),
    .HADDR(HADDR_FLASH),
    .HWDATA(HWDATA_FLASH),
    .HREADYOUT(HREADY_FLASH),
    .HRESP(HRESP_FLASH),
    .HRDATA(HRDATA_FLASH)
);

SRAM_wrapper u_SRAM_wrapper(
    .ACLK(SYS_CLK),
    .ARESETn(SYS_RESETn),
    .AWVALID(AWVALID_RAM),
    .AWREADY(AWREADY_RAM),
    .AWID(AWID_RAM),
    .AWADDR(AWADDR_RAM),
    .AWLEN(AWLEN_RAM),
    .AWSIZE(AWSIZE_RAM),
    .AWBURST(AWBURST_RAM),
    .AWLOCK(AWLOCK_RAM),
    .AWPROT(AWPROT_RAM),
    .AWQOS(4'h0),
    .WVALID(WVALID_RAM),
    .WREADY(WREADY_RAM),
    .WDATA(WDATA_RAM),
    .WSTRB(WSTRB_RAM),
    .WLAST(WLAST_RAM),
    .WPOISON(1'b0),
    .BVALID(BVALID_RAM),
    .BREADY(BREADY_RAM),
    .BID(BID_RAM),
    .BRESP(BRESP_RAM),
    .ARVALID(ARVALID_RAM),
    .ARREADY(ARREADY_RAM),
    .ARID(ARID_RAM),
    .ARADDR(ARADDR_RAM),
    .ARLEN(ARLEN_RAM),
    .ARSIZE(ARSIZE_RAM),
    .ARBURST(ARBURST_RAM),
    .ARLOCK(ARLOCK_RAM),
    .ARPROT(ARPROT_RAM),
    .ARQOS(4'h0),
    .RVALID(RVALID_RAM),
    .RREADY(RREADY_RAM),
    .RID(RID_RAM),
    .RDATA(RDATA_RAM),
    .RRESP(RRESP_RAM),
    .RLAST(RLAST_RAM),
    .RPOISON(),
    .AWAKEUP(1'b1),
    .clk_qreqn(SYS_CLKEN),
    .clk_qacceptn(),
    .clk_qdeny(),
    .clk_qactive(),
    .pwr_qreqn(SYS_CLKEN),
    .pwr_qacceptn(),
    .pwr_qdeny(),
    .pwr_qactive(),
    .ext_gt_qreqn(1'b1),
    .ext_gt_qacceptn(),
    .cfg_gate_resp(1'b0)
);

megasoc_peripheral_subsystem u_megasoc_peripheral_subsystem(
    .PCLK(SYS_CLK),
    .PRESETn(SYS_RESETn),
    .PADDR(PADDR_PERIPHERAL),
    .PENABLE(PENABLE_PERIPHERAL),
    .PWRITE(PWRITE_PERIPHERAL),
    .PWDATA(PWDATA_PERIPHERAL),
    .PSEL(PSELx_PERIPHERAL),
    .PRDATA(PRDATA_PERIPHERAL),
    .PREADY(PREADY_PERIPHERAL),
    .PSLVERR(PSLVERR_PERIPHERAL),
    .UARTRXD(UARTRXD),
    .UARTTXD(UARTTXD),
    .UARTTXEN(UARTTXEN),
    .PERI_IRQS(PERI_IRQS)
);

endmodule