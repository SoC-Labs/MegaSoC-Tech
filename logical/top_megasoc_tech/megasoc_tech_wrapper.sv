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

    // Update 24/09/2025 No "system" DMA needed as not yet getting ethernet or PCIe added
    // // DMA 350 APB Interface Wires
    // output wire [31:0]      PADDR_DMA_CTRL,
    // output wire [31:0]      PWDATA_DMA_CTRL,
    // output wire             PWRITE_DMA_CTRL,
    // output wire [2:0]       PPROT_DMA_CTRL,
    // output wire [3:0]       PSTRB_DMA_CTRL,
    // output wire             PENABLE_DMA_CTRL,
    // output wire             PSELx_DMA_CTRL,
    // input  wire [31:0]      PRDATA_DMA_CTRL,
    // input  wire             PSLVERR_DMA_CTRL,
    // input  wire             PREADY_DMA_CTRL,

    // DMA 350 AXI Interface Wires
    // input  wire [1:0]       AWID_DMA350,
    // input  wire [43:0]      AWADDR_DMA350,
    // input  wire [7:0]       AWLEN_DMA350,
    // input  wire [2:0]       AWSIZE_DMA350,
    // input  wire [1:0]       AWBURST_DMA350,
    // input  wire             AWLOCK_DMA350,
    // input  wire [3:0]       AWCACHE_DMA350,
    // input  wire [2:0]       AWPROT_DMA350,
    // input  wire             AWVALID_DMA350,
    // output wire             AWREADY_DMA350,

    // input  wire [127:0]     WDATA_DMA350,
    // input  wire [15:0]      WSTRB_DMA350,
    // input  wire             WLAST_DMA350,
    // input  wire             WVALID_DMA350,
    // output wire             WREADY_DMA350,

    // output wire [1:0]       BID_DMA350,
    // output wire [1:0]       BRESP_DMA350,
    // output wire             BVALID_DMA350,
    // input  wire             BREADY_DMA350,

    // input  wire [1:0]       ARID_DMA350,
    // input  wire [43:0]      ARADDR_DMA350,
    // input  wire [7:0]       ARLEN_DMA350,
    // input  wire [2:0]       ARSIZE_DMA350,
    // input  wire [1:0]       ARBURST_DMA350,
    // input  wire             ARLOCK_DMA350,
    // input  wire [3:0]       ARCACHE_DMA350,
    // input  wire [2:0]       ARPROT_DMA350,
    // input  wire             ARVALID_DMA350,
    // output wire             ARREADY_DMA350,

    // output wire [1:0]       RID_DMA350,
    // output wire [127:0]     RDATA_DMA350,
    // output wire [1:0]       RRESP_DMA350,
    // output wire             RLAST_DMA350,
    // output wire             RVALID_DMA350,
    // input  wire             RREADY_DMA350,

    // input  wire [3:0]       DMA350_irq_channel,
    // input  wire             DMA350_irq_comb_nonsec,

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

    // PL011 UART
    input  wire             PL011_nUARTCTS,
    input  wire             PL011_nUARTDCD,
    input  wire             PL011_nUARTDSR,
    input  wire             PL011_nUARTRI,
    input  wire             PL011_UARTRXD,
    input  wire             PL011_SIRIN,
    output wire             PL011_UARTTXD,
    output wire             PL011_nSIROUT,
    output wire             PL011_nUARTOut2,
    output wire             PL011_nUARTOut1,
    output wire             PL011_nUARTRTS,
    output wire             PL011_nUARTDTR,

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

    // LPDDR4 Signals
    output wire             DDR4_RESET_N,
    output wire             DDR4_CK_T,
    output wire             DDR4_CK_C,
    output wire [1:0]       DDR4_CKE,
    output wire             DDR4_CS_N,
    output wire [5:0]       DDR4_ADR,
    output wire             DDR4_ODT,
    inout  wire [1:0]       DDR4_DQS_T,
    inout  wire [1:0]       DDR4_DQS_C,
    inout  wire [15:0]      DDR4_DQ,
    inout  wire [1:0]       DDR4_DM_DBI_N,
    inout  wire             DDR4_ALERT_N,
    inout  wire             DDR4_VREF,
    input  wire             DDR4_ZN_SENSE,
    output wire             DDR4_ZN,

    // SDIO PHY to MAC Signals
    output wire             SDIO_o_cfg_ddr,
    output wire             SDIO_o_cfg_ds,
    output wire             SDIO_o_cfg_dscmd,
    output wire [4:0]       SDIO_o_cfg_sample_shift,
    output wire [7:0]       SDIO_o_sdclk,
    output wire             SDIO_o_cmd_en,
    output wire             SDIO_o_cmd_tristate,
    output wire [1:0]       SDIO_o_cmd_data,
    output wire             SDIO_o_data_en,
    output wire             SDIO_o_data_tristate,
    output wire             SDIO_o_rx_en,
    output wire [31:0]      SDIO_o_tx_data,
    input  wire [1:0]       SDIO_i_cmd_strb,
    input  wire [1:0]       SDIO_i_cmd_data,
    input  wire             SDIO_i_cmd_collision,
    input  wire             SDIO_i_card_busy,
    input  wire [1:0]       SDIO_i_rx_strb,
    input  wire [15:0]      SDIO_i_rx_data,
    input  wire             SDIO_i_crcack,
    input  wire             SDIO_i_crcnak,
    output wire             SD_O_1P8V,
    input  wire             SDIO_AC_VALID,
    input  wire [1:0]       SDIO_AC_DATA,
    input  wire             SDIO_AD_VALID,
    input  wire [31:0]      SDIO_AD_DATA
);


