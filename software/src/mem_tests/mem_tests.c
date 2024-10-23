#include "uart_stdout.h"
#include <stdio.h>
#include "system.h"

int main(void) {
    uint32_t errors = 0;
    UartStdOutInit();

    printf("Mem Tests - SoCLabs MegaSoC\n");

    errors += access_addr_wdata(0x00806000,16,0xCAFECAFE);

    if(errors!=0){
        printf("Test Failed\n");
    } else {
        printf("Test Passed\n");
    }
    UartEndSimulation();
}