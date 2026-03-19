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

// ------------------------- TC5 (Final SWRESET functional) -------------------------
static int test_resetctrl_swreset_functional(void) {
    int errors = 0;

    const uint32_t CLEAR_ALL_CAUSES  = 0x1Fu; // STATUS[4:0]
    const uint32_t SWRESET_CAUSE_BIT = 0x1u;  // STATUS bit0
    const uint32_t SWRESET_REQ_BIT   = 0x1u;  // REQ bit0

    printf("Test5: RESETCTRL SWRESET functional\n");

    // Clear causes
    MEGASOC_RESETCTRL->RESET_STATUS = CLEAR_ALL_CAUSES;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS, CLEAR_ALL_CAUSES, 500000u) != 0) {
        printf("  ERROR: timeout clearing causes before SWRESET\n");
        return 1;
    }

    printf("  STATUS after clear-all = 0x%08x\n", (unsigned)MEGASOC_RESETCTRL->RESET_STATUS);

    // Trigger SWRESET
    printf("  Writing RESET_REQ=SWRESET\n");
    MEGASOC_RESETCTRL->RESET_REQ = SWRESET_REQ_BIT;

    // Wait for cause latch
    if (poll_until_set(&MEGASOC_RESETCTRL->RESET_STATUS, SWRESET_CAUSE_BIT, 500000u) != 0) {
        printf("  ERROR: timeout waiting SWRESET cause latch\n");
        errors++;
        return errors;
    }

    uint32_t s1 = MEGASOC_RESETCTRL->RESET_STATUS;
    printf("  STATUS after SWRESET request = 0x%08x\n", (unsigned)s1);

    // Clear SWRESET cause
    MEGASOC_RESETCTRL->RESET_STATUS = SWRESET_CAUSE_BIT;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS, SWRESET_CAUSE_BIT, 500000u) != 0) {
        printf("  ERROR: timeout clearing SWRESET cause\n");
        errors++;
    }

    uint32_t s2 = MEGASOC_RESETCTRL->RESET_STATUS;
    printf("  STATUS after clearing SWRESET = 0x%08x\n", (unsigned)s2);

    // Repeatability: second cycle
    printf("  Repeat SWRESET cycle\n");
    MEGASOC_RESETCTRL->RESET_STATUS = CLEAR_ALL_CAUSES;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS, CLEAR_ALL_CAUSES, 500000u) != 0) {
        printf("  ERROR: timeout clearing causes before repeat cycle\n");
        errors++;
        return errors;
    }

    MEGASOC_RESETCTRL->RESET_REQ = SWRESET_REQ_BIT;

    if (poll_until_set(&MEGASOC_RESETCTRL->RESET_STATUS, SWRESET_CAUSE_BIT, 500000u) != 0) {
        printf("  ERROR: timeout waiting SWRESET cause latch (repeat)\n");
        errors++;
        return errors;
    }

    MEGASOC_RESETCTRL->RESET_STATUS = SWRESET_CAUSE_BIT;
    short_settle(2000);

    if (poll_until_clear(&MEGASOC_RESETCTRL->RESET_STATUS, SWRESET_CAUSE_BIT, 500000u) != 0) {
        printf("  ERROR: timeout clearing SWRESET cause (repeat)\n");
        errors++;
    }

    // Peripheral sanity
    UartStdOutInit();
    printf("  UART OK after SWRESET activity\n");

    return errors;
}

// ------------------------- main -----------------------
int main(void) {
    int errors = 0;

    UartStdOutInit();
    printf("MegaSoC Reset Controller Tests\n");

    errors += test_resetctrl_read_stability();
    errors += test_resetctrl_por_sanity();
    errors += test_resetctrl_status_clear_mask();
    errors += test_resetctrl_undefined_offsets();
    errors += test_resetctrl_swreset_functional();

    printf("Total issues in Reset Controller is = %d\n", errors);

    if (errors) TEST_FAIL();
    else        TEST_PASS();

    while (1) { }
}

