#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "system_level_functions.h"
#include <stdio.h>
#include "gic400.h"
#include "CMSDK.h"
#include "sdiodrv.h"


SDIO * SDIO_S = (SDIO *) 0x01010000;
SDIODRV * SDIODRV_S;
int main(){
    int errors=0;
    char dat[] = {0, 1, 2, 3, 4, 5, 6, 7};
    char read[8];

    UartStdOutInit();

    printf("MegaSoC SDIO Quick tests\n");

    SDIODRV_S = sdio_init(SDIO_S);
    printf("SDIO Initialised\n");
    
    sdio_write(SDIODRV_S, 0, 8, &dat[0]);
    printf("SDIO Written to\n");


    sdio_read(SDIODRV_S, 0, 8, &read[0]);
    printf("SDIO Read from\n");

    for(int i =0; i<8; i++){
        printf("%d",read[i]);
    }

    if(errors==0){
        TEST_PASS();
    }
    else {
        TEST_FAIL();
    }
}

