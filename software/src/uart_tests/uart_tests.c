#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "system_level_functions.h"
#include <stdio.h>
#include "gic400.h"
#include "CMSDK.h"


char* string_in;
int idx=0;

static void UART1_RX_IRQ();
static void UART0_RX_IRQ();

int main(){
    int errors=0;
    UartStdOutInit();

    printf("MegaSoC UART loopback test\n");

    gic_initialise_intr(UART1_RX_INTR, 0, 1, 0);
    gic_install_handler(UART1_RX_INTR, &UART1_RX_IRQ);
    gic_enable_interrupt(UART1_RX_INTR);

    char* string_out = "Hello UART\n";

    CMSDK_UART0->CTRL=0;
    CMSDK_UART1->CTRL=0;
    CMSDK_UART0->BAUDDIV = 10;
    CMSDK_UART1->BAUDDIV = 10;
    CMSDK_UART0->CTRL = 1; // Tx en
    CMSDK_UART1->CTRL = 10; //RX + RXIRQ

    enable_irq();

    for(int i=0; i<12;i++){
        while(CMSDK_UART0->STATE&1){;}
        CMSDK_UART0->DATA=string_out[i];
        call_wfi();
    }

    printf("From UART0 to UART1: %s",string_in);
    for(int i=0; i<12;i++){
        if(string_in[i]!=string_out[i]){
            errors++;
        }
    }

    string_out = "Goodbye UART\n";
    idx=0;
    gic_disable_interrupt(UART1_RX_INTR);

    gic_initialise_intr(UART0_RX_INTR, 0, 1, 0);
    gic_install_handler(UART0_RX_INTR, &UART0_RX_IRQ);
    gic_enable_interrupt(UART0_RX_INTR);

    CMSDK_UART1->CTRL = 1; // Tx en
    CMSDK_UART0->CTRL = 10; //RX + RXIRQ

    for(int i=0; i<14;i++){
        while(CMSDK_UART1->STATE&1){;}
        CMSDK_UART1->DATA=string_out[i];
        call_wfi();
    }

    printf("From UART1 to UART0: %s",string_in);
    for(int i=0; i<14;i++){
        if(string_in[i]!=string_out[i]){
            errors++;
        }
    }

    if(errors!=0){TEST_FAIL();}
    else{TEST_PASS();}
}

static void UART1_RX_IRQ(){
    string_in[idx]=CMSDK_UART1->DATA;
    idx++;
    CMSDK_UART1->INTCLEAR=0xFF;
    return;
}

static void UART0_RX_IRQ(){
    string_in[idx]=CMSDK_UART0->DATA;
    idx++;
    CMSDK_UART0->INTCLEAR=0xFF;
    return;
}