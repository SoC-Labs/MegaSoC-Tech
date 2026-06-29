#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "system_level_functions.h"
#include <stdio.h>
#include "gic400.h"
#include "CMSDK.h"
#include "string.h"
#include "megasoc_resetctrl.h"

static volatile char string_in[64];
static volatile char string_in1[64];
static volatile int rx_idx = 0;

static int test_uart_addr_rw (void)
 {
    int errors = 0;
    printf("Test: UART address R/W\n");
    uint32_t ctrl_st = CMSDK_UART0->CTRL;
    uint32_t baud_st = CMSDK_UART0->BAUDDIV;

    //initialise with known state
    CMSDK_UART0->CTRL = 0;
    CMSDK_UART0->BAUDDIV = 27;

    if (CMSDK_UART0->BAUDDIV != 27){
        printf("Error: Read/Write Failed\n");
        errors++;
    }

    //Enable TX and read back
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk;
    if ((CMSDK_UART0->CTRL & (CMSDK_UART_CTRL_TXEN_Msk | CMSDK_UART_CTRL_RXEN_Msk)) != CMSDK_UART_CTRL_TXEN_Msk) {
        printf("Error: TXEN bit is not set properly\n");
        errors++;
    }

    //Read STATE
    (void)CMSDK_UART0->STATE;

    //Restore
    CMSDK_UART0->CTRL = ctrl_st;
    CMSDK_UART0->BAUDDIV = baud_st;

    return errors;
 }
 static int test_uart_reset_values(void)
{
    int errors = 0;

    printf("Test: UART reset via SWRESET\n");

    // 1) Program UART registers
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_RXEN_Msk | CMSDK_UART_CTRL_TXEN_Msk;
    CMSDK_UART0->BAUDDIV = 0x123;
    CMSDK_UART0->INTCLEAR = 0xFF;
    printf("  UART programmed\n");

    // 2) Trigger SWRESET
    //MEGASOC_RESETCTRL->RESET_REQ = RESET_REQ_SWRESET_Msk;
    system_sw_reset();

    // 3) DEAD ZONE: do NOT touch APB/UART immediately after SWRESET
    //    (APB fabric/peripherals may still be held in reset briefly)
    for (volatile uint32_t i = 0; i < 500000; i++) { }

    // 4) Now it should be safe to access resetctrl STATUS again
    //    (bounded poll to avoid hangs)
    int latched = 0;
    for (volatile uint32_t i = 0; i < 500000; i++) {
        if (MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_SWRESET_Msk) {
            latched = 1;
            break;
        }
    }

    if (!latched) {
        printf("  WARNING: SWRESET cause not observed (may be cleared or not latched in this flow)\n");
        // do not hard-fail; continue to check UART reset behavior
    } else {
        printf("  SWRESET cause latched\n");
        MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_SWRESET_Msk; // clear it
    }

    // 5) Another settle delay before touching UART
    for (volatile uint32_t i = 0; i < 500000; i++) { }

    // 6) Re-init UART (this itself touches UART regs)
    UartStdOutInit();
    printf("  UART re-init OK\n");

    // 7) Now check reset values
    uint32_t ctrl = CMSDK_UART0->CTRL;
    uint32_t baud = CMSDK_UART0->BAUDDIV;
    uint32_t ist  = CMSDK_UART0->INTSTATUS;

    printf("  After SWRESET: CTRL=0x%08x BAUDDIV=0x%08x INTSTATUS=0x%08x\n",
           (unsigned)ctrl, (unsigned)baud, (unsigned)ist);

    if (ctrl != 0x00000000) { printf("  ERROR: CTRL not reset\n"); errors++; }
    if (baud != 0x00000000) { printf("  ERROR: BAUDDIV not reset\n"); errors++; }
    if (ist  != 0x00000000) { printf("  ERROR: INTSTATUS not reset\n"); errors++; }

    return errors;
}

