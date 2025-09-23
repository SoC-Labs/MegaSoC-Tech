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
//  ROM_wrapper                     (u_ROM_wrapper)
//  sl_ahb_sram                     (u_sl_ahb_sram)
//  SRAM_wrapper                    (u_SRAM_wrapper)
//  megasoc_peripheral_subsystem    (u_megasoc_peripheral_subsystem)
//  megasoc_power_control           (u_megasoc_power_control)
//  nic400_megasoc_main             (u_nic400_megasoc_main)
//-----------------------------------------------------------------------------
// To Do
//  - Replace sl_ahb_sram with QSPI controller to use external flash
//  - Add DDR controller for DRAM port on NIC400


`timescale 1ns/1ps

module megasoc_tech_wrapper(
    input  wire             SYS_CLK,
    input  wire             SYS_CLKEN,
    input  wire             RT_CLK, // 32kHz real time clock
    input  wire             SYS_RESETn,

    // MegaSoC system AXI Manager
    axi4.master             EXP_M_AXI,

    // MegaSoC system AXI Subordinate
    axi4.subordinate        EXP_S_AXI,

    input  wire [7:0]       EXP_IRQs,
    // DMA 350 APB Interface Wires
    output wire [31:0]      PADDR_DMA_CTRL,
    output wire [31:0]      PWDATA_DMA_CTRL,
    output wire             PWRITE_DMA_CTRL,
    output wire [2:0]       PPROT_DMA_CTRL,
    output wire [3:0]       PSTRB_DMA_CTRL,
    output wire             PENABLE_DMA_CTRL,
    output wire             PSELx_DMA_CTRL,
    input  wire [31:0]      PRDATA_DMA_CTRL,
    input  wire             PSLVERR_DMA_CTRL,
    input  wire             PREADY_DMA_CTRL,

    // DMA 350 AXI Interface Wires
    input  wire [1:0]       AWID_DMA350,
    input  wire [43:0]      AWADDR_DMA350,
    input  wire [7:0]       AWLEN_DMA350,
    input  wire [2:0]       AWSIZE_DMA350,
    input  wire [1:0]       AWBURST_DMA350,
    input  wire             AWLOCK_DMA350,
    input  wire [3:0]       AWCACHE_DMA350,
    input  wire [2:0]       AWPROT_DMA350,
    input  wire             AWVALID_DMA350,
    output wire             AWREADY_DMA350,

    input  wire [127:0]     WDATA_DMA350,
    input  wire [15:0]      WSTRB_DMA350,
    input  wire             WLAST_DMA350,
    input  wire             WVALID_DMA350,
    output wire             WREADY_DMA350,

    output wire [1:0]       BID_DMA350,
    output wire [1:0]       BRESP_DMA350,
    output wire             BVALID_DMA350,
    input  wire             BREADY_DMA350,

    input  wire [1:0]       ARID_DMA350,
    input  wire [43:0]      ARADDR_DMA350,
    input  wire [7:0]       ARLEN_DMA350,
    input  wire [2:0]       ARSIZE_DMA350,
    input  wire [1:0]       ARBURST_DMA350,
    input  wire             ARLOCK_DMA350,
    input  wire [3:0]       ARCACHE_DMA350,
    input  wire [2:0]       ARPROT_DMA350,
    input  wire             ARVALID_DMA350,
    output wire             ARREADY_DMA350,

    output wire [1:0]       RID_DMA350,
    output wire [127:0]     RDATA_DMA350,
    output wire [1:0]       RRESP_DMA350,
    output wire             RLAST_DMA350,
    output wire             RVALID_DMA350,
    input  wire             RREADY_DMA350,

    input  wire [3:0]       DMA350_irq_channel,
    input  wire             DMA350_irq_comb_nonsec,

    // QSPI Signals
    output wire             QSPI_SCLK,
    output wire             QSPI_nCS,
    output wire [3:0]       QSPI_IO_o,
    input  wire [3:0]       QSPI_IO_i,
    output wire [3:0]       QSPI_IO_e,

    // UART signals
    input  wire             UARTRXD0,
    output wire             UARTTXD0,
    output wire             UARTTXEN0,

    input  wire             UARTRXD1,
    output wire             UARTTXD1,
    output wire             UARTTXEN1,

    // EXTIO Signals
    input  wire [3:0]       iodata4_i,
    output wire [3:0]       iodata4_o,
    output wire [3:0]       iodata4_e,
    output wire [3:0]       iodata4_t,
    output wire             ioreq1_o,
    output wire             ioreq2_o,
    input  wire             ioack_i,

    // DAP-LITE external signals
    input  wire             nTRST,
    input  wire             SWCLKTCK,
    input  wire             SWDITMS,
    input  wire             TDI,
    output wire             TDO,
    output wire             nTDOEN,
    output wire             SWDO,
    output wire             SWDOEN,

    // SPI Bus to Pads
    output wire             SPI_SSn,
    output wire             SPI_SCLK,
    output wire             SPI_MOSI,
    input  wire             SPI_MISO,

    input  wire [15:0]      P0_IN,
    output wire [15:0]      P0_OUT,
    output wire [15:0]      P0_EN,
    output wire [15:0]      P0_FUNC,

    input  wire [15:0]      P1_IN,
    output wire [15:0]      P1_OUT,
    output wire [15:0]      P1_EN,
    output wire [15:0]      P1_FUNC,

    // DDR4 signals
    output wire             DDR4_CK_T,
    output wire             DDR4_CK_C,

    output wire [16:0]      DDR4_ADR,
    output wire [1:0]       DDR4_BA,
    output wire [1:0]       DDR4_BG,

    output wire             DDR4_ACT_n,
    output wire [1:0]       DDR4_CKE,
    output wire [1:0]       DDR4_CS_N,
    output wire [1:0]       DDR4_ODT,
    output wire             DDR_PARITY,

    inout  wire [63:0]      DDR4_DQ,
    inout  wire [7:0]       DDR4_DM_DBI_N,
    inout  wire [7:0]       DDR4_DQS_T,
    inout  wire [7:0]       DDR4_DQS_C,

    output wire             DDR4_RESET_N,

    input  wire             DDR_nALERT,
    input  wire             DDR_nEVENT


);


parameter ID_W=8;
parameter NUM_SPIS=480;


//--------------------------------------
//  Power Control Interfaces
//--------------------------------------
qchannel ROM_qchan_q();
qchannel ROM_qchan_p();
wire ROM_RESETn;
wire AWAKEUP_ROM;

qchannel SRAM_qchan_q();
qchannel SRAM_qchan_p();
wire AWAKEUP_SRAM;
wire SRAM_RESETn;

qchannel CPU_CORE_q();
qchannel CPU_NEON_q();
qchannel CPU_L2_q();
wire     CPU_CORE_PORESETn;
wire     CPU_CORE_WRMRSTn;
wire     CPU_L2_RESETn;

//--------------------------------------
//  Bus Interfaces
//--------------------------------------
// AXI
axi4 #(.DATA_W(128), .ID_W(6), .ADDR_W(44))     CPU_AXI();
axi4 #(.DATA_W(32), .ID_W(ID_W-1), .ADDR_W(32))   GIC_AXI();
axi4 #(.DATA_W(64), .ID_W(ID_W), .ADDR_W(32))   DRAM_AXI();
axi4 #(.DATA_W(64), .ID_W(ID_W), .ADDR_W(32))   SRAM_AXI();
axi4 #(.DATA_W(64), .ID_W(ID_W), .ADDR_W(32))   ROM_AXI();
// AHB
ahb #(.DATA_W(32), .ADDR_W(32))     FLASH_AHB();
ahb #(.DATA_W(32), .ADDR_W(32))     PERIPH_AHB();
ahb #(.DATA_W(32), .ADDR_W(32))     ADP_AHB();
// APB
apb3    CPU_DBG_APB();
apb3    PCK_APB();
apb4    FLASH_CTRL_APB();


wire                CPU_nPRESETDBG;
wire                CPU_PCLKENDBG;


assign CPU_nPRESETDBG = SYS_RESETn;
assign CPU_PCLKENDBG = 1'b1;

wire [(NUM_SPIS-1):0]   CPU_IRQS;
wire                    QSPI_IRQ;
wire [65:0]             PERI_IRQS;

assign CPU_IRQS={{(NUM_SPIS-80){1'b0}},EXP_IRQs, QSPI_IRQ, PERI_IRQS, DMA350_irq_comb_nonsec, DMA350_irq_channel};

// Subordinate AWAKEUP signal generation
assign AWAKEUP_ROM = (ROM_AXI.AWVALID | ROM_AXI.ARVALID | ROM_AXI.WVALID);
assign AWAKEUP_SRAM = (SRAM_AXI.AWVALID | SRAM_AXI.ARVALID | SRAM_AXI.WVALID);


megasoc_cpu_ss #(
    .NUM_GICRID_BITS(ID_W-1),
    .NUM_GICWID_BITS(ID_W-1),
    .NUM_SPIS(NUM_SPIS)
    ) u_megasoc_cpu_ss(
    // Clocks and Reset
    .CPU_CLK(SYS_CLK),
    .RESETn(SYS_RESETn),
    .nPRESETDBG(CPU_nPRESETDBG),
    .CPU_CORE_PORESETn(CPU_CORE_PORESETn),
    .CPU_CORE_WRMRSTn(CPU_CORE_WRMRSTn),
    .CPU_L2_RESETn(CPU_L2_RESETn),

    // Clock Enable signals
    .ACLKENM(1'b1),
    .PCLKENDBG(CPU_PCLKENDBG),

    // Power Management interfaces
    .CPU_CORE_q(CPU_CORE_q),
    .CPU_NEON_q(CPU_NEON_q),
    .CPU_L2_q(CPU_L2_q),

    // Bus interfaces
    .CPU_AXI(CPU_AXI),
    .GIC_AXI(GIC_AXI),
    .CPU_DBG_APB(CPU_DBG_APB),

    .ACINACTM(),
    .RDMEMATTR(),
    .WRMEMATTR(),

    // Off-chip Debug interface
    .nTRST(nTRST),
    .SWCLKTCK(SWCLKTCK),
    .SWDITMS(SWDITMS),
    .TDI(TDI),
    .TDO(TDO),
    .nTDOEN(nTDOEN),
    .SWDO(SWDO),
    .SWDOEN(SWDOEN),

    // Interrupts from System
    .IRQs(CPU_IRQS)
);


megasoc_dram_wrapper #(.ID_W(8)) u_megasoc_dram_wrapper(
    .ACLK(SYS_CLK),
    .ARESETn(SYS_RESETn),

    .DRAM_AXI(DRAM_AXI),

    .DDR4_CK_T(DDR4_CK_T),
    .DDR4_CK_C(DDR4_CK_C),
    .DDR4_ADR(DDR4_ADR),
    .DDR4_BA(DDR4_BA),
    .DDR4_BG(DDR4_BG),
    .DDR4_ACT_n(DDR4_ACT_n),
    .DDR4_CKE(DDR4_CKE),
    .DDR4_CS_N(DDR4_CS_N),
    .DDR4_ODT(DDR4_ODT),
    .DDR_PARITY(DDR_PARITY),
    .DDR4_DQ(DDR4_DQ),
    .DDR4_DM_DBI_N(DDR4_DM_DBI_N),
    .DDR4_DQS_T(DDR4_DQS_T),
    .DDR4_DQS_C(DDR4_DQS_C),
    .DDR4_RESET_N(DDR4_RESET_N),
    .DDR_nALERT(DDR_nALERT),
    .DDR_nEVENT(DDR_nEVENT)
);

ROM_wrapper u_ROM_wrapper(
    .ACLK(SYS_CLK),
    .ARESETn(ROM_RESETn),
    .AWAKEUP(AWAKEUP_ROM),

    // AXI Subordinate Interface
    .ROM_AXI(ROM_AXI),

    // Power Control Q-channels
    .ROM_qchan_q(ROM_qchan_q),
    .ROM_qchan_p(ROM_qchan_p),

    // cfg_gate_resp, if low stall mem in power off, if high give error response
    .cfg_gate_resp(1'b0)
);

top_ahb_qspi #(.DATA_W(32)) u_sl_ahb_qspi(
    .HCLK(SYS_CLK),
    .HRESETn(SYS_RESETn),
    .PCLK(SYS_CLK),
    .PRESETn(SYS_RESETn),

    .HADDR(FLASH_AHB.HADDR),
    .HTRANS(FLASH_AHB.HTRANS),
    .HWRITE(FLASH_AHB.HWRITE),
    .HSIZE(FLASH_AHB.HSIZE),
    .HBURST(FLASH_AHB.HBURST),
    .HPROT(FLASH_AHB.HPROT),
    .HWDATA(FLASH_AHB.HWDATA),
    .HSELx(FLASH_AHB.HSEL),
    .HRDATA(FLASH_AHB.HRDATA),
    .HREADY(FLASH_AHB.HREADY),
    .HREADYOUT(FLASH_AHB.HREADYOUT),
    .HRESP(FLASH_AHB.HRESP),

    .PADDR(FLASH_CTRL_APB.paddr[15:0]),
    .PPROT(FLASH_CTRL_APB.pprot),
    .PSEL(FLASH_CTRL_APB.psel),
    .PENABLE(FLASH_CTRL_APB.penable),
    .PWRITE(FLASH_CTRL_APB.pwrite),
    .PWDATA(FLASH_CTRL_APB.pwdata),
    .PSTRB(FLASH_CTRL_APB.pstrb),
    .PRDATA(FLASH_CTRL_APB.prdata),
    .PREADY(FLASH_CTRL_APB.pready),
    .PSLVERR(FLASH_CTRL_APB.pslverr),

    .QSPI_SCLK(QSPI_SCLK),
    .QSPI_nCS(QSPI_nCS),
    .QSPI_IO_o(QSPI_IO_o),
    .QSPI_IO_i(QSPI_IO_i),
    .QSPI_IO_e(QSPI_IO_e),

    .IRQ_QSPI_FINISHED(QSPI_IRQ)
);


SRAM_wrapper u_SRAM_wrapper(
    // Clock and Reset
    .ACLK(SYS_CLK),
    .ARESETn(SRAM_RESETn),

    // AXI Bus interface
    .SRAM_AXI(SRAM_AXI),
    // AXI Wakeup
    .AWAKEUP(AWAKEUP_SRAM),

    // Power Management signals
    .SRAM_qchan_q(SRAM_qchan_q),
    .SRAM_qchan_p(SRAM_qchan_p),

    .cfg_gate_resp(1'b0)
);

megasoc_peripheral_subsystem u_megasoc_peripheral_subsystem(
    // Clocks and Resets
    .PCLK(SYS_CLK),
    .PRESETn(SYS_RESETn),
    .HCLK(SYS_CLK),
    .HRESETn(SYS_RESETn),
    .RT_CLK(RT_CLK),

    // ADP - AHB manager
    .ADP_AHB(ADP_AHB),
    // Peripheral AHB Subordinate
    .PERIPH_AHB(PERIPH_AHB),

    // UART 0 Off-chip interface
    .UARTRXD0(UARTRXD0),
    .UARTTXD0(UARTTXD0),
    .UARTTXEN0(UARTTXEN0),

    // UART 1 Off-chip interface
    .UARTRXD1(UARTRXD1),
    .UARTTXD1(UARTTXD1),
    .UARTTXEN1(UARTTXEN1),

    // EXTIO Off-chip Debug interface
    .iodata4_i(iodata4_i),
    .iodata4_o(iodata4_o),
    .iodata4_e(iodata4_e),
    .iodata4_t(iodata4_t),
    .ioreq1_o(ioreq1_o),
    .ioreq2_o(ioreq2_o),
    .ioack_i(ioack_i),

    // GPIO Off-chip interface
    .p0_in(P0_IN),
    .p0_out(P0_OUT),
    .p0_en(P0_EN),
    .p0_func(P0_FUNC),
    .p1_in(P1_IN),
    .p1_out(P1_OUT),
    .p1_en(P1_EN),
    .p1_func(P1_FUNC),

    // SPI Peripheral Off chip interface
    .SPI_SSn(SPI_SSn),
    .SPI_SCLK(SPI_SCLK),
    .SPI_MOSI(SPI_MOSI),
    .SPI_MISO(SPI_MISO),

    // Peripheral Interrupts to GIC
    .PERI_IRQS(PERI_IRQS)
);

megasoc_power_control u_megasoc_power_control(
    // Clock and Reset
    .PCLK(SYS_CLK),
    .PRESETn(SYS_RESETn),

    // APB Bus interface to PPUs
    .PCK_APB(PCK_APB),

    // ROM Power management interfaces
    .ROM_qchan_q(ROM_qchan_q),
    .ROM_qchan_p(ROM_qchan_p),
    .ROM_RESETn(ROM_RESETn),

    // SRAM Power management interfaces
    .SRAM_qchan_q(SRAM_qchan_q),
    .SRAM_qchan_p(SRAM_qchan_p),
    .SRAM_RESETn(SRAM_RESETn),

    // CPU Core Power management interfaces
    .CPU_CORE_q(CPU_CORE_q),
    .CPU_CORE_PORESETn(CPU_CORE_PORESETn),
    .CPU_CORE_WRMRSTn(CPU_CORE_WRMRSTn),

    // CPU Advanced SIMD Power management interfaces
    .CPU_NEON_q(CPU_NEON_q),

    // CPU L2 Power management interfaces
    .CPU_L2_q(CPU_L2_q),
    .CPU_L2_RESETn(CPU_L2_RESETn)
);


nic400_megasoc_main u_nic400_megasoc_main(
    .AWID_DRAM(DRAM_AXI.AWID),
    .AWADDR_DRAM(DRAM_AXI.AWADDR),
    .AWLEN_DRAM(DRAM_AXI.AWLEN),
    .AWSIZE_DRAM(DRAM_AXI.AWSIZE),
    .AWBURST_DRAM(DRAM_AXI.AWBURST),
    .AWLOCK_DRAM(DRAM_AXI.AWLOCK),
    .AWCACHE_DRAM(DRAM_AXI.AWCACHE),
    .AWPROT_DRAM(DRAM_AXI.AWPROT),
    .AWVALID_DRAM(DRAM_AXI.AWVALID),
    .AWREADY_DRAM(DRAM_AXI.AWREADY),
    .WDATA_DRAM(DRAM_AXI.WDATA),
    .WSTRB_DRAM(DRAM_AXI.WSTRB),
    .WLAST_DRAM(DRAM_AXI.WLAST),
    .WVALID_DRAM(DRAM_AXI.WVALID),
    .WREADY_DRAM(DRAM_AXI.WREADY),
    .BID_DRAM(DRAM_AXI.BID),
    .BRESP_DRAM(DRAM_AXI.BRESP),
    .BVALID_DRAM(DRAM_AXI.BVALID),
    .BREADY_DRAM(DRAM_AXI.BREADY),
    .ARID_DRAM(DRAM_AXI.ARID),
    .ARADDR_DRAM(DRAM_AXI.ARADDR),
    .ARLEN_DRAM(DRAM_AXI.ARLEN),
    .ARSIZE_DRAM(DRAM_AXI.ARSIZE),
    .ARBURST_DRAM(DRAM_AXI.ARBURST),
    .ARLOCK_DRAM(DRAM_AXI.ARLOCK),
    .ARCACHE_DRAM(DRAM_AXI.ARCACHE),
    .ARPROT_DRAM(DRAM_AXI.ARPROT),
    .ARVALID_DRAM(DRAM_AXI.ARVALID),
    .ARREADY_DRAM(DRAM_AXI.ARREADY),
    .RID_DRAM(DRAM_AXI.RID),
    .RDATA_DRAM(DRAM_AXI.RDATA),
    .RRESP_DRAM(DRAM_AXI.RRESP),
    .RLAST_DRAM(DRAM_AXI.RLAST),
    .RVALID_DRAM(DRAM_AXI.RVALID),
    .RREADY_DRAM(DRAM_AXI.RREADY),

    .AWID_EXP_M(EXP_M_AXI.AWID),
    .AWADDR_EXP_M(EXP_M_AXI.AWADDR),
    .AWLEN_EXP_M(EXP_M_AXI.AWLEN),
    .AWSIZE_EXP_M(EXP_M_AXI.AWSIZE),
    .AWBURST_EXP_M(EXP_M_AXI.AWBURST),
    .AWLOCK_EXP_M(EXP_M_AXI.AWLOCK),
    .AWCACHE_EXP_M(EXP_M_AXI.AWCACHE),
    .AWPROT_EXP_M(EXP_M_AXI.AWPROT),
    .AWVALID_EXP_M(EXP_M_AXI.AWVALID),
    .AWREADY_EXP_M(EXP_M_AXI.AWREADY),
    .WDATA_EXP_M(EXP_M_AXI.WDATA),
    .WSTRB_EXP_M(EXP_M_AXI.WSTRB),
    .WLAST_EXP_M(EXP_M_AXI.WLAST),
    .WVALID_EXP_M(EXP_M_AXI.WVALID),
    .WREADY_EXP_M(EXP_M_AXI.WREADY),
    .BID_EXP_M(EXP_M_AXI.BID),
    .BRESP_EXP_M(EXP_M_AXI.BRESP),
    .BVALID_EXP_M(EXP_M_AXI.BVALID),
    .BREADY_EXP_M(EXP_M_AXI.BREADY),
    .ARID_EXP_M(EXP_M_AXI.ARID),
    .ARADDR_EXP_M(EXP_M_AXI.ARADDR),
    .ARLEN_EXP_M(EXP_M_AXI.ARLEN),
    .ARSIZE_EXP_M(EXP_M_AXI.ARSIZE),
    .ARBURST_EXP_M(EXP_M_AXI.ARBURST),
    .ARLOCK_EXP_M(EXP_M_AXI.ARLOCK),
    .ARCACHE_EXP_M(EXP_M_AXI.ARCACHE),
    .ARPROT_EXP_M(EXP_M_AXI.ARPROT),
    .ARVALID_EXP_M(EXP_M_AXI.ARVALID),
    .ARREADY_EXP_M(EXP_M_AXI.ARREADY),
    .RID_EXP_M(EXP_M_AXI.RID),
    .RDATA_EXP_M(EXP_M_AXI.RDATA),
    .RRESP_EXP_M(EXP_M_AXI.RRESP),
    .RLAST_EXP_M(EXP_M_AXI.RLAST),
    .RVALID_EXP_M(EXP_M_AXI.RVALID),
    .RREADY_EXP_M(EXP_M_AXI.RREADY),


    .HSELx_FLASH(FLASH_AHB.HSEL),
    .HADDR_FLASH(FLASH_AHB.HADDR),
    .HTRANS_FLASH(FLASH_AHB.HTRANS),
    .HWRITE_FLASH(FLASH_AHB.HWRITE),
    .HSIZE_FLASH(FLASH_AHB.HSIZE),
    .HBURST_FLASH(FLASH_AHB.HBURST),
    .HPROT_FLASH(FLASH_AHB.HPROT),
    .HWDATA_FLASH(FLASH_AHB.HWDATA),
    .HRDATA_FLASH(FLASH_AHB.HRDATA),
    .HREADYOUT_FLASH(FLASH_AHB.HREADYOUT),
    .HREADY_FLASH(FLASH_AHB.HREADY),
    .HRESP_FLASH(FLASH_AHB.HRESP),

    .AWID_GIC(GIC_AXI.AWID),
    .AWADDR_GIC(GIC_AXI.AWADDR),
    .AWLEN_GIC(GIC_AXI.AWLEN),
    .AWSIZE_GIC(GIC_AXI.AWSIZE),
    .AWBURST_GIC(GIC_AXI.AWBURST),
    .AWLOCK_GIC(),
    .AWCACHE_GIC(),
    .AWPROT_GIC(GIC_AXI.AWPROT),
    .AWVALID_GIC(GIC_AXI.AWVALID),
    .AWREADY_GIC(GIC_AXI.AWREADY),
    .WDATA_GIC(GIC_AXI.WDATA),
    .WSTRB_GIC(GIC_AXI.WSTRB),
    .WLAST_GIC(),
    .WVALID_GIC(GIC_AXI.WVALID),
    .WREADY_GIC(GIC_AXI.WREADY),
    .BID_GIC(GIC_AXI.BID),
    .BRESP_GIC(GIC_AXI.BRESP),
    .BVALID_GIC(GIC_AXI.BVALID),
    .BREADY_GIC(GIC_AXI.BREADY),
    .ARID_GIC(GIC_AXI.ARID),
    .ARADDR_GIC(GIC_AXI.ARADDR),
    .ARLEN_GIC(GIC_AXI.ARLEN),
    .ARSIZE_GIC(GIC_AXI.ARSIZE),
    .ARBURST_GIC(GIC_AXI.ARBURST),
    .ARLOCK_GIC(),
    .ARCACHE_GIC(),
    .ARPROT_GIC(GIC_AXI.ARPROT),
    .ARVALID_GIC(GIC_AXI.ARVALID),
    .ARREADY_GIC(GIC_AXI.ARREADY),
    .RID_GIC(GIC_AXI.RID),
    .RDATA_GIC(GIC_AXI.RDATA),
    .RRESP_GIC(GIC_AXI.RRESP),
    .RLAST_GIC(GIC_AXI.RLAST),
    .RVALID_GIC(GIC_AXI.RVALID),
    .RREADY_GIC(GIC_AXI.RREADY),

    .HSELx_PERIPHERAL(PERIPH_AHB.HSEL),
    .HADDR_PERIPHERAL(PERIPH_AHB.HADDR),
    .HTRANS_PERIPHERAL(PERIPH_AHB.HTRANS),
    .HWRITE_PERIPHERAL(PERIPH_AHB.HWRITE),
    .HSIZE_PERIPHERAL(PERIPH_AHB.HSIZE),
    .HBURST_PERIPHERAL(PERIPH_AHB.HBURST),
    .HPROT_PERIPHERAL(PERIPH_AHB.HPROT),
    .HWDATA_PERIPHERAL(PERIPH_AHB.HWDATA),
    .HRDATA_PERIPHERAL(PERIPH_AHB.HRDATA),
    .HREADYOUT_PERIPHERAL(PERIPH_AHB.HREADYOUT),
    .HREADY_PERIPHERAL(PERIPH_AHB.HREADY),
    .HRESP_PERIPHERAL(PERIPH_AHB.HRESP),

    .AWID_RAM(SRAM_AXI.AWID),
    .AWADDR_RAM(SRAM_AXI.AWADDR),
    .AWLEN_RAM(SRAM_AXI.AWLEN),
    .AWSIZE_RAM(SRAM_AXI.AWSIZE),
    .AWBURST_RAM(SRAM_AXI.AWBURST),
    .AWLOCK_RAM(SRAM_AXI.AWLOCK),
    .AWCACHE_RAM(SRAM_AXI.AWCACHE),
    .AWPROT_RAM(SRAM_AXI.AWPROT),
    .AWVALID_RAM(SRAM_AXI.AWVALID),
    .AWREADY_RAM(SRAM_AXI.AWREADY),
    .WDATA_RAM(SRAM_AXI.WDATA),
    .WSTRB_RAM(SRAM_AXI.WSTRB),
    .WLAST_RAM(SRAM_AXI.WLAST),
    .WVALID_RAM(SRAM_AXI.WVALID),
    .WREADY_RAM(SRAM_AXI.WREADY),
    .BID_RAM(SRAM_AXI.BID),
    .BRESP_RAM(SRAM_AXI.BRESP),
    .BVALID_RAM(SRAM_AXI.BVALID),
    .BREADY_RAM(SRAM_AXI.BREADY),
    .ARID_RAM(SRAM_AXI.ARID),
    .ARADDR_RAM(SRAM_AXI.ARADDR),
    .ARLEN_RAM(SRAM_AXI.ARLEN),
    .ARSIZE_RAM(SRAM_AXI.ARSIZE),
    .ARBURST_RAM(SRAM_AXI.ARBURST),
    .ARLOCK_RAM(SRAM_AXI.ARLOCK),
    .ARCACHE_RAM(SRAM_AXI.ARCACHE),
    .ARPROT_RAM(SRAM_AXI.ARPROT),
    .ARVALID_RAM(SRAM_AXI.ARVALID),
    .ARREADY_RAM(SRAM_AXI.ARREADY),
    .RID_RAM(SRAM_AXI.RID),
    .RDATA_RAM(SRAM_AXI.RDATA),
    .RRESP_RAM(SRAM_AXI.RRESP),
    .RLAST_RAM(SRAM_AXI.RLAST),
    .RVALID_RAM(SRAM_AXI.RVALID),
    .RREADY_RAM(SRAM_AXI.RREADY),

    .AWID_ROM(ROM_AXI.AWID),
    .AWADDR_ROM(ROM_AXI.AWADDR),
    .AWLEN_ROM(ROM_AXI.AWLEN),
    .AWSIZE_ROM(ROM_AXI.AWSIZE),
    .AWBURST_ROM(ROM_AXI.AWBURST),
    .AWLOCK_ROM(ROM_AXI.AWLOCK),
    .AWCACHE_ROM(ROM_AXI.AWCACHE),
    .AWPROT_ROM(ROM_AXI.AWPROT),
    .AWVALID_ROM(ROM_AXI.AWVALID),
    .AWREADY_ROM(ROM_AXI.AWREADY),
    .WDATA_ROM(ROM_AXI.WDATA),
    .WSTRB_ROM(ROM_AXI.WSTRB),
    .WLAST_ROM(ROM_AXI.WLAST),
    .WVALID_ROM(ROM_AXI.WVALID),
    .WREADY_ROM(ROM_AXI.WREADY),
    .BID_ROM(ROM_AXI.BID),
    .BRESP_ROM(ROM_AXI.BRESP),
    .BVALID_ROM(ROM_AXI.BVALID),
    .BREADY_ROM(ROM_AXI.BREADY),
    .ARID_ROM(ROM_AXI.ARID),
    .ARADDR_ROM(ROM_AXI.ARADDR),
    .ARLEN_ROM(ROM_AXI.ARLEN),
    .ARSIZE_ROM(ROM_AXI.ARSIZE),
    .ARBURST_ROM(ROM_AXI.ARBURST),
    .ARLOCK_ROM(ROM_AXI.ARLOCK),
    .ARCACHE_ROM(ROM_AXI.ARCACHE),
    .ARPROT_ROM(ROM_AXI.ARPROT),
    .ARVALID_ROM(ROM_AXI.ARVALID),
    .ARREADY_ROM(ROM_AXI.ARREADY),
    .RID_ROM(ROM_AXI.RID),
    .RDATA_ROM(ROM_AXI.RDATA),
    .RRESP_ROM(ROM_AXI.RRESP),
    .RLAST_ROM(ROM_AXI.RLAST),
    .RVALID_ROM(ROM_AXI.RVALID),
    .RREADY_ROM(ROM_AXI.RREADY),

    .PADDR_DEBUG(CPU_DBG_APB.paddr),
    .PWDATA_DEBUG(CPU_DBG_APB.pwdata),
    .PWRITE_DEBUG(CPU_DBG_APB.pwrite),
    .PENABLE_DEBUG(CPU_DBG_APB.penable),
    .PSELx_DEBUG(CPU_DBG_APB.psel),
    .PRDATA_DEBUG(CPU_DBG_APB.prdata),
    .PSLVERR_DEBUG(CPU_DBG_APB.pslverr),
    .PREADY_DEBUG(CPU_DBG_APB.pready),

    .PADDR_DMA_CTRL(PADDR_DMA_CTRL),
    .PWDATA_DMA_CTRL(PWDATA_DMA_CTRL),
    .PWRITE_DMA_CTRL(PWRITE_DMA_CTRL),
    .PPROT_DMA_CTRL(PPROT_DMA_CTRL),
    .PSTRB_DMA_CTRL(PSTRB_DMA_CTRL),
    .PENABLE_DMA_CTRL(PENABLE_DMA_CTRL),
    .PSELx_DMA_CTRL(PSELx_DMA_CTRL),
    .PRDATA_DMA_CTRL(PRDATA_DMA_CTRL),
    .PSLVERR_DMA_CTRL(PSLVERR_DMA_CTRL),
    .PREADY_DMA_CTRL(PREADY_DMA_CTRL),


    .PADDR_FLASH_CTRL(FLASH_CTRL_APB.paddr),
    .PWDATA_FLASH_CTRL(FLASH_CTRL_APB.pwdata),
    .PWRITE_FLASH_CTRL(FLASH_CTRL_APB.pwrite),
    .PPROT_FLASH_CTRL(FLASH_CTRL_APB.pprot),
    .PSTRB_FLASH_CTRL(FLASH_CTRL_APB.pstrb),
    .PENABLE_FLASH_CTRL(FLASH_CTRL_APB.penable),
    .PSELx_FLASH_CTRL(FLASH_CTRL_APB.psel),
    .PRDATA_FLASH_CTRL(FLASH_CTRL_APB.prdata),
    .PSLVERR_FLASH_CTRL(FLASH_CTRL_APB.pslverr),
    .PREADY_FLASH_CTRL(FLASH_CTRL_APB.pready),

    .PADDR_PCK_CTRL(PCK_APB.paddr),
    .PWDATA_PCK_CTRL(PCK_APB.pwdata),
    .PWRITE_PCK_CTRL(PCK_APB.pwrite),
    .PENABLE_PCK_CTRL(PCK_APB.penable),
    .PSELx_PCK_CTRL(PCK_APB.psel),
    .PRDATA_PCK_CTRL(PCK_APB.prdata),
    .PSLVERR_PCK_CTRL(PCK_APB.pslverr),
    .PREADY_PCK_CTRL(PCK_APB.pready),

    .AWID_A53(CPU_AXI.AWID),
    .AWADDR_A53(CPU_AXI.AWADDR),
    .AWLEN_A53(CPU_AXI.AWLEN),
    .AWSIZE_A53(CPU_AXI.AWSIZE),
    .AWBURST_A53(CPU_AXI.AWBURST),
    .AWLOCK_A53(CPU_AXI.AWLOCK),
    .AWCACHE_A53(CPU_AXI.AWCACHE),
    .AWPROT_A53(CPU_AXI.AWPROT),
    .AWVALID_A53(CPU_AXI.AWVALID),
    .AWREADY_A53(CPU_AXI.AWREADY),
    .WDATA_A53(CPU_AXI.WDATA),
    .WSTRB_A53(CPU_AXI.WSTRB),
    .WLAST_A53(CPU_AXI.WLAST),
    .WVALID_A53(CPU_AXI.WVALID),
    .WREADY_A53(CPU_AXI.WREADY),
    .BID_A53(CPU_AXI.BID),
    .BRESP_A53(CPU_AXI.BRESP),
    .BVALID_A53(CPU_AXI.BVALID),
    .BREADY_A53(CPU_AXI.BREADY),
    .ARID_A53(CPU_AXI.ARID),
    .ARADDR_A53(CPU_AXI.ARADDR),
    .ARLEN_A53(CPU_AXI.ARLEN),
    .ARSIZE_A53(CPU_AXI.ARSIZE),
    .ARBURST_A53(CPU_AXI.ARBURST),
    .ARLOCK_A53(CPU_AXI.ARLOCK),
    .ARCACHE_A53(CPU_AXI.ARCACHE),
    .ARPROT_A53(CPU_AXI.ARPROT),
    .ARVALID_A53(CPU_AXI.ARVALID),
    .ARREADY_A53(CPU_AXI.ARREADY),
    .RID_A53(CPU_AXI.RID),
    .RDATA_A53(CPU_AXI.RDATA),
    .RRESP_A53(CPU_AXI.RRESP),
    .RLAST_A53(CPU_AXI.RLAST),
    .RVALID_A53(CPU_AXI.RVALID),
    .RREADY_A53(CPU_AXI.RREADY),

    .HADDR_ADP(ADP_AHB.HADDR),
    .HTRANS_ADP(ADP_AHB.HTRANS),
    .HWRITE_ADP(ADP_AHB.HWRITE),
    .HSIZE_ADP(ADP_AHB.HSIZE),
    .HBURST_ADP(ADP_AHB.HBURST),
    .HPROT_ADP(ADP_AHB.HPROT),
    .HWDATA_ADP(ADP_AHB.HWDATA),
    .HRDATA_ADP(ADP_AHB.HRDATA),
    .HREADY_ADP(ADP_AHB.HREADY),
    .HRESP_ADP(ADP_AHB.HRESP),

    .AWID_DMA350(AWID_DMA350),
    .AWADDR_DMA350(AWADDR_DMA350),
    .AWLEN_DMA350(AWLEN_DMA350),
    .AWSIZE_DMA350(AWSIZE_DMA350),
    .AWBURST_DMA350(AWBURST_DMA350),
    .AWLOCK_DMA350(AWLOCK_DMA350),
    .AWCACHE_DMA350(AWCACHE_DMA350),
    .AWPROT_DMA350(AWPROT_DMA350),
    .AWVALID_DMA350(AWVALID_DMA350),
    .AWREADY_DMA350(AWREADY_DMA350),
    .WDATA_DMA350(WDATA_DMA350),
    .WSTRB_DMA350(WSTRB_DMA350),
    .WLAST_DMA350(WLAST_DMA350),
    .WVALID_DMA350(WVALID_DMA350),
    .WREADY_DMA350(WREADY_DMA350),
    .BID_DMA350(BID_DMA350),
    .BRESP_DMA350(BRESP_DMA350),
    .BVALID_DMA350(BVALID_DMA350),
    .BREADY_DMA350(BREADY_DMA350),
    .ARID_DMA350(ARID_DMA350),
    .ARADDR_DMA350(ARADDR_DMA350),
    .ARLEN_DMA350(ARLEN_DMA350),
    .ARSIZE_DMA350(ARSIZE_DMA350),
    .ARBURST_DMA350(ARBURST_DMA350),
    .ARLOCK_DMA350(ARLOCK_DMA350),
    .ARCACHE_DMA350(ARCACHE_DMA350),
    .ARPROT_DMA350(ARPROT_DMA350),
    .ARVALID_DMA350(ARVALID_DMA350),
    .ARREADY_DMA350(ARREADY_DMA350),
    .RID_DMA350(RID_DMA350),
    .RDATA_DMA350(RDATA_DMA350),
    .RRESP_DMA350(RRESP_DMA350),
    .RLAST_DMA350(RLAST_DMA350),
    .RVALID_DMA350(RVALID_DMA350),
    .RREADY_DMA350(RREADY_DMA350),

    .AWID_EXP_S(EXP_S_AXI.AWID),
    .AWADDR_EXP_S(EXP_S_AXI.AWADDR),
    .AWLEN_EXP_S(EXP_S_AXI.AWLEN),
    .AWSIZE_EXP_S(EXP_S_AXI.AWSIZE),
    .AWBURST_EXP_S(EXP_S_AXI.AWBURST),
    .AWLOCK_EXP_S(EXP_S_AXI.AWLOCK),
    .AWCACHE_EXP_S(EXP_S_AXI.AWCACHE),
    .AWPROT_EXP_S(EXP_S_AXI.AWPROT),
    .AWVALID_EXP_S(EXP_S_AXI.AWVALID),
    .AWREADY_EXP_S(EXP_S_AXI.AWREADY),
    .WDATA_EXP_S(EXP_S_AXI.WDATA),
    .WSTRB_EXP_S(EXP_S_AXI.WSTRB),
    .WLAST_EXP_S(EXP_S_AXI.WLAST),
    .WVALID_EXP_S(EXP_S_AXI.WVALID),
    .WREADY_EXP_S(EXP_S_AXI.WREADY),
    .BID_EXP_S(EXP_S_AXI.BID),
    .BRESP_EXP_S(EXP_S_AXI.BRESP),
    .BVALID_EXP_S(EXP_S_AXI.BVALID),
    .BREADY_EXP_S(EXP_S_AXI.BREADY),
    .ARID_EXP_S(EXP_S_AXI.ARID),
    .ARADDR_EXP_S(EXP_S_AXI.ARADDR),
    .ARLEN_EXP_S(EXP_S_AXI.ARLEN),
    .ARSIZE_EXP_S(EXP_S_AXI.ARSIZE),
    .ARBURST_EXP_S(EXP_S_AXI.ARBURST),
    .ARLOCK_EXP_S(EXP_S_AXI.ARLOCK),
    .ARCACHE_EXP_S(EXP_S_AXI.ARCACHE),
    .ARPROT_EXP_S(EXP_S_AXI.ARPROT),
    .ARVALID_EXP_S(EXP_S_AXI.ARVALID),
    .ARREADY_EXP_S(EXP_S_AXI.ARREADY),
    .RID_EXP_S(EXP_S_AXI.RID),
    .RDATA_EXP_S(EXP_S_AXI.RDATA),
    .RRESP_EXP_S(EXP_S_AXI.RRESP),
    .RLAST_EXP_S(EXP_S_AXI.RLAST),
    .RVALID_EXP_S(EXP_S_AXI.RVALID),
    .RREADY_EXP_S(EXP_S_AXI.RREADY),

    .clk0clk(SYS_CLK),
    .clk0clken(SYS_CLKEN),
    .clk0resetn(SYS_RESETn)
);


endmodule
