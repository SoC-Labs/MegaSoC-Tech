//==============================================================================
// sram_warm_reset_retention_tests.c -- BUG-40 fix guard (option A), 2026-11-11
//
// The BUG-40 fix (option A) resets the SRAM PPU and LPD with the warm reset
// but keeps the SRAM PCSM (power switches) on power-on reset, so SRAM stays
// powered and keeps its contents across a SW reset. This test pins that
// behaviour down before and after the RTL change.
//   Phase A: write a 1 KB pattern at 0x0080_C000 (unused by bootloader and
//            tests), clean it to SRAM, request RESET_REQ.SWRESET.
//   Phase B: invalidate and read the pattern back; every word must match.
// Run: make run_vcs TESTNAME=sram_warm_reset_retention_tests VCS_SIM_OPTION="+vcs+flush+log +rom_check +cpu_stall_cycles=200000"
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "megasoc_resetctrl.h"
#include <stdio.h>
#include <stdint.h>

#define PAT_BASE   0x0080C000UL
#define PAT_WORDS  256u

static inline void dsb_sy(void) { __asm__ volatile ("dsb sy" ::: "memory"); }
static inline uint32_t pat(uint32_t i) { return 0x5EED0000u ^ (i * 0x9E3779B1u); }

int main(void)
{
  volatile uint32_t *p = (volatile uint32_t *)PAT_BASE;
  uint32_t t = 500000u;
  int errors = 0;

  UartStdOutInit();
  printf("MegaSoC SRAM retention across SW reset (BUG-40 fix guard)\n");

  if ((MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_SWRESET_Msk) != 0u) {
    for (unsigned long a = PAT_BASE; a < PAT_BASE + PAT_WORDS * 4UL; a += 64UL)
      __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
    dsb_sy();
    for (uint32_t i = 0; i < PAT_WORDS; i++) {
      if (p[i] != pat(i)) {
        if (errors < 4)
          printf("[BUG-40 RET] FAIL: 0x%08lx = 0x%08x, expected 0x%08x\n",
                 PAT_BASE + i * 4UL, (unsigned)p[i], (unsigned)pat(i));
        errors++;
      }
    }
    printf("[BUG-40 RET] Phase-B: %u/%u words retained across SWRESET\n",
           (unsigned)(PAT_WORDS - (uint32_t)errors), (unsigned)PAT_WORDS);
    MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;
    printf("Total number of sram retention errors: %d\n", errors);
    if (errors == 0) { TEST_PASS(); } else { TEST_FAIL(); }
    while (1) { }
  }

  MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;
  dsb_sy();
  while ((MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_ALL_Msk) != 0u && --t) { }
  for (uint32_t i = 0; i < PAT_WORDS; i++) p[i] = pat(i);
  for (unsigned long a = PAT_BASE; a < PAT_BASE + PAT_WORDS * 4UL; a += 64UL)
    __asm__ volatile ("dc cvac, %0" : : "r"(a) : "memory");
  dsb_sy();
  printf("[BUG-40 RET] Phase-A: pattern written and cleaned, requesting SWRESET\n");
  MEGASOC_RESETCTRL->RESET_REQ = RESET_REQ_SWRESET_Msk;
  dsb_sy();
  for (volatile uint32_t i = 0; i < 200000u; i++) { }
  printf("[BUG-40 RET] FAIL: execution continued after SWRESET\n");
  TEST_FAIL();
  while (1) { }
}
