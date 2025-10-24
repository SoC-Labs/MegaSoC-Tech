interface axi4l #(
    parameter DATA_W=32,
    parameter ADDR_W=32
);
    wire [ADDR_W-1:0]       AWADDR;
    wire [2:0]              AWPROT;
    wire                    AWVALID;
    wire                    AWREADY;

    wire [DATA_W-1:0]       WDATA;
    wire [(DATA_W/8)-1:0]   WSTRB;
    wire                    WVALID;
    wire                    WREADY;

    wire [1:0]              BRESP;
    wire                    BVALID;
    wire                    BREADY;

    wire [ADDR_W-1:0]       ARADDR;
    wire [2:0]              ARPROT;
    wire                    ARVALID;
    wire                    ARREADY;

    wire [DATA_W-1:0]       RDATA;
    wire [1:0]              RRESP;
    wire                    RVALID;
    wire                    RREADY;

    modport subordinate (
        input AWADDR, AWPROT, AWVALID,
        WDATA, WSTRB, WVALID, BREADY,
        ARADDR, ARPROT, ARVALID, RREADY,
        output AWREADY, WREADY, BRESP, BVALID, ARREADY,
        RDATA, RRESP, RVALID
    );
    modport master (
        input AWREADY, WREADY, BRESP, BVALID, ARREADY,
        RDATA, RRESP, RVALID,
        output AWADDR, AWPROT, AWVALID,
        WDATA, WSTRB, WVALID, BREADY,
        ARADDR, ARPROT, ARVALID, RREADY
    );

endinterface
