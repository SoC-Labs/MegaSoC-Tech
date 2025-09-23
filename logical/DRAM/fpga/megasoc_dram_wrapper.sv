
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


ddr_design u_ddr_design(
        .c0_sys_clk_n_0(ACLK),
        .c0_sys_clk_p_0(~ACLK),
        .sys_rst_0(ARESETn),
        .c0_ddr4_aresetn_0(ARESETn),


        .c0_ddr4_act_n_0(DDR4_ACT_n),
        .c0_ddr4_adr_0(DDR4_ADR),
        .c0_ddr4_ba_0(DDR4_BA),
        .c0_ddr4_bg_0(DDR4_BG),
        .c0_ddr4_ck_c_0(DDR4_CK_C),
        .c0_ddr4_ck_t_0(DDR4_CK_T),
        .c0_ddr4_cke_0(DDR4_CKE),
        .c0_ddr4_cs_n_0(DDR4_CS_N),
        .c0_ddr4_dm_dbi_n_0(DDR4_DM_DBI_N),
        .c0_ddr4_dq_0(DDR4_DQ),
        .c0_ddr4_dqs_c_0(DDR4_DQS_C),
        .c0_ddr4_dqs_t_0(DDR4_DQS_T),
        .c0_ddr4_odt_0(DDR4_ODT),
        .c0_ddr4_reset_n_0(DDR4_RESET_N),

        .c0_ddr4_s_axi_araddr_0({2'b00,DRAM_AXI.ARADDR[30:0]}),
        .c0_ddr4_s_axi_arburst_0(DRAM_AXI.ARBURST),
        .c0_ddr4_s_axi_arcache_0(DRAM_AXI.ARCACHE),
        .c0_ddr4_s_axi_arid_0(DRAM_AXI.ARID),
        .c0_ddr4_s_axi_arlen_0(DRAM_AXI.ARLEN),
        .c0_ddr4_s_axi_arlock_0(DRAM_AXI.ARLOCK),
        .c0_ddr4_s_axi_arprot_0(DRAM_AXI.ARPROT),
        .c0_ddr4_s_axi_arqos_0(4'h0),
        .c0_ddr4_s_axi_arready_0(DRAM_AXI.ARREADY),
        .c0_ddr4_s_axi_arsize_0(DRAM_AXI.ARSIZE),
        .c0_ddr4_s_axi_arvalid_0(DRAM_AXI.ARVALID),

        .c0_ddr4_s_axi_awaddr_0({2'b00,DRAM_AXI.AWADDR[30:0]}),
        .c0_ddr4_s_axi_awburst_0(DRAM_AXI.AWBURST),
        .c0_ddr4_s_axi_awcache_0(DRAM_AXI.AWCACHE),
        .c0_ddr4_s_axi_awid_0(DRAM_AXI.AWID),
        .c0_ddr4_s_axi_awlen_0(DRAM_AXI.AWLEN),
        .c0_ddr4_s_axi_awlock_0(DRAM_AXI.AWLOCK),
        .c0_ddr4_s_axi_awprot_0(DRAM_AXI.AWPROT),
        .c0_ddr4_s_axi_awqos_0(4'h0),
        .c0_ddr4_s_axi_awready_0(DRAM_AXI.AWREADY),
        .c0_ddr4_s_axi_awsize_0(DRAM_AXI.AWSIZE),
        .c0_ddr4_s_axi_awvalid_0(DRAM_AXI.AWVALID),

        .c0_ddr4_s_axi_bid_0(DRAM_AXI.BID),
        .c0_ddr4_s_axi_bready_0(DRAM_AXI.BREADY),
        .c0_ddr4_s_axi_bresp_0(DRAM_AXI.BRESP),
        .c0_ddr4_s_axi_bvalid_0(DRAM_AXI.BVALID),

        .c0_ddr4_s_axi_rdata_0(DRAM_AXI.RDATA),
        .c0_ddr4_s_axi_rid_0(DRAM_AXI.RID),
        .c0_ddr4_s_axi_rlast_0(DRAM_AXI.RLAST),
        .c0_ddr4_s_axi_rready_0(DRAM_AXI.RREADY),
        .c0_ddr4_s_axi_rresp_0(DRAM_AXI.RRESP),
        .c0_ddr4_s_axi_rvalid_0(DRAM_AXI.RVALID),

        .c0_ddr4_s_axi_wdata_0(DRAM_AXI.WDATA),
        .c0_ddr4_s_axi_wlast_0(DRAM_AXI.WLAST),
        .c0_ddr4_s_axi_wready_0(DRAM_AXI.WREADY),
        .c0_ddr4_s_axi_wstrb_0(DRAM_AXI.WSTRB),
        .c0_ddr4_s_axi_wvalid_0(DRAM_AXI.WVALID),

        .c0_ddr4_ui_clk_0(),
        .c0_ddr4_ui_clk_sync_rst_0(),
        .c0_init_calib_complete_0(),
        .dbg_bus_0(),
        .dbg_clk_0()
);



endmodule 