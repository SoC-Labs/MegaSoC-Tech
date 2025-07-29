#include "uart_stdout.h"
#include <stdio.h>
#include "system.h"
#include "sys_memory_map.h"

#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))

int dap_id_check(void);

int main(void) {
    uint32_t errors = 0;
    UartStdOutInit();

    printf("DAP debug port tests - SoCLabs MegaSoC\n");

    if(dap_id_check()!=0){
        printf("DAP not present\n");
        printf ("\n** TEST SKIPPED **\n");
        TEST_FAIL();
    }

    TEST_PASS();
}

/* --------------------------------------------------------------- */
/* Peripheral detection                                            */
/* --------------------------------------------------------------- */
/* Detect the part number to see if device is present              */
int dap_id_check(void)
{
    unsigned char PIDR0, PIDR1, PIDR2, PIDR3, PIDR4, PIDR5, PIDR6, PIDR7;
    PIDR4=HW8_REG(DAP_DBG_BASE+0x0FD0);
    PIDR5=HW8_REG(DAP_DBG_BASE+0x0FD4);
    PIDR6=HW8_REG(DAP_DBG_BASE+0x0FD8);
    PIDR7=HW8_REG(DAP_DBG_BASE+0x0FDC);
    PIDR0=HW8_REG(DAP_DBG_BASE+0x0FE0);
    PIDR1=HW8_REG(DAP_DBG_BASE+0x0FE4);
    PIDR2=HW8_REG(DAP_DBG_BASE+0x0FE8);
    PIDR3=HW8_REG(DAP_DBG_BASE+0x0FEC);
    printf("PIDR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", PIDR0,PIDR1,PIDR2,PIDR3,PIDR4,PIDR5,PIDR6,PIDR7);
  if ((PIDR4 != 0x04) ||
      (PIDR5 != 0x00) ||
      (PIDR6 != 0x00) ||
      (PIDR7 != 0x00) ||
      (PIDR0 != 0xA1) ||
      (PIDR1 != 0xB4) ||
      (PIDR2 != 0x4B) ||
      (PIDR3 != 0x00))
    return 1; /* part ID & ARM ID does not match */
  else
    return 0;
}
