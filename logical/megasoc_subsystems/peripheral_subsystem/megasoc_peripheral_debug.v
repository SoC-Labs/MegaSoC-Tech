
module megasoc_peripheral_debug #(
    parameter FT1248_WIDTH=1
) (
    input  wire                     HCLK,
    input  wire                     HRESETn,
    output wire  [31:0]             HADDR_ADP,
    output wire  [1:0]              HTRANS_ADP,
    output wire                     HWRITE_ADP,
    output wire  [2:0]              HSIZE_ADP,
    output wire  [2:0]              HBURST_ADP,
    output wire  [3:0]              HPROT_ADP,
    output wire  [31:0]             HWDATA_ADP,
    input  wire [31:0]              HRDATA_ADP,
    input  wire                     HREADY_ADP,
    input  wire                     HRESP_ADP,

    input  wire                     PCLK,
    input  wire                     PRESETn,

    input  wire                     USRT0_PSEL,
    output wire [31:0]              USRT0_PRDATA,
    output wire                     USRT0_PREADY,
    output wire                     USRT0_PSLVERR,

    input  wire                     USRT1_PSEL,
    output wire [31:0]              USRT1_PRDATA,
    output wire                     USRT1_PREADY,
    output wire                     USRT1_PSLVERR,

    input  wire [11:0]              USRT_PADDRm,
    input  wire                     USRT_PENABLE,
    input  wire                     USRT_PWRITE,
    input  wire [31:0]              USRT_PWDATA,

    input  wire [3:0]               iodata4_i,
    output wire [3:0]               iodata4_o,
    output wire [3:0]               iodata4_e,
    output wire [3:0]               iodata4_t,
    output wire                     ioreq1_o,
    output wire                     ioreq2_o,
    input  wire                     ioack_i,

    output wire                     usrt0_txint,
    output wire                     usrt0_rxint,
    output wire                     usrt0_txovrint,
    output wire                     usrt0_rxovrint,
    output wire                     usrt0_combined_int,

    output wire                     usrt1_txint,
    output wire                     usrt1_rxint,
    output wire                     usrt1_txovrint,
    output wire                     usrt1_rxovrint,
    output wire                     usrt1_combined_int
);


 // STDIN to ADP controller
wire                     STD_RXD_TVALID;
wire             [ 7:0]  STD_RXD_TDATA;
wire                     STD_RXD_TREADY;
// STDOUT to ADP controller
wire                     STD_TXD_TVALID;
wire             [ 7:0]  STD_TXD_TDATA;
wire                     STD_TXD_TREADY;

 // DATIN to ADP controller
wire                     DAT_RXD_TVALID;
wire             [ 7:0]  DAT_RXD_TDATA;
wire                     DAT_RXD_TREADY;
// DATOUT to ADP controller
wire                     DAT_TXD_TVALID;
wire             [ 7:0]  DAT_TXD_TDATA;
wire                     DAT_TXD_TREADY;

wire                     ADP_RXD_TVALID;
wire            [ 7:0]   ADP_RXD_TDATA ;
wire                     ADP_RXD_TREADY;
wire                     ADP_TXD_TVALID;
wire             [ 7:0]  ADP_TXD_TDATA ;
wire                     ADP_TXD_TREADY;

wire [7:0]  GPIO;
socdebug_ahb u_socdebug_ahb(
    .HCLK(HCLK),
    .HRESETn(HRESETn),
    .HADDR32_o(HADDR_ADP),
    .HBURST3_o(HBURST_ADP),
    .HMASTLOCK_o(),
    .HPROT4_o(HPROT_ADP),
    .HSIZE3_o(HSIZE_ADP),
    .HTRANS2_o(HTRANS_ADP),
    .HWDATA32_o(HWDATA_ADP),
    .HWRITE_o(HWRITE_ADP),
    .HRDATA32_i(HRDATA_ADP),
    .HREADY_i(HREADY_ADP),
    .HRESP_i(HRESP_ADP),

    .ADP_RXD_TVALID_o(ADP_RXD_TVALID),
    .ADP_RXD_TDATA_o(ADP_RXD_TDATA),
    .ADP_RXD_TREADY_i(ADP_RXD_TREADY),

    .ADP_TXD_TVALID_i(ADP_TXD_TVALID),
    .ADP_TXD_TDATA_i(ADP_TXD_TDATA),
    .ADP_TXD_TREADY_o(ADP_TXD_TREADY),

    .STD_RXD_TVALID_o(STD_RXD_TVALID),
    .STD_RXD_TDATA_o(STD_RXD_TDATA),
    .STD_RXD_TREADY_i(STD_RXD_TREADY),

    .STD_TXD_TVALID_i(STD_TXD_TVALID),
    .STD_TXD_TDATA_i(STD_TXD_TDATA),
    .STD_TXD_TREADY_o(STD_TXD_TREADY),

    .GPO8_o(GPIO),
    .GPI8_i(GPIO)
);

