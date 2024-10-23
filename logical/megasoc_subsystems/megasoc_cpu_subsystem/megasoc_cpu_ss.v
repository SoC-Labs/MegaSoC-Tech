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

  // ACE Interface; Clock and Configuration Signals
  input   wire                      ACLKENM,
  input   wire                      ACINACTM,
  output  wire [  7: 0]             RDMEMATTR,
  output  wire [  7: 0]             WRMEMATTR,
  // ACE Interface; Write Address Channel Signals
  input   wire                      AWREADYM,
  output  wire                      AWVALIDM,
  output  wire  [  5: 0]            AWIDM,
  output  wire  [ 43: 0]            AWADDRM,
  output  wire  [  7: 0]            AWLENM,
  output  wire  [  2: 0]            AWSIZEM,
  output  wire  [  1: 0]            AWBURSTM,
  output  wire                      AWLOCKM,
  output  wire  [  3: 0]            AWCACHEM,
  output  wire  [  2: 0]            AWPROTM,
  // ACE Interface; Write Data Channel Signals
  input   wire                      WREADYM,
  output  wire                      WVALIDM,
  output  wire  [  5: 0]            WIDM,
  output  wire  [127: 0]            WDATAM,
  output  wire  [ 15: 0]            WSTRBM,
  output  wire                      WLASTM,
  // ACE Interface; Write Response Channel Signals
  output  wire                      BREADYM,
  input   wire                      BVALIDM,
  input   wire  [  5: 0]            BIDM,
  input   wire  [  1: 0]            BRESPM,
  // ACE Interface; Read Address Channel Signals
  input   wire                      ARREADYM,
  output  wire                      ARVALIDM,
  output  wire  [  5: 0]            ARIDM,
  output  wire  [ 43: 0]            ARADDRM,
  output  wire  [  7: 0]            ARLENM,
  output  wire  [  2: 0]            ARSIZEM,
  output  wire  [  1: 0]            ARBURSTM,
  output  wire                      ARLOCKM,
  output  wire  [  3: 0]            ARCACHEM,
  output  wire  [  2: 0]            ARPROTM,
  // ACE Interface; Read Data Channel Signals
  output  wire                      RREADYM,
  input   wire                      RVALIDM,
  input   wire  [  5: 0]            RIDM,
  input   wire  [127: 0]            RDATAM,
  input   wire  [  1: 0]            RRESPM,
  input   wire                      RLASTM,
  // APB Interface Signals
  input   wire                      nPRESETDBG,
  input   wire                      PCLKENDBG,
  input   wire                      PSELDBG,
  input   wire  [ 30: 2]            PADDRDBG,
  input   wire                      PENABLEDBG,
  input   wire                      PWRITEDBG,
  input   wire  [ 31: 0]            PWDATADBG,
  output  wire  [ 31: 0]            PRDATADBG,
  output  wire                      PREADYDBG,
  output  wire                      PSLVERRDBG,

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
  // AXI read address channel
  input  wire [NUM_GICRID_BITS-1:0] GIC_ARID,
  input  wire                [14:0] GIC_ARADDR,
  input  wire                 [7:0] GIC_ARLEN,
  input  wire                 [2:0] GIC_ARSIZE,
  input  wire                 [1:0] GIC_ARBURST,
  input  wire                 [2:0] GIC_ARPROT,
  input  wire                 [2:0] GIC_ARUSER,
  input  wire                       GIC_ARVALID,
  output wire                       GIC_ARREADY,

  // AXI read data channel
  output wire [NUM_GICRID_BITS-1:0] GIC_RID,
  output wire                [31:0] GIC_RDATA,
  output wire                       GIC_RLAST,
  output wire                 [1:0] GIC_RRESP,
  output wire                       GIC_RVALID,
  input  wire                       GIC_RREADY,

  // AXI write address channel
  input  wire [NUM_GICWID_BITS-1:0] GIC_AWID,
  input  wire                [14:0] GIC_AWADDR,
  input  wire                 [7:0] GIC_AWLEN,
  input  wire                 [2:0] GIC_AWSIZE,
  input  wire                 [1:0] GIC_AWBURST,
  input  wire                 [2:0] GIC_AWPROT,
  input  wire                 [2:0] GIC_AWUSER,
  input  wire                       GIC_AWVALID,
  output wire                       GIC_AWREADY,

  // AXI write data channel
  input  wire                [31:0] GIC_WDATA,
  input  wire                 [3:0] GIC_WSTRB,
  input  wire                       GIC_WVALID,
  output wire                       GIC_WREADY,

  // AXI write response channel
  output wire [NUM_GICWID_BITS-1:0] GIC_BID,
  output wire                 [1:0] GIC_BRESP,
  output wire                       GIC_BVALID,
  input  wire                       GIC_BREADY,

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
  assign cfg_periphbase = 22'b0000000001000000000000;
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

  assign AWIDM[5] = 1'b0;
  assign WIDM[5] = 1'b0;
  assign BIDM[5] = 1'b0;
  CortexA53_1
    u_cortexa53
      (// Clocks and resets
       .CLKIN                       (CPU_CLK),
       .nCPUPORESET                 ({NUM_CPUS{RESETn}}),
       .nCORERESET                  ({NUM_CPUS{RESETn}}),
       .nPRESETDBG                  (RESETn),
       .nL2RESET                    (RESETn),
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
       .PERIPHBASE                  (cfg_periphbase),
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
       .CNTVALUEB                   (64'd0),
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
       .CPUQACTIVE                  (),
       .CPUQREQn                    ({NUM_CPUS{1'b1}}),
       .CPUQDENY                    (),
       .CPUQACCEPTn                 (),
       .NEONQACTIVE                 (),
       .NEONQREQn                   ({NUM_CPUS{1'b1}}),
       .NEONQDENY                   (),
       .NEONQACCEPTn                (),
       .L2QACTIVE                   (),
       .L2QREQn                     (1'b1),
       .L2QDENY                     (),
       .L2QACCEPTn                  (),


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
       .AWREADYM                    (AWREADYM),
       .AWVALIDM                    (AWVALIDM),
       .AWIDM                       (AWIDM[4:0]),
       .AWADDRM                     (AWADDRM),
       .AWLENM                      (AWLENM),
       .AWSIZEM                     (AWSIZEM),
       .AWBURSTM                    (AWBURSTM),
       .AWBARM                      (),
       .AWDOMAINM                   (),

       .AWLOCKM                     (AWLOCKM),
       .AWCACHEM                    (AWCACHEM),
       .AWPROTM                     (AWPROTM),
       .AWSNOOPM                    (),
       .AWUNIQUEM                   (),
       //  - Write data channel signals
       .WREADYM                     (WREADYM),
       .WVALIDM                     (WVALIDM),
       .WIDM                        (WIDM[4:0]),
       .WDATAM                      (WDATAM),
       .WSTRBM                      (WSTRBM),
       .WLASTM                      (WLASTM),
       //  - Write response channel signals
       .BREADYM                     (BREADYM),
       .BVALIDM                     (BVALIDM),
       .BIDM                        (BIDM[4:0]),
       .BRESPM                      (BRESPM),
       //  - Read address channel signals
       .ARREADYM                    (ARREADYM),
       .ARVALIDM                    (ARVALIDM),
       .ARIDM                       (ARIDM),
       .ARADDRM                     (ARADDRM),
       .ARLENM                      (ARLENM),
       .ARSIZEM                     (ARSIZEM),
       .ARBURSTM                    (ARBURSTM),
       .ARBARM                      (),
       .ARDOMAINM                   (),

       .ARLOCKM                     (ARLOCKM),
       .ARCACHEM                    (ARCACHEM),
       .ARPROTM                     (ARPROTM),
       .ARSNOOPM                    (),
       //  - Read data channel signals
       .RREADYM                     (RREADYM),
       .RVALIDM                     (RVALIDM),
       .RIDM                        (RIDM),
       .RDATAM                      (RDATAM),
       .RRESPM                      ({2'b00, RRESPM}),
       .RLASTM                      (RLASTM),
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
       .DBGROMADDR                  ({28{1'b0}}),
       .DBGROMADDRV                 (1'b0),
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
  .PADDRSYS(PADDRDBG),         // System APB address bus
  .PSELSYS(PSELDBG),          // System APB select
  .PWRITESYS(PWRITEDBG),        // System APB write access
  .PENABLESYS(PENABLEDBG),       // System APB enable signal - indicates second
                  // and subsequent cycles of an APB transfer
  .PWDATASYS(PWDATADBG),        // System APB Write data bus
  .PRDATASYS(PRDATADBG),        // System APB write data bus
  .PREADYSYS(PREADYDBG),        // System APB Ready signal
  .PSLVERRSYS(PSLVERRDBG),       // System APB transfer error signal
  
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
  
  .ARID(GIC_ARID),
  .ARADDR(GIC_ARADDR),
  .ARLEN(GIC_ARLEN),
  .ARSIZE(GIC_ARSIZE),
  .ARBURST(GIC_ARBURST),
  .ARPROT(GIC_ARPROT),
  .ARUSER(GIC_ARUSER),
  .ARVALID(GIC_ARVALID),
  .ARREADY(GIC_ARREADY),
  
  .RID(GIC_RID),
  .RDATA(GIC_RDATA),
  .RLAST(GIC_RLAST),
  .RRESP(GIC_RRESP),
  .RVALID(GIC_RVALID),
  .RREADY(GIC_RREADY),
  
  .AWID(GIC_AWID),
  .AWADDR(GIC_AWADDR),
  .AWLEN(GIC_AWLEN),
  .AWSIZE(GIC_AWSIZE),
  .AWBURST(GIC_AWBURST),
  .AWPROT(GIC_AWPROT),
  .AWUSER(GIC_AWUSER),
  .AWVALID(GIC_AWVALID),
  .AWREADY(GIC_AWREADY),
  
  .WDATA(GIC_WDATA),
  .WSTRB(GIC_WSTRB),
  .WVALID(GIC_WVALID),
  .WREADY(GIC_WREADY),
  
  .BID(GIC_BID),
  .BRESP(GIC_BRESP),
  .BVALID(GIC_BVALID),
  .BREADY(GIC_BREADY),
  
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