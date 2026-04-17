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

static void short_settle(uint32_t loops) {
    for (volatile uint32_t i = 0; i < loops; i++) { }
}

static int poll_until_set(const volatile uint32_t *reg, uint32_t mask, uint32_t timeout) {
    for (uint32_t i = 0; i < timeout; i++) {
        if ((*reg & mask) != 0u) return 0;
    }
    return -1;
}

static int poll_until_clear(volatile uint32_t *reg, uint32_t mask, uint32_t timeout) {
    for (uint32_t i = 0; i < timeout; i++) {
        if ((*reg & mask) == 0u) return 0;
    }
    return -1;
}

static int test_wdog_countdown(){
    int errors = 0;
    uint32_t load_value = 0xFFFFFFFFu;
    uint32_t v1, v2;

    printf("Test1: Watchdog basic Countdown\n");
    // Known state
    CMSDK_WDOG->WDOGCONTROL = 0X0;
    CMSDK_WDOG->WDOGINTCLR = 1;

    CMSDK_WDOG->WDOGLOAD = load_value;
    printf("WDOG Initial load value = 0x%08x\n", (unsigned)load_value);
    CMSDK_WDOG->WDOGCONTROL = CMSDK_WDOG_CONTROL_INTEN_Msk;

    //read value
    v1 = CMSDK_WDOG->WDOGVALUE;
    printf("WDOG value is v1= 0x%08x\n",(unsigned)v1);
    short_settle(10);
    v2 = CMSDK_WDOG->WDOGVALUE;
    printf("WDOG value after some delay is v2=0x%08x\n",(unsigned)v2);

    if (v1 > load_value){
        printf("Error: WDOG countdown is increasing\n");
        errors++;
    }

    if (v2 >= v1){
        printf("Error: WDOG countdown is increasing\n");
        errors++;
    }

    CMSDK_WDOG->WDOGCONTROL = 0X0;
    CMSDK_WDOG->WDOGINTCLR = 1;

    return errors;
}

static int test_wdog_interrupt(){
    int errors = 0;
    uint32_t load_value = 0x100;
    uint32_t ris, mis;

    printf("Test2: Watchdog interrupt assertion\n");
    // Known state
    CMSDK_WDOG->WDOGCONTROL = 0X0;
    CMSDK_WDOG->WDOGINTCLR = 1;

    CMSDK_WDOG->WDOGLOAD = load_value;
    CMSDK_WDOG->WDOGCONTROL = CMSDK_WDOG_CONTROL_INTEN_Msk;

    if(poll_until_set(&CMSDK_WDOG->WDOGRIS, CMSDK_WDOG_RIS_RWDOGINT_Msk, 500000u) != 0){
        errors++;
        printf("Error: Timeout waiting for WDOGRIS to set\n");
    }

    ris = CMSDK_WDOG->WDOGRIS;
    mis = CMSDK_WDOG->WDOGMIS;

    printf("RIS = 0x%08x  and MIS = 0x%08x\n", (unsigned)ris, (unsigned)mis);

    if ((ris & CMSDK_WDOG_RIS_RWDOGINT_Msk) == 0u) {
        printf("  ERROR: WDOGRIS interrupt bit not set\n");
        errors++;
    }

    if ((mis & CMSDK_WDOG_MIS_WDOGINT_Msk) == 0u) {
        printf("  ERROR: WDOGMIS interrupt bit not set\n");
        errors++;
    }

    CMSDK_WDOG->WDOGINTCLR  = 1;
    CMSDK_WDOG->WDOGCONTROL = 0x0;

    return errors;
}

