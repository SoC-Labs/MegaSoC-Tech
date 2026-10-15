//-----------------------------------------------------------------------------
// Self-checking unit testbench for megasoc_lpi_qchannel_adapter
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// A Q-channel controller model on pclk issues random low-power requests; a
// low-power-interface peripheral model on dclk accepts or refuses them at
// random (refusal = CSYSACK LOW with CACTIVE HIGH) and sometimes raises
// CACTIVE while stopped. Checked on every clock:
//   Q-channel rules : QACCEPTn falls only while QREQn LOW, rises only while
//                     QREQn HIGH; QDENY rises only while QREQn LOW, falls only
//                     while QREQn HIGH; QACCEPTn LOW and QDENY HIGH never
//                     together.
//   Interface rules : the adapter changes CSYSREQ only when CSYSACK equals it
//                     (previous handshake complete).
//   Meaning         : every accept seen by the controller was an accept by
//                     the peripheral, every deny was a refusal.
//   Progress        : every request is answered within a time limit.
// Plusargs: +P_HALF +D_HALF (half periods, ns) +SEED +N_REQ +DENY_PCT
//           +RESET_STOPPED (controller holds QREQn LOW through reset)
//-----------------------------------------------------------------------------
`timescale 1ns/1ps

module megasoc_lpi_qchannel_adapter_tb;

real    p_half = 5.0, d_half = 1.25;
integer seed = 1, n_req = 300, deny_pct = 30;
bit     reset_stopped = 0;

reg pclk = 0, dclk = 0, resetn = 0;
always #(p_half) pclk = ~pclk;
always #(d_half) dclk = ~dclk;

qchannel CTRL();
qchannel DEV();

megasoc_lpi_qchannel_adapter u_dut (
    .clk(dclk), .resetn(resetn), .CTRL(CTRL.subordinate), .DEV(DEV.master)
);

integer errors = 0, accepts = 0, denies = 0, wakes = 0;
`define FAIL(msg) begin errors++; if (errors <= 10) $display("[%0t] LPIQ_TB: ERROR %s", $time, msg); end

//-----------------------------------------------------------------------------
// Q-channel controller model (pclk)
//-----------------------------------------------------------------------------
reg qreqn = 1;
assign CTRL.qreqn = qreqn;
reg acc_s1 = 1, acc_s2 = 1, deny_s1 = 0, deny_s2 = 0;
always @(posedge pclk) begin
    acc_s1 <= CTRL.qacceptn; acc_s2 <= acc_s1;
    deny_s1 <= CTRL.qdeny;   deny_s2 <= deny_s1;
end

