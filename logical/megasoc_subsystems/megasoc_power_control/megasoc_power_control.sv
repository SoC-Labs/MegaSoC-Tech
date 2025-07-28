module megasoc_power_control(
    input  wire         PCLK,
    input  wire         PRESETn,

    apb3.subordinate    PCK_APB,
    qchannel.master     ROM_qchan_q,
    qchannel.master     ROM_qchan_p,
    output wire         ROM_RESETn,

    qchannel.master     SRAM_qchan_q,
    qchannel.master     SRAM_qchan_p,
    output wire         SRAM_RESETn,

    qchannel.master     CPU_CORE_q,
    output wire         CPU_CORE_WRMRSTn,
    output wire         CPU_CORE_PORESETn,

    qchannel.master     CPU_NEON_q,

    qchannel.master     CPU_L2_q,
    output wire         CPU_L2_RESETn
);

apb3    ROM_PPU_APB();
apb3    SRAM_PPU_APB();
apb3    CPU_CORE_APB();
apb3    CPU_NEON_APB();
apb3    CPU_L2_APB();

// Assign direct APB connection (pwrite, paddr, etc)
assign ROM_PPU_APB.paddr    =   PCK_APB.paddr;
assign ROM_PPU_APB.penable  =   PCK_APB.penable;
assign ROM_PPU_APB.pwrite   =   PCK_APB.pwrite;
assign ROM_PPU_APB.pwdata   =   PCK_APB.pwdata;

assign SRAM_PPU_APB.paddr    =   PCK_APB.paddr;
assign SRAM_PPU_APB.penable  =   PCK_APB.penable;
assign SRAM_PPU_APB.pwrite   =   PCK_APB.pwrite;
assign SRAM_PPU_APB.pwdata   =   PCK_APB.pwdata;

assign CPU_CORE_APB.paddr    =   PCK_APB.paddr;
assign CPU_CORE_APB.penable  =   PCK_APB.penable;
assign CPU_CORE_APB.pwrite   =   PCK_APB.pwrite;
assign CPU_CORE_APB.pwdata   =   PCK_APB.pwdata;

assign CPU_NEON_APB.paddr    =   PCK_APB.paddr;
assign CPU_NEON_APB.penable  =   PCK_APB.penable;
assign CPU_NEON_APB.pwrite   =   PCK_APB.pwrite;
assign CPU_NEON_APB.pwdata   =   PCK_APB.pwdata;

assign CPU_L2_APB.paddr    =   PCK_APB.paddr;
assign CPU_L2_APB.penable  =   PCK_APB.penable;
assign CPU_L2_APB.pwrite   =   PCK_APB.pwrite;
assign CPU_L2_APB.pwdata   =   PCK_APB.pwdata;

