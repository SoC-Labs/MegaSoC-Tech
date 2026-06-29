#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "system_level_functions.h"
#include <stdio.h>
#include "gic400.h"
#include "CMSDK.h"

uint32_t TEST_RELOAD_VALUE = 50u;
uint32_t timeout = 1000000u;

static int test_timer_addr_rw(void){
  int errors = 0;
  printf("Test: Timer0 address R/W test\n");
  //backup values
  uint32_t ctrl_st = CMSDK_TIMER0->CTRL;
  uint32_t value_st = CMSDK_TIMER0->VALUE;
  uint32_t reload_st = CMSDK_TIMER0->RELOAD;

  //initialise with know values
  CMSDK_TIMER0->CTRL = 0;
  CMSDK_TIMER0->VALUE = 0;
  CMSDK_TIMER0->RELOAD = 0;
  CMSDK_TIMER0->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;

  //R/W
  CMSDK_TIMER0->CTRL = CMSDK_TIMER_CTRL_EN_Msk | CMSDK_TIMER_CTRL_IRQEN_Msk;
  if (CMSDK_TIMER0->CTRL != (CMSDK_TIMER_CTRL_EN_Msk | CMSDK_TIMER_CTRL_IRQEN_Msk)){
    printf("Error: CTRL R/W Failed");
    errors++;
  }

  CMSDK_TIMER0->RELOAD = 0x12345678u;
  if(CMSDK_TIMER0->RELOAD != 0x12345678u){
    printf("Error: RELOAD R/W failed\n");
    errors++;
  }
  // as soon as VALUE is update if clock is connected VALUE decreases
  CMSDK_TIMER0->VALUE = 0xA5A5A5u;
  if(CMSDK_TIMER0->VALUE == 0xA5A5A5u){
    printf("Error: VALUE R/W failed\n");
    errors++;
  }

  (void)CMSDK_TIMER0->INTSTATUS;

  //restore stored values
  CMSDK_TIMER0->CTRL = ctrl_st;
  CMSDK_TIMER0->VALUE = value_st;
  CMSDK_TIMER0->RELOAD = reload_st;
  CMSDK_TIMER0->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;

  return errors;
}


static int test_timer_basic_countdown(void){
int errors = 0;
printf("Test: Basic down count on internal clock\n");
uint32_t cuur_value, last_value;

//known state
CMSDK_TIMER0->CTRL = 0;
CMSDK_TIMER0->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;

//load reload value
CMSDK_TIMER0->RELOAD = 0;
CMSDK_TIMER0->VALUE = TEST_RELOAD_VALUE;
CMSDK_TIMER0->CTRL = CMSDK_TIMER_CTRL_EN_Msk;

last_value = CMSDK_TIMER0->VALUE;

//wait untill value reaches 0
while(timeout--){
  cuur_value = CMSDK_TIMER0->VALUE;
  
  if (cuur_value == 0){
    break;
  }
  //counter monotonicity verification
  if (cuur_value > last_value){
    printf("Error: VALUE increased, last=0x%08lx cur=0x%08lx\n",(unsigned long)last_value, (unsigned long) cuur_value);
    errors++;
    break;
  }

  last_value = cuur_value;

}

if(timeout == 0){
  printf("Error: Timeout waiting timer to reach 0\n");
  errors++;
}

CMSDK_TIMER0->CTRL = 0; // to stop timer if it is stuck

return errors;
}

static int test_timer_intstatus(){
  int errors = 0;
  printf("Test: Timer0 INTSTATUS\n");
  
  // set to known values
  CMSDK_TIMER0->CTRL = 0;
  CMSDK_TIMER0->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;
  CMSDK_TIMER0->RELOAD = TEST_RELOAD_VALUE;
  CMSDK_TIMER0->VALUE = TEST_RELOAD_VALUE;

  CMSDK_TIMER0->CTRL = CMSDK_TIMER_CTRL_EN_Msk | CMSDK_TIMER_CTRL_IRQEN_Msk;

  while (((CMSDK_TIMER0->INTSTATUS & CMSDK_TIMER_INTSTATUS_Msk)== 0 ) && timeout--){}

  if(timeout == 0){
    printf("Error: Timeout waiting for INTSTATUS to change\n");
    errors++;
  }

  //clear interrupt
  CMSDK_TIMER0->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;

  if ((CMSDK_TIMER0->INTSTATUS & CMSDK_TIMER_INTSTATUS_Msk) != 0){
    printf("Error: INTSTATUS did not clear\n");
    errors++;
  }
  //back to know state
  CMSDK_TIMER0->CTRL = 0;

  return errors;
}

