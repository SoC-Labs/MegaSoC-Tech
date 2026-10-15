//-----------------------------------------------------------------------------
// MegaSoC DRAM (LPDDR4) power domain control
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Same structure as megasoc_ROM_power_control: a PPU programmed over APB and a
// two-channel Q-channel distributor in sequencer mode. The two device channels
// go to the LPDDR4 subsystem (through megasoc_lpi_qchannel_adapter on the
// DDR_CLK side):
//   channel 1 : DRAM_qchan_sys  - controller AXI port low-power interface
//   channel 0 : DRAM_qchan_ddrc - controller core low-power interface
//                                 (DRAM into self-refresh)
// The sequencer asks for quiescence from the highest channel down and for
// exit from channel 0 up (PCK-600 TRM r0p3, 2.1.2), so the AXI port stops
// before the controller core, and the core is running again before the AXI
// port reopens.
// Low power is a retention-type state: DRAM contents are kept and the
// controller is not reset. The PPU reset outputs are deliberately not used,
// so leaving low power never resets the controller (no re-initialisation).
// The PPU's power-switch control (PCSM) outputs are unused: the domain has no
// power switch.
//
// The device channels are answered from the DDR_CLK domain, so their
// QACCEPTn / QDENY / QACTIVE are synchronised to PCLK here.
//
// Activity (QACTIVE, the PPU's device-activity input): only the AXI port's
// is used. It is high exactly while an AXI transaction is pending -- also
// one that arrives while the domain is in low power -- so with the PPU in
// dynamic mode any DRAM access wakes the domain (hardware wake-up). The
// controller core's cactive is high whenever the core is running (it only
// falls on entering self-refresh); it is not an activity indication and
// would keep the PPU from ever leaving ON, so it is not used. The core
// channel still accepts or refuses requests on its own.
//-----------------------------------------------------------------------------

module megasoc_DRAM_power_control(
    input  wire         PCLK,
    input  wire         PRESETn,

    apb3.subordinate    DRAM_PPU_APB,
    qchannel.master     DRAM_qchan_sys,
    qchannel.master     DRAM_qchan_ddrc
);

wire        pcsm_preq;
wire [3:0]  pcsm_pstate;
wire        pcsm_paccept;

wire [1:0]  dev_qreqn;
wire [1:0]  dev_qacceptn;
wire [1:0]  dev_qdeny;
wire [1:0]  dev_qactive;

wire        lpd_qacceptn;
wire        lpd_qreqn;
wire        lpd_qdeny;
wire        lpd_qactive;

assign DRAM_qchan_ddrc.qreqn = dev_qreqn[0];
assign DRAM_qchan_sys.qreqn  = dev_qreqn[1];

// Two-flop synchronisers for the DDR_CLK-domain responses
reg [1:0] acc_s1, acc_s2, deny_s1, deny_s2, act_s1, act_s2;
always @(posedge PCLK or negedge PRESETn)
  begin
    if (!PRESETn)
      begin
        acc_s1  <= 2'b11; acc_s2  <= 2'b11;
        deny_s1 <= 2'b00; deny_s2 <= 2'b00;
        act_s1  <= 2'b00; act_s2  <= 2'b00;
      end
    else
      begin
        acc_s1  <= {DRAM_qchan_sys.qacceptn, DRAM_qchan_ddrc.qacceptn};  acc_s2  <= acc_s1;
        deny_s1 <= {DRAM_qchan_sys.qdeny,    DRAM_qchan_ddrc.qdeny};     deny_s2 <= deny_s1;
        act_s1  <= {DRAM_qchan_sys.qactive,  1'b0};                     act_s2  <= act_s1;  // core activity not used (see above)
      end
  end

assign dev_qacceptn = acc_s2;
assign dev_qdeny    = deny_s2;
assign dev_qactive  = act_s2;

pck600_ppu_smc_q u_pck_ppu_dram_q(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftcgen(1'b0),
    .dftisodisable(1'b0),
    .dftrstdisable(1'b0),

    .psel_i(DRAM_PPU_APB.psel),
    .penable_i(DRAM_PPU_APB.penable),
    .paddr_i(DRAM_PPU_APB.paddr),
    .pwrite_i(DRAM_PPU_APB.pwrite),
    .pwdata_i(DRAM_PPU_APB.pwdata),
    .prdata_o(DRAM_PPU_APB.prdata),
    .pready_o(DRAM_PPU_APB.pready),
    .pslverr_o(DRAM_PPU_APB.pslverr),
    .pwakeup_i(PRESETn),

    .irq_o(),

    .dev_qreqn_o(lpd_qreqn),
    .dev_qacceptn_i(lpd_qacceptn),
    .dev_qdeny_i(lpd_qdeny),
    .dev_qactive_i(lpd_qactive),

    .ppuhwstat_o(),
    .devclken_o(),
    .devemuclken_o(),
    .devisolaten_o(),
    .devemuisolaten_o(),
    .devwarmresetn_o(),         // not used: low power must not reset the controller
    .devretresetn_o(),
    .devporesetn_o(),

    .pcsm_preq_o(pcsm_preq),
    .pcsm_pstate_o(pcsm_pstate),
    .pcsm_paccept_i(pcsm_paccept),

    .ppuclk_qreqn_i(PRESETn),
    .ppuclk_qacceptn_o(),
    .ppuclk_qdeny_o(),
    .ppuclk_qactive_o(),

    .ecorevnum_i(4'h0)
);

pck600_lpd_q #(
    .SEQUENCER(1),
    .NUM_QCHL(2),
    .ACTIVE_DENY(0)
) u_pck_lpd_dram_q(
    .clk(PCLK),
    .reset_n(PRESETn),

    .ctrl_qreqn_i(lpd_qreqn),
    .ctrl_qacceptn_o(lpd_qacceptn),
    .ctrl_qdeny_o(lpd_qdeny),
    .ctrl_qactive_o(lpd_qactive),

    .dev_qreqn_o(dev_qreqn),
    .dev_qacceptn_i(dev_qacceptn),
    .dev_qdeny_i(dev_qdeny),
    .dev_qactive_i(dev_qactive),

    .clk_qactive_o(),

    .dftcgen(1'b0)
);

pck600_ppu_pcsm_smc_q u_pck_ppu_dram_pcsm(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftpwrup(1'b0),
    .dftretdisable(1'b0),

    .pcsm_preq_i(pcsm_preq),
    .pcsm_pstate_i(pcsm_pstate),
    .pcsm_paccept_o(pcsm_paccept),

    .lgcpwrn_o(),
    .lgcretn_o(),
    .rampwrn_o(),
    .ramretn_o()
);

endmodule
