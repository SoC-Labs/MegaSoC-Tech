#include "uart_stdout.h"
#include "system.h"
#include <stdio.h>
#include "snps_dram_ctrl.h"
#include "snps_dram_phy.h"
#define HW16_REG(ADDRESS)  (*((volatile uint16_t *)(ADDRESS)))
#define HW32_REG(ADDRESS)  (*((volatile uint32_t  *)(ADDRESS)))
void dwc_ddr_ctrl_init(void);
void dwc_ddr_phy_init_no_training(void);
void dwc_ddrphy_apb_wr(uint32_t offset, uint16_t data);
void dwc_ddrphy_phyinit_userCustom_A_bringupPower ();
void dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy ();
int main(void) {
  uint32_t errors = 0;
  uint8_t simulation =1;
  UartStdOutInit();

  printf("MegaSoC DRAM tests\n");

  printf("** Start DDR CTRL Init **\n");

  // PHY Power up procedure (bring up VDD VDDQ and VAA, start clocks and reset PHY)
  printf("**STEP A**\n");

  dwc_ddrphy_phyinit_userCustom_A_bringupPower ();

  // [dwc_ddrphy_phyinit_userCustom_A_bringupPower] End of dwc_ddrphy_phyinit_userCustom_A_bringupPower()
  // [dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy] Start of dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy()
  // 
  // 
  // //##############################################################
  // //
  // // Step (B) Start Clocks and Reset the PHY 
  // //
  // // See PhyInit App Note for detailed description and function usage
  // //
  // //##############################################################
  // 
  // 
  printf("**STEP B**\n");

  dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy ();

  // Program DWC_ddr_umctl2 registers
    //Note 1: When running training with the PHY. The following controller registers must be programmed to
    //these values at this stage:
    //  INIT0.skip_dram_init=2’b11
    //  PWRCTL.selfref_sw=1’b1
    //Programming them as follows is only allowed for simulation purposes when skipping training:
    //  INIT0.skip_dram_init=0
    //  (that is, SDRAM INIT through the controller)
    //  PWRCTL.selfref_sw=0

  if(simulation){
    SNPS_MCTL2_DDRC->INIT0.B.skip_dram_init=0; 
    SNPS_MCTL2_DDRC->PWRCTL.B.selfref_sw=0;
  } else {
    SNPS_MCTL2_DDRC->INIT0.B.skip_dram_init=3;
    SNPS_MCTL2_DDRC->PWRCTL.B.selfref_sw=1;
  }
  // De-assert reset signal core_ddrc_rstn
  DDR_RESET_CTRL->CORE_RSTn=1;

  // Disable self-refresh, power down and assertion of dfi_dram_clk_disable by
  // setting RFSHCTL3.dis_auto_refresh= 1, PWRCTL.powerdown_en = 0,
  // PWRCTL.selfref_en = 0, and PWRCTL.en_dfi_dram_clk_disable =0
  SNPS_MCTL2_DDRC->RFSHCTL3.B.dis_auto_refresh = 1;
  SNPS_MCTL2_DDRC->PWRCTL.B.powerdown_en = 0;
  SNPS_MCTL2_DDRC->PWRCTL.B.selfref_en=0;
  SNPS_MCTL2_DDRC->PWRCTL.B.en_dfi_dram_clk_disable=0;

  // Set SWCTL.sw_done to ‘0’
  //   If UMCTL2_OCCAP_EN=1 && OCCAPCCFG.occap_en=1, require polling
  //   SWSTAT.sw_done_ack after setting SWCTL.sw_done to ‘0'
  SNPS_MCTL2_DDRC->SWCTL.B.sw_done=0;

  // Set DFIMISC.dfi_init_complete_en to ‘0' (mask transition in phy_dfi_init_complete)
  SNPS_MCTL2_DDRC->DFIMISC.B.dfi_init_complete_en = 0;  

  // Set SWCTL.sw_done to ‘1' (poll swstat.sw_donw_ack)
  SNPS_MCTL2_DDRC->SWCTL.B.sw_done = 1;
  while(SNPS_MCTL2_DDRC->SWSTAT.B.sw_done_ack==0){;}

  // Start PHY initialization and training by
  // accessing relevant PUB registers
  printf("** Start DDR PHY Init **\n");
  dwc_ddr_phy_init_no_training();
  printf("** Finish DDR PHY Init **\n");
//  // 9 Poll the PUB register
//  //  APBONLY.UctShadowRegs[0]=1’b0 
//  while((HW16_REG(DRAM_PHY_CFG_BASE+0x340010)&0x1)!=0){__nop();}
//  // 10 Read the PUB Register
//  //    APBONLY.UctWriteOnlyShadow for training status
//  printf("APBONLY.UctWriteOnlyShadow: 0x%08x\n",HW16_REG(DRAM_PHY_CFG_BASE+0x3400C8));
// 
//  // Write the PUB Register
//  //  APBONLY.DctWriteProt = 0 phy_init See PUB databook for details
//  HW16_REG(DRAM_PHY_CFG_BASE+0x3400C4)=0;
//  printf("Wrote 0 to ABPONLY.DctWriteProt\n");
//  // Poll the PUB register
//  // APBONLY.UctShadowRegs[0]=1’b1 phy_init See PUB databook for details
//  while((HW16_REG(DRAM_PHY_CFG_BASE+0x340010)&0x1)==0){printf("APBONLY.UctWriteOnlyShadow: 0x%08x\n",HW16_REG(DRAM_PHY_CFG_BASE+0x3400C8));}
// 
// 
//  // 13 Write the PUB Register
//  //  APBONLY.DctWriteProt= 1 phy_init See PUB databook for details
//  HW16_REG(DRAM_PHY_CFG_BASE+0x3400C4)=1;
//  printf("Wrote 1 to ABPONLY.DctWriteProt\n");

  // 14 Poll the PUB register MASTER.CalBusy=0 phy_init See PUB databook for details
  while((HW16_REG(DRAM_PHY_CFG_BASE+0x8025c)&0x1)!=0){__nop();}

  // 15 Set SWCTL.sw_done to ‘0’
  //  If UMCTL2_OCCAP_EN=1 && OCCAPCCFG.occap_en=1, require polling
  // SWSTAT.sw_done_ack after setting SWCTL.sw_done to ‘0’
  SNPS_MCTL2_DDRC->SWCTL.B.sw_done=0;
  printf("Wrote 0 to SWCTL.B.sw_done\n");

  // 16 Set DFIMISC.dfi_init_start to ‘1’ 
  SNPS_MCTL2_DDRC->DFIMISC.B.dfi_init_start=1;
  printf("Wrote 1 to DFIMISC.B.dfi_init_start\n");

  // 17 Set SWCTL.sw_done to ‘1’ phy_init
  // Require polling SWSTAT.sw_done_ack after setting SWCTL.sw_done to 1
  SNPS_MCTL2_DDRC->SWCTL.B.sw_done=1;
  printf("Wrote 1 to SWCTL.B.sw_done\n");

  while(SNPS_MCTL2_DDRC->SWSTAT.B.sw_done_ack==0){__nop();}
  printf("SWSTAT.B.sw_done_ack != 0\n");

  // 18 Poll DFISTAT.dfi_init_complete=1 phy_init
  while(SNPS_MCTL2_DDRC->DFISTAT.B.dfi_init_complete==0){__nop();}
  printf("DFISTAT.B.dfi_init_complete != 0\n");

  // 19 Set SWCTL.sw_done to ‘0’
  // If UMCTL2_OCCAP_EN=1 && OCCAPCCFG.occap_en=1, require polling
  // SWSTAT.sw_done_ack after setting SWCTL.sw_done to ‘0’
  SNPS_MCTL2_DDRC->SWCTL.B.sw_done=0;
  printf("Wrote 0 to SWCTL.B.sw_done\n");

  // 20 Set DFIMISC.dfi_init_start to ‘0’ 
  SNPS_MCTL2_DDRC->DFIMISC.B.dfi_init_start=0;
  printf("Wrote 0 to DFIMISC.B.dfi_init_start\n");

  // 21 The following registers may need to be
  // updated after training has completed:
  // ■ RANKCTL.diff_rank_wr_gap
  // ■ RANKCTL.diff_rank_rd_gap
  // ■ DRAMTMG2.rd2wr
  // ■ DRAMTMG2.wr2rd
  // ■ DRAMTMG9.wr2rd_s
  // ■ RANKCTL.diff_rank_wr_gap_msb
  // ■ RANKCTL.diff_rank_rd_gap_msb
  // ■ RANKCTL1.wr2rd_dr
  // ■ DFITMG1.dfi_t_wrdata_delay
  // Also, the following registers related to VREF
  // setting may need to be updated after 2D
  // Training has completed:
  // ■ INIT7.mr6
  // ■ INIT6.mr5

  // 22 Set DFIMISC.dfi_init_complete_en to ‘1’ 
  SNPS_MCTL2_DDRC->DFIMISC.B.dfi_init_complete_en=1;
  printf("Wrote 1 to DFIMISC.B.dfi_init_complete_en\n");

  //23 Set PWRCTL.selfref_sw to ‘0’ 
  SNPS_MCTL2_DDRC->PWRCTL.B.selfref_sw=0;
  printf("Wrote 0 to PWRCTL.B.selfref_sw\n");

  // 24 Set SWCTL.sw_done to ‘1’ 
  // Require polling SWSTAT.sw_done_ack after setting SWCTL.sw_done to 1
  SNPS_MCTL2_DDRC->SWCTL.B.sw_done = 1;
  printf("Wrote 1 to SWCTL.B.sw_done\n");

  // 25 Wait for DWC_ddr_umctl2 to move to normal
  // operating mode by monitoring STAT.operating_mode signal
  while( SNPS_MCTL2_DDRC->STAT.B.operating_mode==0){__nop();}
  printf("STAT.B.operating_mode != 0\n");

  // 26 Set back registers in step 4 to the original
  // values if desired

  SNPS_MCTL2_DDRC->RFSHCTL3.B.dis_auto_refresh = 0;
  SNPS_MCTL2_DDRC->PWRCTL.B.powerdown_en = 1;
  SNPS_MCTL2_DDRC->PWRCTL.B.selfref_en=1;
  SNPS_MCTL2_DDRC->PWRCTL.B.en_dfi_dram_clk_disable=1;

  HW32_REG(DRAM_BASE)=0xA5A5A5A5;
  HW32_REG(DRAM_BASE+0xA5A5A5)=0x5A5A5A5A;

  printf("Wrote over DDR\n");

  if(HW32_REG(DRAM_BASE+0xA5A5A5)!=0xDEADBEEF){
    TEST_FAIL();
  } else {
    TEST_PASS();
  }

  TEST_PASS();
}

void dwc_ddrphy_apb_wr(uint32_t offset, uint16_t data){
  uint32_t *pointer = (uint32_t *) (DRAM_PHY_CFG_BASE + (offset << 2));
  // printf("Writing 0x%04x to addr: 0x%08x\n",data,pointer);
  *pointer = data;
  return;
}

