

module megasoc_ROM_power_control(
    input  wire         PCLK,
    input  wire         PRESETn,

    apb3.subordinate    ROM_PPU_APB,
    qchannel.master     ROM_qchan_q,
    qchannel.master     ROM_qchan_p
);

wire                 pcsm_preq;
wire [3:0]           pcsm_pstate;
wire                 pcsm_paccept;

wire [1:0] dev_qreqn;
wire [1:0] dev_qacceptn;
wire [1:0] dev_qdeny;
wire [1:0] dev_qactive;

assign ROM_qchan_p.qreqn = dev_qreqn[1];
assign ROM_qchan_q.qreqn = dev_qreqn[0];
assign dev_qacceptn  = {ROM_qchan_p.qacceptn, ROM_qchan_q.qacceptn};
assign dev_qdeny    = {ROM_qchan_p.qdeny,   ROM_qchan_q.qdeny};
assign dev_qactive  = {ROM_qchan_p.qactive, ROM_qchan_q.qactive};

pck600_ppu_smc_q u_pck_ppu_rom_q(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftcgen(1'b0),
    .dftisodisable(1'b0),
    .dftrstdisable(1'b0),

    .psel_i(ROM_PPU_APB.psel),
    .penable_i(ROM_PPU_APB.penable),
    .paddr_i(ROM_PPU_APB.paddr),
    .pwrite_i(ROM_PPU_APB.pwrite),
    .pwdata_i(ROM_PPU_APB.pwdata),
    .prdata_o(ROM_PPU_APB.prdata),
    .pready_o(ROM_PPU_APB.pready),
    .pslverr_o(ROM_PPU_APB.pslverr),
    .pwakeup_i(PRESETn),

    .irq_o(),

    .dev_qreqn_o(dev_qreqn),
    .dev_qacceptn_i(dev_qacceptn),
    .dev_qdeny_i(dev_qdeny),
    .dev_qactive_i(dev_qactive),

    .ppuhwstat_o(),
    .devclken_o(),
    .devemuclken_o(),
    .devisolaten_o(),
    .devemuisolaten_o(),
    .devwarmresetn_o(),
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


pck600_ppu_pcsm_smc_q u_pck_ppu_rom_pcsm(
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