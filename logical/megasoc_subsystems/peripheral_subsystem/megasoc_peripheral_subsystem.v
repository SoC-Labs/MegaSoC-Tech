//-----------------------------------------------------------------------------
// MegaSoC Peripheral Subsystem
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Contributors
//
// Daniel Newbrook (d.newbrook@soton.ac.uk)
// 
// Copyright � 2021-4, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Purpose:
//  APB peripheral subsystem for SoC. Takes an APB input (subordinate port) and 
//  muxes to several peripherals including UART and timers
//-----------------------------------------------------------------------------
// Modules instantiated:
//  cmsdk_apb_slave_mux (u_apb_slave_mux)
//  cmsdk_apb_uart      (u_apb_uart_0)
//  cmsdk_apb_timer     (u_apb_timer0)
//  megasoc_peripheral_debug (u_megasoc_peripheral_debug)

module megasoc_peripheral_subsystem #(
    parameter BE = 0 )(
    input  wire         PCLK,
    input  wire         PRESETn,
    input  wire         HCLK,
    input  wire         HRESETn,
    input  wire         RT_CLK, // 32kHz real time clock


    // ADP AHB bus interface
    output wire  [31:0] HADDR_ADP,
    output wire  [1:0]  HTRANS_ADP,
    output wire         HWRITE_ADP,
    output wire  [2:0]  HSIZE_ADP,
    output wire  [2:0]  HBURST_ADP,
    output wire  [3:0]  HPROT_ADP,
    output wire  [31:0] HWDATA_ADP,
    input  wire [31:0]  HRDATA_ADP,
    input  wire         HREADY_ADP,
    input  wire         HRESP_ADP,

    // Peripheral AHB bus interface
    input  wire         HSEL,
    input  wire  [31:0] HADDR,
    input  wire  [1:0]  HTRANS,
    input  wire         HWRITE,
    input  wire  [2:0]  HSIZE,
    input  wire  [2:0]  HBURST,
    input  wire  [3:0]  HPROT,
    input  wire  [31:0] HWDATA,
    input  wire         HREADY,
    output wire [31:0]  HRDATA,
    output wire         HREADYOUT,
    output wire         HRESP,

    input  wire         UARTRXD,
    output wire         UARTTXD,
    output wire         UARTTXEN,   

    output wire         FT_CLK_O,    // SCLK
    output wire         FT_SSN_O,    // SS_N
    input  wire         FT_MISO_I,   // MISO
    output wire         FT_MIOSIO_O, // MIOSIO tristate output when enabled
    output wire         FT_MIOSIO_E, // MIOSIO tristate output enable (active hi)
    output wire         FT_MIOSIO_Z, // MIOSIO tristate output enable (active lo)
    input  wire         FT_MIOSIO_I, // MIOSIO tristate input

    input  wire [15:0]  p0_in,
    output wire [15:0]  p0_out,
    output wire [15:0]  p0_en,
    output wire [15:0]  p0_func,

    input  wire [15:0]  p1_in,
    output wire [15:0]  p1_out,
    output wire [15:0]  p1_en,
    output wire [15:0]  p1_func,

    output wire [39:0]   PERI_IRQS   // Peripheral interrupts to GIC
);

// APB bus interface
wire [15:0] PADDR;
wire        PENABLE;  
wire        PWRITE;   
wire [31:0] PWDATA;   
wire        PSEL;     
wire [31:0] PRDATA;   
wire        PREADY;   
wire        PSLVERR;  
wire [3:0]  PSTRB;
wire [2:0]  PPROT;


wire        UARTCLK;
assign      UARTCLK=PCLK; // TODO generate UARTCLK from elsewhere

// AHB internal wires 
wire        defslv_hsel;   // AHB default slave signals
wire        defslv_hreadyout;
wire [31:0] defslv_hrdata;
wire        defslv_hresp;