static int test_wdog_intclr_reload(void) {
    int errors = 0;
    uint32_t load_value = 0x100;
    uint32_t ris_before, mis_before;
    uint32_t ris_after, mis_after, value_after;

    printf("Test3: Watchdog interrupt clear and reload\n");

    CMSDK_WDOG->WDOGCONTROL = 0x0;
    CMSDK_WDOG->WDOGINTCLR  = 1;
    // load & enable interrupt only
    CMSDK_WDOG->WDOGLOAD = load_value;
    CMSDK_WDOG->WDOGCONTROL = CMSDK_WDOG_CONTROL_INTEN_Msk;

    /* Wait for interrupt assertion */
    if (poll_until_set(&CMSDK_WDOG->WDOGRIS,
                       CMSDK_WDOG_RIS_RWDOGINT_Msk,
                       500000u) != 0) {
        printf("  ERROR: timeout waiting for WDOGRIS to set\n");
        errors++;
    }
    ris_before = CMSDK_WDOG->WDOGRIS;
    mis_before = CMSDK_WDOG->WDOGMIS;                
           
    printf("  Before clear: RIS=0x%08x MIS=0x%08x\n",
           (unsigned)ris_before, (unsigned)mis_before);

    if ((ris_before & CMSDK_WDOG_RIS_RWDOGINT_Msk) == 0u) {
        printf("  ERROR: WDOGRIS bit not set before clear\n");
        errors++;
    }

    if ((mis_before & CMSDK_WDOG_MIS_WDOGINT_Msk) == 0u) {
        printf("  ERROR: WDOGMIS bit not set before clear\n");
        errors++;
    }
    // clear interrupt
    CMSDK_WDOG->WDOGCONTROL = 0x0;
    CMSDK_WDOG->WDOGINTCLR  = 1;

    ris_after   = CMSDK_WDOG->WDOGRIS;
    mis_after   = CMSDK_WDOG->WDOGMIS;
    value_after = CMSDK_WDOG->WDOGVALUE;

    printf("  After clear : RIS=0x%08x MIS=0x%08x VALUE=0x%08x\n",
           (unsigned)ris_after, (unsigned)mis_after, (unsigned)value_after);

    if ((ris_after & CMSDK_WDOG_RIS_RWDOGINT_Msk) != 0u) {
        printf("  ERROR: WDOGRIS bit not cleared by WDOGINTCLR\n");
        errors++;
    }

    if ((mis_after & CMSDK_WDOG_MIS_WDOGINT_Msk) != 0u) {
        printf("  ERROR: WDOGMIS bit not cleared by WDOGINTCLR\n");
        errors++;
    }

   // Reload value check
    if ((value_after == 0u) || (value_after > load_value)) {
        printf("  ERROR: WDOGVALUE not reloaded correctly after WDOGINTCLR\n");
        errors++;
    }

    /* Cleanup */
    CMSDK_WDOG->WDOGCONTROL = 0x0;
    CMSDK_WDOG->WDOGINTCLR  = 1;

    return errors;
}


static int test_wdog_reset_issue_only(){
    printf("Test4A: Watchdog reset request\n");

    MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS,RESET_STATUS_ALL_Msk,500000u) != 0) {
        printf("  ERROR: timeout clearing reset-controller causes before WDOG test\n");
        return 1;
    }

    CMSDK_WDOG->WDOGCONTROL = 0x0;
    CMSDK_WDOG->WDOGINTCLR  = 1;
    CMSDK_WDOG->WDOGLOAD = 0x100u;

    printf("  Enabling WDOG with INTEN|RESEN\n");
    CMSDK_WDOG->WDOGCONTROL =
        CMSDK_WDOG_CONTROL_INTEN_Msk |
        CMSDK_WDOG_CONTROL_RESEN_Msk;

    /* Do NOT clear interrupt.
       Expected sequence:
       first expiry  -> WDOGINT
       second expiry -> WDOGRES -> reset controller -> system reset */
    while (1) { }
}

static int test_wdog_reset_postcheck(void) {
    int errors = 0;
    uint32_t s = MEGASOC_RESETCTRL->RESET_STATUS;

    printf("Test4B: Post-WDOG-reset verification\n");
    printf("  RESET_STATUS after reboot = 0x%08x\n", (unsigned)s);

    if ((s & RESET_STATUS_WDOG_Msk) == 0u) {
        printf("  ERROR: watchdog reset cause bit not set after reboot\n");
        errors++;
    } else {
        printf("  PASS: watchdog reset cause bit observed\n");
    }

    /* Clear watchdog cause bit */
    MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_WDOG_Msk;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS,RESET_STATUS_WDOG_Msk,500000u) != 0) {
        printf("  ERROR: timeout clearing watchdog reset cause bit\n");
        errors++;
    } else {
        printf("  PASS: watchdog reset cause bit cleared\n");
    }

    return errors;
}

 int main(void){
    int errors = 0;
    uint32_t status;
    UartStdOutInit();
    printf("MegaSoC CMSDK WatchDog Integration test\n");

    status = MEGASOC_RESETCTRL->RESET_STATUS;
    printf("Entry RESET_STATUS = 0x%08x\n", (unsigned)status);

    if ((status & RESET_STATUS_WDOG_Msk) != 0){
        errors += test_wdog_reset_postcheck();
        printf("Total errors in Watchdog reset testing = %d\n", errors);
        if (!errors) TEST_PASS();
        else         TEST_FAIL();

        while (1) { }
    }

    errors += test_wdog_countdown();
    errors += test_wdog_interrupt();
    errors += test_wdog_intclr_reload();
    
    if (errors) {
        printf("Pre-WDOG-reset tests failed = %d\n", errors);
        TEST_FAIL();
        while (1) { }
    }

    errors += test_wdog_reset_issue_only();

    // Should never reach here if reset really happened
    printf("  ERROR: execution continued after watchdog reset request path\n");
    TEST_FAIL();

    while (1) { }

 }