task automatic wait_ctrl(input string what, input bit want_acc, input bit acc_v, input bit want_deny, input bit deny_v,
                         output bit got_deny);
    integer t = 0;
    got_deny = 0;
    forever begin
        @(posedge pclk);
        if (want_acc && acc_s2 == acc_v) return;
        if (want_deny && deny_s2 == deny_v) begin got_deny = deny_v; return; end
        if (++t > 2000) begin `FAIL($sformatf("timeout waiting for %s", what)) return; end
    end
endtask

//-----------------------------------------------------------------------------
// Low-power-interface peripheral model (dclk): csysreq = DEV.qreqn,
// csysack = DEV.qacceptn, cactive = DEV.qactive
//-----------------------------------------------------------------------------
reg csysack = 1, cactive = 1;
assign DEV.qacceptn = csysack;
assign DEV.qactive  = cactive;
assign DEV.qdeny    = 1'b0;       // as tied in dram_wrapper
bit  dev_decision_deny;           // last answer the peripheral gave
bit  dev_stopped;                 // peripheral accepted and is quiescent

initial begin : peripheral
    forever begin
        @(posedge dclk);
        if (!DEV.qreqn && csysack) begin                       // low-power request
            repeat ($urandom_range(0, 12)) @(posedge dclk);
            dev_decision_deny = ($urandom_range(0, 99) < deny_pct);
            cactive <= dev_decision_deny;
            csysack <= 1'b0;
            dev_stopped = !dev_decision_deny;
        end else if (DEV.qreqn && !csysack) begin              // exit / end of deny
            repeat ($urandom_range(0, 12)) @(posedge dclk);
            cactive <= 1'b1;
            csysack <= 1'b1;
            dev_stopped = 0;
        end else if (dev_stopped && $urandom_range(0, 99) < 2) begin
            cactive <= 1'b1;                                   // wants to wake (hint only)
            wakes++;
            @(posedge dclk) cactive <= 1'b0;
        end
    end
end

//-----------------------------------------------------------------------------
// Rule checks (dclk: the adapter's outputs are dclk flops)
//-----------------------------------------------------------------------------
bit reset_phase = 0;
reg prev_acc = 1, prev_deny = 0, prev_csysreq = 1;
// QREQn as sampled on the clock edge where the adapter's outputs changed (a
// fast controller may already have reacted by the next edge)
reg prev_qreqn = 1;
always @(posedge dclk) if (resetn) begin
    if (!CTRL.qacceptn && CTRL.qdeny)                     `FAIL("QACCEPTn LOW and QDENY HIGH together")
    if (prev_acc && !CTRL.qacceptn && prev_qreqn)         `FAIL("QACCEPTn fell while QREQn HIGH")
    if (!prev_acc && CTRL.qacceptn && !prev_qreqn)        `FAIL("QACCEPTn rose while QREQn LOW")
    if (!prev_deny && CTRL.qdeny && prev_qreqn)           `FAIL("QDENY rose while QREQn HIGH")
    if (prev_deny && !CTRL.qdeny && !prev_qreqn)          `FAIL("QDENY fell while QREQn LOW")
    if (DEV.qreqn != prev_csysreq && prev_csysreq != csysack)
                                                          `FAIL("CSYSREQ changed before CSYSACK followed")
    if (prev_acc && !CTRL.qacceptn && !reset_phase && !dev_stopped)
                                                          `FAIL("accept reported, peripheral did not accept")
    if (!prev_deny && CTRL.qdeny && !dev_decision_deny)   `FAIL("deny reported, peripheral did not refuse")
    prev_acc <= CTRL.qacceptn; prev_deny <= CTRL.qdeny; prev_csysreq <= DEV.qreqn; prev_qreqn <= CTRL.qreqn;
end

//-----------------------------------------------------------------------------
// Stimulus
//-----------------------------------------------------------------------------
initial begin
    bit d;
    void'($value$plusargs("P_HALF=%f", p_half));
    void'($value$plusargs("D_HALF=%f", d_half));
    void'($value$plusargs("SEED=%d", seed));
    void'($value$plusargs("N_REQ=%d", n_req));
    void'($value$plusargs("DENY_PCT=%d", deny_pct));
    reset_stopped = $test$plusargs("RESET_STOPPED");
    void'($urandom(seed));

    if (reset_stopped) begin                 // controller is in Q_STOPPED at reset
        qreqn = 0; reset_phase = 1;
    end
    repeat (5) @(posedge pclk);
    resetn = 1;

    if (reset_stopped) begin
        wait_ctrl("accept after reset", 1, 0, 0, 0, d);
        repeat ($urandom_range(1, 20)) @(posedge pclk);
        if (DEV.qreqn !== 1'b1) `FAIL("low-power interface asked while stopped out of reset")
        qreqn = 1;
        wait_ctrl("run after reset exit", 1, 1, 0, 0, d);
        reset_phase = 0;
        if (DEV.qreqn !== 1'b1 || csysack !== 1'b1) `FAIL("interface not in run after reset exit")
    end

    for (int i = 0; i < n_req; i++) begin
        repeat ($urandom_range(0, 30)) @(posedge pclk);
        qreqn = 0;
        wait_ctrl("accept or deny", 1, 0, 1, 1, d);
        if (d) begin
            denies++;
            repeat ($urandom_range(0, 5)) @(posedge pclk);
            qreqn = 1;
            wait_ctrl("deny release", 0, 0, 1, 0, d);
        end else begin
            accepts++;
            repeat ($urandom_range(0, 40)) @(posedge pclk);
            qreqn = 1;
            wait_ctrl("exit", 1, 1, 0, 0, d);
        end
    end
    repeat (20) @(posedge pclk);
    if (CTRL.qacceptn !== 1'b1 || CTRL.qdeny !== 1'b0 || DEV.qreqn !== 1'b1)
        `FAIL("not back in run at the end")
    if (accepts == 0 || (deny_pct > 0 && denies == 0)) `FAIL("stimulus did not reach both outcomes")

    $display("LPIQ_TB: %0d requests: %0d accepted, %0d denied, %0d wake hints%s",
             n_req, accepts, denies, wakes, reset_stopped ? ", started stopped" : "");
    $display("LPIQ_TB: %s (%0d errors)", errors ? "FAIL" : "PASS", errors);
    $finish;
end

endmodule
