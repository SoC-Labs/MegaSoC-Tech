

module megasoc_peripheral_addr_decode #(
    parameter BASEADDR_APBSS    = 32'h4000_0000,
    parameter BASEADDR_GPIO0    = 32'h4001_0000,
    parameter BASEADDR_GPIO1    = 32'h4002_0000
) (
    // System Address
    input wire                  hsel,
    input wire [31:0]           haddr,

    // Peripheral Selection
    output wire                 apbsys_hsel,
    output wire                 gpio0_hsel,
    output wire                 gpio1_hsel,

    // Default slave
    output wire                 defslv_hsel
);

assign apbsys_hsel  = hsel & (haddr[31:16]==
                    BASEADDR_APBSS[31:16]);   // 0x40000000
assign gpio0_hsel   = hsel & (haddr[31:12]==
                    BASEADDR_GPIO0[31:12]);   // 0x40010000
assign gpio1_hsel   = hsel & (haddr[31:12]==
                    BASEADDR_GPIO1[31:12]);   // 0x40011000

assign defslv_hsel  = ~(apbsys_hsel |
                        gpio0_hsel   | gpio1_hsel
                        );

endmodule