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
//  DRAM, selected by INC_LPDDR4:
//    megasoc_axi_async_bridge      (u_dram_axi_async_bridge)   INC_LPDDR4
//    dram_wrapper                  (u_lpddr4_subsystem)        INC_LPDDR4
//    megasoc_ahb_to_apb_async      (u_lpddr4_cfg_bridge)       INC_LPDDR4
//    megasoc_dram_wrapper          (u_megasoc_dram_wrapper)    otherwise (fast sim model)
//  ethmac_subsystem_apb            (u_ethmac_subsystem)        INC_ETH
//  megasoc_dma350_wrapper          (u_megasoc_dma350)          INC_DMA
//-----------------------------------------------------------------------------
// To Do
//  - Replace sl_ahb_sram with QSPI controller to use external flash
//  - LPDDR4: connect the Q-channels to the power controller


`timescale 1ns/1ps

module megasoc_tech_wrapper(
    input  wire             SYS_CLK,
    input  wire             SYS_CLKEN,
    input  wire             RT_CLK, // 32kHz real time clock
    input  wire             SYS_RESETn,   // (Here SYS_RESETn is PORESETn)

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
    input  wire             DDR_nEVENT,

    // LPDDR4 DRAM (lpddr4_subsystem). Driven only in INC_LPDDR4 builds; idle
    // otherwise (the default build uses the fast sim DRAM responder).
    input  wire             DDR_CLK,          // LPDDR4 controller/DFI clock (~400 MHz)
    output wire             LPDDR4_RESET_N,
    output wire             LPDDR4_CK_T,
    output wire             LPDDR4_CK_C,
    output wire [1:0]       LPDDR4_CKE,
    output wire [1:0]       LPDDR4_CS,
    output wire [5:0]       LPDDR4_CA,
    output wire             LPDDR4_ODT,
    inout  wire [1:0]       LPDDR4_DQS_T,
    inout  wire [1:0]       LPDDR4_DQS_C,
    inout  wire [15:0]      LPDDR4_DQ,
    inout  wire [1:0]       LPDDR4_DMI,
    inout  wire             LPDDR4_ALERT_N,
    inout  wire             LPDDR4_VREF,
    input  wire             LPDDR4_ZN_SENSE,
    output wire             LPDDR4_ZN,

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
    input  wire [31:0]      SDIO_AD_DATA,

    // Ethernet MII + MDIO (ethmac_subsystem_apb). Driven only in INC_ETH
    // builds; otherwise outputs are 0 and inputs are unused. The ports exist
    // in both builds so the chip pin list does not change.
    input  wire             ETH_TX_CLK,       // 25 MHz (100 Mb/s) / 2.5 MHz (10 Mb/s) from PHY
    output wire [3:0]       ETH_TXD,
    output wire             ETH_TX_EN,
    output wire             ETH_TX_ER,
    input  wire             ETH_RX_CLK,
    input  wire [3:0]       ETH_RXD,
    input  wire             ETH_RX_DV,
    input  wire             ETH_RX_ER,
    input  wire             ETH_COL,
    input  wire             ETH_CRS,
    output wire             ETH_MDC,
    input  wire             ETH_MDIO_i,
    output wire             ETH_MDIO_o,
    output wire             ETH_MDIO_e        // 1 = drive ETH_MDIO_o onto the pad
);

wire SYS_HRESETn;
wire SYS_PRESETn;

wire rst_ahb_ctrl_n_o_from_pss;
wire rst_apb_ctrl_n_o_from_pss;
wire rst_dbg_ctrl_n_o_from_pss;

logic sys_reset_req_from_pss;
logic SYS_RESETn_int;
logic sys_reset_req_sync1, sys_reset_req_sync2;
logic [1:0] ahb_ctrl_sync, apb_ctrl_sync;

always_ff @(posedge SYS_CLK or negedge SYS_RESETn) begin
  if (!SYS_RESETn) begin
    ahb_ctrl_sync <= 2'b00;
    apb_ctrl_sync <= 2'b00;
  end else begin
    ahb_ctrl_sync <= {ahb_ctrl_sync[0], rst_ahb_ctrl_n_o_from_pss};
    apb_ctrl_sync <= {apb_ctrl_sync[0], rst_apb_ctrl_n_o_from_pss};
  end
end

assign SYS_HRESETn = ahb_ctrl_sync[1];
assign SYS_PRESETn = apb_ctrl_sync[1];

always_ff @(posedge SYS_CLK or negedge SYS_RESETn) begin
  if (!SYS_RESETn) begin
    sys_reset_req_sync1 <= 1'b0;
    sys_reset_req_sync2 <= 1'b0;
  end else begin
    sys_reset_req_sync1 <= sys_reset_req_from_pss;
    sys_reset_req_sync2 <= sys_reset_req_sync1;
  end
end

assign SYS_RESETn_int = SYS_RESETn & ~sys_reset_req_sync2;


// AXI ID width on the NIC-400 ROM, RAM and DRAM ports (and the SIE-300 /
// DRAM controllers behind them). 10 since ETH_M and DMA_M were added to the
// NIC (global ID = VID 6 + SIID 3 + IID 1); the SIE-300 config
// (socrates/BP301_SRAM/config/SRAM_ctrl.yaml ID_WIDTH) must match.
parameter ID_W=10;
// AXI ID width on the NIC-400 GIC port (the NIC compresses the ID to 7 bits
// there); independent of ID_W.
parameter GIC_ID_W=7;
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

// DRAM (LPDDR4) power domain, controller side (SYS_CLK): AXI port and
// controller core channels (entry order AXI port first, see
// megasoc_DRAM_power_control)
qchannel DRAM_qchan_sys();
qchannel DRAM_qchan_ddrc();
wire     CPU_CORE_PORESETn;
wire     CPU_CORE_WRMRSTn;
wire     CPU_L2_RESETn;
wire CPU_CORE_WRMRSTn_final;
wire CPU_L2_RESETn_final;
wire CPU_CORE_WRMRSTn_pc;
wire CPU_L2_RESETn_pc;

//--------------------------------------
//  Bus Interfaces
//--------------------------------------
// AXI
axi4 #(.DATA_W(128), .ID_W(6), .ADDR_W(44))     CPU_AXI();
axi4 #(.DATA_W(32), .ID_W(GIC_ID_W), .ADDR_W(32)) GIC_AXI();
axi4 #(.DATA_W(64), .ID_W(ID_W), .ADDR_W(33))   DRAM_AXI();
axi4 #(.DATA_W(64), .ID_W(ID_W), .ADDR_W(32))   SRAM_AXI();
axi4 #(.DATA_W(64), .ID_W(ID_W), .ADDR_W(32))   ROM_AXI();
axi4 #(.DATA_W(32), .ID_W(7)   , .ADDR_W(32))   SDIO_S_AXI();
axi4l #(.DATA_W(32), .ADDR_W(32))               SDIO_S_AXIL();
axi4 #(.DATA_W(64), .ID_W(2)   , .ADDR_W(44))   SDIO_M_AXI();
// AHB
ahb #(.DATA_W(32), .ADDR_W(32))     FLASH_AHB();
ahb #(.DATA_W(32), .ADDR_W(32))     PERIPH_AHB();
ahb #(.DATA_W(32), .ADDR_W(32))     ADP_AHB();
ahb #(.DATA_W(32), .ADDR_W(32))     LPDDR4_CFG_AHB();   // peripheral window 0x41xx_xxxx
// APB
apb3    CPU_DBG_APB();
apb3    PCK_APB();
apb4    FLASH_CTRL_APB();

// Ethernet subsystem and DMA-350 ports on the NIC-400 (added 2026-11).
//   ETH_M  AHB-Lite master into the NIC (Ethernet MAC DMA)
//   DMA_M  AXI4 master into the NIC (DMA-350)
//   ETH_S  APB4 target, 0x0102_0000 (Ethernet registers)
//   DMA_S  APB4 target, 0x0103_0000 (DMA-350 registers)
// ETH_M/ETH_S connect to ethmac_subsystem_apb in INC_ETH builds and are
// tied to idle otherwise. DMA_M/DMA_S connect to megasoc_dma350_wrapper in
// INC_DMA builds and are tied to idle otherwise.
ahb #(.DATA_W(32), .ADDR_W(32))                 ETH_M_AHB();
axi4 #(.DATA_W(64), .ID_W(2), .ADDR_W(44))      DMA_M_AXI();
apb4    ETH_S_APB();
apb4    DMA_S_APB();

wire ETH_IRQ;           // MAC interrupt (INT_SOURCE & INT_MASK), CPU_IRQS[95]
wire ETH_CKSUM_IRQ;     // RX checksum unit interrupt, CPU_IRQS[96]

`ifdef INC_ETH
//--------------------------------------
//  Ethernet subsystem (Ethernet-MAC-AHB submodule, logical/ethernet_mac_ahb)
//--------------------------------------
// OpenCores ethmac (10/100, MII) + HA1588 PTP timestamping + RX checksum
// unit. Register window (ETH_S, 64 KB at 0x0102_0000), decoded inside the
// block on paddr[15:12]:
//   +0x0000  MAC registers (0x000-0x3FF) and buffer descriptors (0x400-0x7FF)
//   +0x1000  HA1588 PTP registers
//   +0x2000  RX checksum unit
// The MAC's buffer-descriptor DMA is the AHB master ETH_M (reaches RAM and
// DRAM only). The block runs on SYS_CLK and the same reset as the NIC and
// the SDIO controller, so a soft reset cannot cut an ETH_M transfer off
// while the NIC side stays out of reset. The MII clocks come from the PHY;
// the crossings to them are inside the block.
// PTP: rtc_clk = SYS_CLK, servo/PHC inputs tied to 0, PTP outputs unused.
// SWAP_DMA_BYTES = 1: the MAC DMA otherwise sends byte 3 of each 32-bit word
// first (big-endian lanes). With 1, bytes go on the wire in memory order, as
// the little-endian A53's network stack lays frames out. Measured by the
// byte-order case of eth_loopback_tests (2026-11-06, with 0: the wire-order
// frame was dropped by the address filter, the word-swapped one received).
ethmac_subsystem_apb #(
    .SWAP_DMA_BYTES(1'b1)
) u_ethmac_subsystem (
    .pclk                       (SYS_CLK),
    .presetn                    (SYS_RESETn_int),

    // AHB-Lite master (MAC DMA) -> NIC-400 ETH_M
    .haddr                      (ETH_M_AHB.HADDR),
    .htrans                     (ETH_M_AHB.HTRANS),
    .hsize                      (ETH_M_AHB.HSIZE),
    .hburst                     (ETH_M_AHB.HBURST),
    .hprot                      (ETH_M_AHB.HPROT),
    .hmastlock                  (),                 // no HMASTLOCK on the NIC port; tied 0 in the block
    .hwrite                     (ETH_M_AHB.HWRITE),
    .hwdata                     (ETH_M_AHB.HWDATA),
    .hrdata                     (ETH_M_AHB.HRDATA),
    .hready                     (ETH_M_AHB.HREADY),
    .hresp                      (ETH_M_AHB.HRESP),

    // APB4 target <- NIC-400 ETH_S (pprot unused)
    .psel                       (ETH_S_APB.psel),
    .penable                    (ETH_S_APB.penable),
    .paddr                      (ETH_S_APB.paddr),
    .pwrite                     (ETH_S_APB.pwrite),
    .pwdata                     (ETH_S_APB.pwdata),
    .pstrb                      (ETH_S_APB.pstrb),
    .prdata                     (ETH_S_APB.prdata),
    .pready                     (ETH_S_APB.pready),
    .pslverr                    (ETH_S_APB.pslverr),

    // PTP real-time clock
    .rtc_clk                    (SYS_CLK),
    .rtc_time_ptp_ns            (),
    .rtc_time_ptp_sec           (),
    .rtc_time_one_pps           (),

    // MII
    .mtx_clk_pad_i              (ETH_TX_CLK),
    .mtxd_pad_o                 (ETH_TXD),
    .mtxen_pad_o                (ETH_TX_EN),
    .mtxerr_pad_o               (ETH_TX_ER),
    .mrx_clk_pad_i              (ETH_RX_CLK),
    .mrxd_pad_i                 (ETH_RXD),
    .mrxdv_pad_i                (ETH_RX_DV),
    .mrxerr_pad_i               (ETH_RX_ER),
    .mcoll_pad_i                (ETH_COL),
    .mcrs_pad_i                 (ETH_CRS),

    // MDIO
    .md_pad_i                   (ETH_MDIO_i),
    .mdc_pad_o                  (ETH_MDC),
    .md_pad_o                   (ETH_MDIO_o),
    .md_padoe_o                 (ETH_MDIO_e),

    // Interrupts (level, SYS_CLK domain)
    .int_o                      (ETH_IRQ),
    .cksum_int_o                (ETH_CKSUM_IRQ),

    // PTP event pulses and servo/PHC interface: unused
    .rx_ptp_event               (),
    .tx_ptp_event               (),
    .ha1588_servo_en            (1'b0),
    .ha1588_sync_interval       (30'h0),
    .phc_seconds                (48'h0),
    .phc_nanoseconds            (30'h0),
    .phc_hw_cap_seconds         (48'h0),
    .phc_hw_cap_nanoseconds     (30'h0),
    .phc_hw_cap_sub_nanoseconds (32'h0),
    .ha1588_hw_capture          (),
    .ha1588_hw_set_time         (),
    .ha1588_hw_set_seconds      (),
    .ha1588_hw_set_nanoseconds  (),
    .ha1588_hw_adj_valid        (),
    .ha1588_hw_adj_ns_incr_frac (),
    .ha1588_servo_locked        (),
    .ha1588_servo_phase_step_active ()
);
`else
// No Ethernet subsystem: ETH_M makes no transfers (HTRANS = IDLE), ETH_S
// completes every access at once with read data 0 and no error (so a CPU
// access to the window cannot hang the bus), MII/MDIO outputs are 0 and
// the interrupts are 0.
assign ETH_M_AHB.HADDR   = 32'h0;
assign ETH_M_AHB.HTRANS  = 2'b00;
assign ETH_M_AHB.HWRITE  = 1'b0;
assign ETH_M_AHB.HSIZE   = 3'b010;
assign ETH_M_AHB.HBURST  = 3'b000;
assign ETH_M_AHB.HPROT   = 4'b0011;
assign ETH_M_AHB.HWDATA  = 32'h0;

assign ETH_S_APB.prdata  = 32'h0;
assign ETH_S_APB.pready  = 1'b1;
assign ETH_S_APB.pslverr = 1'b0;

assign ETH_TXD       = 4'h0;
assign ETH_TX_EN     = 1'b0;
assign ETH_TX_ER     = 1'b0;
assign ETH_MDC       = 1'b0;
assign ETH_MDIO_o    = 1'b0;
assign ETH_MDIO_e    = 1'b0;

assign ETH_IRQ       = 1'b0;
assign ETH_CKSUM_IRQ = 1'b0;
`endif

wire [3:0] DMA_IRQ_CH;      // DMA-350 channel 0-3 interrupts, CPU_IRQS[100:97]
wire       DMA_IRQ_COMB;    // DMA-350 combined Non-secure interrupt, CPU_IRQS[101]

`ifdef INC_DMA
//--------------------------------------
//  DMA-350 (Arm CoreLink DMA-350, generated into logical/dma350/)
//--------------------------------------
// Memory-to-memory configuration: 4 channels, 64-bit data, 44-bit address,
// no triggers, no TrustZone (all channels Non-secure). Registers: DMA_S,
// first 8 KB of the 64 KB window at 0x0103_0000 (the rest reads 0). Bus
// manager: DMA_M, reaching ROM, FLASH, RAM, PERIPHERAL and DRAM.
// Same clock and reset as the NIC and the other bus managers (SDIO, ETH).
megasoc_dma350_wrapper #(
    .ADDR_W (44),
    .DATA_W (64),
    .ID_W   (2),
    .NUM_CH (4)
) u_megasoc_dma350 (
    .clk                (SYS_CLK),
    .resetn             (SYS_RESETn_int),

    .psel               (DMA_S_APB.psel),
    .penable            (DMA_S_APB.penable),
    .paddr              (DMA_S_APB.paddr),
    .pwrite             (DMA_S_APB.pwrite),
    .pwdata             (DMA_S_APB.pwdata),
    .pstrb              (DMA_S_APB.pstrb),
    .pprot              (DMA_S_APB.pprot),
    .prdata             (DMA_S_APB.prdata),
    .pready             (DMA_S_APB.pready),
    .pslverr            (DMA_S_APB.pslverr),

    .awid               (DMA_M_AXI.AWID),
    .awaddr             (DMA_M_AXI.AWADDR),
    .awlen              (DMA_M_AXI.AWLEN),
    .awsize             (DMA_M_AXI.AWSIZE),
    .awburst            (DMA_M_AXI.AWBURST),
    .awlock             (DMA_M_AXI.AWLOCK),
    .awcache            (DMA_M_AXI.AWCACHE),
    .awprot             (DMA_M_AXI.AWPROT),
    .awvalid            (DMA_M_AXI.AWVALID),
    .awready            (DMA_M_AXI.AWREADY),
    .wdata              (DMA_M_AXI.WDATA),
    .wstrb              (DMA_M_AXI.WSTRB),
    .wlast              (DMA_M_AXI.WLAST),
    .wvalid             (DMA_M_AXI.WVALID),
    .wready             (DMA_M_AXI.WREADY),
    .bid                (DMA_M_AXI.BID),
    .bresp              (DMA_M_AXI.BRESP),
    .bvalid             (DMA_M_AXI.BVALID),
    .bready             (DMA_M_AXI.BREADY),
    .arid               (DMA_M_AXI.ARID),
    .araddr             (DMA_M_AXI.ARADDR),
    .arlen              (DMA_M_AXI.ARLEN),
    .arsize             (DMA_M_AXI.ARSIZE),
    .arburst            (DMA_M_AXI.ARBURST),
    .arlock             (DMA_M_AXI.ARLOCK),
    .arcache            (DMA_M_AXI.ARCACHE),
    .arprot             (DMA_M_AXI.ARPROT),
    .arvalid            (DMA_M_AXI.ARVALID),
    .arready            (DMA_M_AXI.ARREADY),
    .rid                (DMA_M_AXI.RID),
    .rdata              (DMA_M_AXI.RDATA),
    .rresp              (DMA_M_AXI.RRESP),
    .rlast              (DMA_M_AXI.RLAST),
    .rvalid             (DMA_M_AXI.RVALID),
    .rready             (DMA_M_AXI.RREADY),

    .irq_channel        (DMA_IRQ_CH),
    .irq_comb_nonsec    (DMA_IRQ_COMB)
);
`else
// No DMA-350: DMA_M makes no requests and is always ready to take a response
assign DMA_M_AXI.AWID    = 2'b0;
assign DMA_M_AXI.AWADDR  = 44'h0;
assign DMA_M_AXI.AWLEN   = 8'h0;
assign DMA_M_AXI.AWSIZE  = 3'b011;
assign DMA_M_AXI.AWBURST = 2'b01;
assign DMA_M_AXI.AWLOCK  = 1'b0;
assign DMA_M_AXI.AWCACHE = 4'b0000;
assign DMA_M_AXI.AWPROT  = 3'b000;
assign DMA_M_AXI.AWVALID = 1'b0;
assign DMA_M_AXI.WDATA   = 64'h0;
assign DMA_M_AXI.WSTRB   = 8'h0;
assign DMA_M_AXI.WLAST   = 1'b0;
assign DMA_M_AXI.WVALID  = 1'b0;
assign DMA_M_AXI.BREADY  = 1'b1;
assign DMA_M_AXI.ARID    = 2'b0;
assign DMA_M_AXI.ARADDR  = 44'h0;
assign DMA_M_AXI.ARLEN   = 8'h0;
assign DMA_M_AXI.ARSIZE  = 3'b011;
assign DMA_M_AXI.ARBURST = 2'b01;
assign DMA_M_AXI.ARLOCK  = 1'b0;
assign DMA_M_AXI.ARCACHE = 4'b0000;
assign DMA_M_AXI.ARPROT  = 3'b000;
assign DMA_M_AXI.ARVALID = 1'b0;
assign DMA_M_AXI.RREADY  = 1'b1;

// DMA_S: complete every access at once with read data 0 and no error, so a
// CPU access to the window cannot hang the bus
assign DMA_S_APB.prdata  = 32'h0;
assign DMA_S_APB.pready  = 1'b1;
assign DMA_S_APB.pslverr = 1'b0;

assign DMA_IRQ_CH        = 4'h0;
assign DMA_IRQ_COMB      = 1'b0;
`endif


wire                CPU_nPRESETDBG;
wire                CPU_PCLKENDBG;


assign CPU_nPRESETDBG = SYS_RESETn_int;
assign CPU_PCLKENDBG = 1'b1;

wire [(NUM_SPIS-1):0]   CPU_IRQS;
wire                    QSPI_IRQ;
wire [71:0]             PERI_IRQS;
wire                    SDIO_IRQ;
wire [7:0]              DRAM_IRQS;      // LPDDR4 subsystem, CPU_IRQS[94:87]; 0 without INC_LPDDR4

// CPU_IRQS[101] DMA_IRQ_COMB, [100:97] DMA_IRQ_CH[3:0] (0 without INC_DMA),
// [96] ETH_CKSUM_IRQ, [95] ETH_IRQ (0 without INC_ETH). GIC interrupt ID =
// CPU_IRQS bit + 32 (DMA: 129-133). Every SPI passes through megasoc_irq_sync
// in megasoc_cpu_ss; the ETH and DMA interrupts are already on SYS_CLK.
assign CPU_IRQS={{(NUM_SPIS-102){1'b0}}, DMA_IRQ_COMB, DMA_IRQ_CH, ETH_CKSUM_IRQ, ETH_IRQ, DRAM_IRQS, SDIO_IRQ ,EXP_IRQs, QSPI_IRQ, PERI_IRQS, 1'b0, 4'h0};

// Subordinate AWAKEUP signal generation
assign AWAKEUP_ROM = (ROM_AXI.AWVALID | ROM_AXI.ARVALID | ROM_AXI.WVALID);
assign AWAKEUP_SRAM = (SRAM_AXI.AWVALID | SRAM_AXI.ARVALID | SRAM_AXI.WVALID);


megasoc_cpu_ss #(
    .NUM_GICRID_BITS(GIC_ID_W),
    .NUM_GICWID_BITS(GIC_ID_W),
    .NUM_SPIS(NUM_SPIS)
    ) u_megasoc_cpu_ss(
    // Clocks and Reset
    .CPU_CLK(SYS_CLK),
    .RESETn(SYS_RESETn_int),
    .nPRESETDBG(CPU_nPRESETDBG),
    .CPU_CORE_PORESETn(CPU_CORE_PORESETn),
    .CPU_CORE_WRMRSTn(CPU_CORE_WRMRSTn_final),
    .CPU_L2_RESETn(CPU_L2_RESETn_final),

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


//--------------------------------------
//  DRAM
//--------------------------------------
`ifdef INC_LPDDR4
// LPDDR4 controller + PHY (megasoc_tech/logical/lpddr4_subsystem). The
// subsystem runs entirely on DDR_CLK (core, AXI and APB clocks tied together,
// as in its standalone verification); DRAM_AXI crosses from SYS_CLK through
// an async bridge. The controller stays in reset until firmware releases it
// through the subsystem's reset-control registers.

// DDR_CLK-domain reset: same source as the NIC-400 (SYS_RESETn_int),
// asserted asynchronously, released synchronously to DDR_CLK.
reg [1:0] ddr_resetn_sync;
wire      DDR_RESETn = ddr_resetn_sync[1];

always @ (posedge DDR_CLK or negedge SYS_RESETn_int)
  begin
    if (!SYS_RESETn_int)
      ddr_resetn_sync <= 2'b00;
    else
      ddr_resetn_sync <= {ddr_resetn_sync[0], 1'b1};
  end

axi4 #(.DATA_W(64), .ID_W(ID_W), .ADDR_W(33))   LPDDR4_AXI();

megasoc_axi_async_bridge #(
    .DATA_W(64),
    .ID_W(ID_W),
    .ADDR_W(33),
    .DEPTH(4)
    ) u_dram_axi_async_bridge(
    .s_aclk(SYS_CLK),
    .s_aresetn(SYS_RESETn_int),
    .S(DRAM_AXI),

    .m_aclk(DDR_CLK),
    .m_aresetn(DDR_RESETn),
    .M(LPDDR4_AXI)
);

// Configuration registers: peripheral AHB window 0x4100_0000-0x41FF_FFFF
// (SYS_CLK) -> APB on DDR_CLK.
//   0x4100_0000 + R      : controller / reset-control register R (64 KB)
//   0x4140_0000 + (N<<2) : PHY register N (4 MB)
apb3      LPDDR4_CFG_APB();
apb4      LPDDR4_PHY_CFG_APB();

// The bridge's two halves must reset together, so its DDR_CLK half takes the
// peripheral AHB reset (SYS_HRESETn), synchronised to DDR_CLK.
reg [1:0] ddr_cfg_resetn_sync;
wire      DDR_CFG_RESETn = ddr_cfg_resetn_sync[1];

always @ (posedge DDR_CLK or negedge SYS_HRESETn)
  begin
    if (!SYS_HRESETn)
      ddr_cfg_resetn_sync <= 2'b00;
    else
      ddr_cfg_resetn_sync <= {ddr_cfg_resetn_sync[0], 1'b1};
  end

megasoc_ahb_to_apb_async u_lpddr4_cfg_bridge(
    .HCLK(SYS_CLK),
    .HRESETn(SYS_HRESETn),
    .AHB(LPDDR4_CFG_AHB),

    .PCLK(DDR_CLK),
    .PRESETn(DDR_CFG_RESETn),
    .CTRL_APB(LPDDR4_CFG_APB),
    .PHY_APB(LPDDR4_PHY_CFG_APB)
);

// Low-power Q-channels from the DRAM power domain (megasoc_power_control,
// SYS_CLK) to the subsystem's two low-power interfaces (DDR_CLK). The adapters
// turn a refused request into a proper Q-channel denial and synchronise the
// request into DDR_CLK (see megasoc_lpi_qchannel_adapter.sv).
qchannel  LPDDR4_SYS_QCHAN();
qchannel  LPDDR4_DDRC_QCHAN();

megasoc_lpi_qchannel_adapter u_lpddr4_sys_qadapter(
    .clk(DDR_CLK),
    .resetn(DDR_RESETn),
    .CTRL(DRAM_qchan_sys),
    .DEV(LPDDR4_SYS_QCHAN)
);

megasoc_lpi_qchannel_adapter u_lpddr4_ddrc_qadapter(
    .clk(DDR_CLK),
    .resetn(DDR_RESETn),
    .CTRL(DRAM_qchan_ddrc),
    .DEV(LPDDR4_DDRC_QCHAN)
);

dram_wrapper #(.ID_W(ID_W)) u_lpddr4_subsystem(
    .ACLK(DDR_CLK),
    .ARESETn(DDR_RESETn),

    .PCLK(DDR_CLK),
    .PRESETn(DDR_RESETn),

    .DRAM_AXI(LPDDR4_AXI),
    .DRAM_AXI_AWQOS(4'h0),
    .DRAM_AXI_ARQOS(4'h0),
    .DRAM_CFG_APB(LPDDR4_CFG_APB),
    .DRAM_PHY_CFG_APB(LPDDR4_PHY_CFG_APB),

    .DRAM_SYS_Qchannel(LPDDR4_SYS_QCHAN),
    .DRAM_DDRC_Qchannel(LPDDR4_DDRC_QCHAN),

    // CPU_IRQS[94:87]
    .PHY_IRQ(DRAM_IRQS[0]),
    .ecc_corrected_err_intr(DRAM_IRQS[1]),
    .ecc_corrected_err_intr_fault(DRAM_IRQS[2]),
    .ecc_uncorrected_err_intr(DRAM_IRQS[3]),
    .ecc_uncorrected_err_intr_fault(DRAM_IRQS[4]),
    .dfi_alert_err_intr(DRAM_IRQS[5]),
    .derate_temp_limit_intr(DRAM_IRQS[6]),
    .derate_temp_limit_intr_fault(DRAM_IRQS[7]),

    // The subsystem names its LPDDR4 pins DDR4_*
    .DDR4_RESET_N(LPDDR4_RESET_N),
    .DDR4_CK_T(LPDDR4_CK_T),
    .DDR4_CK_C(LPDDR4_CK_C),
    .DDR4_CKE(LPDDR4_CKE),
    .DDR4_CS(LPDDR4_CS),
    .DDR4_ADR(LPDDR4_CA),
    .DDR4_ODT(LPDDR4_ODT),
    .DDR4_DQS_T(LPDDR4_DQS_T),
    .DDR4_DQS_C(LPDDR4_DQS_C),
    .DDR4_DQ(LPDDR4_DQ),
    .DDR4_DM_DBI_N(LPDDR4_DMI),
    .DDR4_ALERT_N(LPDDR4_ALERT_N),
    .DDR4_VREF(LPDDR4_VREF),
    .DDR4_ZN_SENSE(LPDDR4_ZN_SENSE),
    .DDR4_ZN(LPDDR4_ZN)
);

`else
// Fast simulation DRAM model: protocol-level AXI responder (no DDR timing).
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

// DRAM power domain without LPDDR4: every request is accepted at once (no
// device behind it), so the PPU behaves the same in both builds
assign DRAM_qchan_sys.qacceptn  = DRAM_qchan_sys.qreqn;
assign DRAM_qchan_sys.qdeny     = 1'b0;
assign DRAM_qchan_sys.qactive   = 1'b0;
assign DRAM_qchan_ddrc.qacceptn = DRAM_qchan_ddrc.qreqn;
assign DRAM_qchan_ddrc.qdeny    = 1'b0;
assign DRAM_qchan_ddrc.qactive  = 1'b0;

// LPDDR4 configuration window not decoded: subordinate side idle
assign LPDDR4_CFG_AHB.HREADYOUT = 1'b1;
assign LPDDR4_CFG_AHB.HRESP     = 1'b0;
assign LPDDR4_CFG_AHB.HRDATA    = 32'd0;

// LPDDR4 pins idle
assign DRAM_IRQS      = 8'd0;
assign LPDDR4_RESET_N = 1'b0;
assign LPDDR4_CK_T    = 1'b0;
assign LPDDR4_CK_C    = 1'b1;
assign LPDDR4_CKE     = 2'b00;
assign LPDDR4_CS      = 2'b00;
assign LPDDR4_CA      = 6'd0;
assign LPDDR4_ODT     = 1'b0;
assign LPDDR4_ZN      = 1'b0;
`endif

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
    .HRESETn(SYS_RESETn),  //PORESETn, External flash will not reset on sw reset
    .PCLK(SYS_CLK),
    .PRESETn(SYS_RESETn),  //PORESETn

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

    .IRQ_QSPI_FINISHED(QSPI_IRQ),
    .DBG_XIP_ACTIVE(),   // debug only, unused in MegaSoC
    .DBG_SOFT_RESET()    // debug only, unused in MegaSoC
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
    .PRESETn(SYS_PRESETn),
    .HCLK(SYS_CLK),
    .HRESETn(SYS_HRESETn),
    .RT_CLK(RT_CLK),

    .FCLK(SYS_CLK),      //Make sure SYS_CLK is not gated
    .PORESETn(SYS_RESETn),  //here SYS_RESETn is PORESETn
    .DBGRESETREQ(1'b0),

    .rst_ahb_ctrl_n_o(rst_ahb_ctrl_n_o_from_pss),
    .rst_apb_ctrl_n_o(rst_apb_ctrl_n_o_from_pss),
    .rst_dbg_ctrl_n_o(rst_dbg_ctrl_n_o_from_pss),
    .sys_reset_req_o(sys_reset_req_from_pss),

    // ADP - AHB manager
    .ADP_AHB(ADP_AHB),
    // Peripheral AHB Subordinate
    .PERIPH_AHB(PERIPH_AHB),
    // LPDDR4 configuration window (INC_LPDDR4)
    .LPDDR4_CFG_AHB(LPDDR4_CFG_AHB),

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
    .WARM_RESETn(SYS_RESETn_int),   // ROM/SRAM PPUs also reset on SW/WDOG reset

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
    .CPU_CORE_WRMRSTn(CPU_CORE_WRMRSTn_pc),

    // CPU Advanced SIMD Power management interfaces
    .CPU_NEON_q(CPU_NEON_q),

    // CPU L2 Power management interfaces
    .CPU_L2_q(CPU_L2_q),
    .CPU_L2_RESETn(CPU_L2_RESETn_pc),

    // DRAM (LPDDR4) power domain
    .DRAM_qchan_sys(DRAM_qchan_sys),
    .DRAM_qchan_ddrc(DRAM_qchan_ddrc)
);

assign CPU_CORE_WRMRSTn_final = CPU_CORE_WRMRSTn_pc & SYS_RESETn_int;
assign CPU_L2_RESETn_final = CPU_L2_RESETn_pc & SYS_RESETn_int;

mkaxi2axil_bridge u_axi2axil (
    .CLK(SYS_CLK),
    .RST_N(SYS_RESETn_int),

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
    // Bring-up safe mode: keep 1.8V switching disabled until PHY/regulator
    // status is wired in (do not self-acknowledge voltage switching).
    .OPT_1P8V(1'b0),
    .OPT_CRCTOKEN(1'b1)
    ) u_sdio_controller (
        .i_clk(SYS_CLK),
        .i_reset(~SYS_RESETn_int),
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
        .i_1p8v(1'b0),
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

    .PADDR_ETH_S(ETH_S_APB.paddr),
    .PWDATA_ETH_S(ETH_S_APB.pwdata),
    .PWRITE_ETH_S(ETH_S_APB.pwrite),
    .PPROT_ETH_S(ETH_S_APB.pprot),
    .PSTRB_ETH_S(ETH_S_APB.pstrb),
    .PENABLE_ETH_S(ETH_S_APB.penable),
    .PSELx_ETH_S(ETH_S_APB.psel),
    .PRDATA_ETH_S(ETH_S_APB.prdata),
    .PSLVERR_ETH_S(ETH_S_APB.pslverr),
    .PREADY_ETH_S(ETH_S_APB.pready),

    .PADDR_DMA_S(DMA_S_APB.paddr),
    .PWDATA_DMA_S(DMA_S_APB.pwdata),
    .PWRITE_DMA_S(DMA_S_APB.pwrite),
    .PPROT_DMA_S(DMA_S_APB.pprot),
    .PSTRB_DMA_S(DMA_S_APB.pstrb),
    .PENABLE_DMA_S(DMA_S_APB.penable),
    .PSELx_DMA_S(DMA_S_APB.psel),
    .PRDATA_DMA_S(DMA_S_APB.prdata),
    .PSLVERR_DMA_S(DMA_S_APB.pslverr),
    .PREADY_DMA_S(DMA_S_APB.pready),

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

    .HADDR_ETH_M(ETH_M_AHB.HADDR),
    .HTRANS_ETH_M(ETH_M_AHB.HTRANS),
    .HWRITE_ETH_M(ETH_M_AHB.HWRITE),
    .HSIZE_ETH_M(ETH_M_AHB.HSIZE),
    .HBURST_ETH_M(ETH_M_AHB.HBURST),
    .HPROT_ETH_M(ETH_M_AHB.HPROT),
    .HWDATA_ETH_M(ETH_M_AHB.HWDATA),
    .HRDATA_ETH_M(ETH_M_AHB.HRDATA),
    .HREADY_ETH_M(ETH_M_AHB.HREADY),
    .HRESP_ETH_M(ETH_M_AHB.HRESP),

    .AWID_DMA_M(DMA_M_AXI.AWID),
    .AWADDR_DMA_M(DMA_M_AXI.AWADDR),
    .AWLEN_DMA_M(DMA_M_AXI.AWLEN),
    .AWSIZE_DMA_M(DMA_M_AXI.AWSIZE),
    .AWBURST_DMA_M(DMA_M_AXI.AWBURST),
    .AWLOCK_DMA_M(DMA_M_AXI.AWLOCK),
    .AWCACHE_DMA_M(DMA_M_AXI.AWCACHE),
    .AWPROT_DMA_M(DMA_M_AXI.AWPROT),
    .AWVALID_DMA_M(DMA_M_AXI.AWVALID),
    .AWREADY_DMA_M(DMA_M_AXI.AWREADY),
    .WDATA_DMA_M(DMA_M_AXI.WDATA),
    .WSTRB_DMA_M(DMA_M_AXI.WSTRB),
    .WLAST_DMA_M(DMA_M_AXI.WLAST),
    .WVALID_DMA_M(DMA_M_AXI.WVALID),
    .WREADY_DMA_M(DMA_M_AXI.WREADY),
    .BID_DMA_M(DMA_M_AXI.BID),
    .BRESP_DMA_M(DMA_M_AXI.BRESP),
    .BVALID_DMA_M(DMA_M_AXI.BVALID),
    .BREADY_DMA_M(DMA_M_AXI.BREADY),
    .ARID_DMA_M(DMA_M_AXI.ARID),
    .ARADDR_DMA_M(DMA_M_AXI.ARADDR),
    .ARLEN_DMA_M(DMA_M_AXI.ARLEN),
    .ARSIZE_DMA_M(DMA_M_AXI.ARSIZE),
    .ARBURST_DMA_M(DMA_M_AXI.ARBURST),
    .ARLOCK_DMA_M(DMA_M_AXI.ARLOCK),
    .ARCACHE_DMA_M(DMA_M_AXI.ARCACHE),
    .ARPROT_DMA_M(DMA_M_AXI.ARPROT),
    .ARVALID_DMA_M(DMA_M_AXI.ARVALID),
    .ARREADY_DMA_M(DMA_M_AXI.ARREADY),
    .RID_DMA_M(DMA_M_AXI.RID),
    .RDATA_DMA_M(DMA_M_AXI.RDATA),
    .RRESP_DMA_M(DMA_M_AXI.RRESP),
    .RLAST_DMA_M(DMA_M_AXI.RLAST),
    .RVALID_DMA_M(DMA_M_AXI.RVALID),
    .RREADY_DMA_M(DMA_M_AXI.RREADY),

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
    .clk0resetn(SYS_RESETn_int)    //if SDIO is treated like a peripheral change to SYS_PRESETn.
);


endmodule
