// Fast simulation DRAM model, used when INC_LPDDR4 is not defined. In
// INC_LPDDR4 builds megasoc_tech_wrapper instantiates the LPDDR4 subsystem
// instead, and this module is compiled out: an uninstantiated module with an
// interface port would become a top-level module with an unconnected
// interface, which is a compile error.
`ifndef INC_LPDDR4

module megasoc_dram_wrapper #(
    parameter ID_W=6
    )(
    input  wire             ACLK,
    input  wire             ARESETn,

    axi4.subordinate        DRAM_AXI,

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
    input  wire             DDR_nEVENT
);


// -----------------------------------------------------------------------
// Sim-only AXI4 RAM responder. No DDR4 controller model exists in this
// tree yet (see "To Do: Add DDR controller for DRAM port on NIC400" in
// megasoc_tech_wrapper.sv), so DRAM_AXI was previously tied off and any
// access to the DRAM window hung forever. This provides a modest backing
// store with address masking/wrapping and legal-address OKAY handshaking
// on the AW/W/B and AR/R channels so DRAM-window accesses complete.
// Not a capacity/performance/timing model -- protocol level only.
// -----------------------------------------------------------------------

localparam MEM_ADDR_W = 14;               // 16K x 64b words = 128KB backing store
localparam MEM_DEPTH  = (1 << MEM_ADDR_W);

reg [63:0] mem [0:MEM_DEPTH-1];

integer i;
initial
  begin
    for (i = 0; i < MEM_DEPTH; i = i + 1)
      mem[i] = 64'd0;
  end

// ---- Write address/data/response channel ----
localparam W_IDLE = 2'd0, W_DATA = 2'd1, W_RESP = 2'd2;

reg [1:0]                        wr_state;
reg [$bits(DRAM_AXI.AWID)-1:0]   wr_id_q;
reg [$bits(DRAM_AXI.AWADDR)-1:0] wr_addr_q;
reg [63:0]                       wr_step_q;
integer                          b;

assign DRAM_AXI.AWREADY = (wr_state == W_IDLE);
assign DRAM_AXI.WREADY  = (wr_state == W_DATA);
assign DRAM_AXI.BVALID  = (wr_state == W_RESP);
assign DRAM_AXI.BID     = wr_id_q;
assign DRAM_AXI.BRESP   = 2'b00;           // OKAY

always @ (posedge ACLK or negedge ARESETn)
  begin
    if (!ARESETn)
      begin
        wr_state  <= W_IDLE;
        wr_id_q   <= {$bits(DRAM_AXI.AWID){1'b0}};
        wr_addr_q <= {$bits(DRAM_AXI.AWADDR){1'b0}};
        wr_step_q <= 64'd0;
      end
    else
      case (wr_state)
        W_IDLE:
          if (DRAM_AXI.AWVALID)
            begin
              wr_id_q   <= DRAM_AXI.AWID;
              wr_addr_q <= DRAM_AXI.AWADDR;
              wr_step_q <= (64'd1 << DRAM_AXI.AWSIZE);
              wr_state  <= W_DATA;
            end

        W_DATA:
          if (DRAM_AXI.WVALID)
            begin
              for (b = 0; b < 8; b = b + 1)
                if (DRAM_AXI.WSTRB[b])
                  mem[wr_addr_q[MEM_ADDR_W+2:3]][b*8 +: 8] <= DRAM_AXI.WDATA[b*8 +: 8];

              wr_addr_q <= wr_addr_q + wr_step_q;

              if (DRAM_AXI.WLAST)
                wr_state <= W_RESP;
            end

        W_RESP:
          if (DRAM_AXI.BREADY)
            wr_state <= W_IDLE;

        default:
          wr_state <= W_IDLE;
      endcase
  end

// ---- Read address/data channel ----
localparam R_IDLE = 1'b0, R_DATA = 1'b1;

reg                              rd_state;
reg [$bits(DRAM_AXI.ARID)-1:0]   rd_id_q;
reg [$bits(DRAM_AXI.ARADDR)-1:0] rd_addr_q;
reg [63:0]                       rd_step_q;
reg [7:0]                        rd_len_q;
reg [7:0]                        rd_beat_q;

assign DRAM_AXI.ARREADY = (rd_state == R_IDLE);
assign DRAM_AXI.RVALID  = (rd_state == R_DATA);
assign DRAM_AXI.RID     = rd_id_q;
assign DRAM_AXI.RRESP   = 2'b00;           // OKAY
assign DRAM_AXI.RDATA   = mem[rd_addr_q[MEM_ADDR_W+2:3]];
assign DRAM_AXI.RLAST   = (rd_beat_q == rd_len_q);

always @ (posedge ACLK or negedge ARESETn)
  begin
    if (!ARESETn)
      begin
        rd_state  <= R_IDLE;
        rd_id_q   <= {$bits(DRAM_AXI.ARID){1'b0}};
        rd_addr_q <= {$bits(DRAM_AXI.ARADDR){1'b0}};
        rd_step_q <= 64'd0;
        rd_len_q  <= 8'd0;
        rd_beat_q <= 8'd0;
      end
    else
      case (rd_state)
        R_IDLE:
          if (DRAM_AXI.ARVALID)
            begin
              rd_id_q   <= DRAM_AXI.ARID;
              rd_addr_q <= DRAM_AXI.ARADDR;
              rd_step_q <= (64'd1 << DRAM_AXI.ARSIZE);
              rd_len_q  <= DRAM_AXI.ARLEN;
              rd_beat_q <= 8'd0;
              rd_state  <= R_DATA;
            end

        R_DATA:
          if (DRAM_AXI.RREADY)
            begin
              if (rd_beat_q == rd_len_q)
                rd_state <= R_IDLE;
              else
                begin
                  rd_beat_q <= rd_beat_q + 8'd1;
                  rd_addr_q <= rd_addr_q + rd_step_q;
                end
            end

        default:
          rd_state <= R_IDLE;
      endcase
  end

endmodule

`endif // INC_LPDDR4