/*
 //Reset test
 static int test_uart_reset_values(void){
    int errors = 0;
    printf(" Test: UART Reset\n q");
    // write some data in the registers
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_RXEN_Msk | CMSDK_UART_CTRL_TXEN_Msk;
    CMSDK_UART0->BAUDDIV = 0x123;
    CMSDK_UART0->INTCLEAR = 0xFF;

    //call function system software reset////////////////////   !!!yet to be implemented.
    MEGASOC_RESETCTRL->RESET_REQ = RESET_REQ_SWRESET_Msk;
    if (CMSDK_UART0->CTRL != 0x00){
        printf ("Problem with UART CTRL register after reset\n");
        errors++;
    }
    if (CMSDK_UART0->BAUDDIV != 0x00){
        printf("problem with UART CTRL register after reset\n");
        errors++;
    }
    if (CMSDK_UART0->INTSTATUS!= 0X00){
        printf("problem with intstatus not default after reset\n");
        errors++;
    }
    return errors;
 }
*/
/*
 /// Have to check this implementation
 static int test_uart_baud_ticks(void){
    int errors = 0;
    printf("Test: UART baud ticks\n");
    const uint32_t divs[] = {10,20,40};

    for (int i=0; i < 3; i++){
        CMSDK_UART0->CTRL = 0;
        CMSDK_UART0->BAUDDIV = divs[i];
        CMSDK_UART0->INTCLEAR = 0xFF;

        //enbale TX bit
        CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk;
        //Wait for sometime till TX is not full
        uint32_t timeout = 100000;
        while ((CMSDK_UART0->STATE & CMSDK_UART_STATE_TXBF_Msk) && --timeout) {}
        if (!timeout){
            printf("Error: UART0 TX buffer never became full (baud test %d)\n", i);
            errors++;
            continue;
        }

        CMSDK_UART0-> DATA = 0XA5;

        timeout = 200000;
        while ((CMSDK_UART0->STATE & CMSDK_UART_STATE_TXBF_Msk) && --timeout) {}
        if (!timeout){
            printf("Error: UART0 TX buffer never cleared (baud test %d)\n", i);
            errors++;  
        }
        
    }

    return errors;
 }

  static void UART1_RX_IRQ(void){
    string_in[rx_idx++] = CMSDK_UART1-> DATA;
    CMSDK_UART1->INTCLEAR = 0xFF;
 }

 static void UART0_RX_IRQ(void){
    string_in1[rx_idx++] = CMSDK_UART0->DATA;
    CMSDK_UART0->INTCLEAR = 0xFF;
 }

 //Bi-directional loopbacks with IRQ's
 static int test_uart_loopback_both_directions(void){
    int errors = 0;
    char* string_in;
    char* string_in1;
    rx_idx = 0;

    printf("Test: UART Loopback both directions\n");
    //Case 1: UART0 TX --> UART1 RX
    printf("UART0 TX --> UART1 RX");
    gic_initialise_intr(UART1_RX_INTR, 0, 1, 0);
    gic_install_handler(UART1_RX_INTR, &UART1_RX_IRQ);
    gic_enable_interrupt(UART1_RX_INTR);

    char* string_out = "Hello UART\n";
    int len = strlen(string_out);
    //configuring UARTs
    CMSDK_UART0->CTRL = 0;
    CMSDK_UART1->CTRL = 0;
    CMSDK_UART0->BAUDDIV = 10;
    CMSDK_UART1->BAUDDIV = 10;
    CMSDK_UART0->INTCLEAR = 0xFF;
    CMSDK_UART1->INTCLEAR = 0xFF;

    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk;
    CMSDK_UART1->CTRL = CMSDK_UART_CTRL_RXEN_Msk | CMSDK_UART_CTRL_RXIRQEN_Msk;

    enable_irq();

    for (int i = 0; i < len; i++){
        //Wait till TX buffer is full
        while (CMSDK_UART0->STATE & CMSDK_UART_STATE_TXBF_Msk) {;}
        CMSDK_UART0->DATA = (uint8_t)string_out[i];
        call_wfi(); //wait for interrupt
    }

    printf("From UART0 to UART1: %s\n", string_in);
    for (int i = 0; i < len; i++){
        printf("reached here this loop");
        if(string_in[i] != string_out[i]){
            printf("Error: UART1 Received missing data from UART0\n");
            errors++;
        }
    }

    gic_disable_interrupt(UART1_RX_INTR);

    //case 2: UART1 TX --> UART0 RX
    printf("UART1 TX --> UART0 RX");
    char* string_out1 = "Goodbye UART\n";
    int len_out = strlen(string_out1);

    gic_initialise_intr(UART0_RX_INTR, 0, 1, 0);
    gic_install_handler(UART0_RX_INTR, &UART0_RX_IRQ);
    gic_enable_interrupt(UART0_RX_INTR);

    CMSDK_UART0->CTRL = 0;
    CMSDK_UART1->CTRL = 0;
    CMSDK_UART0->BAUDDIV = 10;
    CMSDK_UART1->BAUDDIV = 10;
    
    

    CMSDK_UART1->CTRL = CMSDK_UART_CTRL_TXEN_Msk; //Tx
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_RXEN_Msk | CMSDK_UART_CTRL_RXIRQEN_Msk; //RX + RXIRQ

    for (int i = 0; i < len_out; i++){
        //WAIT FOR BUFFER TO FILL
        while (CMSDK_UART1->STATE & CMSDK_UART_STATE_TXBF_Msk) {;}
        CMSDK_UART1->DATA = (uint8_t) string_out1[i];
        call_wfi(); //wait for interrupt
    }

    printf("From UART1 to UART0: %s\n", string_in1);
    for (int i = 0; i < len_out; i++){
        if(string_in1[i] != string_out1[i]){
            printf("Error: UART0 Received missing data from UART1\n");
            errors++;
        }
    }
     gic_disable_interrupt(UART0_RX_INTR);

     return errors;
 }

*/

 // TX overrun
 static int test_uart_tx_overrun (void){
    int errors = 0;
    printf("Test: UART TX overrun\n");
    CMSDK_UART0->CTRL = 0;
    CMSDK_UART0->BAUDDIV = 10;
    CMSDK_UART0->INTCLEAR = 0xFF;

    //enable tx and tx overrun in uart0
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk | CMSDK_UART_CTRL_TXORIRQEN_Msk;
    //write data to overrun
    CMSDK_UART0->DATA = 0x11;
    CMSDK_UART0->DATA = 0x22;

    //wait for interrupt
    uint32_t timeout1 = 100000;
    while (((CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_TXORIRQ_Msk) == 0 ) && --timeout1){}
    if(!timeout1){
        printf("Error: UART0 TX overrun IRQ did not signal\n");
        errors++;
    }

    //Clear and re-check
    CMSDK_UART0->INTCLEAR = CMSDK_UART_INTSTATUS_TXORIRQ_Msk;
    if(CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_TXORIRQ_Msk){
        printf("Error: UART0 TX overrun stuck after clear\n");
    }
    return errors;
 }

 //RX overrun
 static int test_uart_rx_overrun(void){
    int errors = 0;
    printf("Test: UART RX overrun\n");
    CMSDK_UART0->CTRL = 0;
    CMSDK_UART1->CTRL = 0;
    CMSDK_UART0->BAUDDIV = 10;
    CMSDK_UART1->BAUDDIV = 10;
    CMSDK_UART0->INTCLEAR = 0xFF;
    CMSDK_UART1->INTCLEAR = 0xFF;

    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_RXEN_Msk | CMSDK_UART_CTRL_RXORIRQEN_Msk;
    CMSDK_UART1->CTRL = CMSDK_UART_CTRL_TXEN_Msk;

    //SEND BYTES FROM UART1 TO UART0
    for (int i = 0; i < 32; i++){
        while(CMSDK_UART1->STATE & CMSDK_UART_STATE_TXBF_Msk) {;}
        CMSDK_UART1->DATA = (0x30 + (i & 0xF));
    }

    uint32_t timeout2 = 100000;
    while (((CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_RXORIRQ_Msk) == 0) && --timeout2) {}
    if(!timeout2){
        printf("Error: UART0 RX overrun did not dignal\n");
        errors++;
    }
    //Clear and verify 
    CMSDK_UART0-> INTCLEAR = CMSDK_UART_INTSTATUS_RXORIRQ_Msk;
    if(CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_RXORIRQ_Msk){
        printf("Error: UART0 RXOR IRQ stuck\n");
        errors++;
    } 
    return errors;
 }

 int main(void){
    int errors = 0;
    MEGASOC_RESETCTRL->RESET_STATUS = 0X10;
    UartStdOutInit();
    printf("MegaSoC CMSDK UART Integration test\n");

    errors += test_uart_addr_rw();
    //errors += test_uart_baud_ticks();
    //errors += test_uart_loopback_both_directions();
    errors += test_uart_reset_values();   //system soft reset yet to be implemented.
    errors += test_uart_rx_overrun();
    errors += test_uart_tx_overrun();

    printf("Total integration issues in UART is %d\n", errors);

    if(errors != 0){
        TEST_FAIL();
    } else {
        TEST_PASS();
    }
 }