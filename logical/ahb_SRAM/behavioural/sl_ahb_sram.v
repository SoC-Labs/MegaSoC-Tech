//-----------------------------------------------------------------------------
// SoCLabs FPGA SRAM Wrapper 
// - to be substitued with same name file in filelist when moving to ASIC
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Contributors
//
// David Mapstone (d.a.mapstone@soton.ac.uk)
//
// Copyright 2021-3, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------

module sl_ahb_sram #(
    // System Parameters
    parameter SYS_DATA_W = 32,  // System Data Width
    parameter RAM_ADDR_W = 19,  // Size of SRAM
    parameter RAM_DATA_W = 32   // Data Width of RAM
)(
    // --------------------------------------------------------------------------
    // Port Definitions
    // --------------------------------------------------------------------------
    input  wire                  HCLK,      // system bus clock
    input  wire                  HRESETn,   // system bus reset
    input  wire                  HSEL,      // AHB peripheral select
    input  wire                  HREADY,    // AHB ready input
    input  wire            [1:0] HTRANS,    // AHB transfer type
    input  wire            [2:0] HSIZE,     // AHB hsize
    input  wire                  HWRITE,    // AHB hwrite
    input  wire [RAM_ADDR_W-1:0] HADDR,     // AHB address bus
    input  wire [SYS_DATA_W-1:0] HWDATA,    // AHB write data bus
    output wire                  HREADYOUT, // AHB ready output to S->M mux
    output wire                  HRESP,     // AHB response
    output wire [SYS_DATA_W-1:0] HRDATA     // AHB read data bus
);
    
    
    // AHB to SRAM Behavioural

wire   [31:0] SRAMRDATA;
wire [RAM_ADDR_W-3:0] SRAMADDR;
wire    [3:0] SRAMWEN;
wire   [31:0] SRAMWDATA;
wire          SRAMCS;

cmsdk_ahb_ram_beh #(
  .AW(21),
  .filename("app_flash.v8-a.hex"),
  .WS_N(0), 
  .WS_S(0)) u_ahb_sram (
    .HCLK(HCLK),    // Clock
    .HRESETn(HRESETn), // Reset
    .HSEL(HSEL),    // Device select
    .HADDR(HADDR),   // Address
    .HTRANS(HTRANS),  // Transfer control
    .HSIZE(HSIZE),   // Transfer size
    .HWRITE(HWRITE),  // Write control
    .HWDATA(HWDATA),  // Write data
    .HREADY(HREADY),  // Transfer phase done
    .HREADYOUT(HREADYOUT), // Device ready
    .HRDATA(HRDATA),  // Read data output
    .HRESP(HRESP)
  );
endmodule