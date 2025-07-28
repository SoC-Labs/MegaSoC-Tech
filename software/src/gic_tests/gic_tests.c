#include "uart_stdout.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "system.h"
#include <stdio.h>
#include "gic400.h"
#include "CMSDK.h"
#include "irq.h"


int timer0_id_check(void);
int timer_interrupt_test_1(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
static void timer_interrupt(int num, int src);

/* peripheral and component ID values */
#define APB_TIMER_PID4  0x04
#define APB_TIMER_PID5  0x00
#define APB_TIMER_PID6  0x00
#define APB_TIMER_PID7  0x00
#define APB_TIMER_PID0  0x22
#define APB_TIMER_PID1  0xB8
#define APB_TIMER_PID2  0x1B
#define APB_TIMER_PID3  0x00
#define APB_TIMER_CID0  0x0D
#define APB_TIMER_CID1  0xF0
#define APB_TIMER_CID2  0x05
#define APB_TIMER_CID3  0xB1
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))

/* Global variables */
volatile int timer0_irq_occurred;
volatile int timer1_irq_occurred;
volatile int timer0_irq_expected;
volatile int timer1_irq_expected;
volatile int counter;


int main(void) {
  uint32_t errors = 0;
  UartStdOutInit();

  printf("GIC tests - SoCLabs MegaSoC\n");

  if(timer0_id_check()!=0){
    printf("Timer 0 not present skipping test\n");
    printf ("\n** TEST SKIPPED **\n");
    UartEndSimulation();
  }
  // Timer present - continue 
  errors += timer_interrupt_test_1(CMSDK_TIMER0);

  if(errors==0){
    TEST_PASS();
  } else {
    TEST_FAIL();
  }
  UartEndSimulation();
}


/* --------------------------------------------------------------- */
/* Peripheral detection                                            */
/* --------------------------------------------------------------- */
/* Detect the part number to see if device is present              */
int timer0_id_check(void)
{
  uint32_t timer_id;
  uint32_t ID0, ID1;
  uint32_t timer_ctrl;
  timer_ctrl = CMSDK_TIMER0->CTRL;
  ID0=CMSDK_TIMER0->PID0 & 0xFF;
  ID1=CMSDK_TIMER0->PID1 & 0xFF;
  timer_id = CMSDK_TIMER0->PID2 & 0x07;
  if ((ID0 != 0x22) ||
      (ID1 != 0xB8) ||
      (timer_id != 0x03))
    return 1; /* part ID & ARM ID does not match */
  else
    return 0;
}

/* --------------------------------------------------------------- */
/*  Timer interrupt test 1                                         */
/* --------------------------------------------------------------- */
/*
  Interrupt enable:
   Timer is enabled, with reload value set to 0x7F (128 cycles),
   and timer interrupt is enabled.
   check that timer interrupt has take place as least twice
   when counter (software variable) is increased from 0 to 0x300.
   If counter is > 0x300 but less than two timer interrupt is received
   (timerx_irq_occurred < 2), then flag it as time out error.

  Interrupt disable:
   Timer is enabled, with reload value set to 0x1F (32 cycles),
   and timer interrupt is disabled.
   The counter (software variable) is increased from 0 to 0x100.
   Check that timer interrupt did not take place.
   (timer0_irq_occurred and timer1_irq_occurred are 0).

*/
int timer_interrupt_test_1(CMSDK_TIMER_TypeDef *CMSDK_TIMER){
  int return_val=0;
  int err_code=0;

  puts ("Timer interrupt test");
  CMSDK_TIMER->VALUE = 0; /* Disable timer */
  cpu_ret_control(1);

  gic_initialise_intr(TIMER0_INTR,0,1,0);
  gic_install_handler(TIMER0_INTR, &timer_interrupt);
  gic_enable_interrupt(TIMER0_INTR);
  puts ("- Test interrupt generation enabled.");
  timer0_irq_expected = 1;
  timer1_irq_expected = 0;
  timer0_irq_occurred = 0;
  timer1_irq_occurred = 0;


  enable_irq();
  
  CMSDK_TIMER->RELOAD = 0xFFFF;
  CMSDK_TIMER->VALUE  = 0xFFFF;
  CMSDK_TIMER->CTRL   = 0x0009;  /* Timer enabled */
  counter = 0;
  while (( timer0_irq_occurred < 2) && (counter < 0x300)){
    call_wfi();
  };
  CMSDK_TIMER->CTRL   = 0x0000;  /* Stop Timer */
  /* Check timeout has not occurred */
  if (counter >= 0x300) {
     printf("ERROR : Timer interrupt enable fail.\n");
     err_code += (1<<0);
    }
  counter = 0;

  disable_irq();
  gic_disable_interrupt(TIMER0_INTR);
  if (err_code != 0) {
    printf ("ERROR : Interrupt test failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  return(return_val);

}

void timer_interrupt(int num, int src){
  timer0_irq_occurred++;
  CMSDK_TIMER0->INTCLEAR=1;
  return;
}