void dwc_ddrphy_phyinit_userCustom_overrideUserInput (){
  return;
}

void dwc_ddrphy_phyinit_userCustom_A_bringupPower (){
  return;
}

void dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy (){
  int i =0;
  DDR_RESET_CTRL->RESET=1;
  for(i=0;i<2048;i++){__nop();}
  DDR_RESET_CTRL->PWROK=1;

  for(i=0;i<2048;i++){__nop();}
  DDR_RESET_CTRL->RESET=0;
  return;
}

void dwc_ddrphy_phyinit_userCustom_E_setDfiClk(int a){
  return;
}
void dwc_ddrphy_phyinit_userCustom_G_waitFwDone(){
  uint16_t mail=0xff;
  int i =0;
  for(i=0;i<256;i++){__nop();}
  printf("Wait for FW done\n");
  while(mail!=0){
    // 9 Poll the PUB register
    //  APBONLY.UctShadowRegs[0]=1’b0 
    while((HW16_REG(DRAM_PHY_CFG_BASE+0x340010)&0x1)!=0){__nop();}
    printf("Mailbox message available\n");

    // 10 Read the PUB Register
    //    APBONLY.UctWriteOnlyShadow for training status
    mail=HW16_REG(DRAM_PHY_CFG_BASE+0x3400C8);
    printf("APBONLY.UctWriteOnlyShadow: 0x%08x\n",mail);

    // Write the PUB Register
    //  APBONLY.DctWriteProt = 0 phy_init See PUB databook for details
    HW16_REG(DRAM_PHY_CFG_BASE+0x3400C4)=0;
    printf("Wrote 0 to ABPONLY.DctWriteProt\n");
    // Poll the PUB register
    // APBONLY.UctShadowRegs[0]=1’b1 phy_init See PUB databook for details
    while((HW16_REG(DRAM_PHY_CFG_BASE+0x340010)&0x1)==0){printf("APBONLY.UctWriteOnlyShadow: 0x%08x\n",HW16_REG(DRAM_PHY_CFG_BASE+0x3400C8));}

    // 13 Write the PUB Register
    //  APBONLY.DctWriteProt= 1 phy_init See PUB databook for details
    HW16_REG(DRAM_PHY_CFG_BASE+0x3400C4)=1;
    printf("Wrote 1 to ABPONLY.DctWriteProt\n");
  }
  return;
}
void dwc_ddrphy_phyinit_userCustom_H_readMsgBlock(int a){
  return;
}
void dwc_ddrphy_phyinit_userCustom_customPostTrain(){
  return;
}
void dwc_ddrphy_phyinit_userCustom_J_enterMissionMode(){
  return;

}
void dwc_ddr_phy_init_no_training(void){
// [dwc_ddrphy_phyinit_main] Start of dwc_ddrphy_phyinit_main()
// [dwc_ddrphy_phyinit_sequence] Start of dwc_ddrphy_phyinit_sequence()
// [dwc_ddrphy_phyinit_initStruct] Start of dwc_ddrphy_phyinit_initStruct()
// [dwc_ddrphy_phyinit_initStruct] End of dwc_ddrphy_phyinit_initStruct()
// [dwc_ddrphy_phyinit_setDefault] Start of dwc_ddrphy_phyinit_setDefault()
// [dwc_ddrphy_phyinit_setDefault] End of dwc_ddrphy_phyinit_setDefault()


// //##############################################################
// 
// // dwc_ddrphy_phyinit_userCustom_overrideUserInput is a user-editable function.
// //
// // See PhyInit App Note for detailed description and function usage
//
// //##############################################################

dwc_ddrphy_phyinit_userCustom_overrideUserInput ();
// 
//  [dwc_ddrphy_phyinit_userCustom_overrideUserInput] End of dwc_ddrphy_phyinit_userCustom_overrideUserInput()
//[dwc_ddrphy_phyinit_calcMb] Start of dwc_ddrphy_phyinit_calcMb()
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].Pstate to 0x0
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].DRAMFreq to 0x640
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].PllBypassEn to 0x0
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].DfiFreqRatio to 0x2
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].PhyOdtImpedance to 0x0
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].PhyDrvImpedance to 0x0
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].BPZNResVal to 0x0
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].EnabledDQsChA to 0x10
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].CsPresentChA to 0x1
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].EnabledDQsChB to 0x0
// // [dwc_ddrphy_phyinit_softSetMb] Setting mb_LPDDR4_1D[0].CsPresentChB to 0x0
//[dwc_ddrphy_phyinit_calcMb] End of dwc_ddrphy_phyinit_calcMb()
// // [phyinit_print_dat] // ####################################################
// // [phyinit_print_dat] // 
// // [phyinit_print_dat] // Printing Runtime input values
// // [phyinit_print_dat] // 
// // [phyinit_print_dat] // ####################################################
// // [phyinit_print_dat] runtimeConfig.skip_training = 1
// // [phyinit_print_dat] runtimeConfig.Train2D       = 0
// // [phyinit_print_dat] runtimeConfig.debug         = 1
// // [phyinit_print_dat] runtimeConfig.RetEn         = 0
// // [phyinit_print_dat] // ####################################################
// // [phyinit_print_dat] // 
// // [phyinit_print_dat] // Printing values in user input structure
// // [phyinit_print_dat] // 
// // [phyinit_print_dat] // ####################################################
// // [phyinit_print_dat] userInputBasic.Frequency[0] = 800
// // [phyinit_print_dat] userInputBasic.Frequency[1] = 1067
// // [phyinit_print_dat] userInputBasic.Frequency[2] = 933
// // [phyinit_print_dat] userInputBasic.Frequency[3] = 800
// // [phyinit_print_dat] userInputBasic.NumRank_dfi0 = 1
// // [phyinit_print_dat] userInputBasic.ReadDBIEnable[0] = 0
// // [phyinit_print_dat] userInputBasic.ReadDBIEnable[1] = 0
// // [phyinit_print_dat] userInputBasic.ReadDBIEnable[2] = 0
// // [phyinit_print_dat] userInputBasic.ReadDBIEnable[3] = 0
// // [phyinit_print_dat] userInputBasic.Lp4xMode = 0
// // [phyinit_print_dat] userInputBasic.DimmType = 4
// // [phyinit_print_dat] userInputBasic.DfiMode = 0
// // [phyinit_print_dat] userInputBasic.DramType = 2
// // [phyinit_print_dat] userInputBasic.HardMacroVer = 0
// // [phyinit_print_dat] userInputBasic.DfiFreqRatio[0] = 1
// // [phyinit_print_dat] userInputBasic.DfiFreqRatio[1] = 1
// // [phyinit_print_dat] userInputBasic.DfiFreqRatio[2] = 1
// // [phyinit_print_dat] userInputBasic.DfiFreqRatio[3] = 1
// // [phyinit_print_dat] userInputBasic.NumAnib = 3
// // [phyinit_print_dat] userInputBasic.NumDbyte = 2
// // [phyinit_print_dat] userInputBasic.DramDataWidth = 16
// // [phyinit_print_dat] userInputBasic.PllBypass[0] = 0
// // [phyinit_print_dat] userInputBasic.PllBypass[1] = 0
// // [phyinit_print_dat] userInputBasic.PllBypass[2] = 0
// // [phyinit_print_dat] userInputBasic.PllBypass[3] = 0
// // [phyinit_print_dat] userInputBasic.Dfi1Exists = 0
// // [phyinit_print_dat] userInputBasic.Train2D = 0
// // [phyinit_print_dat] userInputBasic.NumRank_dfi1 = 0
// // [phyinit_print_dat] userInputBasic.NumActiveDbyteDfi0 = 2
// // [phyinit_print_dat] userInputBasic.NumPStates = 1
// // [phyinit_print_dat] userInputBasic.NumActiveDbyteDfi1 = 0
// // [phyinit_print_dat] userInputAdvanced.TxSlewRiseAC = 15
// // [phyinit_print_dat] userInputAdvanced.TxSlewFallDQ[0] = 15
// // [phyinit_print_dat] userInputAdvanced.TxSlewFallDQ[1] = 15
// // [phyinit_print_dat] userInputAdvanced.TxSlewFallDQ[2] = 15
// // [phyinit_print_dat] userInputAdvanced.TxSlewFallDQ[3] = 15
// // [phyinit_print_dat] userInputAdvanced.Lp4WLS[0] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4WLS[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4WLS[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4WLS[3] = 0
// // [phyinit_print_dat] userInputAdvanced.TxImpedance[0] = 60
// // [phyinit_print_dat] userInputAdvanced.TxImpedance[1] = 60
// // [phyinit_print_dat] userInputAdvanced.TxImpedance[2] = 60
// // [phyinit_print_dat] userInputAdvanced.TxImpedance[3] = 60
// // [phyinit_print_dat] userInputAdvanced.DramByteSwap = 0
// // [phyinit_print_dat] userInputAdvanced.CalInterval = 3
// // [phyinit_print_dat] userInputAdvanced.D4RxPreambleLength[0] = 0
// // [phyinit_print_dat] userInputAdvanced.D4RxPreambleLength[1] = 0
// // [phyinit_print_dat] userInputAdvanced.D4RxPreambleLength[2] = 0
// // [phyinit_print_dat] userInputAdvanced.D4RxPreambleLength[3] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4PostambleExt[0] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4PostambleExt[1] = 1
// // [phyinit_print_dat] userInputAdvanced.Lp4PostambleExt[2] = 1
// // [phyinit_print_dat] userInputAdvanced.Lp4PostambleExt[3] = 1
// // [phyinit_print_dat] userInputAdvanced.CalOnce = 0
// // [phyinit_print_dat] userInputAdvanced.SnpsUmctlF0RC5x[0] = 0
// // [phyinit_print_dat] userInputAdvanced.SnpsUmctlF0RC5x[1] = 0
// // [phyinit_print_dat] userInputAdvanced.SnpsUmctlF0RC5x[2] = 0
// // [phyinit_print_dat] userInputAdvanced.SnpsUmctlF0RC5x[3] = 0
// // [phyinit_print_dat] userInputAdvanced.WDQSExt = 0
// // [phyinit_print_dat] userInputAdvanced.RxEnBackOff = 1
// // [phyinit_print_dat] userInputAdvanced.Lp4LowPowerDrv = 0
// // [phyinit_print_dat] userInputAdvanced.TrainSequenceCtrl = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4WL[0] = 2
// // [phyinit_print_dat] userInputAdvanced.Lp4WL[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4WL[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4WL[3] = 0
// // [phyinit_print_dat] userInputAdvanced.PhyMstrTrainInterval[0] = 10
// // [phyinit_print_dat] userInputAdvanced.PhyMstrTrainInterval[1] = 10
// // [phyinit_print_dat] userInputAdvanced.PhyMstrTrainInterval[2] = 10
// // [phyinit_print_dat] userInputAdvanced.PhyMstrTrainInterval[3] = 10
// // [phyinit_print_dat] userInputAdvanced.Lp4nWR[0] = 2
// // [phyinit_print_dat] userInputAdvanced.Lp4nWR[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4nWR[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4nWR[3] = 0
// // [phyinit_print_dat] userInputAdvanced.MemAlertPUImp = 5
// // [phyinit_print_dat] userInputAdvanced.TxSlewRiseDQ[0] = 15
// // [phyinit_print_dat] userInputAdvanced.TxSlewRiseDQ[1] = 15
// // [phyinit_print_dat] userInputAdvanced.TxSlewRiseDQ[2] = 15
// // [phyinit_print_dat] userInputAdvanced.TxSlewRiseDQ[3] = 15
// // [phyinit_print_dat] userInputAdvanced.DisableRetraining = 0
// // [phyinit_print_dat] userInputAdvanced.MemAlertVrefLevel = 41
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiRd[0] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiRd[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiRd[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiRd[3] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4RxPreambleMode[0] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4RxPreambleMode[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4RxPreambleMode[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4RxPreambleMode[3] = 0
// // [phyinit_print_dat] userInputAdvanced.DisDynAdrTri[0] = 1
// // [phyinit_print_dat] userInputAdvanced.DisDynAdrTri[1] = 1
// // [phyinit_print_dat] userInputAdvanced.DisDynAdrTri[2] = 1
// // [phyinit_print_dat] userInputAdvanced.DisDynAdrTri[3] = 1
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiWr[0] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiWr[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiWr[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4DbiWr[3] = 0
// // [phyinit_print_dat] userInputAdvanced.SnpsUmctlOpt = 0
// // [phyinit_print_dat] userInputAdvanced.ODTImpedance[0] = 60
// // [phyinit_print_dat] userInputAdvanced.ODTImpedance[1] = 60
// // [phyinit_print_dat] userInputAdvanced.ODTImpedance[2] = 60
// // [phyinit_print_dat] userInputAdvanced.ODTImpedance[3] = 60
// // [phyinit_print_dat] userInputAdvanced.PhyInitSequenceNum = 0
// // [phyinit_print_dat] userInputAdvanced.DisablePhyUpdate = 0
// // [phyinit_print_dat] userInputAdvanced.D4TxPreambleLength[0] = 0
// // [phyinit_print_dat] userInputAdvanced.D4TxPreambleLength[1] = 0
// // [phyinit_print_dat] userInputAdvanced.D4TxPreambleLength[2] = 0
// // [phyinit_print_dat] userInputAdvanced.D4TxPreambleLength[3] = 0
// // [phyinit_print_dat] userInputAdvanced.MemAlertEn = 0
// // [phyinit_print_dat] userInputAdvanced.MemAlertSyncBypass = 0
// // [phyinit_print_dat] userInputAdvanced.PhyMstrMaxReqToAck[0] = 5
// // [phyinit_print_dat] userInputAdvanced.PhyMstrMaxReqToAck[1] = 5
// // [phyinit_print_dat] userInputAdvanced.PhyMstrMaxReqToAck[2] = 5
// // [phyinit_print_dat] userInputAdvanced.PhyMstrMaxReqToAck[3] = 5
// // [phyinit_print_dat] userInputAdvanced.DisableUnusedAddrLns = 0
// // [phyinit_print_dat] userInputAdvanced.ATxImpedance = 40
// // [phyinit_print_dat] userInputAdvanced.ExtCalResVal = 0
// // [phyinit_print_dat] userInputAdvanced.TxSlewFallAC = 15
// // [phyinit_print_dat] userInputAdvanced.EnableHighClkSkewFix = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4RL[0] = 2
// // [phyinit_print_dat] userInputAdvanced.Lp4RL[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4RL[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Lp4RL[3] = 0
// // [phyinit_print_dat] userInputAdvanced.Is2Ttiming[0] = 0
// // [phyinit_print_dat] userInputAdvanced.Is2Ttiming[1] = 0
// // [phyinit_print_dat] userInputAdvanced.Is2Ttiming[2] = 0
// // [phyinit_print_dat] userInputAdvanced.Is2Ttiming[3] = 0
// // [phyinit_print_dat] userInputSim.tDQS2DQ    = 200
// // [phyinit_print_dat] userInputSim.tDQSCK     = 1500
// // [phyinit_print_dat] userInputSim.tSTAOFF[0] = 0
// // [phyinit_print_dat] userInputSim.tSTAOFF[1] = 0
// // [phyinit_print_dat] userInputSim.tSTAOFF[2] = 0
// // [phyinit_print_dat] userInputSim.tSTAOFF[3] = 0
// // [phyinit_print_dat] // ####################################################
// // [phyinit_print_dat] // 
// // [phyinit_print_dat] // Printing values of 1D message block input/inout fields, PState=0
// // [phyinit_print_dat] // 
// // [phyinit_print_dat] // ####################################################
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Reserved00 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MsgMisc = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Pstate = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].PllBypassEn = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].DRAMFreq = 0x640
// // [phyinit_print_dat] mb_LPDDR4_1D[0].DfiFreqRatio = 0x2
// // [phyinit_print_dat] mb_LPDDR4_1D[0].BPZNResVal = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].PhyOdtImpedance = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].PhyDrvImpedance = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].PhyVref = 0x40
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Lp4Misc = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Reserved0E = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].SequenceCtrl = 0x1
// // [phyinit_print_dat] mb_LPDDR4_1D[0].HdtCtrl = 0xff
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Reserved13 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].DFIMRLMargin = 0x2
// // [phyinit_print_dat] mb_LPDDR4_1D[0].UseBroadcastMR = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Lp4Quickboot = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Reserved1A = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].CATrainOpt = 0x1
// // [phyinit_print_dat] mb_LPDDR4_1D[0].X8Mode = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].Share2DVrefResult = 0x1
// // [phyinit_print_dat] mb_LPDDR4_1D[0].PhyConfigOverride = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].EnabledDQsChA = 0x10
// // [phyinit_print_dat] mb_LPDDR4_1D[0].CsPresentChA = 0x1
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR1_A0 = 0x24
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR2_A0 = 0x12
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR3_A0 = 0x9
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR4_A0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR11_A0 = 0x33
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR12_A0 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR13_A0 = 0x28
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR14_A0 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR16_A0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR17_A0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR22_A0 = 0x18
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR24_A0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR1_A1 = 0x24
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR2_A1 = 0x12
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR3_A1 = 0x9
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR4_A1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR11_A1 = 0x33
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR12_A1 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR13_A1 = 0x28
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR14_A1 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR16_A1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR17_A1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR22_A1 = 0x18
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR24_A1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].CATerminatingRankChA = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].EnabledDQsChB = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].CsPresentChB = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR1_B0 = 0x24
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR2_B0 = 0x12
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR3_B0 = 0x9
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR4_B0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR11_B0 = 0x33
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR12_B0 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR13_B0 = 0x28
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR14_B0 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR16_B0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR17_B0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR22_B0 = 0x18
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR24_B0 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR1_B1 = 0x24
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR2_B1 = 0x12
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR3_B1 = 0x9
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR4_B1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR11_B1 = 0x33
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR12_B1 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR13_B1 = 0x28
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR14_B1 = 0x2b
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR16_B1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR17_B1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR22_B1 = 0x18
// // [phyinit_print_dat] mb_LPDDR4_1D[0].MR24_B1 = 0x0
// // [phyinit_print_dat] mb_LPDDR4_1D[0].CATerminatingRankChB = 0x0



