//==============================================================================
// rom_ppu_off_warm_reboot_tests.c -- MEGASOC-BOOT-017 (tc_rom_ppu_off_warm_reboot)
//
// Written 2026-11-10 from megasoc_spec.md section 6.1.1 Q5, section 5.13 and
// section 11 A4. The ROM PPU is POR-reset only, so a SW warm reset keeps
// whatever ROM power policy software left, and the warm re-boot fetch at 0x0
// depends on it.
//
// Case (a), this test (BOOT017_CASE_B undefined):
//   ROM_PPU PWR_POLICY=OFF with PWR_DYN_EN=1, then SWRESET.
//   Pass: the re-boot completes (Phase B is reached: the first fetch at 0x0
//   stalled, the PPU woke the ROM dynamically, the data was correct).
//   Run: make run_vcs TESTNAME=rom_ppu_off_warm_reboot_tests VCS_SIM_OPTION="+vcs+flush+log +rom_check"
//
// Case (b), rom_ppu_off_dyn_off_reboot_tests (includes this file with
// BOOT017_CASE_B defined): PWR_POLICY=OFF with PWR_DYN_EN=0, then SWRESET.
//   Characterisation: the spec expects a stall at the reset vector because
//   cfg_gate_resp=0 stalls the access and nothing wakes the domain. If the
//   re-boot completes, Phase B prints so and passes. If it stalls, the bench
//   (+rom_boot_stall_cycles=N) prints "boot stalled at the reset vector" and
//   ends the run: that outcome is a design issue to raise, not a test bug.
//   Run: make run_vcs TESTNAME=rom_ppu_off_dyn_off_reboot_tests VCS_SIM_OPTION="+vcs+flush+log +rom_check +rom_boot_stall_cycles=200000"
//
// Phase B (both cases) records the ROM PPU state that survived the warm reset
// (policy, dynamic enable, status) and restores PWR_POLICY=ON.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "megasoc_resetctrl.h"
#include "ppu.h"
#include <stdio.h>
#include <stdint.h>
#ifdef BOOT017_WDOG
#include "CMSDK.h"
#endif

#ifdef BOOT017_CASE_B
#define CASE_NAME   "(b) PWR_DYN_EN=0"
#define CASE_DYN_EN 0u
#else
#define CASE_NAME   "(a) PWR_DYN_EN=1"
#define CASE_DYN_EN 1u
#endif

#define ROM_WIN_BASE      0x00000000UL
#define ROM_FIRST_WORD    0xd2800000UL
// Reset source: SW (RESET_REQ.SWRESET) by default; BOOT017_WDOG selects the
// watchdog (WDOGCONTROL INTEN|RESEN, no WDOGINTCLR: reset at the 2nd expiry,
// same sequence as wdog_resen_reset_tests). Added 2026-11-11 for BUG-40.
#ifdef BOOT017_WDOG
#define CAUSE_NAME   "WDOG"
#define CAUSE_MSK    RESET_STATUS_WDOG_Msk
#define WDOG_LOAD    0x100u
#else
#define CAUSE_NAME   "SWRESET"
#define CAUSE_MSK    RESET_STATUS_SWRESET_Msk
#endif

#define PPU_PWR_OFF       0x0u           // DEN0051E Table 3-2: OFF
#define PPU_PWR_ON        0x8u           // DEN0051E Table 3-2: ON
#define PPU_POLL_TIMEOUT  500000u

static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }

static inline void barrier_sy(void)
{
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}

static int ppu_wait(uint32_t want, const char *what)
{
  uint32_t t = PPU_POLL_TIMEOUT, s;
  while ((s = ROM_PPU->PPU_PWSR.B.PWR_STATUS) != want && --t) { }
  if (s != want) {
    printf("[MEGASOC-BOOT-017] TIMEOUT: %s: PWR_STATUS stuck at 0x%x (wanted 0x%x)\n",
           what, (unsigned)s, (unsigned)want);
    return 1;
  }
  return 0;
}

