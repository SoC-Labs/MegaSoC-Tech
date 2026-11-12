//==============================================================================
// sram_ppu_off_dyn_off_reboot_tests.c -- BUG-40, SRAM variant of MEGASOC-BOOT-017 (b)
//
// Written 2026-11-11. SRAM_PPU, like ROM_PPU, is reset by power-on reset
// only, and the boot ROM keeps its stack in SRAM (0x0080_0000). If software
// leaves SRAM_PPU at policy OFF with PWR_DYN_EN=0, a SW reset re-runs the boot
// ROM, whose first stack access stalls forever on the powered-off SRAM
// (cfg_gate_resp=0). Expected today: hang. Expected after the BUG-40 fix
// (SRAM PPU + LPD reset by the warm reset): the PPU returns to its default
// policy, powers SRAM up, and the re-boot reaches Phase B.
//
// Phase A (power-on boot):
//   - prints SRAM_PPU PWRP/PWSR/IDR0 after power-on reset (the SRAM_PPU
//     default policy is not in pck600_config.yaml; this records it);
//   - cleans the data cache over the whole SRAM window so no write-back is
//     pending, then, in register-only assembly (the stack is in SRAM and
//     must not be touched once SRAM is off): writes SRAM_PPU PWRP = OFF with
//     PWR_DYN_EN=0, polls PWSR until OFF, writes RESET_REQ.SWRESET, spins.
//   - If SRAM never reaches OFF, the policy is restored and the test fails.
// Phase B (RESET_STATUS.SWRESET=1): prints SRAM_PPU state after the warm
//   reset, checks a SRAM write/read, TEST_PASS.
// Run with the CPU bus watchdog so a hang ends the run with the stuck address:
//   make run_vcs TESTNAME=sram_ppu_off_dyn_off_reboot_tests VCS_SIM_OPTION="+vcs+flush+log +rom_check +cpu_stall_cycles=200000"
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "megasoc_resetctrl.h"
#include "ppu.h"
#include <stdio.h>
#include <stdint.h>

#define SRAM_BASE        0x00800000UL
#define SRAM_END         0x00810000UL
#define SRAM_PROBE       0x0080C000UL     // unused by bootloader and tests
#define PPU_PWR_OFF      0x0u
#define PPU_PWR_ON       0x8u
#define PWRP_POLICY_MSK  0x0000000Fu
#define PWRP_DYN_EN_MSK  0x00000100u
#define OFF_POLL_LIMIT   1000000u

static inline void dsb_sy(void) { __asm__ volatile ("dsb sy" ::: "memory"); }

// Gate SRAM and request the SW reset without touching memory. Returns only
// if SRAM did not reach OFF (policy restored to the original value first).
static uint32_t gate_sram_and_reset(uint32_t pwrp_off, uint32_t pwrp_orig)
{
  uint64_t pwrp_addr  = (uint64_t)&SRAM_PPU->PPU_PWRP.W;
  uint64_t pwsr_addr  = (uint64_t)&SRAM_PPU->PPU_PWSR;
  uint64_t rreq_addr  = (uint64_t)&MEGASOC_RESETCTRL->RESET_REQ;
  uint64_t limit      = OFF_POLL_LIMIT;
  uint32_t swreset    = (uint32_t)RESET_REQ_SWRESET_Msk;
  uint32_t status;
  __asm__ volatile (
    "str  %w[off], [%[pwrp]]\n\t"
    "dsb  sy\n\t"
    "1:\n\t"
    "ldr  %w[st], [%[pwsr]]\n\t"
    "and  %w[st], %w[st], #0xf\n\t"
    "cbz  %w[st], 2f\n\t"              // PWR_STATUS == OFF
    "subs %[lim], %[lim], #1\n\t"
    "b.ne 1b\n\t"
    "str  %w[orig], [%[pwrp]]\n\t"     // timeout: restore policy, return
    "dsb  sy\n\t"
    "b    3f\n\t"
    "2:\n\t"
    "str  %w[sw], [%[rreq]]\n\t"       // SRAM is OFF: request the SW reset
    "dsb  sy\n\t"
    "4:\n\t"
    "b    4b\n\t"                      // wait for the reset, no memory access
    "3:\n\t"
    : [st] "=&r"(status), [lim] "+r"(limit)
    : [off] "r"(pwrp_off), [orig] "r"(pwrp_orig), [sw] "r"(swreset),
      [pwrp] "r"(pwrp_addr), [pwsr] "r"(pwsr_addr), [rreq] "r"(rreq_addr)
    : "memory", "cc");
  return status;
}