// [dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy] End of dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy()
// 

// //##############################################################
// //
// // Step (C) Initialize PHY Configuration 
// //
// // Load the required PHY configuration registers for the appropriate mode and memory configuration
// //
// //##############################################################
// 
printf("**STEP C*\n");

// // [phyinit_C_initPhyConfig] Start of dwc_ddrphy_phyinit_C_initPhyConfig()
// 
// //##############################################################
// // TxPreDrvMode[2] = userInputBasic.Lp4xMode 
// //##############################################################
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TxSlewRate::TxPreDrvMode to 0x1
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TxSlewRate::TxPreP to 0xf
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TxSlewRate::TxPreN to 0xf
// // [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for TxSlewRate::TxPreP and TxSlewRate::TxPreP are technology specific.
// // [phyinit_C_initPhyConfig] ### NOTE ### Please consult the "Output Slew Rate" section of HSpice Model App Note in specific technology for recommended settings

dwc_ddrphy_apb_wr(0x1005f,0x1ff);
dwc_ddrphy_apb_wr(0x1015f,0x1ff);
dwc_ddrphy_apb_wr(0x1105f,0x1ff);
dwc_ddrphy_apb_wr(0x1115f,0x1ff);
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreDrvMode to 0x1, ANIB=0
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreP to 0xf, ANIB=0
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreN to 0xf, ANIB=0
// // [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for ATxSlewRate::ATxPreP and ATxSlewRate::ATxPreP are technology specific.
// // [phyinit_C_initPhyConfig] ### NOTE ### Please consult the "Output Slew Rate" section of HSpice Model App Note in specific technology for recommended settings

dwc_ddrphy_apb_wr(0x55,0x1ff);
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreDrvMode to 0x1, ANIB=1
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreP to 0xf, ANIB=1
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreN to 0xf, ANIB=1
// // [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for ATxSlewRate::ATxPreP and ATxSlewRate::ATxPreP are technology specific.
// // [phyinit_C_initPhyConfig] ### NOTE ### Please consult the "Output Slew Rate" section of HSpice Model App Note in specific technology for recommended settings

dwc_ddrphy_apb_wr(0x1055,0x1ff);
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreDrvMode to 0x1, ANIB=2
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreP to 0xf, ANIB=2
// // [phyinit_C_initPhyConfig] Programming ATxSlewRate::ATxPreN to 0xf, ANIB=2
// // [phyinit_C_initPhyConfig] ### NOTE ### Optimal setting for ATxSlewRate::ATxPreP and ATxSlewRate::ATxPreP are technology specific.
// // [phyinit_C_initPhyConfig] ### NOTE ### Please consult the "Output Slew Rate" section of HSpice Model App Note in specific technology for recommended settings

