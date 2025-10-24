#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "system_level_functions.h"
#include <stdio.h>
#include "gic400.h"
#include "CMSDK.h"
#include "pl011_uart.h"

pl011_uart_t * uart = (pl011_uart_t *) SYS_PL011_UART;
char string_in[128];
int idx=0;

static void PL011_RX_IRQ();

int main(){
    int errors=0;
    UartStdOutInit();

    printf("MegaSoC PL011 UART loopback test\n");

    // Initialise Interrupts
    gic_initialise_intr(PL011_UARTRXINTR, 0, 1, 0);
    gic_install_handler(PL011_UARTRXINTR, &PL011_RX_IRQ);
    gic_enable_interrupt(PL011_UARTRXINTR);

    char* string_out = "Hello MegaSoC from PL011 UART. This is a long message\n";

    pl011_config_baud((void *)SYS_PL011_UART);
    uart->LCR_H = 0x00000070;
    uart->IFLS = 0x0C;
    uart->MSC = 0x010;
    uart->Ctrl = 0x301;

    enable_irq();

    printf("Send string over PL011\n");
    pl011_uart_print_string((void *) SYS_PL011_UART, string_out);

    while(!(uart->Flags & UARTFR_TXFIFOEMPTY)){;}
    // If idx=0 then no data recieved over uart
    if(idx==0){
        TEST_FAIL();
    }
    // Catch any other characters in RX fifo
    while(!(uart->Flags & (1<<4))){
        string_in[idx]=uart->Data;
        idx++;
    }
 
    printf("From UART: %s \n", string_in);

    printf("Uart Flags: 0x%x\n", uart->Flags);
    if(errors!=0){TEST_FAIL();}
    else{TEST_PASS();}
}


static void PL011_RX_IRQ(){
    disable_irq();
    uart->IRQClear=0x010;
    while(!(uart->Flags & (1<<4))){
        string_in[idx]=uart->Data;
        idx++;
    }
    enable_irq();
    return;
}