/*
static void TIMER0_IRQ_HANDLER(void){
  CMSDK_TIMER0->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;
  int irqhits++;
}
static int test_timer_irq_integration(void){
  int errors = 0;

  //known state
  CMSDK_TIMER0->CTRL = 0;
  CMSDK_TIMER0->RELOAD = TEST_RELOAD_VALUE;
  CMSDK_TIMER0->VALUE = TEST_RELOAD_VALUE;
  CMSDK_TIMER0->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;

  gic_initialise_intr(TIMER0_INTR,0, 1, 0);
  gic_install_handler(TIMER0_INTR, &TIMER0_IRQ_HANDLER);
  gic_enable_interrupt(Timer0_INTR);

  enable_irq();

  CMSDK_TIMER0->CTRL = CMSDK_TIMER_CTRL_EN_Msk | CMSDK_TIMER_CTRL_IRQEN_Msk;

  while((irqhits == 0) && timeout--){
    call_wfi();
  }

  if(timeout == 0 || irqhits == 0){
    printf("Time0 IRQ did not reach CPU\n");
    errors++;
  }

  gic_disable_interrupt(TIMER0_INTR);
  CMSDK_TIMER0->CTRL = 0;

  return errors;
}
*/

static int test_timer_id(uint32_t base_addr){

  int error = 0;

  uint8_t PID4 = 0x04;
  uint8_t PID5 = 0x00;
  uint8_t PID6 = 0x00;
  uint8_t PID7 = 0x00;
  uint8_t PID0 = 0x22;  
  uint8_t PID1 = 0xB8;
  uint8_t PID2 = 0x1B;
  uint8_t PID3 = 0x00;

  uint8_t CID0 = 0x0D;
  uint8_t CID1 = 0xF0;
  uint8_t CID2 = 0x05;
  uint8_t CID3 = 0xB1;

  if(HW_REG_BYTE(base_addr,0xFD0) != PID4){error++;}
  if(HW_REG_BYTE(base_addr,0xFD4) != PID5){error++;}
  if(HW_REG_BYTE(base_addr,0xFD8) != PID6){error++;}
  if(HW_REG_BYTE(base_addr,0xFDC) != PID7){error++;}
  if(HW_REG_BYTE(base_addr,0xFE0) != PID0){error++;}
  if(HW_REG_BYTE(base_addr,0xFE4) != PID1){error++;}
  if(HW_REG_BYTE(base_addr,0xFE8) != PID2){error++;}
  if(HW_REG_BYTE(base_addr,0xFEC) != PID3){error++;}
  if(HW_REG_BYTE(base_addr,0xFF0) != CID0){error++;}
  if(HW_REG_BYTE(base_addr,0xFF4) != CID1){error++;}
  if(HW_REG_BYTE(base_addr,0xFF8) != CID2){error++;}
  if(HW_REG_BYTE(base_addr,0xFFC) != CID3){error++;}
  return error;
}
int main(void){
  int errors =0;
  printf("MegaSoC CMSDK TIMER Integration test\n");
  errors += test_timer_addr_rw();
  errors += test_timer_basic_countdown();
  errors += test_timer_intstatus();
  //errors += test_timer_irq_integration();

  printf("Test: Check ID of Timer0\n");
  if(test_timer_id(TIMER0_BASE)!=0){errors++;}
  printf("Test: Check ID of Timer1\n");
  if(test_timer_id(TIMER1_BASE)!=0){errors++;}
  
  printf("Total number of Timer errors are %d\n", errors);

  if (errors == 0){
    TEST_PASS();
  }else{
    TEST_FAIL();
  }
}