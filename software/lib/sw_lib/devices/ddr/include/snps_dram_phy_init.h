#include "system.h"


#define HW16_REG(ADDRESS)  (*((volatile uint16_t *)(ADDRESS)))
#define HW32_REG(ADDRESS)  (*((volatile uint32_t  *)(ADDRESS)))


void dwc_ddrphy_phyinit_userCustom_overrideUserInput ();

void dwc_ddrphy_apb_wr(uint32_t offset, uint16_t data);
void dwc_ddrphy_phyinit_userCustom_A_bringupPower ();
void dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy ();

void dwc_ddr_phy_init_training();