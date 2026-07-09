interface ahb #(
    parameter DATA_W=32,
    parameter ADDR_W=32
);
    wire                HSEL;
    wire [ADDR_W-1:0]   HADDR;
    wire [1:0]          HTRANS;
    wire                HWRITE;
    wire [2:0]          HSIZE;
    wire [2:0]          HBURST;
    wire [3:0]          HPROT;
    wire [DATA_W-1:0]   HWDATA;
    wire [DATA_W-1:0]   HRDATA;
    wire                HREADYOUT;
    wire                HREADY;
    wire                HRESP;

    modport subordinate (
        input HSEL, HADDR, HTRANS, HWRITE, HSIZE, HBURST,
        HPROT, HWDATA, HREADY,
        output HRDATA, HREADYOUT, HRESP
    );
    modport master (
        input HRDATA, HREADYOUT, HRESP,
        output HSEL, HADDR, HTRANS, HWRITE, HSIZE, HBURST,
        HPROT, HWDATA, HREADY
    );

endinterface
