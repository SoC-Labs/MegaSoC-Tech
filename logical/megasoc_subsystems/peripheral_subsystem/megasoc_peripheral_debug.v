
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
    input  wire                     USRT_PSEL,
    input  wire [11:0]              USRT_PADDRm,
    input  wire                     USRT_PENABLE,
    input  wire                     USRT_PWRITE,
    input  wire [31:0]              USRT_PWDATA,
    output wire [31:0]              USRT_PRDATA,
    output wire                     USRT_PREADY,
    output wire                     USRT_PSLVERR,

    output wire                     FT_CLK_O,    // SCLK
    output wire                     FT_SSN_O,    // SS_N
    input  wire                     FT_MISO_I,   // MISO
    output wire  [FT1248_WIDTH-1:0] FT_MIOSIO_O, // MIOSIO tristate output when enabled
    output wire  [FT1248_WIDTH-1:0] FT_MIOSIO_E, // MIOSIO tristate output enable (active hi)
    output wire  [FT1248_WIDTH-1:0] FT_MIOSIO_Z, // MIOSIO tristate output enable (active lo)
    input  wire  [FT1248_WIDTH-1:0] FT_MIOSIO_I // MIOSIO tristate input

);


 // STDIN to ADP controller
wire                     STD_RXD_TVALID;
wire             [ 7:0]  STD_RXD_TDATA;
wire                     STD_RXD_TREADY;
// STDOUT to ADP controller
wire                     STD_TXD_TVALID;
wire             [ 7:0]  STD_TXD_TDATA;
wire                     STD_TXD_TREADY;

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
socdebug_usrt_control u_usrt_control (
    // APB Clock and Reset Signals
    .PCLK              (PCLK),
    .PCLKG             (PCLK),    // Gated PCLK for bus
    .PRESETn           (PRESETn),

    // APB Interface Signals
    .PSEL              (USRT_PSEL),
    .PADDR             (USRT_PADDRm[11:2]),
    .PENABLE           (USRT_PENABLE),
    .PWRITE            (USRT_PWRITE),
    .PWDATA            (USRT_PWDATA),
    .PRDATA            (USRT_PRDATA),
    .PREADY            (USRT_PREADY),
    .PSLVERR           (USRT_PSLVERR),

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
    .TXINT             ( ),       // Transmit Interrupt
    .RXINT             ( ),       // Receive  Interrupt
    .TXOVRINT          ( ),       // Transmit Overrun Interrupt
    .RXOVRINT          ( ),       // Receive  Overrun Interrupt
    .UARTINT           ( )        // Combined Interrupt
);

// Instantiation of FT1248 Controller
socdebug_ft1248_control #(
    .FT1248_WIDTH (FT1248_WIDTH),
    .FT1248_CLKON (1)
) u_ft1248_control (
    .clk              (HCLK),
    .resetn           (HRESETn),
    .ft_clkdiv        (8'd15),
    .ft_clk_o         (FT_CLK_O),
    .ft_ssn_o         (FT_SSN_O),
    .ft_miso_i        (FT_MISO_I),
    .ft_miosio_o      (FT_MIOSIO_O),
    .ft_miosio_e      (FT_MIOSIO_E),
    .ft_miosio_z      (FT_MIOSIO_Z),
    .ft_miosio_i      (FT_MIOSIO_I),

    // ADP Interface - FT1248 to ADP
    .txd_tvalid       (ADP_TXD_TVALID),
    .txd_tdata        (ADP_TXD_TDATA ),
    .txd_tready       (ADP_TXD_TREADY),
    .txd_tlast        ( ),

    // ADP Interface - FT_ADP to FT1248
    .rxd_tvalid       (ADP_RXD_TVALID),
    .rxd_tdata        (ADP_RXD_TDATA ),
    .rxd_tready       (ADP_RXD_TREADY),
    .rxd_tlast        (1'b0)
);

endmodule