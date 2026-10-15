//-----------------------------------------------------------------------------
// MegaSoC AXI4 asynchronous clock-domain bridge
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Carries AXI4 between two unrelated clocks: the NIC-400 fabric side (S,
// subordinate port, s_aclk) and the LPDDR4 controller side (M, master port,
// m_aclk). Each of the five AXI channels goes through its own
// megasoc_async_fifo:
//
//     AW, W, AR : S -> M          B, R : M -> S
//
// A channel's fields are packed into one FIFO word and unpacked unchanged on
// the other side, so IDs, burst type/length/size, strobes, LAST and
// responses pass through untouched and per-channel ordering is preserved.
// No protocol conversion is done; AXI's own rules (W beats in AW order, B/R
// matched by ID) hold end to end because each channel stays in order.
//
// Throughput: one beat per clock on each side once a FIFO has data/space.
// Latency: a few cycles of the receiving clock per channel (pointer
// synchronisation).
//
// Reset: s_aresetn and m_aresetn must come from the same reset source, each
// de-asserted synchronously to its own clock (see megasoc_async_fifo).
//-----------------------------------------------------------------------------

module megasoc_axi_async_bridge #(
    parameter DATA_W = 64,
    parameter ID_W   = 9,
    parameter ADDR_W = 33,
    parameter DEPTH  = 4      // per-channel FIFO depth, power of two >= 2
    )(
    // Fabric side (NIC-400 master -> this subordinate port)
    input  wire             s_aclk,
    input  wire             s_aresetn,
    axi4.subordinate        S,

    // Controller side (this master port -> DRAM controller)
    input  wire             m_aclk,
    input  wire             m_aresetn,
    axi4.master             M
);

localparam AX_W = ID_W + ADDR_W + 8 + 3 + 2 + 1 + 4 + 3;  // AW / AR word
localparam W_W  = DATA_W + DATA_W/8 + 1;                  // W word
localparam B_W  = ID_W + 2;                               // B word
localparam R_W  = ID_W + DATA_W + 2 + 1;                  // R word

// ---- AW: S -> M ----
megasoc_async_fifo #(.WIDTH(AX_W), .DEPTH(DEPTH)) u_aw_fifo (
    .w_clk    (s_aclk),
    .w_resetn (s_aresetn),
    .w_valid  (S.AWVALID),
    .w_ready  (S.AWREADY),
    .w_data   ({S.AWID, S.AWADDR, S.AWLEN, S.AWSIZE, S.AWBURST, S.AWLOCK, S.AWCACHE, S.AWPROT}),

    .r_clk    (m_aclk),
    .r_resetn (m_aresetn),
    .r_valid  (M.AWVALID),
    .r_ready  (M.AWREADY),
    .r_data   ({M.AWID, M.AWADDR, M.AWLEN, M.AWSIZE, M.AWBURST, M.AWLOCK, M.AWCACHE, M.AWPROT})
);

// ---- W: S -> M ----
megasoc_async_fifo #(.WIDTH(W_W), .DEPTH(DEPTH)) u_w_fifo (
    .w_clk    (s_aclk),
    .w_resetn (s_aresetn),
    .w_valid  (S.WVALID),
    .w_ready  (S.WREADY),
    .w_data   ({S.WDATA, S.WSTRB, S.WLAST}),

    .r_clk    (m_aclk),
    .r_resetn (m_aresetn),
    .r_valid  (M.WVALID),
    .r_ready  (M.WREADY),
    .r_data   ({M.WDATA, M.WSTRB, M.WLAST})
);

// ---- B: M -> S ----
megasoc_async_fifo #(.WIDTH(B_W), .DEPTH(DEPTH)) u_b_fifo (
    .w_clk    (m_aclk),
    .w_resetn (m_aresetn),
    .w_valid  (M.BVALID),
    .w_ready  (M.BREADY),
    .w_data   ({M.BID, M.BRESP}),

    .r_clk    (s_aclk),
    .r_resetn (s_aresetn),
    .r_valid  (S.BVALID),
    .r_ready  (S.BREADY),
    .r_data   ({S.BID, S.BRESP})
);

// ---- AR: S -> M ----
megasoc_async_fifo #(.WIDTH(AX_W), .DEPTH(DEPTH)) u_ar_fifo (
    .w_clk    (s_aclk),
    .w_resetn (s_aresetn),
    .w_valid  (S.ARVALID),
    .w_ready  (S.ARREADY),
    .w_data   ({S.ARID, S.ARADDR, S.ARLEN, S.ARSIZE, S.ARBURST, S.ARLOCK, S.ARCACHE, S.ARPROT}),

    .r_clk    (m_aclk),
    .r_resetn (m_aresetn),
    .r_valid  (M.ARVALID),
    .r_ready  (M.ARREADY),
    .r_data   ({M.ARID, M.ARADDR, M.ARLEN, M.ARSIZE, M.ARBURST, M.ARLOCK, M.ARCACHE, M.ARPROT})
);

// ---- R: M -> S ----
megasoc_async_fifo #(.WIDTH(R_W), .DEPTH(DEPTH)) u_r_fifo (
    .w_clk    (m_aclk),
    .w_resetn (m_aresetn),
    .w_valid  (M.RVALID),
    .w_ready  (M.RREADY),
    .w_data   ({M.RID, M.RDATA, M.RRESP, M.RLAST}),

    .r_clk    (s_aclk),
    .r_resetn (s_aresetn),
    .r_valid  (S.RVALID),
    .r_ready  (S.RREADY),
    .r_data   ({S.RID, S.RDATA, S.RRESP, S.RLAST})
);

endmodule
