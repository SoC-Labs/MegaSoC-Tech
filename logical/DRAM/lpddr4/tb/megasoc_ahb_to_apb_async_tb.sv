//-----------------------------------------------------------------------------
// Unit testbench for megasoc_ahb_to_apb_async
//
// AHB side (HCLK): pipelined AHB-Lite manager -- random single word reads and
//   writes, back-to-back or with IDLE gaps, to the CTRL window, the PHY window
//   and out-of-range offsets.
// APB side (PCLK): two APB register-file subordinates with random wait states.
//   Addresses with bits [7:4] == 4'hE answer PSLVERR (writes are not stored).
// Checks: AHB response (OKAY / two-cycle ERROR) and read data against a
//   reference model; every APB access (port, direction, PADDR, PWDATA)
//   against the AHB transfer that caused it, in order; APB protocol (SETUP then ACCESS, signals stable while
//   waiting, PSTRB/PPROT); PADDR mapping; no APB access for out-of-range
//   offsets; exact APB access count. Ends with AHB2APB_TB: PASS / FAIL.
//
// Plusargs: +H_HALF=<ns> +P_HALF=<ns> +N_TXN=<n> +SEED=<n>
//-----------------------------------------------------------------------------
`timescale 1ns/1ps
module megasoc_ahb_to_apb_async_tb;

real h_half = 10.0, p_half = 1.25;
int  n_txn  = 2000;
int  seed   = 1;

reg HCLK = 0, PCLK = 0, HRESETn = 0, PRESETn = 0;

ahb  #(.DATA_W(32), .ADDR_W(32)) AHB();
apb3 CTRL_APB();
apb4 PHY_APB();

megasoc_ahb_to_apb_async dut (
    .HCLK(HCLK), .HRESETn(HRESETn), .AHB(AHB),
    .PCLK(PCLK), .PRESETn(PRESETn), .CTRL_APB(CTRL_APB), .PHY_APB(PHY_APB)
);

initial begin
    void'($value$plusargs("H_HALF=%f", h_half));
    void'($value$plusargs("P_HALF=%f", p_half));
    void'($value$plusargs("N_TXN=%d",  n_txn));
    void'($value$plusargs("SEED=%d",   seed));
    void'($urandom(seed));
    $display("AHB2APB_TB: HCLK period %0.3f ns, PCLK period %0.3f ns, %0d transfers, seed %0d",
             2*h_half, 2*p_half, n_txn, seed);
end
initial begin #0.29; forever #(h_half) HCLK = ~HCLK; end
initial begin #0.13; forever #(p_half) PCLK = ~PCLK; end
initial begin
    #40;
    @(posedge HCLK) HRESETn <= 1;
    @(posedge PCLK) PRESETn <= 1;
end

int errs = 0;
`define FAIL(MSG) begin errs++; if (errs < 25) $display("FAIL @%0t: %s", $time, MSG); end

// ---------------------------------------------------------------------------
// APB subordinates (PCLK): sparse register files with random wait states
// ---------------------------------------------------------------------------
// every APB access must match the AHB transfer that caused it, in order
typedef struct { bit phy; bit write; logic [31:0] paddr; logic [31:0] wdata; } apb_exp_t;
apb_exp_t apb_exp_q[$];

function automatic void check_apb(input bit phy, input bit write, input logic [31:0] paddr, input logic [31:0] wdata);
    apb_exp_t e;
    if (apb_exp_q.size() == 0) begin errs++; $display("FAIL @%0t: unexpected APB access %s %s %h", $time, phy ? "PHY" : "CTRL", write ? "WR" : "RD", paddr); return; end
    e = apb_exp_q.pop_front();
    if (e.phy != phy || e.write != write || e.paddr !== paddr || (write && e.wdata !== wdata)) begin
        errs++;
        if (errs < 25) $display("FAIL @%0t: APB access %s %s %h data %h, expected %s %s %h data %h", $time,
            phy ? "PHY" : "CTRL", write ? "WR" : "RD", paddr, wdata, e.phy ? "PHY" : "CTRL", e.write ? "WR" : "RD", e.paddr, e.wdata);
    end
endfunction

logic [31:0] ctrl_mem [int];
logic [31:0] phy_mem  [int];
int          apb_accesses = 0;