parameter ID_W=9;
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

qchannel DRAM_SYS_Qchannel();
qchannel DRAM_DDRC_Qchannel();

assign DRAM_SYS_Qchannel.qreqn = 1'b1;
assign DRAM_DDRC_Qchannel.qreqn= 1'b1;
//--------------------------------------
//  Bus Interfaces
//--------------------------------------
// AXI
axi4 #(.DATA_W(128), .ID_W(6),    .ADDR_W(44))  CPU_AXI();
axi4 #(.DATA_W(32),  .ID_W(7),    .ADDR_W(32))  GIC_AXI();
axi4 #(.DATA_W(64),  .ID_W(ID_W), .ADDR_W(33))  DRAM_AXI();
axi4 #(.DATA_W(64),  .ID_W(ID_W), .ADDR_W(32))  SRAM_AXI();
axi4 #(.DATA_W(64),  .ID_W(ID_W), .ADDR_W(32))  ROM_AXI();
axi4 #(.DATA_W(32),  .ID_W(7)   , .ADDR_W(32))  SDIO_S_AXI();
axi4l #(.DATA_W(32), .ADDR_W(32))               SDIO_S_AXIL();
axi4 #(.DATA_W(64),  .ID_W(2)   , .ADDR_W(44))  SDIO_M_AXI();

wire [3:0] DRAM_AXI_AWQOS;
wire [3:0] DRAM_AXI_ARQOS;
// AHB
ahb #(.DATA_W(32), .ADDR_W(32))     FLASH_AHB();
ahb #(.DATA_W(32), .ADDR_W(32))     PERIPH_AHB();
ahb #(.DATA_W(32), .ADDR_W(32))     ADP_AHB();
// APB
apb3    CPU_DBG_APB();
apb3    PCK_APB();
apb4    FLASH_CTRL_APB();
apb3    DRAM_CFG_APB();
apb4    DRAM_PHY_CFG_APB();

wire                CPU_nPRESETDBG;
wire                CPU_PCLKENDBG;


assign CPU_nPRESETDBG = SYS_RESETn;
assign CPU_PCLKENDBG = 1'b1;

wire [(NUM_SPIS-1):0]   CPU_IRQS;
wire                    QSPI_IRQ;
wire [71:0]             PERI_IRQS;
wire                    SDIO_IRQ;

