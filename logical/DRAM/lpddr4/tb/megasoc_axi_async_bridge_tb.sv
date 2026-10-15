//-----------------------------------------------------------------------------
// Unit testbench for megasoc_axi_async_bridge
//
// S side (s_aclk): random AXI manager -- random IDs/addresses/lengths/sizes/
//                  strobes, random VALID gaps, random BREADY/RREADY.
// M side (m_aclk): random AXI subordinate -- random AWREADY/WREADY/ARREADY,
//                  B per write burst (AWID echoed), R bursts with random data,
//                  random BVALID/RVALID gaps.
// Scoreboard: every handshake on both sides of every channel is recorded and
// the streams are compared word for word (a bridge must be a lossless,
// in-order pipe per channel). Ends with BRIDGE_TB: PASS / FAIL.
//
// Plusargs: +S_HALF=<ns> +M_HALF=<ns> (half clock periods), +N_TXN=<n>,
//           +SEED=<n>
//-----------------------------------------------------------------------------
`timescale 1ns/1ps
module megasoc_axi_async_bridge_tb;

localparam DATA_W = 64, ID_W = 9, ADDR_W = 33;
localparam AX_W = ID_W + ADDR_W + 8 + 3 + 2 + 1 + 4 + 3;
localparam W_W  = DATA_W + DATA_W/8 + 1;
localparam B_W  = ID_W + 2;
localparam R_W  = ID_W + DATA_W + 2 + 1;

real s_half = 10.0, m_half = 1.25;
int  n_txn  = 300;
int  seed   = 1;

reg s_aclk = 0, m_aclk = 0, s_aresetn = 0, m_aresetn = 0;

axi4 #(.DATA_W(DATA_W), .ID_W(ID_W), .ADDR_W(ADDR_W)) S();
axi4 #(.DATA_W(DATA_W), .ID_W(ID_W), .ADDR_W(ADDR_W)) M();

megasoc_axi_async_bridge #(.DATA_W(DATA_W), .ID_W(ID_W), .ADDR_W(ADDR_W), .DEPTH(4)) dut (
    .s_aclk(s_aclk), .s_aresetn(s_aresetn), .S(S),
    .m_aclk(m_aclk), .m_aresetn(m_aresetn), .M(M)
);

// ---------------------------------------------------------------------------
// Clocks and reset
// ---------------------------------------------------------------------------
initial begin
    void'($value$plusargs("S_HALF=%f", s_half));
    void'($value$plusargs("M_HALF=%f", m_half));
    void'($value$plusargs("N_TXN=%d",  n_txn));
    void'($value$plusargs("SEED=%d",   seed));
    void'($urandom(seed));
    $display("BRIDGE_TB: s_aclk period %0.3f ns, m_aclk period %0.3f ns, %0d write + %0d read bursts, seed %0d",
             2*s_half, 2*m_half, n_txn, n_txn, seed);
end
initial begin #0.37; forever #(s_half) s_aclk = ~s_aclk; end   // odd start phase
initial begin #0.11; forever #(m_half) m_aclk = ~m_aclk; end

initial begin
    #50;
    @(posedge s_aclk) s_aresetn <= 1;
    @(posedge m_aclk) m_aresetn <= 1;
end

// ---------------------------------------------------------------------------
// S side: random manager
// ---------------------------------------------------------------------------
logic [AX_W-1:0] s_aw, s_ar;
logic [W_W-1:0]  s_w;
logic s_awvalid = 0, s_wvalid = 0, s_arvalid = 0, s_bready = 0, s_rready = 0;

assign {S.AWID, S.AWADDR, S.AWLEN, S.AWSIZE, S.AWBURST, S.AWLOCK, S.AWCACHE, S.AWPROT} = s_aw;
assign {S.ARID, S.ARADDR, S.ARLEN, S.ARSIZE, S.ARBURST, S.ARLOCK, S.ARCACHE, S.ARPROT} = s_ar;
assign {S.WDATA, S.WSTRB, S.WLAST} = s_w;
assign S.AWVALID = s_awvalid;  assign S.WVALID = s_wvalid;  assign S.ARVALID = s_arvalid;
assign S.BREADY  = s_bready;   assign S.RREADY = s_rready;

logic [7:0] aw_len_q[$];          // AW lengths, so W knows how many beats to send

function automatic logic [AX_W-1:0] rand_ax(output logic [7:0] len);
    logic [ID_W-1:0] id; logic [ADDR_W-1:0] a; logic [2:0] sz;
    id  = $urandom;  a = {$urandom, $urandom};
    len = ($urandom % 4 == 0) ? $urandom % 256 : $urandom % 16;
    sz  = $urandom % 4;
    return {id, a, len, sz, 2'($urandom % 3), 1'($urandom), 4'($urandom), 3'($urandom)};
endfunction

// AW sender
initial begin
    logic [7:0] len;
    wait (s_aresetn);
    for (int t = 0; t < n_txn; t++) begin
        repeat ($urandom % 3) @(posedge s_aclk);
        s_aw <= rand_ax(len); s_awvalid <= 1;
        aw_len_q.push_back(len);
        do @(posedge s_aclk); while (!S.AWREADY);
        s_awvalid <= 0;
    end
end

// W sender: bursts in AW order, may run ahead of its AW (legal AXI)
initial begin
    wait (s_aresetn);
    for (int t = 0; t < n_txn; t++) begin
        logic [7:0] len;
        wait (aw_len_q.size() > 0);
        len = aw_len_q.pop_front();
        for (int b = 0; b <= len; b++) begin
            if ($urandom % 4 == 0) begin s_wvalid <= 0; repeat (1 + $urandom % 2) @(posedge s_aclk); end
            s_w <= {{$urandom, $urandom}, 8'($urandom), (b == len)}; s_wvalid <= 1;
            do @(posedge s_aclk); while (!S.WREADY);
        end
        s_wvalid <= 0;
    end
end

// AR sender
initial begin
    logic [7:0] len;
    wait (s_aresetn);
    for (int t = 0; t < n_txn; t++) begin
        repeat ($urandom % 3) @(posedge s_aclk);
        s_ar <= rand_ax(len); s_arvalid <= 1;
        do @(posedge s_aclk); while (!S.ARREADY);
        s_arvalid <= 0;
    end
end

// Random BREADY / RREADY
always @(posedge s_aclk) begin
    s_bready <= ($urandom % 4 != 0);
    s_rready <= ($urandom % 4 != 0);
end

// ---------------------------------------------------------------------------
// M side: random subordinate
// ---------------------------------------------------------------------------
logic m_awready = 0, m_wready = 0, m_arready = 0, m_bvalid = 0, m_rvalid = 0;
logic [B_W-1:0] m_b;
logic [R_W-1:0] m_r;

assign M.AWREADY = m_awready;  assign M.WREADY = m_wready;  assign M.ARREADY = m_arready;
assign M.BVALID  = m_bvalid;   assign M.RVALID = m_rvalid;
assign {M.BID, M.BRESP} = m_b;
assign {M.RID, M.RDATA, M.RRESP, M.RLAST} = m_r;

always @(posedge m_aclk) begin
    m_awready <= ($urandom % 3 != 0);
    m_wready  <= ($urandom % 3 != 0);
    m_arready <= ($urandom % 3 != 0);
end

logic [ID_W-1:0] m_awid_q[$], m_bid_q[$];
logic [ID_W-1:0] m_arid_q[$];
logic [7:0]      m_arlen_q[$];

always @(posedge m_aclk) begin
    if (M.AWVALID && M.AWREADY) m_awid_q.push_back(M.AWID);
    if (M.WVALID && M.WREADY && M.WLAST) begin
        // AW and its last W can arrive in either order; pair them up in order
        fork begin
            wait (m_awid_q.size() > 0);
            m_bid_q.push_back(m_awid_q.pop_front());
        end join_none
    end
    if (M.ARVALID && M.ARREADY) begin m_arid_q.push_back(M.ARID); m_arlen_q.push_back(M.ARLEN); end
end

// B generator
initial begin
    wait (m_aresetn);
    forever begin
        wait (m_bid_q.size() > 0);
        @(posedge m_aclk);
        repeat ($urandom % 3) @(posedge m_aclk);
        m_b <= {m_bid_q.pop_front(), 2'($urandom)}; m_bvalid <= 1;
        do @(posedge m_aclk); while (!M.BREADY);
        m_bvalid <= 0;
    end
end

// R generator
initial begin
    wait (m_aresetn);
    forever begin
        logic [ID_W-1:0] id; logic [7:0] len;
        wait (m_arid_q.size() > 0);
        @(posedge m_aclk);
        id = m_arid_q.pop_front(); len = m_arlen_q.pop_front();
        for (int b = 0; b <= len; b++) begin
            if ($urandom % 4 == 0) begin m_rvalid <= 0; repeat (1 + $urandom % 2) @(posedge m_aclk); end
            m_r <= {id, {$urandom, $urandom}, 2'($urandom), (b == len)}; m_rvalid <= 1;
            do @(posedge m_aclk); while (!M.RREADY);
        end
        m_rvalid <= 0;
    end
end

// ---------------------------------------------------------------------------
// Scoreboard: compare each channel's stream on both sides
// ---------------------------------------------------------------------------
logic [AX_W-1:0] exp_aw[$], exp_ar[$];
logic [W_W-1:0]  exp_w[$];
logic [B_W-1:0]  exp_b[$];
logic [R_W-1:0]  exp_r[$];
int n_aw = 0, n_w = 0, n_b = 0, n_ar = 0, n_r = 0, errs = 0;

`define CHECK(Q, GOT, NAME, CNT) \
    begin \
        if (Q.size() == 0) begin errs++; $display("FAIL %s: word appeared with nothing sent (%h)", NAME, GOT); end \
        else begin \
            automatic logic [$bits(GOT)-1:0] e = Q.pop_front(); \
            if (e !== GOT) begin errs++; if (errs < 20) $display("FAIL %s #%0d: got %h exp %h", NAME, CNT, GOT, e); end \
        end \
        CNT++; \
    end