static void phase_a(void)
{
  uint32_t t = 500000u;
  uint32_t pwrp = SRAM_PPU->PPU_PWRP.W;
  uint32_t pwsr = SRAM_PPU->PPU_PWSR.W;
  uint32_t idr0 = SRAM_PPU->PPU_IDR0;
  uint32_t st;

  // Kept short: c_print() formats into a 160-byte stack buffer (BUG-22/BUG-32).
  printf("[BUG-40 SRAM] Phase-A: SRAM_PPU after POR PWRP=0x%08x PWSR=0x%08x\n",
         (unsigned)pwrp, (unsigned)pwsr);
  printf("[BUG-40 SRAM]   IDR0=0x%08x, default policy=0x%x dyn_en=%u\n",
         (unsigned)idr0, (unsigned)(pwrp & PWRP_POLICY_MSK),
         (unsigned)((pwrp & PWRP_DYN_EN_MSK) ? 1u : 0u));

  MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;   // W1C stale causes
  dsb_sy();
  while ((MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_ALL_Msk) != 0u && --t) { }

  printf("[BUG-40 SRAM] Phase-A: cleaning D-cache over SRAM, gating SRAM (OFF, dyn 0), then SWRESET\n");
  for (unsigned long a = SRAM_BASE; a < SRAM_END; a += 64UL)
    __asm__ volatile ("dc cvac, %0" : : "r"(a) : "memory");
  dsb_sy();

  st = gate_sram_and_reset((pwrp & ~(PWRP_POLICY_MSK | PWRP_DYN_EN_MSK)) | PPU_PWR_OFF, pwrp);
  printf("[BUG-40 SRAM] FAIL Phase-A: SRAM_PPU did not reach OFF (PWR_STATUS=0x%x), policy restored\n",
         (unsigned)st);
  TEST_FAIL();
  while (1) { }
}

static int phase_b(void)
{
  int errors = 0;
  uint32_t status = MEGASOC_RESETCTRL->RESET_STATUS;
  uint32_t pwrp = SRAM_PPU->PPU_PWRP.W;
  uint32_t pwsr = SRAM_PPU->PPU_PWSR.W;
  volatile uint32_t *probe = (volatile uint32_t *)SRAM_PROBE;

  printf("[BUG-40 SRAM] Phase-B: re-boot COMPLETED after SRAM was left OFF (dyn 0)\n");
  printf("[BUG-40 SRAM] Phase-B: RESET_STATUS=0x%08x\n", (unsigned)status);
  printf("[BUG-40 SRAM] SRAM_PPU after warm reset: PWRP=0x%08x PWSR=0x%08x\n",
         (unsigned)pwrp, (unsigned)pwsr);
  if ((status & RESET_STATUS_SWRESET_Msk) == 0u) {
    printf("[BUG-40 SRAM] FAIL Phase-B: RESET_STATUS.SWRESET not set\n");
    errors++;
  }
  if ((pwsr & 0xFu) != PPU_PWR_ON) {
    printf("[BUG-40 SRAM] FAIL Phase-B: SRAM_PPU PWR_STATUS=0x%x, expected ON\n", (unsigned)(pwsr & 0xFu));
    errors++;
  }
  MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;

  *probe = 0xC0FFEE40u;
  __asm__ volatile ("dc civac, %0" : : "r"(SRAM_PROBE) : "memory");
  dsb_sy();
  if (*probe != 0xC0FFEE40u) {
    printf("[BUG-40 SRAM] FAIL Phase-B: SRAM write/read-back at 0x%08lx failed\n", SRAM_PROBE);
    errors++;
  }
  return errors;
}

int main(void)
{
  int errors;

  UartStdOutInit();
  printf("MegaSoC SRAM PPU off warm re-boot test (BUG-40, SRAM variant of BOOT-017 b)\n");

  if ((MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_SWRESET_Msk) != 0u) {
    errors = phase_b();
    printf("Total number of sram ppu off reboot errors: %d\n", errors);
    if (errors == 0) { TEST_PASS(); } else { TEST_FAIL(); }
    while (1) { }
  }
  phase_a();
  return 0;
}
