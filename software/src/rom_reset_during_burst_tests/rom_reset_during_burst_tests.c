//==============================================================================
// rom_reset_during_burst_tests.c -- MEGASOC-BOOT-011 (tc_reset_during_rom_burst)
//
// Written 2026-11-10 from megasoc_spec.md section 6.1.1 Q3 and section 10 S12.
// A SW warm reset resets the NIC-400 (SYS_RESETn_int) but not the ROM SIE-300
// (ROM_RESETn is POR-only). This test asserts SWRESET while the CPU keeps
// ROM_AXI busy with back-to-back line fills, then checks the reboot.
//
// Run with the bench checker on:
//   make run_vcs TESTNAME=rom_reset_during_burst_tests VCS_SIM_OPTION="+vcs+flush+log +rom_check"
//
// Two phases, told apart by RESET_STATUS.SWRESET (same as soft_reset_tests):
//   Phase A: clear RESET_STATUS, invalidate the whole ROM window, write
//            RESET_REQ.SWRESET, then sweep the window with independent loads
//            (no barrier at all after the store) until the reset takes the CPU.
//   Phase B: RESET_STATUS.SWRESET == 1; ROM word 0 is correct; a few ROM
//            lines read back without abort; TEST_PASS.
// Pass/fail evidence from the bench (megasoc_rom_boundary_checker):
//   - "warm reset assert #n: k ROM read burst(s) in flight": k > 0 shows the
//     reset really hit an active burst (k == 0 means the timing missed and
//     the run only counts as an ordinary warm reboot);
//   - no "stale beat" error (an R beat after the reset release for an AR
//     accepted before it);
//   - "boot #2 (warm): first ROM_AXI fetch @0x0 ... matches reference".
// Only the CPU-fetch source is covered here. The SDIO-DMA-read source needs
// the VIP bench (SD card model) and is not exercised by this test.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "megasoc_resetctrl.h"
#include <stdio.h>
#include <stdint.h>

#define ROM_WIN_BASE    0x00000000UL
#define ROM_WIN_END     0x00010000UL
#define ROM_FIRST_WORD  0xd2800000UL
#define LINE_BYTES      64UL
#define BLOCK_LINES     8UL
#define POLL_TIMEOUT    500000u

static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }

static inline void barrier_sy(void)
{
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}

static volatile uint32_t g_abort_count = 0;
void abort_handler(void) { g_abort_count++; }