dwc_ddrphy_apb_wr(0x2055,0x1ff);
dwc_ddrphy_apb_wr(0x200c5,0xb);
// // [phyinit_C_initPhyConfig] Pstate=0,  Memclk=800MHz, Programming PllCtrl2 to b based on DfiClk frequency = 400.
// 
// //##############################################################
// //
// // Program ARdPtrInitVal based on Frequency and PLL Bypass inputs
// // The values programmed here assume ideal properties of DfiClk
// // and Pclk including:
// // - DfiClk skew
// // - DfiClk jitter
// // - DfiClk PVT variations
// // - Pclk skew
// // - Pclk jitter
// //
// // PLL Bypassed mode:
// //     For MemClk frequency > 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 2-6
// //     For MemClk frequency < 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 1-6
// //
// // PLL Enabled mode:
// //     For MemClk frequency > 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 1-6
// //     For MemClk frequency < 933MHz, the valid range of ARdPtrInitVal_p0[3:0] is: 0-6
// //
// //##############################################################
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming ARdPtrInitVal to 0x2
dwc_ddrphy_apb_wr(0x2002e,0x2);
// 
// //##############################################################
// // Seq0BGPR4       = 0: Make ProcOdtAlwaysOn = 0 and ProcOdtAlwaysOff = 0 
// //##############################################################
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming ProcOdtCtl: Seq0BGPR4.ProcOdtAlwaysOff  to 0x0
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming ProcOdtCtl: Seq0BGPR4.ProcOdtAlwaysOn   to 0x0
dwc_ddrphy_apb_wr(0x90204,0x0);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DqsPreambleControl::TwoTckRxDqsPre to 0x1
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DqsPreambleControl::TwoTckTxDqsPre to 0x1
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DqsPreambleControl::PositionDfeInit to 0x0
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DqsPreambleControl::LP4TglTwoTckTxDqsPre to 0x1
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DqsPreambleControl::LP4PostambleExt to 0x0
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DqsPreambleControl::LP4SttcPreBridgeRxEn to 0x1
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DqsPreambleControl to 0xa3
dwc_ddrphy_apb_wr(0x20024,0xa3);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DbyteDllModeCntrl to 0x2
dwc_ddrphy_apb_wr(0x2003a,0x2);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DllLockParam to 0x212
dwc_ddrphy_apb_wr(0x2007d,0x212);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DllGainCtl to 0x61
dwc_ddrphy_apb_wr(0x2007c,0x61);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming ProcOdtTimeCtl to 0xa
dwc_ddrphy_apb_wr(0x20056,0xa);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TxOdtDrvStren::ODTStrenP to 0x0
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TxOdtDrvStren::ODTStrenN to 0x18
dwc_ddrphy_apb_wr(0x1004d,0x000);
dwc_ddrphy_apb_wr(0x1014d,0x000);
dwc_ddrphy_apb_wr(0x1104d,0x000);
dwc_ddrphy_apb_wr(0x1114d,0x000);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TxImpedanceCtrl1::DrvStrenFSDqP to 0x18
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TxImpedanceCtrl1::DrvStrenFSDqN to 0x18
dwc_ddrphy_apb_wr(0x10049,0x618);
dwc_ddrphy_apb_wr(0x10149,0x618);
dwc_ddrphy_apb_wr(0x11049,0x618);
dwc_ddrphy_apb_wr(0x11149,0x618);
// // [phyinit_C_initPhyConfig] Programming ATxImpedance::ADrvStrenP to 0x3
// // [phyinit_C_initPhyConfig] Programming ATxImpedance::ADrvStrenN to 0x3
dwc_ddrphy_apb_wr(0x43,0x63);
dwc_ddrphy_apb_wr(0x1043,0x63);
dwc_ddrphy_apb_wr(0x2043,0x63);
// // [phyinit_C_initPhyConfig] Programming DfiMode to 0x1
dwc_ddrphy_apb_wr(0x20018,0x1);
// // [phyinit_C_initPhyConfig] Programming DfiCAMode to 0x4
dwc_ddrphy_apb_wr(0x20075,0x4);
// // [phyinit_C_initPhyConfig] Programming CalDrvStr0::CalDrvStrPd50 to 0x0
// // [phyinit_C_initPhyConfig] Programming CalDrvStr0::CalDrvStrPu50 to 0x0
dwc_ddrphy_apb_wr(0x20050,0x0);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming CalUclkInfo::CalUClkTicksPer1uS to 0x190
dwc_ddrphy_apb_wr(0x20008,0x190);
// // [phyinit_C_initPhyConfig] Programming CalRate::CalInterval to 0x3
// // [phyinit_C_initPhyConfig] Programming CalRate::CalOnce to 0x0
dwc_ddrphy_apb_wr(0x20088,0x3);
// // [phyinit_C_initPhyConfig] Pstate=0, Programming VrefInGlobal::GlobalVrefInSel to 0x4
// // [phyinit_C_initPhyConfig] Pstate=0, Programming VrefInGlobal::GlobalVrefInDAC to 0x65
// // [phyinit_C_initPhyConfig] Pstate=0, Programming VrefInGlobal to 0x32c
dwc_ddrphy_apb_wr(0x200b2,0x32c);
// // [phyinit_C_initPhyConfig] Pstate=0, Programming DqDqsRcvCntrl::MajorModeDbyte to 0x2
// // [phyinit_C_initPhyConfig] Pstate=0, Programming DqDqsRcvCntrl to 0x5a1
dwc_ddrphy_apb_wr(0x10043,0x5a1);
dwc_ddrphy_apb_wr(0x10143,0x5a1);
dwc_ddrphy_apb_wr(0x11043,0x5a1);
dwc_ddrphy_apb_wr(0x11143,0x5a1);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DfiFreqRatio_p0 to 0x1
dwc_ddrphy_apb_wr(0x200fa,0x1);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TristateModeCA::DisDynAdrTri_p0 to 0x1
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming TristateModeCA::DDR2TMode_p0 to 0x0
dwc_ddrphy_apb_wr(0x20019,0x1);
// // [phyinit_C_initPhyConfig] Programming DfiFreqXlat*
dwc_ddrphy_apb_wr(0x200f0,0x1);
dwc_ddrphy_apb_wr(0x200f1,0x0);
dwc_ddrphy_apb_wr(0x200f2,0x4444);
dwc_ddrphy_apb_wr(0x200f3,0x8888);
dwc_ddrphy_apb_wr(0x200f4,0x5555);
dwc_ddrphy_apb_wr(0x200f5,0x0);
dwc_ddrphy_apb_wr(0x200f6,0x0);
dwc_ddrphy_apb_wr(0x200f7,0xf000);
// // [phyinit_C_initPhyConfig] Disabling Lane 8 Receiver to save power.0
dwc_ddrphy_apb_wr(0x1004a,0x500);
// // [phyinit_C_initPhyConfig] Disabling Lane 8 Receiver to save power.1
dwc_ddrphy_apb_wr(0x1104a,0x500);
// // [phyinit_C_initPhyConfig] Programming MasterX4Config::X4TG to 0x0
dwc_ddrphy_apb_wr(0x20025,0x0);
// // [phyinit_C_initPhyConfig] Pstate=0, Memclk=800MHz, Programming DMIPinPresent::RdDbiEnabled to 0x0
dwc_ddrphy_apb_wr(0x2002d,0x0);
dwc_ddrphy_apb_wr(0x2002c,0x0);
// // [phyinit_C_initPhyConfig] End of dwc_ddrphy_phyinit_C_initPhyConfig()
// 
// 
// //##############################################################
// //
// // dwc_ddrphy_phyihunit_userCustom_customPreTrain is a user-editable function.
// //
// // See PhyInit App Note for detailed description and function usage
// //
// //##############################################################
// 
// // [phyinit_userCustom_customPreTrain] Start of dwc_ddrphy_phyinit_userCustom_customPreTrain()
// // [phyinit_userCustom_customPreTrain] End of dwc_ddrphy_phyinit_userCustom_customPreTrain()


// //##############################################################
// //
// // Training firmware is *NOT* executed. This function replaces these steps
// // in the PHY Initialization sequence:
// //
// //  (E) Set the PHY input clocks to the desired frequency 
// //  (F) Write the Message Block parameters for the training firmware 
// //  (G) Execute the Training Firmware 
// //  (H) Read the Message Block results
// //
// //##############################################################

printf("**SKIP TRAINING*\n");
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Start of dwc_ddrphy_phyinit_progCsrSkipTrain()
// [dwc_ddrphy_phyinit_progCsrSkipTrain] NumRank_total = 1
// [dwc_ddrphy_phyinit_progCsrSkipTrain] PHY_Rx_Fifo_Dly = 2700
// [dwc_ddrphy_phyinit_progCsrSkipTrain] PHY_Tx_Insertion_Dly = 200
// [dwc_ddrphy_phyinit_progCsrSkipTrain] PHY_Rx_Insertion_Dly = 200
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming DFIMRL to 0x5
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming HwtMRL to 0x5
dwc_ddrphy_apb_wr(0x10020,0x5);
dwc_ddrphy_apb_wr(0x11020,0x5);
dwc_ddrphy_apb_wr(0x20020,0x5);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming TxDqsDlyTg0 to 0x100
dwc_ddrphy_apb_wr(0x100d0,0x100);
dwc_ddrphy_apb_wr(0x101d0,0x100);
dwc_ddrphy_apb_wr(0x110d0,0x100);
dwc_ddrphy_apb_wr(0x111d0,0x100);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming TxDqDlyTg0 to 0x1b
dwc_ddrphy_apb_wr(0x100c0,0x1b);
dwc_ddrphy_apb_wr(0x101c0,0x1b);
dwc_ddrphy_apb_wr(0x102c0,0x1b);
dwc_ddrphy_apb_wr(0x103c0,0x1b);
dwc_ddrphy_apb_wr(0x104c0,0x1b);
dwc_ddrphy_apb_wr(0x105c0,0x1b);
dwc_ddrphy_apb_wr(0x106c0,0x1b);
dwc_ddrphy_apb_wr(0x107c0,0x1b);
dwc_ddrphy_apb_wr(0x108c0,0x1b);
dwc_ddrphy_apb_wr(0x110c0,0x1b);
dwc_ddrphy_apb_wr(0x111c0,0x1b);
dwc_ddrphy_apb_wr(0x112c0,0x1b);
dwc_ddrphy_apb_wr(0x113c0,0x1b);
dwc_ddrphy_apb_wr(0x114c0,0x1b);
dwc_ddrphy_apb_wr(0x115c0,0x1b);
dwc_ddrphy_apb_wr(0x116c0,0x1b);
dwc_ddrphy_apb_wr(0x117c0,0x1b);
dwc_ddrphy_apb_wr(0x118c0,0x1b);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming RxEnDly_10to6=7, Rxendly_5to0=4
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming RxEnDlyTg0 to 0x1c4
dwc_ddrphy_apb_wr(0x10080,0x1c4);
dwc_ddrphy_apb_wr(0x10180,0x1c4);
dwc_ddrphy_apb_wr(0x11080,0x1c4);
dwc_ddrphy_apb_wr(0x11180,0x1c4);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Programming PIE RL=14 WL=8
dwc_ddrphy_apb_wr(0x90201,0xc00);
dwc_ddrphy_apb_wr(0x90202,0x6);
dwc_ddrphy_apb_wr(0x90203,0x1800);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming HwtLpCsEnA to 0x1
dwc_ddrphy_apb_wr(0x20072,0x1);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming HwtLpCsEnB to 0x0
dwc_ddrphy_apb_wr(0x20073,0x0);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming PptDqsCntInvTrnTg0 to 0xb
dwc_ddrphy_apb_wr(0x100ae,0xb);
dwc_ddrphy_apb_wr(0x110ae,0xb);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming PptCtlStatic CSR
dwc_ddrphy_apb_wr(0x100aa,0x501);
dwc_ddrphy_apb_wr(0x110aa,0x50d);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming HwtCAMode to 0x34
dwc_ddrphy_apb_wr(0x20077,0x34);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming DllGainCtl::DllGainIV=0x3, DllGainTV=0x5
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming DllGainCtl to 0x53
dwc_ddrphy_apb_wr(0x2007c,0x53);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming DllLockParam::LcdlSeed0 to 126 
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Pstate=0, Memclk=800MHz, Programming DllLockParam to 0x7e2
dwc_ddrphy_apb_wr(0x2007d,0x7e2);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming AcsmCtrl23 to 0x10f
dwc_ddrphy_apb_wr(0x400c0,0x10f);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming PllCtrl3::PllDacValIn to 0x10
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming PllCtrl3::PllForceCal to 0x1
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming PllCtrl3::PllMaxRange to 0x1f
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Programming PllCtrl3 to 0x61f0
dwc_ddrphy_apb_wr(0x200cb,0x61f0);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] Special skipTraining configuration to Prevernt DRAM Commands on the first dfi status interface handshake.
// [dwc_ddrphy_phyinit_progCsrSkipTrain] In order to see this behavior, the frist dfi_freq should be in the range of 0x0f < dfi_freq_sel[4:0] < 0x14.
dwc_ddrphy_apb_wr(0x90028,0x0);
// [dwc_ddrphy_phyinit_progCsrSkipTrain] End of dwc_ddrphy_phyinit_progCsrSkipTrain()
// // [phyinit_I_loadPIEImage] Start of dwc_ddrphy_phyinit_I_loadPIEImage()
// 
// 
// //##############################################################
// //
// // (I) Load PHY Init Engine Image 
// // 
// // Load the PHY Initialization Engine memory with the provided initialization sequence.
// // See PhyInit App Note for detailed description and function usage
// // 
// // For LPDDR3/LPDDR4, this sequence will include the necessary retraining code.
// // 
// //##############################################################
// 
// 
// // Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0. 
// // This allows the memory controller unrestricted access to the configuration CSRs. 
printf("**STEP I*\n");