static void phase_a(void)
{
  uint32_t t = PPU_POLL_TIMEOUT;

  printf("[MEGASOC-BOOT-017] Phase-A case %s: ROM_PPU PWRP=0x%08x PWR_STATUS=0x%x\n",
         CASE_NAME, (unsigned)ROM_PPU->PPU_PWRP.W, (unsigned)ROM_PPU->PPU_PWSR.B.PWR_STATUS);
  if (ROM_PPU->PPU_PWSR.B.PWR_STATUS != PPU_PWR_ON) {
    printf("[MEGASOC-BOOT-017] FAIL Phase-A: ROM_PPU not ON at start\n");
    TEST_FAIL();
    while (1) { }
  }

  MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;   // W1C stale causes
  barrier_sy();
  while ((MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_ALL_Msk) != 0u && --t) { }

  ROM_PPU->PPU_PWRP.B.PWR_DYN_EN = CASE_DYN_EN;
  ROM_PPU->PPU_PWRP.B.PWR_POLICY = PPU_PWR_OFF;
  barrier_sy();
#if defined(BOOT017_NO_WAIT) && defined(BOOT017_WDOG)
#error "BOOT017_NO_WAIT is SWRESET only"
#endif
#ifdef BOOT017_NO_WAIT
  // BUG-40 transition case (2026-11-11): request the reset straight after
  // the OFF request, without waiting for PWR_STATUS=OFF and without a print
  // in between, so the warm reset lands while ROM_PPU may still be moving
  // ON -> OFF. Phase B reports the PPU state that came out of it.
  MEGASOC_RESETCTRL->RESET_REQ = RESET_REQ_SWRESET_Msk;
  barrier_sy();
#else
  if (ppu_wait(PPU_PWR_OFF, "ROM_PPU OFF request")) {
    ROM_PPU->PPU_PWRP.B.PWR_POLICY = PPU_PWR_ON;
    TEST_FAIL();
    while (1) { }
  }
  printf("[MEGASOC-BOOT-017] Phase-A: ROM_PPU OFF confirmed (PWRP=0x%08x), requesting %s reset\n",
         (unsigned)ROM_PPU->PPU_PWRP.W, CAUSE_NAME);
#ifdef BOOT017_WDOG
  CMSDK_WDOG->WDOGCONTROL = 0x0u;
  CMSDK_WDOG->WDOGINTCLR  = 1u;
  CMSDK_WDOG->WDOGLOAD    = WDOG_LOAD;
  CMSDK_WDOG->WDOGCONTROL = CMSDK_WDOG_CONTROL_INTEN_Msk | CMSDK_WDOG_CONTROL_RESEN_Msk;
#else
  MEGASOC_RESETCTRL->RESET_REQ = RESET_REQ_SWRESET_Msk;
#endif
  barrier_sy();
#endif
#ifdef BOOT017_WDOG
  for (volatile uint32_t i = 0; i < 20000000u; i++) { }   // the WDOG needs two full expiries
#else
  for (volatile uint32_t i = 0; i < 200000u; i++) { }
#endif
}

static int phase_b(void)
{
  int errors = 0;
  uint32_t status = MEGASOC_RESETCTRL->RESET_STATUS;
  uint32_t pwrp   = ROM_PPU->PPU_PWRP.W;
  uint32_t pwst   = ROM_PPU->PPU_PWSR.B.PWR_STATUS;
  uint32_t v;

  printf("[MEGASOC-BOOT-017] Phase-B case %s: re-boot COMPLETED with the ROM PPU left OFF\n", CASE_NAME);
  // Kept short: c_print() formats into a 160-byte stack buffer (BUG-22/BUG-32).
  printf("[MEGASOC-BOOT-017] Phase-B: RESET_STATUS=0x%08x\n", (unsigned)status);
  printf("[MEGASOC-BOOT-017] ROM_PPU after warm reset: PWRP=0x%08x PWR_STATUS=0x%x\n",
         (unsigned)pwrp, (unsigned)pwst);
  printf("[MEGASOC-BOOT-017]   PWR_POLICY=0x%x PWR_DYN_EN=%u (characterisation)\n",
         (unsigned)ROM_PPU->PPU_PWRP.B.PWR_POLICY, (unsigned)ROM_PPU->PPU_PWRP.B.PWR_DYN_EN);
  if ((status & CAUSE_MSK) == 0u) {
    printf("[MEGASOC-BOOT-017] FAIL Phase-B: RESET_STATUS.%s not set\n", CAUSE_NAME);
    errors++;
  }
  MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;

  // Restore the ROM domain to ON and check ROM content.
  ROM_PPU->PPU_PWRP.B.PWR_POLICY = PPU_PWR_ON;
  barrier_sy();
  errors += ppu_wait(PPU_PWR_ON, "ROM_PPU ON restore");
  __asm__ volatile ("dc civac, %0" : : "r"(ROM_WIN_BASE) : "memory");
  barrier_sy();
  v = rd32(ROM_WIN_BASE);
  if (v != ROM_FIRST_WORD) {
    printf("[MEGASOC-BOOT-017] FAIL Phase-B: ROM word @0x0 = 0x%08x, expected 0x%08x\n",
           (unsigned)v, (unsigned)ROM_FIRST_WORD);
    errors++;
  }
  return errors;
}

int main(void)
{
  int errors;

  UartStdOutInit();
  printf("MegaSoC ROM PPU off warm re-boot test (tc_rom_ppu_off_warm_reboot), case %s, %s\n", CASE_NAME, CAUSE_NAME);
  printf("vplan: MEGASOC-BOOT-017\n");

  if ((MEGASOC_RESETCTRL->RESET_STATUS & CAUSE_MSK) != 0u) {
    errors = phase_b();
    printf("Total number of rom ppu off warm reboot errors: %d\n", errors);
    if (errors == 0) { TEST_PASS(); } else { TEST_FAIL(); }
    while (1) { }
  }

  phase_a();
  printf("[MEGASOC-BOOT-017] FAIL: execution continued after the %s request\n", CAUSE_NAME);
  ROM_PPU->PPU_PWRP.B.PWR_POLICY = PPU_PWR_ON;
  TEST_FAIL();
  while (1) { }
}