wire        apbsys_hsel;  // APB subsystem AHB interface signals
wire        apbsys_hreadyout;
wire [31:0] apbsys_hrdata;
wire        apbsys_hresp;

wire        gpio0_hsel;   // AHB GPIO bus interface signals
wire        gpio0_hreadyout;
wire [31:0] gpio0_hrdata;
wire        gpio0_hresp;

wire        gpio1_hsel;   // AHB GPIO bus interface signals
wire        gpio1_hreadyout;
wire [31:0] gpio1_hrdata;
wire        gpio1_hresp;


// Internal APB signals for UART0
wire        PSEL_UART0;
wire        PREADY_UART0;
wire [31:0] PRDATA_UART0;
wire        PSLVERR_UART0;

// Internal APB signals for Timer0
wire        PSEL_TIMER0;
wire        PREADY_TIMER0;
wire [31:0] PRDATA_TIMER0;
wire        PSLVERR_TIMER0;

// Internal APB signals for Timer0
wire        PSEL_USRT;
wire        PREADY_USRT;
wire [31:0] PRDATA_USRT;
wire        PSLVERR_USRT;

// Interrupt Signals 
wire [15:0] gpio0_int;
wire        gpio0_comb_int;
wire [15:0] gpio1_int;
wire        gpio1_comb_int;
wire        timer0_int;
wire        uart0_txint;
wire        uart0_rxint;
wire        uart0_txovrint;
wire        uart0_rxovrint;
wire        uart0_combined_int;

assign PERI_IRQS[0] = uart0_txint;
assign PERI_IRQS[1] = uart0_rxint;
assign PERI_IRQS[2] = uart0_txovrint;
assign PERI_IRQS[3] = uart0_rxovrint;
assign PERI_IRQS[4] = uart0_combined_int;
assign PERI_IRQS[5] = timer0_int;
assign PERI_IRQS[6] = gpio0_comb_int;
assign PERI_IRQS[7] = gpio1_comb_int;
assign PERI_IRQS[23:8] = gpio0_int;
assign PERI_IRQS[39:24] = gpio1_int;


