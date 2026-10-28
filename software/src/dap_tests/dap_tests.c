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
/* Detect the CoreSight ROM table of the DAP to see if the debug block is  */
/* present. The Component ID registers (0xFF0-0xFFC) of every CoreSight   */
/* component read 0xB105_x00D with x = component class; class 1 is a ROM  */
/* table, so this is independent of how DapRomDefs.v is configured. The   */
/* Peripheral ID registers (designer/part number) are printed for         */
/* information only: the Arm-supplied DapRomDefs.v leaves them unset.     */
int dap_id_check(void)
{
    unsigned char PIDR0, PIDR1, PIDR2, PIDR3, PIDR4, PIDR5, PIDR6, PIDR7;
    unsigned char CIDR0, CIDR1, CIDR2, CIDR3;
    unsigned int  cid;

    PIDR4=HW8_REG(DAP_DBG_BASE+0x0FD0);
    PIDR5=HW8_REG(DAP_DBG_BASE+0x0FD4);
    PIDR6=HW8_REG(DAP_DBG_BASE+0x0FD8);
    PIDR7=HW8_REG(DAP_DBG_BASE+0x0FDC);
    PIDR0=HW8_REG(DAP_DBG_BASE+0x0FE0);
    PIDR1=HW8_REG(DAP_DBG_BASE+0x0FE4);
    PIDR2=HW8_REG(DAP_DBG_BASE+0x0FE8);
    PIDR3=HW8_REG(DAP_DBG_BASE+0x0FEC);
    CIDR0=HW8_REG(DAP_DBG_BASE+0x0FF0);
    CIDR1=HW8_REG(DAP_DBG_BASE+0x0FF4);
    CIDR2=HW8_REG(DAP_DBG_BASE+0x0FF8);
    CIDR3=HW8_REG(DAP_DBG_BASE+0x0FFC);
    printf("PIDR = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", PIDR0,PIDR1,PIDR2,PIDR3,PIDR4,PIDR5,PIDR6,PIDR7);
    cid = ((unsigned int)CIDR3 << 24) | ((unsigned int)CIDR2 << 16) | ((unsigned int)CIDR1 << 8) | CIDR0;
    printf("CIDR = 0x%08x (class %d)\n", cid, (CIDR1 >> 4) & 0xF);

    if (cid != 0xB105100D)
        return 1; /* not a CoreSight ROM table: debug block not reachable */

    if ((PIDR1 & 0xF0) == 0 && (PIDR2 & 0x07) == 0 && PIDR4 == 0)
        printf("Note: ROM table designer/part ID not configured (DapRomDefs.v)\n");

    return 0;
}
