#include "uart_stdout.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "system.h"
#include <stdio.h>
#include "cpu_asm_codes.h"
#include "ppu.h"
#include "gic400.h"
#include "CMSDK.h"
#include "irq.h"

// Low power Tests
int cpu_core_low_power_test(void);
int rom_low_power_test(void);

// Auxillary Functions
static void timer_interrupt(int num, int src);


int main(void) {
    uint32_t errors = 0;
    UartStdOutInit();

    printf("Low Power Tests - SoCLabs MegaSoC\n");

    printf(" - CPU Core LP test \n");
    errors += cpu_core_low_power_test();


    printf(" - ROM LP test \n");
    errors += rom_low_power_test();


    if(errors!=0){
        TEST_FAIL();
    } else {
        TEST_PASS();
    }
    UartEndSimulation();
}

int cpu_core_low_power_test(void){
    // Set PPU to dynamic retention mode
    ppu_set_power_policy(A53_CORE_PPU, LOGIC_RET_F1);
    printf("Set LOGIC_RET_F1 \n");


    // Enable CPU Dynamic retention
    cpu_ret_control(1);

    // Initialise Timer 0 interrupts
    gic_initialise_intr(TIMER0_INTR,0,1,0);
    gic_install_handler(TIMER0_INTR, &timer_interrupt);
    gic_enable_interrupt(TIMER0_INTR);

    enable_irq();

    CMSDK_TIMER0->RELOAD = 0xFFFF;
    CMSDK_TIMER0->VALUE  = 0xFFFF;
    CMSDK_TIMER0->CTRL   = 0x0009;  /* Timer enabled */

    call_wfi();

    return 0;
}

int rom_low_power_test(void){
    uint64_t ROM_DATA;
    ROM_PPU->PPU_IESR.W=0;
    ROM_PPU->PPU_IMR.W=0x00;
    ROM_PPU->PPU_ISR.W=0xFF;
    ROM_PPU->PPU_PWRP.B.PWR_DYN_EN=1;
    // ROM_PPU->PPU_PWCR.W=0x12001;
    printf("PWCR: 0x%x\n", ROM_PPU->PPU_PWCR.W);
    printf("ISR: 0x%x\n", ROM_PPU->PPU_ISR.W);


    ppu_set_power_policy(ROM_PPU,0);
    printf("Set OFF \n");
    while(ROM_PPU->PPU_PWSR.B.PWR_STATUS != 0){
        printf("Status: 0x%x\n", ROM_PPU->PPU_PWSR.W);
        ppu_set_power_policy(ROM_PPU,0);
    }
    printf("Current Power Policy 0x%x\n", ROM_PPU->PPU_PWRP.B.PWR_POLICY);

    printf("Read from ROM\n"); // Should work as PPU in Dynamic mode
    ROM_DATA = HW_REG(0,0);
    printf("ROM Data: 0x%x \n",ROM_DATA);

    ppu_set_power_policy(ROM_PPU,8);
    printf("Set On \n");
    while(ROM_PPU->PPU_PWSR.B.PWR_STATUS != 8){
        printf("Status: 0x%x\n", ROM_PPU->PPU_PWSR.W);
        printf("ISR: 0x%x\n", ROM_PPU->PPU_ISR.W);
        ppu_set_power_policy(ROM_PPU,8);
    }
    printf("Current Power Policy 0x%x\n", ROM_PPU->PPU_PWRP.B.PWR_POLICY);
    
    return 0;
}


void timer_interrupt(int num, int src){
  CMSDK_TIMER0->INTCLEAR=1;
  return;
}