megasoc_peripheral_addr_decode #(
    .BASEADDR_APBSS(32'h4000_0000),
    .BASEADDR_GPIO0(32'h4001_0000),
    .BASEADDR_GPIO1(32'h4002_0000)
) u_peripheral_addr_decode (
    .hsel(HSEL),
    .haddr(HADDR),
    .apbsys_hsel(apbsys_hsel),
    .gpio0_hsel(gpio0_hsel),
    .gpio1_hsel(gpio1_hsel),
    .defslv_hsel(defslv_hsel)
);

cmsdk_ahb_slave_mux #(
    .PORT0_ENABLE  (1), // APB subsystem bridge
    .PORT1_ENABLE  (1), // GPIO Port 0
    .PORT2_ENABLE  (1), // GPIO Port 1
    .PORT3_ENABLE  (0), // SYS control
    .PORT4_ENABLE  (0), // Default
    .PORT5_ENABLE  (0), // 
    .PORT6_ENABLE  (0), // 
    .PORT7_ENABLE  (0),
    .PORT8_ENABLE  (0),
    .PORT9_ENABLE  (0),
    .DW            (32)  
) u_ahb_slave_mux_sys_bus (
    .HCLK         (HCLK),
    .HRESETn      (HRESETn),
    .HREADY       (HREADY),
    .HSEL0        (apbsys_hsel),     // Input Port 0
    .HREADYOUT0   (apbsys_hreadyout),
    .HRESP0       (apbsys_hresp),
    .HRDATA0      (apbsys_hrdata),
    .HSEL1        (gpio0_hsel),      // Input Port 1
    .HREADYOUT1   (gpio0_hreadyout),
    .HRESP1       (gpio0_hresp),
    .HRDATA1      (gpio0_hrdata),
    .HSEL2        (gpio1_hsel),      // Input Port 2
    .HREADYOUT2   (gpio1_hreadyout),
    .HRESP2       (gpio1_hresp),
    .HRDATA2      (gpio1_hrdata),
    .HSEL3        (1'b0),    // Input Port 3
    .HREADYOUT3   (defslv_hreadyout),
    .HRESP3       (defslv_hresp),
    .HRDATA3      (defslv_hrdata),
    .HSEL4        (defslv_hsel),     // Input Port 4
    .HREADYOUT4   (defslv_hreadyout),
    .HRESP4       (defslv_hresp),
    .HRDATA4      (defslv_hrdata),
    .HSEL5        (1'b0),     // Input Port 5
    .HREADYOUT5   (defslv_hreadyout),
    .HRESP5       (defslv_hresp),
    .HRDATA5      (defslv_hrdata),
    .HSEL6        (1'b0),     // Input Port 6
    .HREADYOUT6   (defslv_hreadyout),
    .HRESP6       (defslv_hresp),
    .HRDATA6      (defslv_hrdata),
    .HSEL7        (1'b0),     // Input Port 7
    .HREADYOUT7   (defslv_hreadyout),
    .HRESP7       (defslv_hresp),
    .HRDATA7      (defslv_hrdata),
    .HSEL8        (1'b0),     // Input Port 8
    .HREADYOUT8   (defslv_hreadyout),
    .HRESP8       (defslv_hresp),
    .HRDATA8      (defslv_hrdata),
    .HSEL9        (1'b0),     // Input Port 9
    .HREADYOUT9   (defslv_hreadyout),
    .HRESP9       (defslv_hresp),
    .HRDATA9      (defslv_hrdata),

    .HREADYOUT    (HREADYOUT),   // Outputs
    .HRESP        (HRESP),
    .HRDATA       (HRDATA)
);

// Default slave
cmsdk_ahb_default_slave u_ahb_default_slave_1 (
.HCLK         (HCLK),
.HRESETn      (HRESETn),
.HSEL         (defslv_hsel),
.HTRANS       (HTRANS),
.HREADY       (HREADY),
.HREADYOUT    (defslv_hreadyout),
.HRESP        (defslv_hresp)
);
assign   defslv_hrdata = 32'hDEADBEEF; // Default slave do not have read data



cmsdk_ahb_gpio #(
    .ALTERNATE_FUNC_MASK(16'hFFFF),
    .ALTERNATE_FUNC_DEFAULT(16'h0000),
    .BE(0)
) u_cmsdk_ahb_gpio_0 (
    .HCLK(HCLK),        
    .HRESETn(HRESETn),     
    .FCLK(HCLK),        
    .HSEL(gpio0_hsel),        
    .HREADY(HREADY),      
    .HTRANS(HTRANS),      
    .HSIZE(HSIZE),       
    .HWRITE(HWRITE),      
    .HADDR(HADDR[11:0]),       
    .HWDATA(HWDATA),     
    .HREADYOUT(gpio0_hreadyout),       
    .HRESP(gpio0_hresp),       
    .HRDATA(gpio0_hrdata),   

    .ECOREVNUM(4'h0),   

    .PORTIN(p0_in),   
    .PORTOUT(p0_out),     
    .PORTEN(p0_en),      
    .PORTFUNC(p0_func),   

    .GPIOINT(gpio0_int),     
    .COMBINT(gpio0_comb_int)      
);

cmsdk_ahb_gpio #(
    .ALTERNATE_FUNC_MASK(16'hFFFF),
    .ALTERNATE_FUNC_DEFAULT(16'h0000),
    .BE(0)
) u_cmsdk_ahb_gpio_1 (
    .HCLK(HCLK),        
    .HRESETn(HRESETn),     
    .FCLK(HCLK),        
    .HSEL(gpio1_hsel),        
    .HREADY(HREADY),      
    .HTRANS(HTRANS),      
    .HSIZE(HSIZE),       
    .HWRITE(HWRITE),      
    .HADDR(HADDR[11:0]),       
    .HWDATA(HWDATA),     
    .HREADYOUT(gpio1_hreadyout),       
    .HRESP(gpio1_hresp),       
    .HRDATA(gpio1_hrdata),   

    .ECOREVNUM(4'h0),   

    .PORTIN(p1_in),   
    .PORTOUT(p1_out),     
    .PORTEN(p1_en),      
    .PORTFUNC(p1_func),   

    .GPIOINT(gpio1_int),     
    .COMBINT(gpio1_comb_int)      
);
  // endian handling
  wire             bigendian;
  assign           bigendian = (BE!=0) ? 1'b1 : 1'b0;

  wire   [31:0]    hwdata_le; // Little endian write data
  wire   [31:0]    hrdata_le; // Little endian read data
  wire             reg_be_swap_ctrl_en = HSEL & HTRANS[1] & HREADY & bigendian;
  reg     [1:0]    reg_be_swap_ctrl; // registered byte swap control
  wire    [1:0]    nxt_be_swap_ctrl; // next state of byte swap control

  assign nxt_be_swap_ctrl[1] = bigendian & (HSIZE[1:0]==2'b10); // Swap upper and lower half word
  assign nxt_be_swap_ctrl[0] = bigendian & (HSIZE[1:0]!=2'b00); // Swap byte within hafword

  // Register byte swap control for data phase
  always @(posedge HCLK or negedge HRESETn)
    begin
    if (~HRESETn)
      reg_be_swap_ctrl <= 2'b00;
    else if (reg_be_swap_ctrl_en)
      reg_be_swap_ctrl <= nxt_be_swap_ctrl;
    end

  // swap byte within half word
  wire  [31:0] hwdata_mux_1 = (reg_be_swap_ctrl[0] & bigendian) ?
     {HWDATA[23:16],HWDATA[31:24],HWDATA[7:0],HWDATA[15:8]}:
     {HWDATA[31:24],HWDATA[23:16],HWDATA[15:8],HWDATA[7:0]};
  // swap lower and upper half word
  assign       hwdata_le    = (reg_be_swap_ctrl[1] & bigendian) ?
     {hwdata_mux_1[15: 0],hwdata_mux_1[31:16]}:
     {hwdata_mux_1[31:16],hwdata_mux_1[15:0]};
  // swap byte within half word
  wire  [31:0] hrdata_mux_1 = (reg_be_swap_ctrl[0] & bigendian) ?
     {hrdata_le[23:16],hrdata_le[31:24],hrdata_le[ 7:0],hrdata_le[15:8]}:
     {hrdata_le[31:24],hrdata_le[23:16],hrdata_le[15:8],hrdata_le[7:0]};
  // swap lower and upper half word
  assign       apbsys_hrdata       = (reg_be_swap_ctrl[1] & bigendian) ?
     {hrdata_mux_1[15: 0],hrdata_mux_1[31:16]}:
     {hrdata_mux_1[31:16],hrdata_mux_1[15:0]};

// AHB to APB bus bridge
cmsdk_ahb_to_apb #(
    .ADDRWIDTH      (16),
    .REGISTER_RDATA (1),
    .REGISTER_WDATA (0)
)   u_ahb_to_apb (
    // AHB side
    .HCLK     (HCLK),
    .HRESETn  (HRESETn),
    .HSEL     (apbsys_hsel),
    .HADDR    (HADDR[15:0]),
    .HTRANS   (HTRANS),
    .HSIZE    (HSIZE),
    .HPROT    (HPROT),
    .HWRITE   (HWRITE),
    .HREADY   (HREADY),
    .HWDATA   (hwdata_le),

    .HREADYOUT(apbsys_hreadyout), // AHB Outputs
    .HRDATA   (hrdata_le),
    .HRESP    (apb_hres),

    .PADDR    (PADDR[15:0]),
    .PSEL     (PSEL),
    .PENABLE  (PENABLE),
    .PSTRB    (PSTRB),
    .PPROT    (PPROT),
    .PWRITE   (PWRITE),
    .PWDATA   (PWDATA),

    .APBACTIVE(APBACTIVE),
    .PCLKEN   (1'b1),     // APB clock enable signal

    .PRDATA   (PRDATA),
    .PREADY   (PREADY),
    .PSLVERR  (PSLVERR)
);


// CMSDK APB Slave Mux (from Corstone 101) 
cmsdk_apb_slave_mux #(
    .PORT0_ENABLE(1),
    .PORT1_ENABLE(1),
    .PORT2_ENABLE(1),
    .PORT3_ENABLE(0),
    .PORT4_ENABLE(0),
    .PORT5_ENABLE(0),
    .PORT6_ENABLE(0),
    .PORT7_ENABLE(0),
    .PORT8_ENABLE(0),
    .PORT9_ENABLE(0),
    .PORT10_ENABLE(0),
    .PORT11_ENABLE(0),
    .PORT12_ENABLE(0),
    .PORT13_ENABLE(0),
    .PORT14_ENABLE(0),
    .PORT15_ENABLE(0)
    ) u_apb_slave_mux (
    .DECODE4BIT(PADDR[15:12]),
    .PSEL(PSEL),   

    .PSEL0(PSEL_UART0),
    .PREADY0(PREADY_UART0),
    .PRDATA0(PRDATA_UART0),
    .PSLVERR0(PSLVERR_UART0),  

    .PSEL1(PSEL_TIMER0),
    .PREADY1(PREADY_TIMER0),
    .PRDATA1(PRDATA_TIMER0),
    .PSLVERR1(PSLVERR_TIMER0),  

    .PSEL2(PSEL_USRT),
    .PREADY2(PREADY_USRT),
    .PRDATA2(PRDATA_USRT),
    .PSLVERR2(PSLVERR_USRT),   

    .PSEL3(),
    .PREADY3(1'b0),
    .PRDATA3(32'd0),
    .PSLVERR3(1'b0),   

    .PSEL4(),
    .PREADY4(1'b0),
    .PRDATA4(32'd0),
    .PSLVERR4(1'b0),   

    .PSEL5(),
    .PREADY5(1'b0),
    .PRDATA5(32'd0),
    .PSLVERR5(1'b0),   

    .PSEL6(),
    .PREADY6(1'b0),
    .PRDATA6(32'd0),
    .PSLVERR6(1'b0),   

    .PSEL7(),
    .PREADY7(1'b0),
    .PRDATA7(32'd0),
    .PSLVERR7(1'b0),   

    .PSEL8(),
    .PREADY8(1'b0),
    .PRDATA8(32'd0),
    .PSLVERR8(1'b0),   

    .PSEL9(),
    .PREADY9(1'b0),
    .PRDATA9(32'd0),
    .PSLVERR9(1'b0),   

    .PSEL10(),
    .PREADY10(1'b0),
    .PRDATA10(32'd0),
    .PSLVERR10(1'b0),  

    .PSEL11(),
    .PREADY11(1'b0),
    .PRDATA11(32'd0),
    .PSLVERR11(1'b0),  

    .PSEL12(),
    .PREADY12(1'b0),
    .PRDATA12(32'd0),
    .PSLVERR12(1'b0),  

    .PSEL13(),
    .PREADY13(1'b0),
    .PRDATA13(32'd0),
    .PSLVERR13(1'b0),  

    .PSEL14(),
    .PREADY14(1'b0),
    .PRDATA14(32'd0),
    .PSLVERR14(1'b0),  

    .PSEL15(),
    .PREADY15(1'b0),
    .PRDATA15(32'd0),
    .PSLVERR15(1'b0),  

    .PREADY(PREADY),
    .PRDATA(PRDATA),
    .PSLVERR(PSLVERR)
);


cmsdk_apb_uart u_apb_uart_0(
    .PCLK              (PCLK),     // Peripheral clock
    .PCLKG             (PCLK),    // Gated PCLK for bus
    .PRESETn           (PRESETn),  // Reset

    .PSEL              (PSEL_UART0),     // APB interface inputs
    .PADDR             (PADDR[11:2]),
    .PENABLE           (PENABLE),
    .PWRITE            (PWRITE),
    .PWDATA            (PWDATA),

    .PRDATA            (PRDATA_UART0),   // APB interface outputs
    .PREADY            (PREADY_UART0),
    .PSLVERR           (PSLVERR_UART0),

    .ECOREVNUM         (4'h0),// Engineering-change-order revision bits

    .RXD               (UARTRXD),      // Receive data

    .TXD               (UARTTXD),      // Transmit data
    .TXEN              (UARTTXEN),     // Transmit Enabled

    .BAUDTICK          (),   // Baud rate x16 tick output (for testing)

    .TXINT             (uart0_txint),       // Transmit Interrupt
    .RXINT             (uart0_rxint),       // Receive  Interrupt
    .TXOVRINT          (uart0_txovrint),    // Transmit Overrun Interrupt
    .RXOVRINT          (uart0_rxovrint),    // Receive  Overrun Interrupt
    .UARTINT           (uart0_combined_int) // Combined Interrupt
);



cmsdk_apb_timer u_apb_timer0(
    .PCLK(PCLK),    // PCLK for timer operation
    .PCLKG(PCLK),   // Gated clock
    .PRESETn(PRESETn), // Reset
    .PSEL(PSEL_TIMER0),    // Device select
    .PADDR(PADDR[11:2]),   // Address
    .PENABLE(PENABLE), // Transfer control
    .PWRITE(PWRITE),  // Write control
    .PWDATA(PWDATA),  // Write data
    .ECOREVNUM(4'h0),// Engineering-change-order revision bits
    .PRDATA(PRDATA_TIMER0),  // Read data
    .PREADY(PREADY_TIMER0),  // Device ready
    .PSLVERR(PSLVERR_TIMER0), // Device error response
    .EXTIN(1'b1),   // Extenal input
    .TIMERINT(timer0_int)
);


megasoc_peripheral_debug #(
    .FT1248_WIDTH(1)
) u_megasoc_peripheral_debug(
    .HCLK(HCLK),
    .HRESETn(HRESETn),
    .HADDR_ADP(HADDR_ADP),
    .HTRANS_ADP(HTRANS_ADP),
    .HWRITE_ADP(HWRITE_ADP),
    .HSIZE_ADP(HSIZE_ADP),
    .HBURST_ADP(HBURST_ADP),
    .HPROT_ADP(HPROT_ADP),
    .HWDATA_ADP(HWDATA_ADP),
    .HRDATA_ADP(HRDATA_ADP),
    .HREADY_ADP(HREADY_ADP),
    .HRESP_ADP(HRESP_ADP),

    .PCLK(PCLK),
    .PRESETn(PRESETn),
    .USRT_PSEL(PSEL_USRT),
    .USRT_PADDRm(PADDR[11:0]),
    .USRT_PENABLE(PENABLE),
    .USRT_PWRITE(PWRITE),
    .USRT_PWDATA(PWDATA),
    .USRT_PRDATA(PRDATA_USRT),
    .USRT_PREADY(PREADY_USRT),
    .USRT_PSLVERR(PSLVERR_USRT),

    .FT_CLK_O(FT_CLK_O),
    .FT_SSN_O(FT_SSN_O),
    .FT_MISO_I(FT_MISO_I),
    .FT_MIOSIO_O(FT_MIOSIO_O),
    .FT_MIOSIO_E(FT_MIOSIO_E),
    .FT_MIOSIO_Z(FT_MIOSIO_Z),
    .FT_MIOSIO_I(FT_MIOSIO_I)
);

endmodule