// CMSDK APB Slave Mux (from Corstone 101) 
cmsdk_apb_slave_mux #(
    .PORT0_ENABLE(1),
    .PORT1_ENABLE(1),
    .PORT2_ENABLE(1),
    .PORT3_ENABLE(1),
    .PORT4_ENABLE(1),
    .PORT5_ENABLE(0),
    .PORT6_ENABLE(0),
    .PORT7_ENABLE(0),
    .PORT8_ENABLE(0),
    .PORT9_ENABLE(0),
    .PORT10_ENABLE(0),
    .PORT11_ENABLE(0),
    .PORT12_ENABLE(0),
    .PORT13_ENABLE(0),
    .PORT14_ENABLE(0),
    .PORT15_ENABLE(0)
    ) u_apb_slave_mux (
    .DECODE4BIT(PCK_APB.paddr[15:12]),
    .PSEL(PCK_APB.psel),

    .PSEL0(ROM_PPU_APB.psel),
    .PREADY0(ROM_PPU_APB.pready),
    .PRDATA0(ROM_PPU_APB.prdata),
    .PSLVERR0(ROM_PPU_APB.pslverr),

    .PSEL1(SRAM_PPU_APB.psel),
    .PREADY1(SRAM_PPU_APB.pready),
    .PRDATA1(SRAM_PPU_APB.prdata),
    .PSLVERR1(SRAM_PPU_APB.pslverr),

    .PSEL2(CPU_CORE_APB.psel),
    .PREADY2(CPU_CORE_APB.pready),
    .PRDATA2(CPU_CORE_APB.prdata),
    .PSLVERR2(CPU_CORE_APB.pslverr),

    .PSEL3(CPU_NEON_APB.psel),
    .PREADY3(CPU_NEON_APB.pready),
    .PRDATA3(CPU_NEON_APB.prdata),
    .PSLVERR3(CPU_NEON_APB.pslverr),

    .PSEL4(CPU_L2_APB.psel),
    .PREADY4(CPU_L2_APB.pready),
    .PRDATA4(CPU_L2_APB.prdata),
    .PSLVERR4(CPU_L2_APB.pslverr),

    .PSEL5(),
    .PREADY5(1'b0),
    .PRDATA5(32'd0),
    .PSLVERR5(1'b0),

    .PSEL6(),
    .PREADY6(1'b0),
    .PRDATA6(32'd0),
    .PSLVERR6(1'b0),

    .PSEL7(),
    .PREADY7(1'b0),
    .PRDATA7(32'd0),
    .PSLVERR7(1'b0),

    .PSEL8(),
    .PREADY8(1'b0),
    .PRDATA8(32'd0),
    .PSLVERR8(1'b0),

    .PSEL9(),
    .PREADY9(1'b0),
    .PRDATA9(32'd0),
    .PSLVERR9(1'b0),

    .PSEL10(),
    .PREADY10(1'b0),
    .PRDATA10(32'd0),
    .PSLVERR10(1'b0),

    .PSEL11(),
    .PREADY11(1'b0),
    .PRDATA11(32'd0),
    .PSLVERR11(1'b0),

    .PSEL12(),
    .PREADY12(1'b0),
    .PRDATA12(32'd0),
    .PSLVERR12(1'b0),

    .PSEL13(),
    .PREADY13(1'b0),
    .PRDATA13(32'd0),
    .PSLVERR13(1'b0),

    .PSEL14(),
    .PREADY14(1'b0),
    .PRDATA14(32'd0),
    .PSLVERR14(1'b0),

    .PSEL15(),
    .PREADY15(1'b0),
    .PRDATA15(32'd0),
    .PSLVERR15(1'b0),

    .PREADY(PCK_APB.pready),
    .PRDATA(PCK_APB.prdata),
    .PSLVERR(PCK_APB.pslverr)
);


megasoc_ROM_power_control u_megasoc_ROM_power_control(
    .PCLK(PCLK),
    .PRESETn(PRESETn),
    .ROM_PPU_APB(ROM_PPU_APB),
    .ROM_qchan_q(ROM_qchan_q),
    .ROM_qchan_p(ROM_qchan_p),
    .ROM_RESETn(ROM_RESETn)
);

megasoc_SRAM_power_control u_megasoc_SRAM_power_control(
    .PCLK(PCLK),
    .PRESETn(PRESETn),
    .SRAM_PPU_APB(SRAM_PPU_APB),
    .SRAM_qchan_q(SRAM_qchan_q),
    .SRAM_qchan_p(SRAM_qchan_p),
    .SRAM_RESETn(SRAM_RESETn)
);

megasoc_CPU_power_control u_megasoc_CPU_power_control(
    .PCLK(PCLK),
    .PRESETn(PRESETn),

    .CPU_CORE_APB(CPU_CORE_APB),
    .CPU_NEON_APB(CPU_NEON_APB),
    .CPU_L2_APB(CPU_L2_APB),

    .CPU_CORE_q(CPU_CORE_q),
    .CPU_CORE_PORESETn(CPU_CORE_PORESETn),
    .CPU_CORE_WRMRSTn(CPU_CORE_WRMRSTn),

    .CPU_NEON_q(CPU_NEON_q),

    .CPU_L2_q(CPU_L2_q),
    .CPU_L2_RESETn(CPU_L2_RESETn)
);

endmodule