function automatic bit is_err_addr(input logic [31:0] a);
    return (a[7:4] == 4'hE);
endfunction

// CTRL (APB3)
logic        c_ready = 0, c_err = 0;
logic [31:0] c_rdata = 0;
int          c_wait = 0;
assign CTRL_APB.pready  = c_ready;
assign CTRL_APB.pslverr = c_err;
assign CTRL_APB.prdata  = c_rdata;

// PHY (APB4)
logic        y_ready = 0, y_err = 0;
logic [31:0] y_rdata = 0;
int          y_wait = 0;
assign PHY_APB.pready  = y_ready;
assign PHY_APB.pslverr = y_err;
assign PHY_APB.prdata  = y_rdata;

// One generic APB subordinate process per port
`define APB_SLAVE(P, MEM, READY, ERR, RDATA, WAITN, ID, NAME, AMASK) \
always @(posedge PCLK) begin \
    READY <= 0; ERR <= 0; \
    if (PRESETn && P.psel && !P.penable) WAITN <= $urandom % 4; \
    if (PRESETn && P.psel && P.penable && !READY) begin \
        if (WAITN > 0) WAITN <= WAITN - 1; \
        else begin \
            READY <= 1; \
            ERR   <= is_err_addr(P.paddr); \
            if (!P.pwrite) RDATA <= MEM.exists(P.paddr) ? MEM[P.paddr] : 32'h0; \
            else if (!is_err_addr(P.paddr)) MEM[P.paddr] = P.pwdata; \
        end \
    end \
end \
/* protocol: ACCESS must follow SETUP, signals stable during the transfer */ \
logic [31:0] ID``_paddr_q; logic ID``_pwrite_q; logic [31:0] ID``_pwdata_q; logic ID``_inxfer = 0; \
always @(posedge PCLK) if (PRESETn) begin \
    if (P.psel && !P.penable) begin \
        if (ID``_inxfer) `FAIL($sformatf("%s: SETUP while previous transfer open", NAME)) \
        ID``_inxfer <= 1; ID``_paddr_q <= P.paddr; ID``_pwrite_q <= P.pwrite; ID``_pwdata_q <= P.pwdata; \
        if ((P.paddr & ~AMASK) != 0) `FAIL($sformatf("%s: PADDR %h outside port range", NAME, P.paddr)) \
    end \
    if (P.penable && !P.psel) `FAIL($sformatf("%s: PENABLE without PSEL", NAME)) \
    if (P.psel && P.penable) begin \
        if (!ID``_inxfer) `FAIL($sformatf("%s: ACCESS without SETUP", NAME)) \
        if (P.paddr !== ID``_paddr_q || P.pwrite !== ID``_pwrite_q || (P.pwrite && P.pwdata !== ID``_pwdata_q)) \
            `FAIL($sformatf("%s: signals changed during transfer", NAME)) \
        if (READY) begin ID``_inxfer <= 0; apb_accesses++; check_apb((NAME == "PHY"), P.pwrite, P.paddr, P.pwdata); end \
    end \
end

`APB_SLAVE(CTRL_APB, ctrl_mem, c_ready, c_err, c_rdata, c_wait, ctrl, "CTRL", 32'h0000_FFFF)
`APB_SLAVE(PHY_APB,  phy_mem,  y_ready, y_err, y_rdata, y_wait, phy,  "PHY",  32'h003F_FFFF)

always @(posedge PCLK) if (PRESETn) begin
    if (CTRL_APB.psel && PHY_APB.psel) `FAIL("both APB ports selected")
    if (PHY_APB.psel && (PHY_APB.pstrb !== 4'hF || PHY_APB.pprot !== 3'd0)) `FAIL("PHY PSTRB/PPROT not F/0")
end

// ---------------------------------------------------------------------------
// AHB manager (HCLK), pipelined: address phase N+1 overlaps data phase N
// ---------------------------------------------------------------------------
logic        m_hsel = 0, m_hwrite = 0;
logic [1:0]  m_htrans = 2'b00;
logic [31:0] m_haddr = 0, m_hwdata = 0;
assign AHB.HSEL   = m_hsel;
assign AHB.HADDR  = m_haddr;
assign AHB.HTRANS = m_htrans;
assign AHB.HWRITE = m_hwrite;
assign AHB.HSIZE  = 3'b010;
assign AHB.HBURST = 3'b000;
assign AHB.HPROT  = 4'b0011;
assign AHB.HWDATA = m_hwdata;
assign AHB.HREADY = AHB.HREADYOUT;              // single subordinate on this bus

// reference model of what the registers should hold
logic [31:0] ref_ctrl [int];
logic [31:0] ref_phy  [int];
int          exp_apb  = 0;

typedef struct { bit valid; bit write; logic [31:0] addr; logic [31:0] wdata; } xfer_t;
xfer_t dp;                                       // transfer currently in its data phase
int    issued = 0, completed = 0, n_err_rsp = 0, n_rd = 0, n_wr = 0;

function automatic logic [31:0] rand_addr();
    int k = $urandom % 10;
    logic [31:0] base = 32'h4100_0000;
    if (k < 4)      return base | {16'd0, 16'($urandom) & 16'hFFFC};                   // CTRL
    else if (k < 8) return base | 32'h0040_0000 | (32'($urandom) & 32'h003F_FFFC);   // PHY
    else if (k < 9) return base | 32'h0001_0000 | (32'($urandom) & 32'h003E_FFFC);   // gap: 0x01_0000-0x3F_FFFF
    else            return base | 32'h0080_0000 | (32'($urandom) & 32'h007F_FFFC);   // gap: 0x80_0000-0xFF_FFFF
endfunction

// target of an in-window offset: 0 CTRL, 1 PHY, 2 none
function automatic int tgt_of(input logic [31:0] a);
    if (a[23:16] == 8'h00) return 0;
    if (a[23:22] == 2'b01) return 1;
    return 2;
endfunction

function automatic logic [31:0] paddr_of(input logic [31:0] a);
    return (tgt_of(a) == 0) ? {16'd0, a[15:0]} : {10'd0, a[21:0]};
endfunction

// complete the data phase of 'dp' at this edge
task automatic check_completion();
    int t; logic [31:0] pa; bit exp_err; logic [31:0] exp_rd;
    t  = tgt_of(dp.addr);
    pa = paddr_of(dp.addr);
    exp_err = (t == 2) || is_err_addr(pa);
    if (t != 2) exp_apb++;
    if (AHB.HRESP !== exp_err)
        `FAIL($sformatf("addr %h %s: HRESP %b expected %b", dp.addr, dp.write ? "WR" : "RD", AHB.HRESP, exp_err))
    if (exp_err) n_err_rsp++;
    if (dp.write) begin
        n_wr++;
        if (!exp_err) begin if (t == 0) ref_ctrl[pa] = dp.wdata; else ref_phy[pa] = dp.wdata; end
    end else begin
        n_rd++;
        if (!exp_err) begin
            exp_rd = (t == 0) ? (ref_ctrl.exists(pa) ? ref_ctrl[pa] : 0) : (ref_phy.exists(pa) ? ref_phy[pa] : 0);
            if (AHB.HRDATA !== exp_rd) `FAIL($sformatf("addr %h RD: HRDATA %h expected %h", dp.addr, AHB.HRDATA, exp_rd))
        end
    end
    completed++;
endtask

// ERROR responses must be two cycles: HRESP=1 with HREADYOUT=0, then 1/1
logic prev_err_first = 0;
always @(posedge HCLK) if (HRESETn) begin
    if (AHB.HRESP && AHB.HREADYOUT && !prev_err_first) `FAIL("ERROR response without its first (HREADYOUT=0) cycle")
    prev_err_first <= AHB.HRESP && !AHB.HREADYOUT;
end

initial begin
    dp.valid = 0;
    wait (HRESETn);
    @(posedge HCLK);
    // write-then-read of a few fixed registers first, then random traffic
    forever begin
        @(posedge HCLK);
        if (AHB.HREADY) begin
            // data phase of dp (if any) completes at this edge
            if (dp.valid) check_completion();
            // address phase presented before this edge moves to the data phase
            dp.valid = m_hsel && m_htrans[1];
            dp.write = m_hwrite;
            dp.addr  = m_haddr;
            if (dp.valid && dp.write) begin dp.wdata = {$urandom}; m_hwdata <= dp.wdata; end
            if (dp.valid && tgt_of(dp.addr) != 2) begin
                apb_exp_t e;
                e.phy = (tgt_of(dp.addr) == 1); e.write = dp.write; e.paddr = paddr_of(dp.addr); e.wdata = dp.wdata;
                apb_exp_q.push_back(e);
            end
            // next address phase ('next_xfer' is what is driven after this
            // edge; the m_* signals still hold the old values until then)
            begin
                bit next_xfer;
                next_xfer = (issued < n_txn) && ($urandom % 3 != 0);
                if (next_xfer) begin
                    m_hsel   <= 1; m_htrans <= 2'b10;
                    m_haddr  <= (issued % 5 == 4 && issued > 0) ? dp.addr : rand_addr();   // revisit: read back recent
                    m_hwrite <= ($urandom % 2);
                    issued++;
                end else begin
                    m_hsel <= ($urandom % 2); m_htrans <= 2'b00;          // IDLE (HSEL may stay high)
                end
                if (issued >= n_txn && !dp.valid && !next_xfer) break;
            end
        end
    end
    m_hsel <= 0; m_htrans <= 2'b00;                                       // park the bus
    repeat (50) @(posedge HCLK);
    if (apb_exp_q.size() != 0) `FAIL($sformatf("%0d expected APB accesses never happened", apb_exp_q.size()))
    if (completed != n_txn) `FAIL($sformatf("completed %0d of %0d transfers", completed, n_txn))
    if (apb_accesses != exp_apb) `FAIL($sformatf("APB accesses %0d expected %0d", apb_accesses, exp_apb))
    $display("AHB2APB_TB: %0d transfers (%0d WR, %0d RD), %0d error responses, %0d APB accesses",
             completed, n_wr, n_rd, n_err_rsp, apb_accesses);
    if (errs == 0) $display("AHB2APB_TB: PASS"); else $display("AHB2APB_TB: FAIL (%0d errors)", errs);
    $finish;
end

initial begin
    #20ms;
    $display("AHB2APB_TB: FAIL (timeout) completed %0d of %0d", completed, n_txn);
    $finish;
end

endmodule