// Invalidate BLOCK_LINES lines starting at base, one barrier at the end.
static inline void civac_block(unsigned long base)
{
  for (unsigned long i = 0; i < BLOCK_LINES; i++)
    __asm__ volatile ("dc civac, %0" : : "r"(base + i * LINE_BYTES) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
}

// Eight independent loads, one per line, so several line fills can be
// outstanding at once. The sum is consumed so the loads are not removed.
static inline uint64_t touch_block(unsigned long base)
{
  uint64_t a, b, c, d, e, f, g, h;
  __asm__ volatile (
    "ldr %0, [%8]\n\t"        "ldr %1, [%8, #64]\n\t"
    "ldr %2, [%8, #128]\n\t"  "ldr %3, [%8, #192]\n\t"
    "ldr %4, [%8, #256]\n\t"  "ldr %5, [%8, #320]\n\t"
    "ldr %6, [%8, #384]\n\t"  "ldr %7, [%8, #448]\n\t"
    : "=&r"(a), "=&r"(b), "=&r"(c), "=&r"(d), "=&r"(e), "=&r"(f), "=&r"(g), "=&r"(h)
    : "r"(base) : "memory");
  return a + b + c + d + e + f + g + h;
}

// Invalidate every line of the 64 KB ROM window. Run once, with a DSB, before
// the reset request, so the sweep after it misses on every line.
static void civac_window(void)
{
  for (unsigned long a = 0; a < ROM_WIN_END; a += LINE_BYTES)
    __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
}

static void phase_a(void)
{
  volatile uint64_t sink = 0;
  uint32_t t = POLL_TIMEOUT;

  printf("[MEGASOC-BOOT-011] Phase-A: clearing RESET_STATUS\n");
  MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;
  barrier_sy();
  while ((MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_ALL_Msk) != 0u && --t) { }
  printf("[MEGASOC-BOOT-011] Phase-A: RESET_STATUS = 0x%08x, starting ROM line-fill traffic and SWRESET\n",
         (unsigned)MEGASOC_RESETCTRL->RESET_STATUS);

  civac_window();
  MEGASOC_RESETCTRL->RESET_REQ = RESET_REQ_SWRESET_Msk;
  // No barrier from here on. Run 2026-11-10 #1 put "civac x8; dsb sy" inside
  // the loop: the DSB also waits for the RESET_REQ store to complete, so the
  // reset landed while the core sat in the DSB with nothing on ROM_AXI
  // (0 bursts in flight). Now the core only issues loads: one pass over the
  // freshly invalidated window is 1024 line fills, far longer than the reset
  // request takes to act. Later passes hit in the cache, which is harmless.
  for (uint32_t pass = 0; pass < 64u; pass++)   // bounded: falls through to FAIL if no reset
    for (unsigned long base = 0; base < ROM_WIN_END; base += BLOCK_LINES * LINE_BYTES)
      sink += touch_block(base);
}

static int phase_b(void)
{
  int errors = 0;
  uint32_t status = MEGASOC_RESETCTRL->RESET_STATUS;
  uint32_t before = g_abort_count;
  uint32_t v;
  uint64_t sum = 0;

  printf("[MEGASOC-BOOT-011] Phase-B: RESET_STATUS after reboot = 0x%08x\n", (unsigned)status);
  if ((status & RESET_STATUS_SWRESET_Msk) == 0u) {
    printf("[MEGASOC-BOOT-011] FAIL Phase-B: RESET_STATUS.SWRESET not set\n");
    errors++;
  }
  MEGASOC_RESETCTRL->RESET_STATUS = RESET_STATUS_ALL_Msk;   // W1C

  __asm__ volatile ("dc civac, %0" : : "r"(ROM_WIN_BASE) : "memory");
  barrier_sy();
  v = rd32(ROM_WIN_BASE);
  if (v != ROM_FIRST_WORD) {
    printf("[MEGASOC-BOOT-011] FAIL Phase-B: ROM word @0x0 = 0x%08x, expected 0x%08x\n",
           (unsigned)v, (unsigned)ROM_FIRST_WORD);
    errors++;
  }

  // Re-read the lines Phase A was hammering; the bench compares the data.
  for (unsigned long base = 0; base < 0x1000UL; base += BLOCK_LINES * LINE_BYTES) {
    civac_block(base);
    sum += touch_block(base);
  }
  barrier_sy();
  if (g_abort_count != before) {
    printf("[MEGASOC-BOOT-011] FAIL Phase-B: %u abort(s) on post-reset ROM reads\n",
           (unsigned)(g_abort_count - before));
    errors++;
  }
  // Kept short: c_print() formats into a 160-byte stack buffer (BUG-22/BUG-32).
  printf("[MEGASOC-BOOT-011] Phase-B: post-reset ROM reads done (sum 0x%016llx)\n",
         (unsigned long long)sum);
  printf("[MEGASOC-BOOT-011] in-flight count, stale-beat check: see MEGASOC_ROM_CHECK lines\n");
  return errors;
}

int main(void)
{
  int errors;

  UartStdOutInit();
  printf("MegaSoC warm reset during ROM burst test (tc_reset_during_rom_burst)\n");
  printf("vplan: MEGASOC-BOOT-011\n");

  if ((MEGASOC_RESETCTRL->RESET_STATUS & RESET_STATUS_SWRESET_Msk) != 0u) {
    errors = phase_b();
    printf("Total number of rom reset during burst errors: %d\n", errors);
    if (errors == 0) { TEST_PASS(); } else { TEST_FAIL(); }
    while (1) { }
  }

  phase_a();
  printf("[MEGASOC-BOOT-011] FAIL: execution continued after SWRESET\n");
  TEST_FAIL();
  while (1) { }
}