// sender side records
always @(posedge s_aclk) if (s_aresetn) begin
    if (S.AWVALID && S.AWREADY) exp_aw.push_back({S.AWID, S.AWADDR, S.AWLEN, S.AWSIZE, S.AWBURST, S.AWLOCK, S.AWCACHE, S.AWPROT});
    if (S.WVALID  && S.WREADY)  exp_w.push_back({S.WDATA, S.WSTRB, S.WLAST});
    if (S.ARVALID && S.ARREADY) exp_ar.push_back({S.ARID, S.ARADDR, S.ARLEN, S.ARSIZE, S.ARBURST, S.ARLOCK, S.ARCACHE, S.ARPROT});
end
always @(posedge m_aclk) if (m_aresetn) begin
    if (M.BVALID && M.BREADY) exp_b.push_back({M.BID, M.BRESP});
    if (M.RVALID && M.RREADY) exp_r.push_back({M.RID, M.RDATA, M.RRESP, M.RLAST});
end

// receiver side checks
always @(posedge m_aclk) if (m_aresetn) begin
    if (M.AWVALID && M.AWREADY) `CHECK(exp_aw, ({M.AWID, M.AWADDR, M.AWLEN, M.AWSIZE, M.AWBURST, M.AWLOCK, M.AWCACHE, M.AWPROT}), "AW", n_aw)
    if (M.WVALID  && M.WREADY)  `CHECK(exp_w,  ({M.WDATA, M.WSTRB, M.WLAST}), "W", n_w)
    if (M.ARVALID && M.ARREADY) `CHECK(exp_ar, ({M.ARID, M.ARADDR, M.ARLEN, M.ARSIZE, M.ARBURST, M.ARLOCK, M.ARCACHE, M.ARPROT}), "AR", n_ar)
end
always @(posedge s_aclk) if (s_aresetn) begin
    if (S.BVALID && S.BREADY) `CHECK(exp_b, ({S.BID, S.BRESP}), "B", n_b)
    if (S.RVALID && S.RREADY) `CHECK(exp_r, ({S.RID, S.RDATA, S.RRESP, S.RLAST}), "R", n_r)
end

// X/Z on any VALID after reset is a bridge bug
always @(posedge s_aclk) if (s_aresetn && ($isunknown(S.AWREADY) || $isunknown(S.WREADY) || $isunknown(S.ARREADY) || $isunknown(S.BVALID) || $isunknown(S.RVALID)))
    begin errs++; $display("FAIL: X/Z on S-side handshake at %0t", $time); end
always @(posedge m_aclk) if (m_aresetn && ($isunknown(M.AWVALID) || $isunknown(M.WVALID) || $isunknown(M.ARVALID) || $isunknown(M.BREADY) || $isunknown(M.RREADY)))
    begin errs++; $display("FAIL: X/Z on M-side handshake at %0t", $time); end

// ---------------------------------------------------------------------------
// End of test
// ---------------------------------------------------------------------------
int exp_r_beats = 0;
always @(posedge s_aclk) if (S.ARVALID && S.ARREADY) exp_r_beats += S.ARLEN + 1;

int w_beats_sent = 0;
always @(posedge s_aclk) if (S.WVALID && S.WREADY) w_beats_sent++;

initial begin
    wait (s_aresetn);
    wait (n_aw == n_txn && n_ar == n_txn && n_b == n_txn && n_w == w_beats_sent && n_r == exp_r_beats && n_r > 0);
    repeat (20) @(posedge s_aclk);
    if (exp_aw.size() || exp_w.size() || exp_ar.size() || exp_b.size() || exp_r.size()) begin
        errs++; $display("FAIL: words left in flight: AW %0d W %0d AR %0d B %0d R %0d",
                         exp_aw.size(), exp_w.size(), exp_ar.size(), exp_b.size(), exp_r.size());
    end
    $display("BRIDGE_TB: checked AW %0d, W %0d beats, B %0d, AR %0d, R %0d beats", n_aw, n_w, n_b, n_ar, n_r);
    if (errs == 0) $display("BRIDGE_TB: PASS"); else $display("BRIDGE_TB: FAIL (%0d errors)", errs);
    $finish;
end

initial begin
    #50ms;
    $display("BRIDGE_TB: FAIL (timeout) AW %0d/%0d W %0d/%0d B %0d/%0d AR %0d/%0d R %0d/%0d",
             n_aw, n_txn, n_w, w_beats_sent, n_b, n_txn, n_ar, n_txn, n_r, exp_r_beats);
    $finish;
end

endmodule