assign CPU_IRQS={{(NUM_SPIS-87){1'b0}}, SDIO_IRQ ,EXP_IRQs, QSPI_IRQ, PERI_IRQS, 1'b0, 4'h0};

// Subordinate AWAKEUP signal generation
assign AWAKEUP_ROM = (ROM_AXI.AWVALID | ROM_AXI.ARVALID | ROM_AXI.WVALID);
assign AWAKEUP_SRAM = (SRAM_AXI.AWVALID | SRAM_AXI.ARVALID | SRAM_AXI.WVALID);


megasoc_cpu_ss #(
    .NUM_GICRID_BITS(ID_W-2),
    .NUM_GICWID_BITS(ID_W-2),
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

    .PCLK(SYS_CLK),
    .PRESETn(SYS_RESETn),

    .DRAM_AXI(DRAM_AXI),
    .DRAM_AXI_AWQOS(DRAM_AXI_AWQOS),
    .DRAM_AXI_ARQOS(DRAM_AXI_ARQOS),
    .DRAM_CFG_APB(DRAM_CFG_APB),
    .DRAM_PHY_CFG_APB(DRAM_PHY_CFG_APB),

    .DRAM_SYS_Qchannel(DRAM_SYS_Qchannel),
    .DRAM_DDRC_Qchannel(DRAM_DDRC_Qchannel),

    .DDR4_RESET_N(DDR4_RESET_N),
    .DDR4_CK_T(DDR4_CK_T),
    .DDR4_CK_C(DDR4_CK_C),
    .DDR4_CKE(DDR4_CKE),
    .DDR4_CS_N(DDR4_CS_N),
    .DDR4_ADR(DDR4_ADR),
    .DDR4_ODT(DDR4_ODT),
    .DDR4_DQS_T(DDR4_DQS_T),
    .DDR4_DQS_C(DDR4_DQS_C),
    .DDR4_DQ(DDR4_DQ),
    .DDR4_DM_DBI_N(DDR4_DM_DBI_N),

    .DDR4_ALERT_N(DDR4_ALERT_N),
    .DDR4_VREF(DDR4_VREF),
    .DDR4_ZN_SENSE(DDR4_ZN_SENSE),
    .DDR4_ZN(DDR4_ZN)
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

    // PL011 DMA request
    .UARTTXDMACLR(1'b0),
    .UARTRXDMACLR(1'b0),
    .UARTTXDMASREQ(),
    .UARTTXDMABREQ(),
    .UARTRXDMASREQ(),
    .UARTRXDMABREQ(),
    // UART 0 Off-chip interface
    .UARTRXD0(UARTRXD0),
    .UARTTXD0(UARTTXD0),
    .UARTTXEN0(UARTTXEN0),

    // UART 1 Off-chip interface
    .UARTRXD1(UARTRXD1),
    .UARTTXD1(UARTTXD1),
    .UARTTXEN1(UARTTXEN1),

    // PL011 UART off-chip interface
    .PL011_nUARTCTS(PL011_nUARTCTS),
    .PL011_nUARTDCD(PL011_nUARTDCD),
    .PL011_nUARTDSR(PL011_nUARTDSR),
    .PL011_nUARTRI(PL011_nUARTRI),
    .PL011_UARTRXD(PL011_UARTRXD),
    .PL011_SIRIN(PL011_SIRIN),
    .PL011_UARTTXD(PL011_UARTTXD),
    .PL011_nSIROUT(PL011_nSIROUT),
    .PL011_nUARTOut2(PL011_nUARTOut2),
    .PL011_nUARTOut1(PL011_nUARTOut1),
    .PL011_nUARTRTS(PL011_nUARTRTS),
    .PL011_nUARTDTR(PL011_nUARTDTR),

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

mkaxi2axil_bridge u_axi2axil (
    .CLK(SYS_CLK),
    .RST_N(SYS_RESETn),

    .AXI4_AWVALID(SDIO_S_AXI.AWVALID),
    .AXI4_AWID(SDIO_S_AXI.AWID),
    .AXI4_AWADDR(SDIO_S_AXI.AWADDR),
    .AXI4_AWLEN(SDIO_S_AXI.AWLEN),
    .AXI4_AWSIZE(SDIO_S_AXI.AWSIZE),
    .AXI4_AWBURST(SDIO_S_AXI.AWBURST),
    .AXI4_AWLOCK(SDIO_S_AXI.AWLOCK),
    .AXI4_AWCACHE(SDIO_S_AXI.AWCACHE),
    .AXI4_AWPROT(SDIO_S_AXI.AWPROT),
    .AXI4_AWQOS(4'h0),
    .AXI4_AWREGION(4'h0),
    .AXI4_AWREADY(SDIO_S_AXI.AWREADY),
    .AXI4_WVALID(SDIO_S_AXI.WVALID),
    .AXI4_WDATA(SDIO_S_AXI.WDATA),
    .AXI4_WSTRB(SDIO_S_AXI.WSTRB),
    .AXI4_WLAST(SDIO_S_AXI.WLAST),
    .AXI4_WREADY(SDIO_S_AXI.WREADY),
    .AXI4_BVALID(SDIO_S_AXI.BVALID),
    .AXI4_BID(SDIO_S_AXI.BID),
    .AXI4_BRESP(SDIO_S_AXI.BRESP),
    .AXI4_BREADY(SDIO_S_AXI.BREADY),
    .AXI4_ARVALID(SDIO_S_AXI.ARVALID),
    .AXI4_ARID(SDIO_S_AXI.ARID),
    .AXI4_ARADDR(SDIO_S_AXI.ARADDR),
    .AXI4_ARLEN(SDIO_S_AXI.ARLEN),
    .AXI4_ARSIZE(SDIO_S_AXI.ARSIZE),
    .AXI4_ARBURST(SDIO_S_AXI.ARBURST),
    .AXI4_ARLOCK(SDIO_S_AXI.ARLOCK),
    .AXI4_ARCACHE(SDIO_S_AXI.ARCACHE),
    .AXI4_ARPROT(SDIO_S_AXI.ARPROT),
    .AXI4_ARQOS(4'h0),
    .AXI4_ARREGION(4'h0),
    .AXI4_ARREADY(SDIO_S_AXI.ARREADY),
    .AXI4_RVALID(SDIO_S_AXI.RVALID),
    .AXI4_RID(SDIO_S_AXI.RID),
    .AXI4_RDATA(SDIO_S_AXI.RDATA),
    .AXI4_RRESP(SDIO_S_AXI.RRESP),
    .AXI4_RLAST(SDIO_S_AXI.RLAST),
    .AXI4_RREADY(SDIO_S_AXI.RREADY),

    .AXI4L_AWVALID(SDIO_S_AXIL.AWVALID),
    .AXI4L_AWADDR(SDIO_S_AXIL.AWADDR),
    .AXI4L_AWPROT(SDIO_S_AXIL.AWPROT),
    .AXI4L_AWREADY(SDIO_S_AXIL.AWREADY),
    .AXI4L_WVALID(SDIO_S_AXIL.WVALID),
    .AXI4L_WDATA(SDIO_S_AXIL.WDATA),
    .AXI4L_WSTRB(SDIO_S_AXIL.WSTRB),
    .AXI4L_WREADY(SDIO_S_AXIL.WREADY),
    .AXI4L_BVALID(SDIO_S_AXIL.BVALID),
    .AXI4L_BRESP(SDIO_S_AXIL.BRESP),
    .AXI4L_BREADY(SDIO_S_AXIL.BREADY),
    .AXI4L_ARVALID(SDIO_S_AXIL.ARVALID),
    .AXI4L_ARADDR(SDIO_S_AXIL.ARADDR),
    .AXI4L_ARPROT(SDIO_S_AXIL.ARPROT),
    .AXI4L_ARREADY(SDIO_S_AXIL.ARREADY),
    .AXI4L_RVALID(SDIO_S_AXIL.RVALID),
    .AXI4L_RRESP(SDIO_S_AXIL.RRESP),
    .AXI4L_RDATA(SDIO_S_AXIL.RDATA),
    .AXI4L_RREADY(SDIO_S_AXIL.RREADY)
);

wire sd_1p8v;
assign SD_O_1P8V = sd_1p8v;

`define SDIO_AXI
sdio #(
    .OPT_DMA(1'b1),
    .ADDRESS_WIDTH(44),
    .DMA_DW(64),
    .AXI_IW(2), // ID Widtth
    .OPT_LITTLE_ENDIAN(1'b1),
    .OPT_SERDES(1'b0),
    .OPT_DDR(1'b0),
    .OPT_EMMC(1'b0),
    .OPT_1P8V(1'b1),
    .OPT_CRCTOKEN(1'b1)
    ) u_sdio_controller (
        .i_clk(SYS_CLK),
        .i_reset(~SYS_RESETn),
        // Control Interface AXI lite
        .S_AXIL_AWVALID(SDIO_S_AXIL.AWVALID),
        .S_AXIL_AWREADY(SDIO_S_AXIL.AWREADY),
        .S_AXIL_AWADDR(SDIO_S_AXIL.AWADDR[4:0]),
        .S_AXIL_AWPROT(SDIO_S_AXIL.AWPROT),

        .S_AXIL_WVALID(SDIO_S_AXIL.WVALID),
        .S_AXIL_WREADY(SDIO_S_AXIL.WREADY),
        .S_AXIL_WDATA(SDIO_S_AXIL.WDATA),
        .S_AXIL_WSTRB(SDIO_S_AXIL.WSTRB),

        .S_AXIL_BVALID(SDIO_S_AXIL.BVALID),
        .S_AXIL_BREADY(SDIO_S_AXIL.BREADY),
        .S_AXIL_BRESP(SDIO_S_AXIL.BRESP),

        .S_AXIL_ARVALID(SDIO_S_AXIL.ARVALID),
        .S_AXIL_ARREADY(SDIO_S_AXIL.ARREADY),
        .S_AXIL_ARADDR(SDIO_S_AXIL.ARADDR[4:0]),
        .S_AXIL_ARPROT(SDIO_S_AXIL.ARPROT),

        .S_AXIL_RVALID(SDIO_S_AXIL.RVALID),
        .S_AXIL_RREADY(SDIO_S_AXIL.RREADY),
        .S_AXIL_RDATA(SDIO_S_AXIL.RDATA),
        .S_AXIL_RRESP(SDIO_S_AXIL.RRESP),
        // DMA AXI Manager
        .M_AXI_AWVALID(SDIO_M_AXI.AWVALID),
        .M_AXI_AWREADY(SDIO_M_AXI.AWREADY),
        .M_AXI_AWID(SDIO_M_AXI.AWID),
        .M_AXI_AWADDR(SDIO_M_AXI.AWADDR),
        .M_AXI_AWLEN(SDIO_M_AXI.AWLEN),
        .M_AXI_AWSIZE(SDIO_M_AXI.AWSIZE),
        .M_AXI_AWBURST(SDIO_M_AXI.AWBURST),
        .M_AXI_AWLOCK(SDIO_M_AXI.AWLOCK),
        .M_AXI_AWCACHE(SDIO_M_AXI.AWCACHE),
        .M_AXI_AWPROT(SDIO_M_AXI.AWPROT),
        .M_AXI_AWQOS(),

        .M_AXI_WVALID(SDIO_M_AXI.WVALID),
        .M_AXI_WREADY(SDIO_M_AXI.WREADY),
        .M_AXI_WDATA(SDIO_M_AXI.WDATA),
        .M_AXI_WSTRB(SDIO_M_AXI.WSTRB),
        .M_AXI_WLAST(SDIO_M_AXI.WLAST),

        .M_AXI_BVALID(SDIO_M_AXI.BVALID),
        .M_AXI_BID(SDIO_M_AXI.BID),
        .M_AXI_BREADY(SDIO_M_AXI.BREADY),
        .M_AXI_BRESP(SDIO_M_AXI.BRESP),

        .M_AXI_ARVALID(SDIO_M_AXI.ARVALID),
        .M_AXI_ARREADY(SDIO_M_AXI.ARREADY),
        .M_AXI_ARID(SDIO_M_AXI.ARID),
        .M_AXI_ARADDR(SDIO_M_AXI.ARADDR),
        .M_AXI_ARLEN(SDIO_M_AXI.ARLEN),
        .M_AXI_ARSIZE(SDIO_M_AXI.ARSIZE),
        .M_AXI_ARBURST(SDIO_M_AXI.ARBURST),
        .M_AXI_ARLOCK(SDIO_M_AXI.ARLOCK),
        .M_AXI_ARCACHE(SDIO_M_AXI.ARCACHE),
        .M_AXI_ARPROT(SDIO_M_AXI.ARPROT),
        .M_AXI_ARQOS(),

        .M_AXI_RVALID(SDIO_M_AXI.RVALID),
        .M_AXI_RREADY(SDIO_M_AXI.RREADY),
        .M_AXI_RID(SDIO_M_AXI.RID),
        .M_AXI_RDATA(SDIO_M_AXI.RDATA),
        .M_AXI_RLAST(SDIO_M_AXI.RLAST),
        .M_AXI_RRESP(SDIO_M_AXI.RRESP),

        // External Stream interface
        .s_valid(),
        .s_ready(),
        .s_data(),
        .m_valid(),
        .m_ready(),
        .m_data(),
        .m_last(),

        // ?
        .i_card_detect(1'b1),
        .o_hwreset_n(),
        .o_1p8v(sd_1p8v),
        .i_1p8v(sd_1p8v),
        .o_int(SDIO_IRQ),

        // Interface to PHY
        .o_cfg_ddr(SDIO_o_cfg_ddr),
        .o_cfg_ds(SDIO_o_cfg_ds),
        .o_cfg_dscmd(SDIO_o_cfg_dscmd),
        .o_cfg_sample_shift(SDIO_o_cfg_sample_shift),
        .o_sdclk(SDIO_o_sdclk),
        .o_cmd_en(SDIO_o_cmd_en),
        .o_cmd_tristate(SDIO_o_cmd_tristate),
        .o_cmd_data(SDIO_o_cmd_data),
        .o_data_en(SDIO_o_data_en),
        .o_data_tristate(SDIO_o_data_tristate),
        .o_rx_en(SDIO_o_rx_en),
        .o_tx_data(SDIO_o_tx_data),
        .i_cmd_strb(SDIO_i_cmd_strb),
        .i_cmd_data(SDIO_i_cmd_data),
        .i_cmd_collision(SDIO_i_cmd_collision),
        .i_card_busy(SDIO_i_card_busy),
        .i_rx_strb(SDIO_i_rx_strb),
        .i_rx_data(SDIO_i_rx_data),
        .i_crcack(SDIO_i_crcack),
        .i_crcnak(SDIO_i_crcnak),
        .S_AC_VALID(SDIO_AC_VALID),
        .S_AC_DATA(SDIO_AC_DATA),
        .S_AD_VALID(SDIO_AD_VALID),
        .S_AD_DATA(SDIO_AD_DATA)
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
    .AWQOS_DRAM(DRAM_AXI_AWQOS),
    .ARQOS_DRAM(DRAM_AXI_ARQOS),

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

    .AWID_SDIO_S(SDIO_S_AXI.AWID),
    .AWADDR_SDIO_S(SDIO_S_AXI.AWADDR),
    .AWLEN_SDIO_S(SDIO_S_AXI.AWLEN),
    .AWSIZE_SDIO_S(SDIO_S_AXI.AWSIZE),
    .AWBURST_SDIO_S(SDIO_S_AXI.AWBURST),
    .AWLOCK_SDIO_S(SDIO_S_AXI.AWLOCK),
    .AWCACHE_SDIO_S(SDIO_S_AXI.AWCACHE),
    .AWPROT_SDIO_S(SDIO_S_AXI.AWPROT),
    .AWVALID_SDIO_S(SDIO_S_AXI.AWVALID),
    .AWREADY_SDIO_S(SDIO_S_AXI.AWREADY),
    .WDATA_SDIO_S(SDIO_S_AXI.WDATA),
    .WSTRB_SDIO_S(SDIO_S_AXI.WSTRB),
    .WLAST_SDIO_S(SDIO_S_AXI.WLAST),
    .WVALID_SDIO_S(SDIO_S_AXI.WVALID),
    .WREADY_SDIO_S(SDIO_S_AXI.WREADY),
    .BID_SDIO_S(SDIO_S_AXI.BID),
    .BRESP_SDIO_S(SDIO_S_AXI.BRESP),
    .BVALID_SDIO_S(SDIO_S_AXI.BVALID),
    .BREADY_SDIO_S(SDIO_S_AXI.BREADY),
    .ARID_SDIO_S(SDIO_S_AXI.ARID),
    .ARADDR_SDIO_S(SDIO_S_AXI.ARADDR),
    .ARLEN_SDIO_S(SDIO_S_AXI.ARLEN),
    .ARSIZE_SDIO_S(SDIO_S_AXI.ARSIZE),
    .ARBURST_SDIO_S(SDIO_S_AXI.ARBURST),
    .ARLOCK_SDIO_S(SDIO_S_AXI.ARLOCK),
    .ARCACHE_SDIO_S(SDIO_S_AXI.ARCACHE),
    .ARPROT_SDIO_S(SDIO_S_AXI.ARPROT),
    .ARVALID_SDIO_S(SDIO_S_AXI.ARVALID),
    .ARREADY_SDIO_S(SDIO_S_AXI.ARREADY),
    .RID_SDIO_S(SDIO_S_AXI.RID),
    .RDATA_SDIO_S(SDIO_S_AXI.RDATA),
    .RRESP_SDIO_S(SDIO_S_AXI.RRESP),
    .RLAST_SDIO_S(SDIO_S_AXI.RLAST),
    .RVALID_SDIO_S(SDIO_S_AXI.RVALID),
    .RREADY_SDIO_S(SDIO_S_AXI.RREADY),


    .PADDR_DEBUG(CPU_DBG_APB.paddr),
    .PWDATA_DEBUG(CPU_DBG_APB.pwdata),
    .PWRITE_DEBUG(CPU_DBG_APB.pwrite),
    .PENABLE_DEBUG(CPU_DBG_APB.penable),
    .PSELx_DEBUG(CPU_DBG_APB.psel),
    .PRDATA_DEBUG(CPU_DBG_APB.prdata),
    .PSLVERR_DEBUG(CPU_DBG_APB.pslverr),
    .PREADY_DEBUG(CPU_DBG_APB.pready),


    .PADDR_DRAM_CFG(DRAM_CFG_APB.paddr),
    .PWDATA_DRAM_CFG(DRAM_CFG_APB.pwdata),
    .PWRITE_DRAM_CFG(DRAM_CFG_APB.pwrite),
    .PENABLE_DRAM_CFG(DRAM_CFG_APB.penable),
    .PSELx_DRAM_CFG(DRAM_CFG_APB.psel),
    .PRDATA_DRAM_CFG(DRAM_CFG_APB.prdata),
    .PSLVERR_DRAM_CFG(DRAM_CFG_APB.pslverr),
    .PREADY_DRAM_CFG(DRAM_CFG_APB.pready),

    .PADDR_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.paddr),
    .PWDATA_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.pwdata),
    .PWRITE_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.pwrite),
    .PPROT_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.pprot),
    .PSTRB_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.pstrb),
    .PENABLE_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.penable),
    .PSELx_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.psel),
    .PRDATA_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.prdata),
    .PSLVERR_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.pslverr),
    .PREADY_DRAM_PHY_CFG(DRAM_PHY_CFG_APB.pready),

    // .PADDR_DMA_CTRL(PADDR_DMA_CTRL),
    // .PWDATA_DMA_CTRL(PWDATA_DMA_CTRL),
    // .PWRITE_DMA_CTRL(PWRITE_DMA_CTRL),
    // .PPROT_DMA_CTRL(PPROT_DMA_CTRL),
    // .PSTRB_DMA_CTRL(PSTRB_DMA_CTRL),
    // .PENABLE_DMA_CTRL(PENABLE_DMA_CTRL),
    // .PSELx_DMA_CTRL(PSELx_DMA_CTRL),
    // .PRDATA_DMA_CTRL(PRDATA_DMA_CTRL),
    // .PSLVERR_DMA_CTRL(PSLVERR_DMA_CTRL),
    // .PREADY_DMA_CTRL(PREADY_DMA_CTRL),


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

    // .AWID_DMA350(AWID_DMA350),
    // .AWADDR_DMA350(AWADDR_DMA350),
    // .AWLEN_DMA350(AWLEN_DMA350),
    // .AWSIZE_DMA350(AWSIZE_DMA350),
    // .AWBURST_DMA350(AWBURST_DMA350),
    // .AWLOCK_DMA350(AWLOCK_DMA350),
    // .AWCACHE_DMA350(AWCACHE_DMA350),
    // .AWPROT_DMA350(AWPROT_DMA350),
    // .AWVALID_DMA350(AWVALID_DMA350),
    // .AWREADY_DMA350(AWREADY_DMA350),
    // .WDATA_DMA350(WDATA_DMA350),
    // .WSTRB_DMA350(WSTRB_DMA350),
    // .WLAST_DMA350(WLAST_DMA350),
    // .WVALID_DMA350(WVALID_DMA350),
    // .WREADY_DMA350(WREADY_DMA350),
    // .BID_DMA350(BID_DMA350),
    // .BRESP_DMA350(BRESP_DMA350),
    // .BVALID_DMA350(BVALID_DMA350),
    // .BREADY_DMA350(BREADY_DMA350),
    // .ARID_DMA350(ARID_DMA350),
    // .ARADDR_DMA350(ARADDR_DMA350),
    // .ARLEN_DMA350(ARLEN_DMA350),
    // .ARSIZE_DMA350(ARSIZE_DMA350),
    // .ARBURST_DMA350(ARBURST_DMA350),
    // .ARLOCK_DMA350(ARLOCK_DMA350),
    // .ARCACHE_DMA350(ARCACHE_DMA350),
    // .ARPROT_DMA350(ARPROT_DMA350),
    // .ARVALID_DMA350(ARVALID_DMA350),
    // .ARREADY_DMA350(ARREADY_DMA350),
    // .RID_DMA350(RID_DMA350),
    // .RDATA_DMA350(RDATA_DMA350),
    // .RRESP_DMA350(RRESP_DMA350),
    // .RLAST_DMA350(RLAST_DMA350),
    // .RVALID_DMA350(RVALID_DMA350),
    // .RREADY_DMA350(RREADY_DMA350),

    .AWID_SDIO_M(SDIO_M_AXI.AWID),
    .AWADDR_SDIO_M(SDIO_M_AXI.AWADDR),
    .AWLEN_SDIO_M(SDIO_M_AXI.AWLEN),
    .AWSIZE_SDIO_M(SDIO_M_AXI.AWSIZE),
    .AWBURST_SDIO_M(SDIO_M_AXI.AWBURST),
    .AWLOCK_SDIO_M(SDIO_M_AXI.AWLOCK),
    .AWCACHE_SDIO_M(SDIO_M_AXI.AWCACHE),
    .AWPROT_SDIO_M(SDIO_M_AXI.AWPROT),
    .AWVALID_SDIO_M(SDIO_M_AXI.AWVALID),
    .AWREADY_SDIO_M(SDIO_M_AXI.AWREADY),
    .WDATA_SDIO_M(SDIO_M_AXI.WDATA),
    .WSTRB_SDIO_M(SDIO_M_AXI.WSTRB),
    .WLAST_SDIO_M(SDIO_M_AXI.WLAST),
    .WVALID_SDIO_M(SDIO_M_AXI.WVALID),
    .WREADY_SDIO_M(SDIO_M_AXI.WREADY),
    .BID_SDIO_M(SDIO_M_AXI.BID),
    .BRESP_SDIO_M(SDIO_M_AXI.BRESP),
    .BVALID_SDIO_M(SDIO_M_AXI.BVALID),
    .BREADY_SDIO_M(SDIO_M_AXI.BREADY),
    .ARID_SDIO_M(SDIO_M_AXI.ARID),
    .ARADDR_SDIO_M(SDIO_M_AXI.ARADDR),
    .ARLEN_SDIO_M(SDIO_M_AXI.ARLEN),
    .ARSIZE_SDIO_M(SDIO_M_AXI.ARSIZE),
    .ARBURST_SDIO_M(SDIO_M_AXI.ARBURST),
    .ARLOCK_SDIO_M(SDIO_M_AXI.ARLOCK),
    .ARCACHE_SDIO_M(SDIO_M_AXI.ARCACHE),
    .ARPROT_SDIO_M(SDIO_M_AXI.ARPROT),
    .ARVALID_SDIO_M(SDIO_M_AXI.ARVALID),
    .ARREADY_SDIO_M(SDIO_M_AXI.ARREADY),
    .RID_SDIO_M(SDIO_M_AXI.RID),
    .RDATA_SDIO_M(SDIO_M_AXI.RDATA),
    .RRESP_SDIO_M(SDIO_M_AXI.RRESP),
    .RLAST_SDIO_M(SDIO_M_AXI.RLAST),
    .RVALID_SDIO_M(SDIO_M_AXI.RVALID),
    .RREADY_SDIO_M(SDIO_M_AXI.RREADY),

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
