#include "uart_stdout.h"
#include <stdio.h>
#include "system.h"
#include "ppu.h"

#define ROM_PPU_BASE 0x01200000
#define ROM_PPU ((SCP_PPU_1_1_TypeDef *) ROM_PPU_BASE)

int main(void) {
    uint32_t errors = 0;
    UartStdOutInit();

    printf("Low Power Tests - SoCLabs MegaSoC\n");

    ROM_PPU->PPU_IESR.W=0;
    ROM_PPU->PPU_IMR.W=0x00;
    ROM_PPU->PPU_ISR.W=0xFF;
    // ROM_PPU->PPU_PWCR.W=0x12001;
    printf("PWCR: 0x%x\n", ROM_PPU->PPU_PWCR.W);
    printf("ISR: 0x%x\n", ROM_PPU->PPU_ISR.W);

  
    ppu_set_power_policy(ROM_PPU,0);
    printf("Set off\n");
    while(ROM_PPU->PPU_PWSR.B.PWR_STATUS != 0){
        printf("Status: 0x%x\n", ROM_PPU->PPU_PWSR.W);
        printf("ISR: 0x%x\n", ROM_PPU->PPU_ISR.W);
    }
    printf("Current Power Policy 0x%x\n", ROM_PPU->PPU_PWRP.B.PWR_POLICY);

    //ROM_PPU->PPU_UNLK.W=1;
    ppu_set_power_policy(ROM_PPU,8);
    //ROM_PPU->PPU_UNLK.W=1;
    ppu_check_power_policy(ROM_PPU,8);

    if(errors!=0){
        TEST_FAIL();
    } else {
        TEST_PASS();
    }
    UartEndSimulation();
}