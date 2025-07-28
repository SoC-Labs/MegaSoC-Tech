
module megasoc_CPU_power_control(
    input  wire     PCLK,
    input  wire     PRESETn,

    apb3.subordinate    CPU_CORE_APB,
    apb3.subordinate    CPU_NEON_APB,
    apb3.subordinate    CPU_L2_APB,

    qchannel.master     CPU_CORE_q,
    output wire         CPU_CORE_WRMRSTn,
    output wire         CPU_CORE_PORESETn,

    qchannel.master     CPU_NEON_q,

    qchannel.master     CPU_L2_q,
    output wire         CPU_L2_RESETn
);
wire        CPU_CORE_pcsm_preq;
wire [15:0] CPU_CORE_pcsm_pstate;
wire        CPU_CORE_pcsm_paccept;

wire        CPU_NEON_pcsm_preq;
wire [15:0] CPU_NEON_pcsm_pstate;
wire        CPU_NEON_pcsm_paccept;

wire        CPU_L2_pcsm_preq;
wire [15:0] CPU_L2_pcsm_pstate;
wire        CPU_L2_pcsm_paccept;

pck600_ppu_ca53_core0_q u_pck_ppu_cpu_core_q(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftcgen(1'b0),
    .dftisodisable(1'b0),
    .dftrstdisable(1'b0),

    .psel_i(CPU_CORE_APB.psel),
    .penable_i(CPU_CORE_APB.penable),
    .paddr_i(CPU_CORE_APB.paddr),
    .pwrite_i(CPU_CORE_APB.pwrite),
    .pwdata_i(CPU_CORE_APB.pwdata),
    .prdata_o(CPU_CORE_APB.prdata),
    .pready_o(CPU_CORE_APB.pready),
    .pslverr_o(CPU_CORE_APB.pslverr),
    .pwakeup_i(PRESETn),

    .irq_o(),

    .dev_qreqn_o(CPU_CORE_q.qreqn),
    .dev_qacceptn_i(CPU_CORE_q.qacceptn),
    .dev_qdeny_i(CPU_CORE_q.qdeny),
    .dev_qactive_i(CPU_CORE_q.qactive),

    .ppuhwstat_o(),
    .devclken_o(),
    .devemuclken_o(),
    .devisolaten_o(),
    .devemuisolaten_o(),
    .devwarmresetn_o(CPU_CORE_WRMRSTn),
    .devretresetn_o(),
    .devporesetn_o(CPU_CORE_PORESETn),

    .pcsm_preq_o(CPU_CORE_pcsm_preq),
    .pcsm_pstate_o(CPU_CORE_pcsm_pstate),
    .pcsm_paccept_i(CPU_CORE_pcsm_paccept),

    .ppuclk_qreqn_i(PRESETn),
    .ppuclk_qacceptn_o(),
    .ppuclk_qdeny_o(),
    .ppuclk_qactive_o(),

    .ecorevnum_i(4'h0)

);

pck600_ppu_ca53_neon_q u_pck_ppu_cpu_neon_q(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftcgen(1'b0),
    .dftisodisable(1'b0),
    .dftrstdisable(1'b0),

    .psel_i(CPU_NEON_APB.psel),
    .penable_i(CPU_NEON_APB.penable),
    .paddr_i(CPU_NEON_APB.paddr),
    .pwrite_i(CPU_NEON_APB.pwrite),
    .pwdata_i(CPU_NEON_APB.pwdata),
    .prdata_o(CPU_NEON_APB.prdata),
    .pready_o(CPU_NEON_APB.pready),
    .pslverr_o(CPU_NEON_APB.pslverr),
    .pwakeup_i(PRESETn),

    .irq_o(),

    .dev_qreqn_o(CPU_NEON_q.qreqn),
    .dev_qacceptn_i(CPU_NEON_q.qacceptn),
    .dev_qdeny_i(CPU_NEON_q.qdeny),
    .dev_qactive_i(CPU_NEON_q.qactive),

    .ppuhwstat_o(),
    .devclken_o(),
    .devemuclken_o(),
    .devisolaten_o(),
    .devemuisolaten_o(),
    .devwarmresetn_o(),
    .devretresetn_o(),
    .devporesetn_o(),

    .pcsm_preq_o(CPU_NEON_pcsm_preq),
    .pcsm_pstate_o(CPU_NEON_pcsm_pstate),
    .pcsm_paccept_i(CPU_NEON_pcsm_paccept),

    .ppuclk_qreqn_i(PRESETn),
    .ppuclk_qacceptn_o(),
    .ppuclk_qdeny_o(),
    .ppuclk_qactive_o(),

    .ecorevnum_i(4'h0)

);

pck600_ppu_ca53_core0_q u_pck_ppu_cpu_L2_q(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftcgen(1'b0),
    .dftisodisable(1'b0),
    .dftrstdisable(1'b0),

    .psel_i(CPU_L2_APB.psel),
    .penable_i(CPU_L2_APB.penable),
    .paddr_i(CPU_L2_APB.paddr),
    .pwrite_i(CPU_L2_APB.pwrite),
    .pwdata_i(CPU_L2_APB.pwdata),
    .prdata_o(CPU_L2_APB.prdata),
    .pready_o(CPU_L2_APB.pready),
    .pslverr_o(CPU_L2_APB.pslverr),
    .pwakeup_i(PRESETn),

    .irq_o(),

    .dev_qreqn_o(CPU_L2_q.qreqn),
    .dev_qacceptn_i(CPU_L2_q.qacceptn),
    .dev_qdeny_i(CPU_L2_q.qdeny),
    .dev_qactive_i(CPU_L2_q.qactive),

    .ppuhwstat_o(),
    .devclken_o(),
    .devemuclken_o(),
    .devisolaten_o(),
    .devemuisolaten_o(),
    .devwarmresetn_o(),
    .devretresetn_o(),
    .devporesetn_o(CPU_L2_RESETn),

    .pcsm_preq_o(CPU_L2_pcsm_preq),
    .pcsm_pstate_o(CPU_L2_pcsm_pstate),
    .pcsm_paccept_i(CPU_L2_pcsm_paccept),

    .ppuclk_qreqn_i(PRESETn),
    .ppuclk_qacceptn_o(),
    .ppuclk_qdeny_o(),
    .ppuclk_qactive_o(),

    .ecorevnum_i(4'h0)
);

pck600_ppu_pcsm_ca53_core0_q u_pck_ppu_pcsm_cpu_core(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftpwrup(1'b0),
    .dftretdisable(1'b0),

    .pcsm_preq_i(CPU_CORE_pcsm_preq),
    .pcsm_pstate_i(CPU_CORE_pcsm_pstate),
    .pcsm_paccept_o(CPU_CORE_pcsm_paccept),

    .lgcpwrn_o(),
    .lgcretn_o(),
    .rampwrn_o(),
    .ramretn_o()
);

pck600_ppu_pcsm_ca53_neon_q u_pck_ppu_pcsm_cpu_neon(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftpwrup(1'b0),
    .dftretdisable(1'b0),

    .pcsm_preq_i(CPU_NEON_pcsm_preq),
    .pcsm_pstate_i(CPU_NEON_pcsm_pstate),
    .pcsm_paccept_o(CPU_NEON_pcsm_paccept),

    .lgcpwrn_o(),
    .lgcretn_o(),
    .rampwrn_o(),
    .ramretn_o()
);

pck600_ppu_pcsm_ca53_core0_q u_pck_ppu_pcsm_cpu_L2(
    .clk(PCLK),
    .reset_n(PRESETn),

    .dftpwrup(1'b0),
    .dftretdisable(1'b0),

    .pcsm_preq_i(CPU_L2_pcsm_preq),
    .pcsm_pstate_i(CPU_L2_pcsm_pstate),
    .pcsm_paccept_o(CPU_L2_pcsm_paccept),

    .lgcpwrn_o(),
    .lgcretn_o(),
    .rampwrn_o(),
    .ramretn_o()

);

endmodule