// Instantiation of USRT Controller
socdebug_usrt_control u_usrt0_control (
    // APB Clock and Reset Signals
    .PCLK              (PCLK),
    .PCLKG             (PCLK),    // Gated PCLK for bus
    .PRESETn           (PRESETn),

    // APB Interface Signals
    .PSEL              (USRT0_PSEL),
    .PADDR             (USRT_PADDRm[11:2]),
    .PENABLE           (USRT_PENABLE),
    .PWRITE            (USRT_PWRITE),
    .PWDATA            (USRT_PWDATA),
    .PRDATA            (USRT0_PRDATA),
    .PREADY            (USRT0_PREADY),
    .PSLVERR           (USRT0_PSLVERR),

    .ECOREVNUM         (4'h0),

    // ADP Interface - From USRT to ADP
    .TX_VALID_o        (STD_TXD_TVALID),
    .TX_DATA8_o        (STD_TXD_TDATA ),
    .TX_READY_i        (STD_TXD_TREADY),

    // ADP Interface - From ADP to USRT
    .RX_VALID_i        (STD_RXD_TVALID),
    .RX_DATA8_i        (STD_RXD_TDATA ),
    .RX_READY_o        (STD_RXD_TREADY),

    // Interrupt Interfaces
    .TXINT             (usrt0_txint ),       // Transmit Interrupt
    .RXINT             (usrt0_rxint ),       // Receive  Interrupt
    .TXOVRINT          (usrt0_txovrint ),       // Transmit Overrun Interrupt
    .RXOVRINT          (usrt0_rxovrint ),       // Receive  Overrun Interrupt
    .UARTINT           (usrt0_combined_int )        // Combined Interrupt
);

socdebug_usrt_control u_usrt1_control (
    // APB Clock and Reset Signals
    .PCLK              (PCLK),
    .PCLKG             (PCLK),    // Gated PCLK for bus
    .PRESETn           (PRESETn),

    // APB Interface Signals
    .PSEL              (USRT1_PSEL),
    .PADDR             (USRT_PADDRm[11:2]),
    .PENABLE           (USRT_PENABLE),
    .PWRITE            (USRT_PWRITE),
    .PWDATA            (USRT_PWDATA),
    .PRDATA            (USRT1_PRDATA),
    .PREADY            (USRT1_PREADY),
    .PSLVERR           (USRT1_PSLVERR),

    .ECOREVNUM         (4'h0),

    // ADP Interface - From USRT to ADP
    .TX_VALID_o        (DAT_TXD_TVALID),
    .TX_DATA8_o        (DAT_TXD_TDATA ),
    .TX_READY_i        (DAT_TXD_TREADY),

    // ADP Interface - From ADP to USRT
    .RX_VALID_i        (DAT_RXD_TVALID),
    .RX_DATA8_i        (DAT_RXD_TDATA ),
    .RX_READY_o        (DAT_RXD_TREADY),

    // Interrupt Interfaces
    .TXINT             (usrt1_txint ),       // Transmit Interrupt
    .RXINT             (usrt1_rxint ),       // Receive  Interrupt
    .TXOVRINT          (usrt1_txovrint ),       // Transmit Overrun Interrupt
    .RXOVRINT          (usrt1_rxovrint ),       // Receive  Overrun Interrupt
    .UARTINT           (usrt1_combined_int )        // Combined Interrupt
);





extio8x4_axis_initiator u_extio8x4_axis_initiator(
  .clk             ( HCLK          ),
  .resetn          ( HRESETn       ),
  .testmode        ( 1'b0      ),
// RX 4-channel AXIS interface
  .axis_rx0_tvalid ( ADP_RXD_TVALID ),
  .axis_rx0_tdata8 ( ADP_RXD_TDATA  ),
  .axis_rx0_tready ( ADP_RXD_TREADY ),

  .axis_rx1_tvalid ( DAT_TXD_TVALID ),
  .axis_rx1_tdata8 ( DAT_TXD_TDATA  ),
  .axis_rx1_tready ( DAT_TXD_TREADY ),

  .axis_tx0_tvalid ( ADP_TXD_TVALID ),
  .axis_tx0_tdata8 ( ADP_TXD_TDATA  ),
  .axis_tx0_tready ( ADP_TXD_TREADY ),

  .axis_tx1_tvalid ( DAT_RXD_TVALID ),
  .axis_tx1_tdata8 ( DAT_RXD_TDATA  ),
  .axis_tx1_tready ( DAT_RXD_TREADY ),
// external io interface
  .iodata4_a       ( iodata4_i       ),
  .iodata4_o       ( iodata4_o       ),
  .iodata4_e       ( iodata4_e       ),
  .iodata4_t       ( iodata4_t       ),
  .ioreq1_o        ( ioreq1_o        ),
  .ioreq2_o        ( ioreq2_o        ),
  .ioack_a         ( ioack_i         )

);


endmodule