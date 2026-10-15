

module megasoc_peripheral_addr_decode #(
    parameter BASEADDR_APBSS    = 32'h4000_0000,
    parameter BASEADDR_GPIO0    = 32'h4001_0000,
    parameter BASEADDR_GPIO1    = 32'h4002_0000,
    parameter BASEADDR_LPDDR4   = 32'h4100_0000   // 16 MB LPDDR4 configuration window
) (
    // System Address
    input wire                  hsel,
    input wire [31:0]           haddr,

    // Peripheral Selection
    output wire                 apbsys_hsel,
    output wire                 gpio0_hsel,
    output wire                 gpio1_hsel,
    output wire                 lpddr4_hsel,       // 0 unless INC_LPDDR4

    // Default slave
    output wire                 defslv_hsel
);

assign apbsys_hsel  = hsel & (haddr[31:16]==
                    BASEADDR_APBSS[31:16]);   // 0x40000000
assign gpio0_hsel   = hsel & (haddr[31:12]==
                    BASEADDR_GPIO0[31:12]);   // 0x40010000
assign gpio1_hsel   = hsel & (haddr[31:12]==
                    BASEADDR_GPIO1[31:12]);   // 0x40011000

`ifdef INC_LPDDR4
assign lpddr4_hsel  = hsel & (haddr[31:24]==
                    BASEADDR_LPDDR4[31:24]);  // 0x41000000
`else
assign lpddr4_hsel  = 1'b0;                   // window not present: default slave
`endif

assign defslv_hsel  = ~(apbsys_hsel |
                        gpio0_hsel   | gpio1_hsel |
                        lpddr4_hsel
                        );

endmodule