dwc_ddrphy_apb_wr(0xd0000,0x0);
// // [phyinit_I_loadPIEImage] Programming PIE Production Code
dwc_ddrphy_apb_wr(0x90000,0x10);
dwc_ddrphy_apb_wr(0x90001,0x400);
dwc_ddrphy_apb_wr(0x90002,0x10e);
dwc_ddrphy_apb_wr(0x90003,0x0);
dwc_ddrphy_apb_wr(0x90004,0x0);
dwc_ddrphy_apb_wr(0x90005,0x8);
dwc_ddrphy_apb_wr(0x90029,0xb);
dwc_ddrphy_apb_wr(0x9002a,0x480);
dwc_ddrphy_apb_wr(0x9002b,0x109);
dwc_ddrphy_apb_wr(0x9002c,0x8);
dwc_ddrphy_apb_wr(0x9002d,0x448);
dwc_ddrphy_apb_wr(0x9002e,0x139);
dwc_ddrphy_apb_wr(0x9002f,0x8);
dwc_ddrphy_apb_wr(0x90030,0x478);
dwc_ddrphy_apb_wr(0x90031,0x109);
dwc_ddrphy_apb_wr(0x90032,0x0);
dwc_ddrphy_apb_wr(0x90033,0xe8);
dwc_ddrphy_apb_wr(0x90034,0x109);
dwc_ddrphy_apb_wr(0x90035,0x2);
dwc_ddrphy_apb_wr(0x90036,0x10);
dwc_ddrphy_apb_wr(0x90037,0x139);
dwc_ddrphy_apb_wr(0x90038,0xb);
dwc_ddrphy_apb_wr(0x90039,0x7c0);
dwc_ddrphy_apb_wr(0x9003a,0x139);
dwc_ddrphy_apb_wr(0x9003b,0x44);
dwc_ddrphy_apb_wr(0x9003c,0x633);
dwc_ddrphy_apb_wr(0x9003d,0x159);
dwc_ddrphy_apb_wr(0x9003e,0x14f);
dwc_ddrphy_apb_wr(0x9003f,0x630);
dwc_ddrphy_apb_wr(0x90040,0x159);
dwc_ddrphy_apb_wr(0x90041,0x47);
dwc_ddrphy_apb_wr(0x90042,0x633);
dwc_ddrphy_apb_wr(0x90043,0x149);
dwc_ddrphy_apb_wr(0x90044,0x4f);
dwc_ddrphy_apb_wr(0x90045,0x633);
dwc_ddrphy_apb_wr(0x90046,0x179);
dwc_ddrphy_apb_wr(0x90047,0x8);
dwc_ddrphy_apb_wr(0x90048,0xe0);
dwc_ddrphy_apb_wr(0x90049,0x109);
dwc_ddrphy_apb_wr(0x9004a,0x0);
dwc_ddrphy_apb_wr(0x9004b,0x7c8);
dwc_ddrphy_apb_wr(0x9004c,0x109);
dwc_ddrphy_apb_wr(0x9004d,0x0);
dwc_ddrphy_apb_wr(0x9004e,0x1);
dwc_ddrphy_apb_wr(0x9004f,0x8);
dwc_ddrphy_apb_wr(0x90050,0x0);
dwc_ddrphy_apb_wr(0x90051,0x45a);
dwc_ddrphy_apb_wr(0x90052,0x9);
dwc_ddrphy_apb_wr(0x90053,0x0);
dwc_ddrphy_apb_wr(0x90054,0x448);
dwc_ddrphy_apb_wr(0x90055,0x109);
dwc_ddrphy_apb_wr(0x90056,0x40);
dwc_ddrphy_apb_wr(0x90057,0x633);
dwc_ddrphy_apb_wr(0x90058,0x179);
dwc_ddrphy_apb_wr(0x90059,0x1);
dwc_ddrphy_apb_wr(0x9005a,0x618);
dwc_ddrphy_apb_wr(0x9005b,0x109);
dwc_ddrphy_apb_wr(0x9005c,0x40c0);
dwc_ddrphy_apb_wr(0x9005d,0x633);
dwc_ddrphy_apb_wr(0x9005e,0x149);
dwc_ddrphy_apb_wr(0x9005f,0x8);
dwc_ddrphy_apb_wr(0x90060,0x4);
dwc_ddrphy_apb_wr(0x90061,0x48);
dwc_ddrphy_apb_wr(0x90062,0x4040);
dwc_ddrphy_apb_wr(0x90063,0x633);
dwc_ddrphy_apb_wr(0x90064,0x149);
dwc_ddrphy_apb_wr(0x90065,0x0);
dwc_ddrphy_apb_wr(0x90066,0x4);
dwc_ddrphy_apb_wr(0x90067,0x48);
dwc_ddrphy_apb_wr(0x90068,0x40);
dwc_ddrphy_apb_wr(0x90069,0x633);
dwc_ddrphy_apb_wr(0x9006a,0x149);
dwc_ddrphy_apb_wr(0x9006b,0x10);
dwc_ddrphy_apb_wr(0x9006c,0x4);
dwc_ddrphy_apb_wr(0x9006d,0x18);
dwc_ddrphy_apb_wr(0x9006e,0x0);
dwc_ddrphy_apb_wr(0x9006f,0x4);
dwc_ddrphy_apb_wr(0x90070,0x78);
dwc_ddrphy_apb_wr(0x90071,0x549);
dwc_ddrphy_apb_wr(0x90072,0x633);
dwc_ddrphy_apb_wr(0x90073,0x159);
dwc_ddrphy_apb_wr(0x90074,0xd49);
dwc_ddrphy_apb_wr(0x90075,0x633);
dwc_ddrphy_apb_wr(0x90076,0x159);
dwc_ddrphy_apb_wr(0x90077,0x94a);
dwc_ddrphy_apb_wr(0x90078,0x633);
dwc_ddrphy_apb_wr(0x90079,0x159);
dwc_ddrphy_apb_wr(0x9007a,0x441);
dwc_ddrphy_apb_wr(0x9007b,0x633);
dwc_ddrphy_apb_wr(0x9007c,0x149);
dwc_ddrphy_apb_wr(0x9007d,0x42);
dwc_ddrphy_apb_wr(0x9007e,0x633);
dwc_ddrphy_apb_wr(0x9007f,0x149);
dwc_ddrphy_apb_wr(0x90080,0x1);
dwc_ddrphy_apb_wr(0x90081,0x633);
dwc_ddrphy_apb_wr(0x90082,0x149);
dwc_ddrphy_apb_wr(0x90083,0x0);
dwc_ddrphy_apb_wr(0x90084,0xe0);
dwc_ddrphy_apb_wr(0x90085,0x109);
dwc_ddrphy_apb_wr(0x90086,0xa);
dwc_ddrphy_apb_wr(0x90087,0x10);
dwc_ddrphy_apb_wr(0x90088,0x109);
dwc_ddrphy_apb_wr(0x90089,0x9);
dwc_ddrphy_apb_wr(0x9008a,0x3c0);
dwc_ddrphy_apb_wr(0x9008b,0x149);
dwc_ddrphy_apb_wr(0x9008c,0x9);
dwc_ddrphy_apb_wr(0x9008d,0x3c0);
dwc_ddrphy_apb_wr(0x9008e,0x159);
dwc_ddrphy_apb_wr(0x9008f,0x18);
dwc_ddrphy_apb_wr(0x90090,0x10);
dwc_ddrphy_apb_wr(0x90091,0x109);
dwc_ddrphy_apb_wr(0x90092,0x0);
dwc_ddrphy_apb_wr(0x90093,0x3c0);
dwc_ddrphy_apb_wr(0x90094,0x109);
dwc_ddrphy_apb_wr(0x90095,0x18);
dwc_ddrphy_apb_wr(0x90096,0x4);
dwc_ddrphy_apb_wr(0x90097,0x48);
dwc_ddrphy_apb_wr(0x90098,0x18);
dwc_ddrphy_apb_wr(0x90099,0x4);
dwc_ddrphy_apb_wr(0x9009a,0x58);
dwc_ddrphy_apb_wr(0x9009b,0xb);
dwc_ddrphy_apb_wr(0x9009c,0x10);
dwc_ddrphy_apb_wr(0x9009d,0x109);
dwc_ddrphy_apb_wr(0x9009e,0x1);
dwc_ddrphy_apb_wr(0x9009f,0x10);
dwc_ddrphy_apb_wr(0x900a0,0x109);
dwc_ddrphy_apb_wr(0x900a1,0x5);
dwc_ddrphy_apb_wr(0x900a2,0x7c0);
dwc_ddrphy_apb_wr(0x900a3,0x109);
dwc_ddrphy_apb_wr(0x40000,0x811);
dwc_ddrphy_apb_wr(0x40020,0x880);
dwc_ddrphy_apb_wr(0x40040,0x0);
dwc_ddrphy_apb_wr(0x40060,0x0);
dwc_ddrphy_apb_wr(0x40001,0x4008);
dwc_ddrphy_apb_wr(0x40021,0x83);
dwc_ddrphy_apb_wr(0x40041,0x4f);
dwc_ddrphy_apb_wr(0x40061,0x0);
dwc_ddrphy_apb_wr(0x40002,0x4040);
dwc_ddrphy_apb_wr(0x40022,0x83);
dwc_ddrphy_apb_wr(0x40042,0x51);
dwc_ddrphy_apb_wr(0x40062,0x0);
dwc_ddrphy_apb_wr(0x40003,0x811);
dwc_ddrphy_apb_wr(0x40023,0x880);
dwc_ddrphy_apb_wr(0x40043,0x0);
dwc_ddrphy_apb_wr(0x40063,0x0);
dwc_ddrphy_apb_wr(0x40004,0x720);
dwc_ddrphy_apb_wr(0x40024,0xf);
dwc_ddrphy_apb_wr(0x40044,0x1740);
dwc_ddrphy_apb_wr(0x40064,0x0);
dwc_ddrphy_apb_wr(0x40005,0x16);
dwc_ddrphy_apb_wr(0x40025,0x83);
dwc_ddrphy_apb_wr(0x40045,0x4b);
dwc_ddrphy_apb_wr(0x40065,0x0);
dwc_ddrphy_apb_wr(0x40006,0x716);
dwc_ddrphy_apb_wr(0x40026,0xf);
dwc_ddrphy_apb_wr(0x40046,0x2001);
dwc_ddrphy_apb_wr(0x40066,0x0);
dwc_ddrphy_apb_wr(0x40007,0x716);
dwc_ddrphy_apb_wr(0x40027,0xf);
dwc_ddrphy_apb_wr(0x40047,0x2800);
dwc_ddrphy_apb_wr(0x40067,0x0);
dwc_ddrphy_apb_wr(0x40008,0x716);
dwc_ddrphy_apb_wr(0x40028,0xf);
dwc_ddrphy_apb_wr(0x40048,0xf00);
dwc_ddrphy_apb_wr(0x40068,0x0);
dwc_ddrphy_apb_wr(0x40009,0x720);
dwc_ddrphy_apb_wr(0x40029,0xf);
dwc_ddrphy_apb_wr(0x40049,0x1400);
dwc_ddrphy_apb_wr(0x40069,0x0);
dwc_ddrphy_apb_wr(0x4000a,0xe08);
dwc_ddrphy_apb_wr(0x4002a,0xc15);
dwc_ddrphy_apb_wr(0x4004a,0x0);
dwc_ddrphy_apb_wr(0x4006a,0x0);
dwc_ddrphy_apb_wr(0x4000b,0x625);
dwc_ddrphy_apb_wr(0x4002b,0x15);
dwc_ddrphy_apb_wr(0x4004b,0x0);
dwc_ddrphy_apb_wr(0x4006b,0x0);
dwc_ddrphy_apb_wr(0x4000c,0x4028);
dwc_ddrphy_apb_wr(0x4002c,0x80);
dwc_ddrphy_apb_wr(0x4004c,0x0);
dwc_ddrphy_apb_wr(0x4006c,0x0);
dwc_ddrphy_apb_wr(0x4000d,0xe08);
dwc_ddrphy_apb_wr(0x4002d,0xc1a);
dwc_ddrphy_apb_wr(0x4004d,0x0);
dwc_ddrphy_apb_wr(0x4006d,0x0);
dwc_ddrphy_apb_wr(0x4000e,0x625);
dwc_ddrphy_apb_wr(0x4002e,0x1a);
dwc_ddrphy_apb_wr(0x4004e,0x0);
dwc_ddrphy_apb_wr(0x4006e,0x0);
dwc_ddrphy_apb_wr(0x4000f,0x4040);
dwc_ddrphy_apb_wr(0x4002f,0x80);
dwc_ddrphy_apb_wr(0x4004f,0x0);
dwc_ddrphy_apb_wr(0x4006f,0x0);
dwc_ddrphy_apb_wr(0x40010,0x2604);
dwc_ddrphy_apb_wr(0x40030,0x15);
dwc_ddrphy_apb_wr(0x40050,0x0);
dwc_ddrphy_apb_wr(0x40070,0x0);
dwc_ddrphy_apb_wr(0x40011,0x708);
dwc_ddrphy_apb_wr(0x40031,0x5);
dwc_ddrphy_apb_wr(0x40051,0x0);
dwc_ddrphy_apb_wr(0x40071,0x2002);
dwc_ddrphy_apb_wr(0x40012,0x8);
dwc_ddrphy_apb_wr(0x40032,0x80);
dwc_ddrphy_apb_wr(0x40052,0x0);
dwc_ddrphy_apb_wr(0x40072,0x0);
dwc_ddrphy_apb_wr(0x40013,0x2604);
dwc_ddrphy_apb_wr(0x40033,0x1a);
dwc_ddrphy_apb_wr(0x40053,0x0);
dwc_ddrphy_apb_wr(0x40073,0x0);
dwc_ddrphy_apb_wr(0x40014,0x708);
dwc_ddrphy_apb_wr(0x40034,0xa);
dwc_ddrphy_apb_wr(0x40054,0x0);
dwc_ddrphy_apb_wr(0x40074,0x2002);
dwc_ddrphy_apb_wr(0x40015,0x4040);
dwc_ddrphy_apb_wr(0x40035,0x80);
dwc_ddrphy_apb_wr(0x40055,0x0);
dwc_ddrphy_apb_wr(0x40075,0x0);
dwc_ddrphy_apb_wr(0x40016,0x60a);
dwc_ddrphy_apb_wr(0x40036,0x15);
dwc_ddrphy_apb_wr(0x40056,0x1200);
dwc_ddrphy_apb_wr(0x40076,0x0);
dwc_ddrphy_apb_wr(0x40017,0x61a);
dwc_ddrphy_apb_wr(0x40037,0x15);
dwc_ddrphy_apb_wr(0x40057,0x1300);
dwc_ddrphy_apb_wr(0x40077,0x0);
dwc_ddrphy_apb_wr(0x40018,0x60a);
dwc_ddrphy_apb_wr(0x40038,0x1a);
dwc_ddrphy_apb_wr(0x40058,0x1200);
dwc_ddrphy_apb_wr(0x40078,0x0);
dwc_ddrphy_apb_wr(0x40019,0x642);
dwc_ddrphy_apb_wr(0x40039,0x1a);
dwc_ddrphy_apb_wr(0x40059,0x1300);
dwc_ddrphy_apb_wr(0x40079,0x0);
dwc_ddrphy_apb_wr(0x4001a,0x4808);
dwc_ddrphy_apb_wr(0x4003a,0x880);
dwc_ddrphy_apb_wr(0x4005a,0x0);
dwc_ddrphy_apb_wr(0x4007a,0x0);
dwc_ddrphy_apb_wr(0x900a4,0x0);
dwc_ddrphy_apb_wr(0x900a5,0x790);
dwc_ddrphy_apb_wr(0x900a6,0x11a);
dwc_ddrphy_apb_wr(0x900a7,0x8);
dwc_ddrphy_apb_wr(0x900a8,0x7aa);
dwc_ddrphy_apb_wr(0x900a9,0x2a);
dwc_ddrphy_apb_wr(0x900aa,0x10);
dwc_ddrphy_apb_wr(0x900ab,0x7b2);
dwc_ddrphy_apb_wr(0x900ac,0x2a);
dwc_ddrphy_apb_wr(0x900ad,0x0);
dwc_ddrphy_apb_wr(0x900ae,0x7c8);
dwc_ddrphy_apb_wr(0x900af,0x109);
dwc_ddrphy_apb_wr(0x900b0,0x10);
dwc_ddrphy_apb_wr(0x900b1,0x10);
dwc_ddrphy_apb_wr(0x900b2,0x109);
dwc_ddrphy_apb_wr(0x900b3,0x10);
dwc_ddrphy_apb_wr(0x900b4,0x2a8);
dwc_ddrphy_apb_wr(0x900b5,0x129);
dwc_ddrphy_apb_wr(0x900b6,0x8);
dwc_ddrphy_apb_wr(0x900b7,0x370);
dwc_ddrphy_apb_wr(0x900b8,0x129);
dwc_ddrphy_apb_wr(0x900b9,0xa);
dwc_ddrphy_apb_wr(0x900ba,0x3c8);
dwc_ddrphy_apb_wr(0x900bb,0x1a9);
dwc_ddrphy_apb_wr(0x900bc,0xc);
dwc_ddrphy_apb_wr(0x900bd,0x408);
dwc_ddrphy_apb_wr(0x900be,0x199);
dwc_ddrphy_apb_wr(0x900bf,0x14);
dwc_ddrphy_apb_wr(0x900c0,0x790);
dwc_ddrphy_apb_wr(0x900c1,0x11a);
dwc_ddrphy_apb_wr(0x900c2,0x8);
dwc_ddrphy_apb_wr(0x900c3,0x4);
dwc_ddrphy_apb_wr(0x900c4,0x18);
dwc_ddrphy_apb_wr(0x900c5,0xe);
dwc_ddrphy_apb_wr(0x900c6,0x408);
dwc_ddrphy_apb_wr(0x900c7,0x199);
dwc_ddrphy_apb_wr(0x900c8,0x8);
dwc_ddrphy_apb_wr(0x900c9,0x8568);
dwc_ddrphy_apb_wr(0x900ca,0x108);
dwc_ddrphy_apb_wr(0x900cb,0x18);
dwc_ddrphy_apb_wr(0x900cc,0x790);
dwc_ddrphy_apb_wr(0x900cd,0x16a);
dwc_ddrphy_apb_wr(0x900ce,0x8);
dwc_ddrphy_apb_wr(0x900cf,0x1d8);
dwc_ddrphy_apb_wr(0x900d0,0x169);
dwc_ddrphy_apb_wr(0x900d1,0x10);
dwc_ddrphy_apb_wr(0x900d2,0x8558);
dwc_ddrphy_apb_wr(0x900d3,0x168);
dwc_ddrphy_apb_wr(0x900d4,0x70);
dwc_ddrphy_apb_wr(0x900d5,0x788);
dwc_ddrphy_apb_wr(0x900d6,0x16a);
dwc_ddrphy_apb_wr(0x900d7,0x1ff8);
dwc_ddrphy_apb_wr(0x900d8,0x85a8);
dwc_ddrphy_apb_wr(0x900d9,0x1e8);
dwc_ddrphy_apb_wr(0x900da,0x50);
dwc_ddrphy_apb_wr(0x900db,0x798);
dwc_ddrphy_apb_wr(0x900dc,0x16a);
dwc_ddrphy_apb_wr(0x900dd,0x60);
dwc_ddrphy_apb_wr(0x900de,0x7a0);
dwc_ddrphy_apb_wr(0x900df,0x16a);
dwc_ddrphy_apb_wr(0x900e0,0x8);
dwc_ddrphy_apb_wr(0x900e1,0x8310);
dwc_ddrphy_apb_wr(0x900e2,0x168);
dwc_ddrphy_apb_wr(0x900e3,0x8);
dwc_ddrphy_apb_wr(0x900e4,0xa310);
dwc_ddrphy_apb_wr(0x900e5,0x168);
dwc_ddrphy_apb_wr(0x900e6,0xa);
dwc_ddrphy_apb_wr(0x900e7,0x408);
dwc_ddrphy_apb_wr(0x900e8,0x169);
dwc_ddrphy_apb_wr(0x900e9,0x6e);
dwc_ddrphy_apb_wr(0x900ea,0x0);
dwc_ddrphy_apb_wr(0x900eb,0x68);
dwc_ddrphy_apb_wr(0x900ec,0x0);
dwc_ddrphy_apb_wr(0x900ed,0x408);
dwc_ddrphy_apb_wr(0x900ee,0x169);
dwc_ddrphy_apb_wr(0x900ef,0x0);
dwc_ddrphy_apb_wr(0x900f0,0x8310);
dwc_ddrphy_apb_wr(0x900f1,0x168);
dwc_ddrphy_apb_wr(0x900f2,0x0);
dwc_ddrphy_apb_wr(0x900f3,0xa310);
dwc_ddrphy_apb_wr(0x900f4,0x168);
dwc_ddrphy_apb_wr(0x900f5,0x1ff8);
dwc_ddrphy_apb_wr(0x900f6,0x85a8);
dwc_ddrphy_apb_wr(0x900f7,0x1e8);
dwc_ddrphy_apb_wr(0x900f8,0x68);
dwc_ddrphy_apb_wr(0x900f9,0x798);
dwc_ddrphy_apb_wr(0x900fa,0x16a);
dwc_ddrphy_apb_wr(0x900fb,0x78);
dwc_ddrphy_apb_wr(0x900fc,0x7a0);
dwc_ddrphy_apb_wr(0x900fd,0x16a);
dwc_ddrphy_apb_wr(0x900fe,0x68);
dwc_ddrphy_apb_wr(0x900ff,0x790);
dwc_ddrphy_apb_wr(0x90100,0x16a);
dwc_ddrphy_apb_wr(0x90101,0x8);
dwc_ddrphy_apb_wr(0x90102,0x8b10);
dwc_ddrphy_apb_wr(0x90103,0x168);
dwc_ddrphy_apb_wr(0x90104,0x8);
dwc_ddrphy_apb_wr(0x90105,0xab10);
dwc_ddrphy_apb_wr(0x90106,0x168);
dwc_ddrphy_apb_wr(0x90107,0xa);
dwc_ddrphy_apb_wr(0x90108,0x408);
dwc_ddrphy_apb_wr(0x90109,0x169);
dwc_ddrphy_apb_wr(0x9010a,0x58);
dwc_ddrphy_apb_wr(0x9010b,0x0);
dwc_ddrphy_apb_wr(0x9010c,0x68);
dwc_ddrphy_apb_wr(0x9010d,0x0);
dwc_ddrphy_apb_wr(0x9010e,0x408);
dwc_ddrphy_apb_wr(0x9010f,0x169);
dwc_ddrphy_apb_wr(0x90110,0x0);
dwc_ddrphy_apb_wr(0x90111,0x8b10);
dwc_ddrphy_apb_wr(0x90112,0x168);
dwc_ddrphy_apb_wr(0x90113,0x1);
dwc_ddrphy_apb_wr(0x90114,0xab10);
dwc_ddrphy_apb_wr(0x90115,0x168);
dwc_ddrphy_apb_wr(0x90116,0x0);
dwc_ddrphy_apb_wr(0x90117,0x1d8);
dwc_ddrphy_apb_wr(0x90118,0x169);
dwc_ddrphy_apb_wr(0x90119,0x80);
dwc_ddrphy_apb_wr(0x9011a,0x790);
dwc_ddrphy_apb_wr(0x9011b,0x16a);
dwc_ddrphy_apb_wr(0x9011c,0x18);
dwc_ddrphy_apb_wr(0x9011d,0x7aa);
dwc_ddrphy_apb_wr(0x9011e,0x6a);
dwc_ddrphy_apb_wr(0x9011f,0xa);
dwc_ddrphy_apb_wr(0x90120,0x0);
dwc_ddrphy_apb_wr(0x90121,0x1e9);
dwc_ddrphy_apb_wr(0x90122,0x8);
dwc_ddrphy_apb_wr(0x90123,0x8080);
dwc_ddrphy_apb_wr(0x90124,0x108);
dwc_ddrphy_apb_wr(0x90125,0xf);
dwc_ddrphy_apb_wr(0x90126,0x408);
dwc_ddrphy_apb_wr(0x90127,0x169);
dwc_ddrphy_apb_wr(0x90128,0xc);
dwc_ddrphy_apb_wr(0x90129,0x0);
dwc_ddrphy_apb_wr(0x9012a,0x68);
dwc_ddrphy_apb_wr(0x9012b,0x9);
dwc_ddrphy_apb_wr(0x9012c,0x0);
dwc_ddrphy_apb_wr(0x9012d,0x1a9);
dwc_ddrphy_apb_wr(0x9012e,0x0);
dwc_ddrphy_apb_wr(0x9012f,0x408);
dwc_ddrphy_apb_wr(0x90130,0x169);
dwc_ddrphy_apb_wr(0x90131,0x0);
dwc_ddrphy_apb_wr(0x90132,0x8080);
dwc_ddrphy_apb_wr(0x90133,0x108);
dwc_ddrphy_apb_wr(0x90134,0x8);
dwc_ddrphy_apb_wr(0x90135,0x7aa);
dwc_ddrphy_apb_wr(0x90136,0x6a);
dwc_ddrphy_apb_wr(0x90137,0x0);
dwc_ddrphy_apb_wr(0x90138,0x8568);
dwc_ddrphy_apb_wr(0x90139,0x108);
dwc_ddrphy_apb_wr(0x9013a,0xb7);
dwc_ddrphy_apb_wr(0x9013b,0x790);
dwc_ddrphy_apb_wr(0x9013c,0x16a);
dwc_ddrphy_apb_wr(0x9013d,0x1f);
dwc_ddrphy_apb_wr(0x9013e,0x0);
dwc_ddrphy_apb_wr(0x9013f,0x68);
dwc_ddrphy_apb_wr(0x90140,0x8);
dwc_ddrphy_apb_wr(0x90141,0x8558);
dwc_ddrphy_apb_wr(0x90142,0x168);
dwc_ddrphy_apb_wr(0x90143,0xf);
dwc_ddrphy_apb_wr(0x90144,0x408);
dwc_ddrphy_apb_wr(0x90145,0x169);
dwc_ddrphy_apb_wr(0x90146,0xd);
dwc_ddrphy_apb_wr(0x90147,0x0);
dwc_ddrphy_apb_wr(0x90148,0x68);
dwc_ddrphy_apb_wr(0x90149,0x0);
dwc_ddrphy_apb_wr(0x9014a,0x408);
dwc_ddrphy_apb_wr(0x9014b,0x169);
dwc_ddrphy_apb_wr(0x9014c,0x0);
dwc_ddrphy_apb_wr(0x9014d,0x8558);
dwc_ddrphy_apb_wr(0x9014e,0x168);
dwc_ddrphy_apb_wr(0x9014f,0x8);
dwc_ddrphy_apb_wr(0x90150,0x3c8);
dwc_ddrphy_apb_wr(0x90151,0x1a9);
dwc_ddrphy_apb_wr(0x90152,0x3);
dwc_ddrphy_apb_wr(0x90153,0x370);
dwc_ddrphy_apb_wr(0x90154,0x129);
dwc_ddrphy_apb_wr(0x90155,0x20);
dwc_ddrphy_apb_wr(0x90156,0x2aa);
dwc_ddrphy_apb_wr(0x90157,0x9);
dwc_ddrphy_apb_wr(0x90158,0x0);
dwc_ddrphy_apb_wr(0x90159,0x400);
dwc_ddrphy_apb_wr(0x9015a,0x10e);
dwc_ddrphy_apb_wr(0x9015b,0x8);
dwc_ddrphy_apb_wr(0x9015c,0xe8);
dwc_ddrphy_apb_wr(0x9015d,0x109);
dwc_ddrphy_apb_wr(0x9015e,0x0);
dwc_ddrphy_apb_wr(0x9015f,0x8140);
dwc_ddrphy_apb_wr(0x90160,0x10c);
dwc_ddrphy_apb_wr(0x90161,0x10);
dwc_ddrphy_apb_wr(0x90162,0x8138);
dwc_ddrphy_apb_wr(0x90163,0x10c);
dwc_ddrphy_apb_wr(0x90164,0x8);
dwc_ddrphy_apb_wr(0x90165,0x7c8);
dwc_ddrphy_apb_wr(0x90166,0x101);
dwc_ddrphy_apb_wr(0x90167,0x8);
dwc_ddrphy_apb_wr(0x90168,0x448);
dwc_ddrphy_apb_wr(0x90169,0x109);
dwc_ddrphy_apb_wr(0x9016a,0xf);
dwc_ddrphy_apb_wr(0x9016b,0x7c0);
dwc_ddrphy_apb_wr(0x9016c,0x109);
dwc_ddrphy_apb_wr(0x9016d,0x0);
dwc_ddrphy_apb_wr(0x9016e,0xe8);
dwc_ddrphy_apb_wr(0x9016f,0x109);
dwc_ddrphy_apb_wr(0x90170,0x47);
dwc_ddrphy_apb_wr(0x90171,0x630);
dwc_ddrphy_apb_wr(0x90172,0x109);
dwc_ddrphy_apb_wr(0x90173,0x8);
dwc_ddrphy_apb_wr(0x90174,0x618);
dwc_ddrphy_apb_wr(0x90175,0x109);
dwc_ddrphy_apb_wr(0x90176,0x8);
dwc_ddrphy_apb_wr(0x90177,0xe0);
dwc_ddrphy_apb_wr(0x90178,0x109);
dwc_ddrphy_apb_wr(0x90179,0x0);
dwc_ddrphy_apb_wr(0x9017a,0x7c8);
dwc_ddrphy_apb_wr(0x9017b,0x109);
dwc_ddrphy_apb_wr(0x9017c,0x8);
dwc_ddrphy_apb_wr(0x9017d,0x8140);
dwc_ddrphy_apb_wr(0x9017e,0x10c);
dwc_ddrphy_apb_wr(0x9017f,0x0);
dwc_ddrphy_apb_wr(0x90180,0x478);
dwc_ddrphy_apb_wr(0x90181,0x109);
dwc_ddrphy_apb_wr(0x90182,0x0);
dwc_ddrphy_apb_wr(0x90183,0x1);
dwc_ddrphy_apb_wr(0x90184,0x8);
dwc_ddrphy_apb_wr(0x90185,0x8);
dwc_ddrphy_apb_wr(0x90186,0x4);
dwc_ddrphy_apb_wr(0x90187,0x8);
dwc_ddrphy_apb_wr(0x90188,0x8);
dwc_ddrphy_apb_wr(0x90189,0x7c8);
dwc_ddrphy_apb_wr(0x9018a,0x101);
dwc_ddrphy_apb_wr(0x90006,0x0);
dwc_ddrphy_apb_wr(0x90007,0x0);
dwc_ddrphy_apb_wr(0x90008,0x8);
dwc_ddrphy_apb_wr(0x90009,0x0);
dwc_ddrphy_apb_wr(0x9000a,0x0);
dwc_ddrphy_apb_wr(0x9000b,0x0);
dwc_ddrphy_apb_wr(0xd00e7,0x400);
dwc_ddrphy_apb_wr(0x90017,0x0);
dwc_ddrphy_apb_wr(0x9001f,0x29);
dwc_ddrphy_apb_wr(0x90026,0x6a);
dwc_ddrphy_apb_wr(0x400d0,0x0);
dwc_ddrphy_apb_wr(0x400d1,0x101);
dwc_ddrphy_apb_wr(0x400d2,0x105);
dwc_ddrphy_apb_wr(0x400d3,0x107);
dwc_ddrphy_apb_wr(0x400d4,0x10f);
dwc_ddrphy_apb_wr(0x400d5,0x202);
dwc_ddrphy_apb_wr(0x400d6,0x20a);
dwc_ddrphy_apb_wr(0x400d7,0x20b);
dwc_ddrphy_apb_wr(0x2003a,0x2);
// // [phyinit_I_loadPIEImage] Pstate=0,  Memclk=800MHz, Programming Seq0BDLY0 to 0x32
dwc_ddrphy_apb_wr(0x2000b,0x32);
// // [phyinit_I_loadPIEImage] Pstate=0,  Memclk=800MHz, Programming Seq0BDLY1 to 0x64
dwc_ddrphy_apb_wr(0x2000c,0x64);
// // [phyinit_I_loadPIEImage] Pstate=0,  Memclk=800MHz, Programming Seq0BDLY2 to 0x3e8
dwc_ddrphy_apb_wr(0x2000d,0x3e8);
// // [phyinit_I_loadPIEImage] Pstate=0,  Memclk=800MHz, Programming Seq0BDLY3 to 0x2c
dwc_ddrphy_apb_wr(0x2000e,0x2c);
dwc_ddrphy_apb_wr(0x9000c,0x0);
dwc_ddrphy_apb_wr(0x9000d,0x173);
dwc_ddrphy_apb_wr(0x9000e,0x60);
dwc_ddrphy_apb_wr(0x9000f,0x6110);
dwc_ddrphy_apb_wr(0x90010,0x2152);
dwc_ddrphy_apb_wr(0x90011,0xdfbd);
// // [phyinit_I_loadPIEImage] Disabling DRAM drift compensation.
dwc_ddrphy_apb_wr(0x90012,0xffff);
dwc_ddrphy_apb_wr(0x90013,0x6152);
// // [phyinit_I_loadPIEImage] Pstate=0, Programming AcsmPlayback0x0 to 0xe0
dwc_ddrphy_apb_wr(0x40080,0xe0);
// // [phyinit_I_loadPIEImage] Pstate=0, Programming AcsmPlayback1x0 to 0x12
dwc_ddrphy_apb_wr(0x40081,0x12);
// // [phyinit_I_loadPIEImage] Pstate=0, Programming AcsmPlayback0x1 to 0xe0
dwc_ddrphy_apb_wr(0x40082,0xe0);
// // [phyinit_I_loadPIEImage] Pstate=0, Programming AcsmPlayback1x1 to 0x12
dwc_ddrphy_apb_wr(0x40083,0x12);
// // [phyinit_I_loadPIEImage] Pstate=0, Programming AcsmPlayback0x2 to 0xe0
dwc_ddrphy_apb_wr(0x40084,0xe0);
// // [phyinit_I_loadPIEImage] Pstate=0, Programming AcsmPlayback1x2 to 0x12
dwc_ddrphy_apb_wr(0x40085,0x12);
// // [phyinit_I_loadPIEImage] Programing Training Hardware Registers for mission mode retraining
dwc_ddrphy_apb_wr(0x400fd,0xf);
dwc_ddrphy_apb_wr(0x10011,0x1);
dwc_ddrphy_apb_wr(0x10012,0x1);
dwc_ddrphy_apb_wr(0x10013,0x180);
dwc_ddrphy_apb_wr(0x10018,0x1);
dwc_ddrphy_apb_wr(0x10002,0x6209);
dwc_ddrphy_apb_wr(0x100b2,0x1);
dwc_ddrphy_apb_wr(0x101b4,0x1);
dwc_ddrphy_apb_wr(0x102b4,0x1);
dwc_ddrphy_apb_wr(0x103b4,0x1);
dwc_ddrphy_apb_wr(0x104b4,0x1);
dwc_ddrphy_apb_wr(0x105b4,0x1);
dwc_ddrphy_apb_wr(0x106b4,0x1);
dwc_ddrphy_apb_wr(0x107b4,0x1);
dwc_ddrphy_apb_wr(0x108b4,0x1);
dwc_ddrphy_apb_wr(0x11011,0x1);
dwc_ddrphy_apb_wr(0x11012,0x1);
dwc_ddrphy_apb_wr(0x11013,0x180);
dwc_ddrphy_apb_wr(0x11018,0x1);
dwc_ddrphy_apb_wr(0x11002,0x6209);
dwc_ddrphy_apb_wr(0x110b2,0x1);
dwc_ddrphy_apb_wr(0x111b4,0x1);
dwc_ddrphy_apb_wr(0x112b4,0x1);
dwc_ddrphy_apb_wr(0x113b4,0x1);
dwc_ddrphy_apb_wr(0x114b4,0x1);
dwc_ddrphy_apb_wr(0x115b4,0x1);
dwc_ddrphy_apb_wr(0x116b4,0x1);
dwc_ddrphy_apb_wr(0x117b4,0x1);
dwc_ddrphy_apb_wr(0x118b4,0x1);
// // [phyinit_I_loadPIEImage] Turn on calibration and hold idle until dfi_init_start is asserted sequence is triggered.
dwc_ddrphy_apb_wr(0x20089,0x1);
// // [phyinit_I_loadPIEImage] Programming CalRate::CalInterval to 0x3
// // [phyinit_I_loadPIEImage] Programming CalRate::CalOnce to 0x0
// // [phyinit_I_loadPIEImage] Programming CalRate::CalRun to 0x1
dwc_ddrphy_apb_wr(0x20088,0x13);
// // [phyinit_I_loadPIEImage] Disabling Ucclk (PMU)
dwc_ddrphy_apb_wr(0xc0080,0x2);
// // [phyinit_I_loadPIEImage] Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1. 

