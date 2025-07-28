

module megasoc_SRAM_power_control(
    input  wire         PCLK,
    input  wire         PRESETn,

    apb3.subordinate    SRAM_PPU_APB,
    qchannel.master     SRAM_qchan_q,
    qchannel.master     SRAM_qchan_p,

    output wire         SRAM_RESETn
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

wire        devwarmresetn_o;
wire        devporesetn_o;
wire        devretresetn_o;

assign SRAM_RESETn = devwarmresetn_o & devporesetn_o & devretresetn_o;

assign SRAM_qchan_p.qreqn = dev_qreqn[0];
assign SRAM_qchan_q.qreqn = dev_qreqn[1];
assign dev_qacceptn  = {SRAM_qchan_q.qacceptn, SRAM_qchan_p.qacceptn};
assign dev_qdeny    = {SRAM_qchan_q.qdeny,   SRAM_qchan_p.qdeny};
assign dev_qactive  = {SRAM_qchan_q.qactive, SRAM_qchan_p.qactive};

pck600_ppu_smc_q u_pck_ppu_sram_q(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftcgen(1'b0),
    .dftisodisable(1'b0),
    .dftrstdisable(1'b0),

    .psel_i(SRAM_PPU_APB.psel),
    .penable_i(SRAM_PPU_APB.penable),
    .paddr_i(SRAM_PPU_APB.paddr),
    .pwrite_i(SRAM_PPU_APB.pwrite),
    .pwdata_i(SRAM_PPU_APB.pwdata),
    .prdata_o(SRAM_PPU_APB.prdata),
    .pready_o(SRAM_PPU_APB.pready),
    .pslverr_o(SRAM_PPU_APB.pslverr),
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
    .devwarmresetn_o(devwarmresetn_o),
    .devretresetn_o(devretresetn_o),
    .devporesetn_o(devporesetn_o),

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
) u_pck_lpd_rom_q(
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

pck600_ppu_pcsm_smc_q u_pck_ppu_sram_pcsm(
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