//-----------------------------------------------------------------------------
// MegaSoC CPU Subsystem
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Contributors
//
// Daniel Newbrook (d.newbrook@soton.ac.uk)
// 
// Copyright � 2021-4, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Purpose:
//  CPU subsystem combines CPU (currently only A53 although may be a wrapper for
//  different CPU instantiations at some point), interrupt controller and debugger
//-----------------------------------------------------------------------------
// Modules instantiated:
//  CortexA53_1         (u_cortexa53)
//  DAPLITE             (u_daplite)
//  megasoc_irq_sync    (u_megasoc_irq_sync)
//  GIC400              (u_GIC400)

module megasoc_cpu_ss #(
  parameter NUM_GICRID_BITS = 12,
  parameter NUM_GICWID_BITS = 12,
  parameter NUM_SPIS    = 480

)(
  // Clock and Reset signals
  input  wire                       CPU_CLK,
  input  wire                       RESETn,

  input  wire                       CPU_CORE_PORESETn,
  input  wire                       CPU_CORE_WRMRSTn,
  input  wire                       CPU_L2_RESETn,

  qchannel.subordinate              CPU_CORE_q,
  qchannel.subordinate              CPU_NEON_q,
  qchannel.subordinate              CPU_L2_q,

  // ACE Interface; Clock and Configuration Signals
  input   wire                      ACLKENM,
  input   wire                      ACINACTM,
  output  wire [  7: 0]             RDMEMATTR,
  output  wire [  7: 0]             WRMEMATTR,

  // AXI Interface
  axi4.master                       CPU_AXI,

  // APB Interface Signals
  input   wire                      nPRESETDBG,
  input   wire                      PCLKENDBG,

  apb3.subordinate                  CPU_DBG_APB,

  // DAP-LITE external signals
  input  wire                       nTRST,
  input  wire                       SWCLKTCK,
  input  wire                       SWDITMS,
  input  wire                       TDI,
  output wire                       TDO,
  output wire                       nTDOEN,
  output wire                       SWDO,
  output wire                       SWDOEN,

  // GIC AXI interface signals
  axi4.subordinate                  GIC_AXI,

  input  wire [NUM_SPIS-1:0]        IRQs

);

  localparam NUM_CPUS    = 1;
  localparam NUM_EXP_SHD_INT = 64;

  wire [(NUM_CPUS-1):0] cfg_cfgend;
  wire [(NUM_CPUS-1):0] cfg_aa64naa32;
  wire [(NUM_CPUS-1):0] cfg_vinithi;
  wire [(NUM_CPUS-1):0] cfg_cfgte;
  wire [39:18]          cfg_periphbase;
  wire [7:0]            cfg_clusterid;


  assign cfg_cfgend = {NUM_CPUS{1'b0}};
  assign cfg_aa64naa32  = {NUM_CPUS{1'b1}};
  assign cfg_vinithi    = {NUM_CPUS{1'b1}};
  assign cfg_cfgte      = {NUM_CPUS{1'b1}};
  assign cfg_periphbase = 22'h000044; // Base address for GIC 0x01100000UL
  assign cfg_clusterid  = 8'h00;


  wire [(NUM_CPUS-1):0] nFIQCPU;
  wire [(NUM_CPUS-1):0] nIRQCPU;
  wire [(NUM_CPUS-1):0] nVFIQCPU;
  wire [(NUM_CPUS-1):0] nVIRQCPU;
  wire [(NUM_CPUS-1):0] nFIQOUT;
  wire [(NUM_CPUS-1):0] nIRQOUT;

  wire [(NUM_CPUS-1):0] nCNTPSIRQ;
  wire [(NUM_CPUS-1):0] nCNTPNSIRQ;
  wire [(NUM_CPUS-1):0] nCNTVIRQ;
  wire [(NUM_CPUS-1):0] nCNTHPIRQ;

  wire [31:0]           PRDATADBG_CPU;
  wire                  PREADYDBG_CPU;
  wire                  PSLVERRDBG_CPU;
  wire [31:2]           PADDRDBG_CPU;
  wire                  PSELDBG_CPU;
  wire                  PWRITEDBG_CPU;
  wire                  PENABLEDBG_CPU;
  wire [31:0]           PWDATADBG_CPU;

  reg [63:0]            CNTVALUEB;

  always @(posedge CPU_CLK or negedge CPU_CORE_PORESETn) begin
    if(~CPU_CORE_PORESETn)
      CNTVALUEB <= 64'd0;
    else
      CNTVALUEB <= CNTVALUEB + 1;
  end

  assign CPU_AXI.AWID[5] = 1'b0;
  //assign CPU_AXI.WID[5] = 1'b0;
  //assign CPU_AXI.BID[5] = 1'b0;
  CORTEXA53
    u_cortexa53
      (// Clocks and resets
       .CLKIN                       (CPU_CLK),
       .nCPUPORESET                 (CPU_CORE_PORESETn),
       .nCORERESET                  (CPU_CORE_WRMRSTn),
       .nPRESETDBG                  (CPU_CORE_PORESETn),
       .nL2RESET                    (CPU_L2_RESETn),
       .nMBISTRESET                 (1'b1),       // No MBIST
       .L2RSTDISABLE                (1'b0),
       .WARMRSTREQ                  (),

       // Configuration signals
       .CFGEND                      (cfg_cfgend),
       .VINITHI                     (cfg_vinithi),
       .CFGTE                       (cfg_cfgte),
       .CP15SDISABLE                ({NUM_CPUS{1'b0}}),
       .CLUSTERIDAFF1               (cfg_clusterid),
       .CLUSTERIDAFF2               (8'h00),
       .AA64nAA32                   (cfg_aa64naa32),
       .RVBARADDR0                  ({38{1'b0}}),

       // Interrupt signals
       .nFIQ                        (nFIQCPU),
       .nIRQ                        (nIRQCPU),
       .nSEI                        ({NUM_CPUS{1'b1}}),
       .nVFIQ                       (nVFIQCPU),
       .nVIRQ                       (nVIRQCPU),
       .nVSEI                       ({NUM_CPUS{1'b1}}),
       .nREI                        ({NUM_CPUS{1'b1}}),
       .nVCPUMNTIRQ                 (),
       .PERIPHBASE                  (cfg_periphbase), // Base address for GIC
       .GICCDISABLE                 (1'b1),
       .ICDTVALID                   (1'b0),
       .ICDTREADY                   (),
       .ICDTDATA                    ({16{1'b0}}),
       .ICDTLAST                    (1'b0),
       .ICDTDEST                    (2'b00),
       .ICCTVALID                   (),
       .ICCTREADY                   (1'b0),
       .ICCTDATA                    (),
       .ICCTLAST                    (),
       .ICCTID                      (),

       // Generic timer signals
       .CNTVALUEB                   (CNTVALUEB),
       .CNTCLKEN                    (1'b1),
       .nCNTPNSIRQ                  (nCNTPNSIRQ),
       .nCNTPSIRQ                   (nCNTPSIRQ),
       .nCNTVIRQ                    (nCNTVIRQ),
       .nCNTHPIRQ                   (nCNTHPIRQ),

       // Power management signals
       .CLREXMONREQ                 (1'b0),
       .CLREXMONACK                 (),
       .EVENTI                      (1'b0),
       .EVENTO                      (),
       .STANDBYWFI                  (),
       .STANDBYWFE                  (),
       .STANDBYWFIL2                (),
       .L2FLUSHREQ                  (1'b0),
       .L2FLUSHDONE                 (),
       .SMPEN                       (),

       .CPUQACTIVE                  (CPU_CORE_q.qactive),
       .CPUQREQn                    (CPU_CORE_q.qreqn),
       .CPUQDENY                    (CPU_CORE_q.qdeny),
       .CPUQACCEPTn                 (CPU_CORE_q.qacceptn),
       .NEONQACTIVE                 (CPU_NEON_q.qactive),
       .NEONQREQn                   (CPU_NEON_q.qreqn),
       .NEONQDENY                   (CPU_NEON_q.qdeny),
       .NEONQACCEPTn                (CPU_NEON_q.qacceptn),
       .L2QACTIVE                   (CPU_L2_q.qactive),
       .L2QREQn                     (CPU_L2_q.qreqn),
       .L2QDENY                     (CPU_L2_q.qdeny),
       .L2QACCEPTn                  (CPU_L2_q.qacceptn),


       // ACE/Skyros interface signals
       //   NB. the execution testbench bus model is a simple single master,
       //   single slave system and therefore cache maintenance operations are
       //   disabled. The bus model is a simple in-order device so barriers are
       //   also disabled in the ACE configuration. However, in a Skyros system
       //   SYSBARDISABLE must be HIGH, so the Skyros subsystem does process
       //   barriers.
       .nEXTERRIRQ                  (),
       .BROADCASTCACHEMAINT         (1'b0),
       .BROADCASTINNER              (1'b0),
       .BROADCASTOUTER              (1'b0),
       .SYSBARDISABLE               (1'b1),

       // ACE interface
       //   - Clock and configuration signals
       .ACLKENM                     (1'b1),
       .ACINACTM                    (1'b1),
       .RDMEMATTR                   (),
       .WRMEMATTR                   (),
       //  - Write address channel signals
       .AWREADYM                    (CPU_AXI.AWREADY),
       .AWVALIDM                    (CPU_AXI.AWVALID),
       .AWIDM                       (CPU_AXI.AWID[4:0]),
       .AWADDRM                     (CPU_AXI.AWADDR),
       .AWLENM                      (CPU_AXI.AWLEN),
       .AWSIZEM                     (CPU_AXI.AWSIZE),
       .AWBURSTM                    (CPU_AXI.AWBURST),
       .AWBARM                      (),
       .AWDOMAINM                   (),

       .AWLOCKM                     (CPU_AXI.AWLOCK),
       .AWCACHEM                    (CPU_AXI.AWCACHE),
       .AWPROTM                     (CPU_AXI.AWPROT),
       .AWSNOOPM                    (),
       .AWUNIQUEM                   (),
       //  - Write data channel signals
       .WREADYM                     (CPU_AXI.WREADY),
       .WVALIDM                     (CPU_AXI.WVALID),
       .WIDM                        (),
       .WDATAM                      (CPU_AXI.WDATA),
       .WSTRBM                      (CPU_AXI.WSTRB),
       .WLASTM                      (CPU_AXI.WLAST),
       //  - Write response channel signals
       .BREADYM                     (CPU_AXI.BREADY),
       .BVALIDM                     (CPU_AXI.BVALID),
       .BIDM                        (CPU_AXI.BID[4:0]),
       .BRESPM                      (CPU_AXI.BRESP),
       //  - Read address channel signals
       .ARREADYM                    (CPU_AXI.ARREADY),
       .ARVALIDM                    (CPU_AXI.ARVALID),
       .ARIDM                       (CPU_AXI.ARID),
       .ARADDRM                     (CPU_AXI.ARADDR),
       .ARLENM                      (CPU_AXI.ARLEN),
       .ARSIZEM                     (CPU_AXI.ARSIZE),
       .ARBURSTM                    (CPU_AXI.ARBURST),
       .ARBARM                      (),
       .ARDOMAINM                   (),

       .ARLOCKM                     (CPU_AXI.ARLOCK),
       .ARCACHEM                    (CPU_AXI.ARCACHE),
       .ARPROTM                     (CPU_AXI.ARPROT),
       .ARSNOOPM                    (),
       //  - Read data channel signals
       .RREADYM                     (CPU_AXI.RREADY),
       .RVALIDM                     (CPU_AXI.RVALID),
       .RIDM                        (CPU_AXI.RID),
       .RDATAM                      (CPU_AXI.RDATA),
       .RRESPM                      ({2'b00, CPU_AXI.RRESP}),
       .RLASTM                      (CPU_AXI.RLAST),
       //  - Coherency address channel signals
       .ACREADYM                    (),
       .ACVALIDM                    (1'b0),
       .ACADDRM                     (44'h0),
       .ACPROTM                     (3'b000),
       .ACSNOOPM                    (4'h0),
       //  - Coherency response channel signals
       .CRREADYM                    (1'b0),
       .CRVALIDM                    (),
       .CRRESPM                     (),
       //  - Coherency data channel signals
       .CDREADYM                    (1'b0),
       .CDVALIDM                    (),
       .CDDATAM                     (),
       .CDLASTM                     (),
       //  - Read/write acknowledge signals
       .RACKM                       (),
       .WACKM                       (),

       // Debug APB interface signals
       .PCLKENDBG                   (1'b1),
       .PSELDBG                     (PSELDBG_CPU),
       .PADDRDBG                    (PADDRDBG_CPU[21:2]),
       .PADDRDBG31                  (PADDRDBG_CPU[31]),
       .PENABLEDBG                  (PENABLEDBG_CPU),
       .PWRITEDBG                   (PWRITEDBG_CPU),
       .PWDATADBG                   (PWDATADBG_CPU),
       .PRDATADBG                   (PRDATADBG_CPU),
       .PREADYDBG                   (PREADYDBG_CPU),
       .PSLVERRDBG                  (PSLVERRDBG_CPU),

       // Miscellaneous debug signals
       .DBGROMADDR                  (28'h0060000), //0x60000000
       .DBGROMADDRV                 (1'b1),
       .DBGACK                      (),
       .nCOMMIRQ                    (),
       .COMMRX                      (),
       .COMMTX                      (),
       .EDBGRQ                      ({NUM_CPUS{1'b0}}),
       .DBGEN                       ({NUM_CPUS{1'b1}}),
       .NIDEN                       ({NUM_CPUS{1'b1}}),
       .SPIDEN                      ({NUM_CPUS{1'b1}}),
       .SPNIDEN                     ({NUM_CPUS{1'b1}}),
       .DBGRSTREQ                   (),
       .DBGNOPWRDWN                 (),
       .DBGPWRDUP                   ({NUM_CPUS{1'b1}}),
       .DBGPWRUPREQ                 (),
       .DBGL1RSTDISABLE             (1'b0),

       // ATB interface signals
       .ATCLKEN                     (1'b1),
       .ATREADYM0                   (1'b1),
       .AFVALIDM0                   (1'b0),
       .ATDATAM0                    (),
       .ATVALIDM0                   (),
       .ATBYTESM0                   (),
       .AFREADYM0                   (),
       .ATIDM0                      (),


       // Miscellaneous ETM signals
       .SYNCREQM0                   (1'b0),
       .TSVALUEB                    (64'd0),

       // CTI interface signals:
       .CTICHIN                     (4'h0),
       .CTICHOUTACK                 (4'h0),
       .CTICHOUT                    (),
       .CTICHINACK                  (),
       .CISBYPASS                   (1'b1),
       .CIHSBYPASS                  ({4{1'b1}}),
       .CTIIRQ                      (),
       .CTIIRQACK                   ({NUM_CPUS{1'b1}}),

       // PMU signals
       .nPMUIRQ                     (),
       .PMUEVENT0                   (),

       // DFT signals
       .DFTSE                       (1'b0),
       .DFTRSTDISABLE               (1'b0),
       .DFTRAMHOLD                  (1'b0),
       .DFTMCPHOLD                  (1'b0),

       // MBIST interface signals
       .MBISTREQ                    (1'b0)
      );


DAPLITE u_daplite(
// External power-on reset

  .nPOTRST(RESETn),        // Power-on reset
  
// External JTAG/SW connections to SWJDP-DP

  .nTRST(nTRST),          // TAP Reset (Asynchronous) 
  .SWCLKTCK(SWCLKTCK),       // TAP Clock 
  .SWDITMS(SWDITMS),        // TAP Mode/ SW Data In 
  .TDI(TDI),            // JTAG TAP Data In .   
  .TDO(TDO),            // Asynchronous JTAG TAP Data Out
  .nTDOEN(nTDOEN),         // Asynchronous JTAG TAP Data Out Enable
  .SWDO(SWDO),           // SW Data Out
  .SWDOEN(SWDOEN),         // SW Data Out Enable

// SWJ-DP Status
  .JTAGNSW(),        // Current TAP Mode of operation
  .JTAGTOP(),        // JTAG TAP controller in one of top 4 states;
                                // i.e. TLR, RTI, Sel-DR or Sel-IR

// SWJ-DP power and reset controller interface

  .CDBGPWRUPACK(1'b1),   // Debug Power Domain power-up acknowledge
  .CSYSPWRUPACK(1'b1),   // System Power Domain power-up acknowledge
  .CDBGRSTACK(1'b1),     // Debug reset acknowledge from reset controller
  .CDBGPWRUPREQ(),   // Debug Power Domain power-up request
  .CSYSPWRUPREQ(),   // System Power Domain power-up request
  .CDBGRSTREQ(),     // Debug reset request to reset controller

// Power Domain controls
  
  .nCDBGPWRDN(1'b1),      // Debug infrastructure power-down control
  .nCSOCPWRDN(1'b1),      // External system (SOC domain) 
                                 // power-down control  

  // APB-AP device enable input
  .DEVICEEN(1'b1),       // Device enable

  // Software Access Enable
  .DBGSWENABLE(),    // Provided to block/grant software access to
                                // things other than the APB-Mux
  
  // APB-MUX port connections
  // System APB port

  // System Clock / Reset Pins
  .PCLKSYS(CPU_CLK),         // System APB clock (typically HCLK)
  .PCLKENSYS(1'b1),       // Enable term for PCLKSYS domain
  .PRESETSYSn(RESETn),      // Resets the APB interface connected to
                                 // the system bus
   
  // System Slave port (driven by system APB)
  .PADDRSYS(CPU_DBG_APB.paddr[30:2]),         // System APB address bus
  .PSELSYS(CPU_DBG_APB.psel),          // System APB select
  .PWRITESYS(pwrite),        // System APB write access
  .PENABLESYS(CPU_DBG_APB.penable),       // System APB enable signal - indicates second
                  // and subsequent cycles of an APB transfer
  .PWDATASYS(CPU_DBG_APB.pwdata),        // System APB Write data bus
  .PRDATASYS(CPU_DBG_APB.prdata),        // System APB write data bus
  .PREADYSYS(CPU_DBG_APB.pready),        // System APB Ready signal
  .PSLVERRSYS(CPU_DBG_APB.pslverr),       // System APB transfer error signal
  
  //Debug APB port
  
  .PCLKDBG(CPU_CLK),           // Debug APB clock
  .PCLKENDBG(1'b1),         // Enable term for PCLKDBG domain
  .PRESETDBGn(RESETn),        // Reset for arbitration logic,
                                   // APB-AP slave port and
                                   // Debug-APB master port   
  .PRDATADBG(PRDATADBG_CPU),         // CoreSight Peripheral Read data bus
  .PREADYDBG(PREADYDBG_CPU),         // Debug-APB Ready signal
  .PSLVERRDBG(PSLVERRDBG_CPU),        // Debug-APB transfer error signal
  .PADDRDBG(PADDRDBG_CPU),          // Debug-APB address bus
  .PSELDBG(PSELDBG_CPU),           // Debug-APB select
  .PWRITEDBG(PWRITEDBG_CPU),         // Debug-APB write access
  .PENABLEDBG(PENABLEDBG_CPU),        // Debug-APB enable signal
  .PWDATADBG(PWDATADBG_CPU),         // Debug-APB write data bus

// Scan enable connection

  .SE(1'b0)             // Scan Enable

);

wire [NUM_SPIS-1:0] IRQs_ss;

megasoc_irq_sync #(.NUM_SPIS(NUM_SPIS)) u_megasoc_irq_sync(
  .CLK(CPU_CLK),
  .RESETn(RESETn),
  .SPI_i(IRQs),
  .SPI_o(IRQs_ss)
);


GIC400 #(
  .NUM_CPUS(NUM_CPUS),
  .NUM_SPIS(NUM_SPIS),
  .NUM_WID_BITS(NUM_GICWID_BITS),
  .NUM_RID_BITS(NUM_GICRID_BITS)
) u_GIC400(
  .CLK(CPU_CLK),
  .nRESET(RESETn),
  .DFTRSTDISABLE(1'b0),
  .DFTSE(1'b0),

  .CFGSDISABLE(1'b0),

  .ARID(GIC_AXI.ARID),
  .ARADDR(GIC_AXI.ARADDR[14:0]),
  .ARLEN(GIC_AXI.ARLEN),
  .ARSIZE(GIC_AXI.ARSIZE),
  .ARBURST(GIC_AXI.ARBURST),
  .ARPROT(GIC_AXI.ARPROT),
  .ARUSER(3'h0),
  .ARVALID(GIC_AXI.ARVALID),
  .ARREADY(GIC_AXI.ARREADY),

  .RID(GIC_AXI.RID),
  .RDATA(GIC_AXI.RDATA),
  .RLAST(GIC_AXI.RLAST),
  .RRESP(GIC_AXI.RRESP),
  .RVALID(GIC_AXI.RVALID),
  .RREADY(GIC_AXI.RREADY),

  .AWID(GIC_AXI.AWID),
  .AWADDR(GIC_AXI.AWADDR[14:0]),
  .AWLEN(GIC_AXI.AWLEN),
  .AWSIZE(GIC_AXI.AWSIZE),
  .AWBURST(GIC_AXI.AWBURST),
  .AWPROT(GIC_AXI.AWPROT),
  .AWUSER(3'h0),
  .AWVALID(GIC_AXI.AWVALID),
  .AWREADY(GIC_AXI.AWREADY),

  .WDATA(GIC_AXI.WDATA),
  .WSTRB(GIC_AXI.WSTRB),
  .WVALID(GIC_AXI.WVALID),
  .WREADY(GIC_AXI.WREADY),

  .BID(GIC_AXI.BID),
  .BRESP(GIC_AXI.BRESP),
  .BVALID(GIC_AXI.BVALID),
  .BREADY(GIC_AXI.BREADY),

  .IRQS(IRQs_ss),

  .nLEGACYFIQ(1'b1),
  .nLEGACYIRQ(1'b1),
  .nCNTPSIRQ(nCNTPSIRQ),
  .nCNTPNSIRQ(nCNTPNSIRQ),
  .nCNTVIRQ(nCNTVIRQ),
  .nCNTHPIRQ(nCNTHPIRQ),

  .nIRQCPU(nIRQCPU),
  .nFIQCPU(nFIQCPU),
  .nVIRQCPU(nVIRQCPU),
  .nVFIQCPU(nVFIQCPU),

  .nIRQOUT(nIRQOUT),
  .nFIQOUT(nFIQOUT)
);


endmodule