dwc_ddrphy_apb_wr(0xd0000,0x0); //DWC_DDRPHYA_APBONLY0_MicroContMuxSel
dwc_ddrphy_apb_wr(0x10068,0x007F);
dwc_ddrphy_apb_wr(0x10069,0x007F);
dwc_ddrphy_apb_wr(0x1006A,0x007F);
dwc_ddrphy_apb_wr(0x1006B,0x007F);
dwc_ddrphy_apb_wr(0x10168,0x007F);
dwc_ddrphy_apb_wr(0x10169,0x007F);
dwc_ddrphy_apb_wr(0x1016A,0x007F);
dwc_ddrphy_apb_wr(0x1016B,0x007F);
dwc_ddrphy_apb_wr(0x10268,0x007F);
dwc_ddrphy_apb_wr(0x10269,0x007F);
dwc_ddrphy_apb_wr(0x1026A,0x007F);
dwc_ddrphy_apb_wr(0x1026B,0x007F);
dwc_ddrphy_apb_wr(0x10368,0x007F);
dwc_ddrphy_apb_wr(0x10369,0x007F);
dwc_ddrphy_apb_wr(0x1036A,0x007F);
dwc_ddrphy_apb_wr(0x1036B,0x007F);
dwc_ddrphy_apb_wr(0x10468,0x007F);
dwc_ddrphy_apb_wr(0x10469,0x007F);
dwc_ddrphy_apb_wr(0x1046A,0x007F);
dwc_ddrphy_apb_wr(0x1046B,0x007F);
dwc_ddrphy_apb_wr(0x10568,0x007F);
dwc_ddrphy_apb_wr(0x10569,0x007F);
dwc_ddrphy_apb_wr(0x1056A,0x007F);
dwc_ddrphy_apb_wr(0x1056B,0x007F);
dwc_ddrphy_apb_wr(0x10668,0x007F);
dwc_ddrphy_apb_wr(0x10669,0x007F);
dwc_ddrphy_apb_wr(0x1066A,0x007F);
dwc_ddrphy_apb_wr(0x1066B,0x007F);
dwc_ddrphy_apb_wr(0x10768,0x007F);
dwc_ddrphy_apb_wr(0x10769,0x007F);
dwc_ddrphy_apb_wr(0x1076A,0x007F);
dwc_ddrphy_apb_wr(0x1076B,0x007F);
dwc_ddrphy_apb_wr(0x10868,0x007F);
dwc_ddrphy_apb_wr(0x10869,0x007F);
dwc_ddrphy_apb_wr(0x1086A,0x007F);
dwc_ddrphy_apb_wr(0x1086B,0x007F);
dwc_ddrphy_apb_wr(0xd0000,0x1);
// // [phyinit_I_loadPIEImage] End of dwc_ddrphy_phyinit_I_loadPIEImage()
// 
// 
// //##############################################################
// //
// // dwc_ddrphy_phyinit_userCustom_customPostTrain is a user-editable function.
// //
// // See PhyInit App Note for detailed description and function usage
// 
// //##############################################################
// 
dwc_ddrphy_phyinit_userCustom_customPostTrain ();

