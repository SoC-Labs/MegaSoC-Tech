#include "uart_stdout.h"
#include "system.h"
#include <stdio.h>
#include "snps_dram_ctrl.h"
#include "snps_dram_phy.h"
#include "snps_dram_phy_init.h"
#define HW16_REG(ADDRESS)  (*((volatile uint16_t *)(ADDRESS)))
#define HW32_REG(ADDRESS)  (*((volatile uint32_t  *)(ADDRESS)))
#define HW64_REG(ADDRESS)  (*((volatile uint64_t  *)(ADDRESS)))

void dwc_ddr_ctrl_init(void);

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
  dwc_ddr_phy_init_training();
  printf("** Finish DDR PHY Init **\n");
  
  // // 9 Poll the PUB register
  // //  APBONLY.UctShadowRegs[0]=1’b0 
  // while((HW16_REG(DRAM_PHY_CFG_BASE+0x340010)&0x1)!=0){__nop();}
  // // 10 Read the PUB Register
  // //    APBONLY.UctWriteOnlyShadow for training status
  // printf("APBONLY.UctWriteOnlyShadow: 0x%08x\n",HW16_REG(DRAM_PHY_CFG_BASE+0x3400C8));

  // // Write the PUB Register
  // //  APBONLY.DctWriteProt = 0 phy_init See PUB databook for details
  // HW16_REG(DRAM_PHY_CFG_BASE+0x3400C4)=0;
  // printf("Wrote 0 to ABPONLY.DctWriteProt\n");
  // // Poll the PUB register
  // // APBONLY.UctShadowRegs[0]=1’b1 phy_init See PUB databook for details
  // while((HW16_REG(DRAM_PHY_CFG_BASE+0x340010)&0x1)==0){printf("APBONLY.UctWriteOnlyShadow: 0x%08x\n",HW16_REG(DRAM_PHY_CFG_BASE+0x3400C8));}


  // // 13 Write the PUB Register
  // //  APBONLY.DctWriteProt= 1 phy_init See PUB databook for details
  // HW16_REG(DRAM_PHY_CFG_BASE+0x3400C4)=1;
  // printf("Wrote 1 to ABPONLY.DctWriteProt\n");

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

  HW64_REG(DRAM_BASE)=0xA5A5A5A5A5A5A5A5;

  printf("Wrote over DDR\n");

  if(HW64_REG(DRAM_BASE)!=0xA5A5A5A5A5A5A5A5){
    TEST_FAIL();
  } else {
    TEST_PASS();
  }

  TEST_PASS();
}
