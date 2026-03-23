#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "system_level_functions.h"
#include <stdio.h>
#include <stdint.h>
#include "megasoc_resetctrl.h"

// ------------------------- helpers -------------------------
static void short_settle(uint32_t loops) {
    for (volatile uint32_t i = 0; i < loops; i++) { }
}

static int poll_until_set(volatile uint32_t *reg, uint32_t mask, uint32_t timeout) {
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

// ------------------------- TC1 -------------------------
static int test_resetctrl_read_stability(void) {
    int errors = 0;
    printf("Test1: RESETCTRL read stability\n");

    uint32_t s1 = MEGASOC_RESETCTRL->RESET_STATUS;
    uint32_t s2 = MEGASOC_RESETCTRL->RESET_STATUS;

    printf("  STATUS read1=0x%08x read2=0x%08x\n", (unsigned)s1, (unsigned)s2);

    if (s1 != s2) {
        printf("  ERROR: STATUS unstable across back-to-back reads\n");
        errors++;
    }
    if ((s1 & ~0x1Fu) != 0u) {
        printf("  ERROR: STATUS has unexpected upper bits set: 0x%08x\n", (unsigned)s1);
        errors++;
    }
    return errors;
}

// ------------------------- TC2 -------------------------
static int test_resetctrl_por_sanity(void) {
    int errors = 0;
    printf("Test2: RESETCTRL POR sanity\n");

    uint32_t s = MEGASOC_RESETCTRL->RESET_STATUS;
    printf("  STATUS=0x%08x\n", (unsigned)s);

    // POR is bit4 in RTL (0x10)
    if ((s & 0x10u) == 0u) {
        printf("  WARNING: POR bit not set (may be cleared by earlier flow)\n");
        errors++; 
    }
    return errors;
}

// ------------------------- TC3 -------------------------
static int test_resetctrl_status_clear_mask(void) {
    int errors = 0;
    printf("Test3: RESETCTRL STATUS clear-mask\n");

    uint32_t before = MEGASOC_RESETCTRL->RESET_STATUS;
    printf("  STATUS before clear = 0x%08x\n", (unsigned)before);

    // Clear all cause bits [4:0]
    MEGASOC_RESETCTRL->RESET_STATUS = 0x1Fu;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS, 0x1Fu, 500000u) != 0) {
        uint32_t s = MEGASOC_RESETCTRL->RESET_STATUS;
        printf("  ERROR: timeout clearing STATUS. STATUS=0x%08x\n", (unsigned)s);
        errors++;
        return errors;
    }

    uint32_t after = MEGASOC_RESETCTRL->RESET_STATUS;
    printf("  STATUS after clear  = 0x%08x\n", (unsigned)after);

    if ((after & 0x1Fu) != 0u) {
        printf("  ERROR: STATUS bits did not clear, remaining=0x%08x\n", (unsigned)(after & 0x1Fu));
        errors++;
    }
    return errors;
}

// ------------------------- TC4 -------------------------
static int test_resetctrl_undefined_offsets(void) {
    int errors = 0;
    printf("Test4: RESETCTRL undefined offsets\n");

    volatile uint32_t *base = (volatile uint32_t *)SYS_RESETCTRL_BASE;

    uint32_t r08a = base[2];  // 0x08
    uint32_t r08b = base[2];
    uint32_t r0Ca = base[3];  // 0x0C
    uint32_t r0Cb = base[3];

    printf("  Read1[0x08]=0x%08x Read2[0x08]=0x%08x\n", (unsigned)r08a, (unsigned)r08b);
    printf("  Read1[0x0C]=0x%08x Read2[0x0C]=0x%08x\n", (unsigned)r0Ca, (unsigned)r0Cb);

    if (r08a != r08b) { printf("  ERROR: 0x08 read unstable\n"); errors++; }
    if (r0Ca != r0Cb) { printf("  ERROR: 0x0C read unstable\n"); errors++; }

    return errors;
}

// ------------------------- TC5 -------------------------
static int test_resetctrl_swreset_issue_only(void) {
    printf("Test5A: issue SW reset\n");

    // Clear any old sticky causes before starting the SWRESET test
    MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS, RESET_STATUS_ALL_Msk, 500000u) != 0) {
        printf("  ERROR: timeout clearing causes before SWRESET\n");
        return 1;
    }

    printf("  STATUS before SWRESET request = 0x%08x\n",(unsigned)MEGASOC_RESETCTRL->RESET_STATUS);

    printf("  Triggering software reset...\n");
    MEGASOC_RESETCTRL->RESET_REQ = RESET_REQ_SWRESET_Msk;

    if (poll_until_set(&MEGASOC_RESETCTRL->RESET_STATUS, RESET_STATUS_SWRESET_Msk, 500000u) == 0) {
        printf("  INFO: SWRESET cause latched before CPU restart\n");
    }

    while (1) { }
}

static int test_resetctrl_swreset_postcheck(void) {
    int errors = 0;
    uint32_t s = MEGASOC_RESETCTRL->RESET_STATUS;

    printf("Test5B: post-SWRESET verification\n");
    printf("  STATUS after reboot = 0x%08x\n", (unsigned)s);

    if ((s & RESET_STATUS_SWRESET_Msk) == 0u) {
        printf("  ERROR: SWRESET cause bit not set after reboot\n");
        errors++;
    } else {
        printf("  PASS: SWRESET cause bit observed after reboot\n");
    }

    UartStdOutInit();
    printf("  PASS: test method re-entered after SWRESET\n");

    // Clear the SWRESET cause and confirm clear works
    MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_SWRESET_Msk;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS, RESET_STATUS_SWRESET_Msk, 500000u) != 0) {
        printf("  ERROR: timeout clearing SWRESET cause after reboot\n");
        errors++;
    } else {
        printf("  PASS: SWRESET cause bit cleared successfully\n");
    }

    return errors;
}

// ------------------------- main -------------------------
int main(void) {
    int errors = 0;
    uint32_t status;
    UartStdOutInit();
    printf("MegaSoC Reset Controller Tests\n");

    status = MEGASOC_RESETCTRL->RESET_STATUS;
    printf("Entry RESET_STATUS = 0x%08x\n", (unsigned)status);

    // Post-reset path: If SWRESET is set then reboot phase
    if ((status & RESET_STATUS_SWRESET_Msk) != 0u) {
        errors += test_resetctrl_swreset_postcheck();

        printf("Total issues in Reset Controller = %d\n", errors);
        if (errors) TEST_FAIL();
        else        TEST_PASS();

        while (1) { }
    }
    errors += test_resetctrl_read_stability();
    errors += test_resetctrl_por_sanity();
    errors += test_resetctrl_status_clear_mask();
    errors += test_resetctrl_undefined_offsets();

    if (errors) {
        printf("Pre-SWRESET tests already failed: %d\n", errors);
        TEST_FAIL();
        while (1) { }
    }

    // This should restart the system and come back through main()
    errors += test_resetctrl_swreset_issue_only();

    // Reaching here means reset did not happen
    printf("ERROR: execution continued after SWRESET request\n");
    TEST_FAIL();

    while (1) { }
}

