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
*/

  static void UART1_RX_IRQ(int id, int src){ (void)id; (void)src;
    char c = CMSDK_UART1->DATA;
    if (rx_idx < 63) string_in[rx_idx++] = c;      // bounded: a flood must not overrun the buffer
    CMSDK_UART1->INTCLEAR = 0xFF;
 }

 static void UART0_RX_IRQ(int id, int src){ (void)id; (void)src;
    char c = CMSDK_UART0->DATA;
    if (rx_idx < 63) string_in1[rx_idx++] = c;
    CMSDK_UART0->INTCLEAR = 0xFF;
 }


 /* UART0 is the console. TXBF clear only says the holding register is free: the character in the shift
    register still needs one frame (87 us at 115200). Wait for it before UART0 is reconfigured, or the last
    character of the previous line goes out at the new rate and is lost. ~2 ms at 20 MHz. */
 static void console_drain(void)
 {
    while (CMSDK_UART0->STATE & CMSDK_UART_STATE_TXBF_Msk) {;}
    for (volatile int t = 0; t < 4000; t++) {;}
 }

 //Bi-directional loopbacks with IRQ's
 static int test_uart_loopback_both_directions(void){
    int errors = 0;


    rx_idx = 0;

    printf("Test: UART Loopback both directions\n");
    //Case 1: UART0 TX --> UART1 RX
    printf("UART0 TX --> UART1 RX");
    gic_initialise_intr(UART1_RX_INTR, 0, 1, 0);
    gic_install_handler(UART1_RX_INTR, &UART1_RX_IRQ);
    gic_enable_interrupt(UART1_RX_INTR);

    char* string_out = "Hello UART\n";
    int len = strlen(string_out);
    //configuring UARTs. UART1 RX is wired to UART0 TX (loopback): let the console text above leave UART0
    //before the receiver is enabled, or UART1 locks on to the middle of a character and loses framing.
    uint32_t baud_console = CMSDK_UART0->BAUDDIV;
    console_drain();
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
        // Wait for the RX interrupt to have stored this character. Not WFI: with code fetched from
        // flash the interrupt can arrive before the WFI is reached (first iteration), and a WFI
        // entered after its interrupt has already been taken sleeps for ever (seen on the board).
        uint32_t t = 2000000;
        while ((rx_idx <= i) && --t) {;}
        if (!t) { printf("Error: UART1 RX interrupt timeout at char %d\n", i); errors++; break; }
    }

    // UART0 TX is looped into UART1 RX: stop the receiver and its interrupt BEFORE printing, or every
    // console byte (sent at the console rate, received at divider 10) raises an RX interrupt and the
    // handler floods string_in (observed: >600 interrupts, memory corruption, hang).
    gic_disable_interrupt(UART1_RX_INTR);
    CMSDK_UART1->CTRL = 0;
    CMSDK_UART1->INTCLEAR = 0xFF;
    string_in[len] = 0;
    CMSDK_UART0->BAUDDIV = baud_console;              // results at the console rate
    printf("From UART0 to UART1: %s\n", (const char *)string_in);
    for (int i = 0; i < len; i++){
        if(string_in[i] != string_out[i]){
            printf("Error: UART1 Received missing data from UART0\n");
            errors++;
        }
    }


    //case 2: UART1 TX --> UART0 RX
    printf("UART1 TX --> UART0 RX");
    char* string_out1 = "Goodbye UART\n";
    int len_out = strlen(string_out1);

    gic_initialise_intr(UART0_RX_INTR, 0, 1, 0);
    gic_install_handler(UART0_RX_INTR, &UART0_RX_IRQ);
    gic_enable_interrupt(UART0_RX_INTR);

    console_drain();                               // let "UART1 TX --> UART0 RX" leave the console first
    CMSDK_UART0->CTRL = 0;
    CMSDK_UART1->CTRL = 0;
    CMSDK_UART0->BAUDDIV = 10;
    CMSDK_UART1->BAUDDIV = 10;
    
    

    CMSDK_UART1->CTRL = CMSDK_UART_CTRL_TXEN_Msk; //Tx
    rx_idx = 0;
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk | CMSDK_UART_CTRL_RXEN_Msk | CMSDK_UART_CTRL_RXIRQEN_Msk; //RX + RXIRQ; UART0 is also the console: keep TX on

    for (int i = 0; i < len_out; i++){
        //WAIT FOR BUFFER TO FILL
        while (CMSDK_UART1->STATE & CMSDK_UART_STATE_TXBF_Msk) {;}
        CMSDK_UART1->DATA = (uint8_t) string_out1[i];
        uint32_t t = 2000000;                           // see case 1: poll, no WFI
        while ((rx_idx <= i) && --t) {;}
        if (!t) { printf("Error: UART0 RX interrupt timeout at char %d\n", i); errors++; break; }
    }

    gic_disable_interrupt(UART0_RX_INTR);
    CMSDK_UART1->CTRL = 0;
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk;      // console only; RX + RX IRQ off before printing
    CMSDK_UART0->INTCLEAR = 0xFF;
    string_in1[len_out] = 0;
    CMSDK_UART0->BAUDDIV = baud_console;
    printf("From UART1 to UART0: %s\n", (const char *)string_in1);
    for (int i = 0; i < len_out; i++){
        if(string_in1[i] != string_out1[i]){
            printf("Error: UART0 Received missing data from UART1\n");
            errors++;
        }
    }
     CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk | CMSDK_UART_CTRL_RXEN_Msk;   // console back to TX+RX

     return errors;
 }



 // TX overrun: UART0 (the console) at divider 10. The bytes written here reach the console as two
 // glitch frames at the fast rate; nothing is printed until the console rate is restored.
 static int test_uart_tx_overrun (void){
    int errors = 0;
    int not_set = 0, stuck = 0;
    printf("Test: UART TX overrun\n");
    uint32_t baud_console = CMSDK_UART0->BAUDDIV;
    console_drain();
    CMSDK_UART0->CTRL = 0;
    CMSDK_UART0->BAUDDIV = 10;
    CMSDK_UART0->INTCLEAR = 0xFF;

    //enable tx and tx overrun in uart0
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk | CMSDK_UART_CTRL_TXORIRQEN_Msk;
    // Three writes, not two: with the transmitter idle the UART moves the first byte from the holding
    // register into the shift register on the next PCLK (tx_buf_clear), so a second write finds the
    // register free and nothing overruns. The third write lands while the second byte waits for the
    // first to be shifted out (>= 160 PCLK) - that is the overrun.
    CMSDK_UART0->DATA = 0x11;
    CMSDK_UART0->DATA = 0x22;
    CMSDK_UART0->DATA = 0x33;

    //wait for interrupt
    uint32_t timeout1 = 100000;
    while (((CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_TXORIRQ_Msk) == 0 ) && --timeout1){}
    if(!timeout1) not_set = 1;

    //Clear and re-check
    CMSDK_UART0->INTCLEAR = CMSDK_UART_INTSTATUS_TXORIRQ_Msk;
    if(CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_TXORIRQ_Msk) stuck = 1;

    // console back before anything is printed
    console_drain();
    CMSDK_UART0->BAUDDIV = baud_console;
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk;
    if (not_set) { printf("Error: UART0 TX overrun IRQ did not signal\n"); errors++; }
    if (stuck)   { printf("Error: UART0 TX overrun stuck after clear\n"); errors++; }
    return errors;
 }

 //RX overrun: UART1 TX -> UART0 RX (loopback) at divider 10, UART0 never reads DATA
 static int test_uart_rx_overrun(void){
    int errors = 0;
    int not_set = 0, stuck = 0;
    printf("Test: UART RX overrun\n");
    uint32_t baud_console = CMSDK_UART0->BAUDDIV;
    console_drain();
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
    // TXBF clear only means the last byte has left the holding register: two characters can still be
    // in flight, and each one that lands on the full RX buffer is a new overrun. Let the line go quiet
    // before the flag is checked and cleared, otherwise a late character re-sets it between the clear
    // and the re-check (timing dependent: passed from ROM in simulation, failed from flash on the board).
    while (CMSDK_UART1->STATE & CMSDK_UART_STATE_TXBF_Msk) {;}
    for (volatile int t = 0; t < 4000; t++) {;}

    if ((CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_RXORIRQ_Msk) == 0) not_set = 1;
    //Clear and verify
    CMSDK_UART0->INTCLEAR = CMSDK_UART_INTSTATUS_RXORIRQ_Msk;
    if (CMSDK_UART0->INTSTATUS & CMSDK_UART_INTSTATUS_RXORIRQ_Msk) stuck = 1;

    // Console back BEFORE anything is printed: UART0 TX is off here and UartPutc() spins on TXBF for
    // ever when the transmitter is disabled (the first error message hung the board).
    while (CMSDK_UART0->STATE & CMSDK_UART_STATE_RXBF_Msk) { volatile uint32_t d = CMSDK_UART0->DATA; (void)d; }
    CMSDK_UART0->INTCLEAR = 0xFF;
    CMSDK_UART0->BAUDDIV = baud_console;
    CMSDK_UART0->CTRL = CMSDK_UART_CTRL_TXEN_Msk;
    if (not_set) { printf("Error: UART0 RX overrun did not signal\n"); errors++; }
    if (stuck)   { printf("Error: UART0 RXOR IRQ stuck after clear\n"); errors++; }
    return errors;
 }

 int main(void){
    int errors = 0;
    MEGASOC_RESETCTRL->RESET_STATUS = 0X10;
    UartStdOutInit();
    printf("MegaSoC CMSDK UART Integration test\n");

    errors += test_uart_addr_rw();
    //errors += test_uart_baud_ticks();
    errors += test_uart_loopback_both_directions();
#ifdef SKIP_SWRESET_TEST
    // For builds whose RTL has no megasoc_reset_ctrl (e.g. the HAPS-SX FPGA snapshot, APB slot 11 unconnected):
    // system_sw_reset() would spin for ever waiting for a reset that never comes. APP_TESTDEFINES=-DSKIP_SWRESET_TEST
    printf("Test: UART reset via SWRESET - skipped (no reset controller in FPGA build)\n");
#else
    errors += test_uart_reset_values();   //system soft reset yet to be implemented.
#endif
    errors += test_uart_rx_overrun();
    errors += test_uart_tx_overrun();

    printf("Total integration issues in UART is %d\n", errors);

    if(errors != 0){
        TEST_FAIL();
    } else {
        TEST_PASS();
    }
 }