// // [dwc_ddrphy_phyinit_userCustom_customPostTrain] End of dwc_ddrphy_phyinit_userCustom_customPostTrain()
// // [dwc_ddrphy_phyinit_userCustom_J_enterMissionMode] Start of dwc_ddrphy_phyinit_userCustom_J_enterMissionMode()
// 
// 
// //##############################################################
// //
// // (J) Initialize the PHY to Mission Mode through DFI Initialization 
// //
// // Initialize the PHY to mission mode as follows: 
// //
// // 1. Set the PHY input clocks to the desired frequency. 
// // 2. Initialize the PHY to mission mode by performing DFI Initialization. 
// //    Please see the DFI specification for more information. See the DFI frequency bus encoding in section <XXX>.
// // Note: The PHY training firmware initializes the DRAM state. if skip 
// // training is used, the DRAM state is not initialized. 
// //
// //##############################################################
// 
dwc_ddrphy_phyinit_userCustom_J_enterMissionMode ();

// 
// // [dwc_ddrphy_phyinit_userCustom_J_enterMissionMode] End of dwc_ddrphy_phyinit_userCustom_J_enterMissionMode()
// [dwc_ddrphy_phyinit_sequence] End of dwc_ddrphy_phyinit_sequence()
// [dwc_ddrphy_phyinit_main] End of dwc_ddrphy_phyinit_main()
  return;
}