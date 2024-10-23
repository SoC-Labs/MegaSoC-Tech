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

module megasoc_peripheral_subsystem(
    input  wire         PCLK,
    input  wire         PRESETn,

    // APB bus interface
    input  wire [31:0]  PADDR,
    input  wire         PENABLE,  
    input  wire         PWRITE,   
    input  wire [31:0]  PWDATA,   
    input  wire         PSEL,     
    output wire [31:0]  PRDATA,   
    output wire         PREADY,   
    output wire         PSLVERR,  

    input  wire         UARTRXD,
    output wire         UARTTXD,
    output wire         UARTTXEN,   

    output wire [5:0]   PERI_IRQS   // Peripheral interrupts to GIC
);

wire        UARTCLK;
assign      UARTCLK=PCLK; // TODO generate UARTCLK from elsewhere

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

// CMSDK APB Slave Mux (from Corstone 101) 
cmsdk_apb_slave_mux #(
    .PORT0_ENABLE(1),
    .PORT1_ENABLE(1),
    .PORT2_ENABLE(0),
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

    .PSEL2(),
    .PREADY2(1'b0),
    .PRDATA2(32'd0),
    .PSLVERR2(1'b0),   

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

wire    uart0_txint;
wire    uart0_rxint;
wire    uart0_txovrint;
wire    uart0_rxovrint;
wire    uart0_combined_int;

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

assign PERI_IRQS[0] = uart0_txint;
assign PERI_IRQS[1] = uart0_rxint;
assign PERI_IRQS[2] = uart0_txovrint;
assign PERI_IRQS[3] = uart0_rxovrint;
assign PERI_IRQS[4] = uart0_combined_int;

wire timer0_int;

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

assign PERI_IRQS[5] = timer0_int;

endmodule