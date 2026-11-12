//==============================================================================
// rom_window_bounds_tests.c — MegaSoC ROM window vs bootrom image size
//                             (tc_rom_window_bounds)
//
// Written by test-agent from verif/spec/megasoc_spec.md (2026-08-03) ONLY.
// Black-box SoC integration checks: software-visible bus behavior at the
// CPU_AXI -> NIC400 -> ROM_AXI boundary. No expectations based on
// Arm-internal implementation details, and no reads/dependency on
// Arm/vendor RTL (bootrom.v is project-owned, but its byte-for-byte
// generated content beyond the documented first word is NOT assumed here —
// only the address decode/reachability contract is checked).
//
// Targeted vplan item (verif/vplan/megasoc_vplan.yaml):
//
//   MEGASOC-BOOT-005 bootrom_window_vs_image_size (spec §10 S7, priority P2)
//     - Bootrom image space is 128 KB (bootrom.v W_ADDR[13:0] x 64b) but the
//       NIC ROM window on the CPU view is only 64 KB (0x0000-0xFFFF, spec §2
//       row ROM). Required vehicle per vplan `cover`: "explicit check in
//       tc_rom_window_bounds (read 0x0000, 0xFFF8; compare vs image)".
//     - Hard pass/fail requirements (per vplan "characterize" language —
//       the whole-window-reachable claim is a hard requirement; the *content*
//       observed at the top of the window is characterization, not a fixed
//       expectation, because nothing in project-owned sources documents what
//       the boot image actually contains at word offset 0x1FFF):
//         1. Word read at 0x0000 returns the documented preloaded bootrom
//            first word 0xd2800000 (spec §6.1 step 3, same evidence as
//            MEGASOC-BOOT-001/ADDR-001) — proves the bottom of the window is
//            fetchable through NIC400->ROM_AXI with correct data.
//         2. A 64-bit read at 0xFFF8 (last legal doubleword of the 64 KB
//            window, 8 bytes from 0x10000) completes without a synchronous
//            external abort and returns a STABLE value across repeated
//            reads and across byte-lane vs double-word reassembly — proves
//            the *entire* 64 KB window (not just the bottom) is legally
//            reachable and returns real (non-X/non-glitching) data, without
//            assuming particular image content there.
//         3. The observed value at 0xFFF8 is printed verbatim so a human/
//            downstream agent can cross-check it against the generated
//            bootrom.v image content if/when that becomes available — this
//            is the "characterize what a read at the top of the window
//            returns" clause (suspected issue S7).
//     - Out of scope for this item (explicitly NOT checked here): whether
//       the upper 64 KB of the 128 KB bootrom image (word offsets
//       0x2000-0x3FFF, i.e. CPU addresses that would be 0x10000-0x1FFFF if
//       they existed) is reachable at all — per spec §10 S7 that address
//       range is outside the NIC400 ROM window on the CPU view and is
//       covered by MEGASOC-ADDR-015 (window_boundary_addresses,
//       one-past-end/hole behavior), not this item.
//
// Self-checking: every check compares against a spec-derived expectation and
// prints the vplan id in the failure message, e.g.
//   [MEGASOC-BOOT-005] expected 0xd2800000 got 0x00000000
// Unambiguous end-of-test result via TEST_PASS()/TEST_FAIL().
//
// Config knobs: directed test, no randomization. Build-time verbosity via
// `make ... TEST_VERBOSITY=1`.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include <stdio.h>
#include <stdint.h>

// --------------------------------------------------------------------------
// Spec-derived expectations (megasoc_spec.md §2 row ROM, §6.1 step 3, §10 S7)
// --------------------------------------------------------------------------
#define ROM_WIN_BASE      0x00000000UL  // §2 row ROM: CPU-view window base
#define ROM_WIN_LAST      0x0000FFFFUL  // §2 row ROM: CPU-view window top (64 KB)
#define ROM_FIRST_WORD    0xd2800000UL  // §6.1 step 3: A64 'mov x0,#0'
#define ROM_TOP_DWORD     0x0000FFF8UL  // last legal 64-bit location in window
#define ROM_IMAGE_BYTES   (128UL * 1024UL)  // §10 S7: W_ADDR[13:0] x 64b = 128 KB
#define ROM_WINDOW_BYTES  (64UL * 1024UL)   // §10 S7 / §2: NIC ROM window = 64 KB

// --------------------------------------------------------------------------
// Access helpers (same pattern as addr_decode_tests.c)
// --------------------------------------------------------------------------
static inline uint64_t rd64(unsigned long a)
{ uint64_t v; __asm__ volatile ("ldr %0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline uint8_t rd8(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldrb %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return (uint8_t)v; }

// Clean+invalidate the cache line holding 'a' to the point of coherency so
// each read genuinely crosses the NIC400 boundary (no-op if uncached; DC
// CIVAC is architecturally valid either way). Same pattern as
// addr_decode_tests.c cache_civac().
static inline void cache_civac(unsigned long a)
{
  __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}

static int check_eq(const char *vplan_id, const char *what,
                    unsigned long long expected, unsigned long long got)
{
  if (expected != got) {
    printf("[%s] %s: expected 0x%llx got 0x%llx\n",
           vplan_id, what, expected, got);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] %s: OK (0x%llx)\n", vplan_id, what, got);
#endif
  return 0;
}

// --------------------------------------------------------------------------
// MEGASOC-BOOT-005 sub-check 1: bottom of the ROM window (0x0000) is
// fetchable through the window with the documented preloaded content.
// --------------------------------------------------------------------------
static int test_rom_window_bottom(void)
{
  int errors = 0;
  printf("Test: MEGASOC-BOOT-005 rom_window bottom (0x00000000)\n");

  cache_civac(ROM_WIN_BASE);
  errors += check_eq("MEGASOC-BOOT-005", "ROM word @0x00000000",
                     ROM_FIRST_WORD, rd32(ROM_WIN_BASE));

  printf("[MEGASOC-BOOT-005] ROM window bottom read completed without abort (OKAY)\n");
  return errors;
}

// --------------------------------------------------------------------------
// MEGASOC-BOOT-005 sub-check 2: top of the ROM window (0xFFF8, the last
// legal 64-bit location before the 64 KB boundary at 0x10000) is legally
// reachable and returns stable, self-consistent data. This is a hard
// pass/fail requirement on REACHABILITY and DATA STABILITY (the "entire
// 64 KB window is legally readable through the window, no bus error" claim
// from the task), NOT on any particular expected content (the actual boot
// image content at this offset is not documented in project-owned sources).
// --------------------------------------------------------------------------
static int test_rom_window_top(void)
{
  int errors = 0;
  uint64_t dw1, dw2;
  uint32_t reasm;
  printf("Test: MEGASOC-BOOT-005 rom_window top (0x%08lx)\n",
         (unsigned long)ROM_TOP_DWORD);

  // Address-range precondition: 0xFFF8 must actually be the last aligned
  // 64-bit location strictly inside the documented 64 KB window
  // (ROM_WIN_BASE..ROM_WIN_LAST), i.e. 0xFFF8+7 == 0xFFFF == ROM_WIN_LAST.
  if ((ROM_TOP_DWORD + 7UL) != ROM_WIN_LAST) {
    printf("[MEGASOC-BOOT-005] ROM_TOP_DWORD+7 (0x%08lx) != ROM_WIN_LAST (0x%08lx)\n",
           (unsigned long)(ROM_TOP_DWORD + 7UL), (unsigned long)ROM_WIN_LAST);
    errors++;
  }

  // First 64-bit read: completing this load without a synchronous external
  // abort IS the software-visible OKAY response (no CPU_AXI abort vector is
  // installed by this test, so an aborting access would not return here at
  // all — reaching the checks below already proves reachability).
  cache_civac(ROM_TOP_DWORD);
  dw1 = rd64(ROM_TOP_DWORD);

  // Second independent 64-bit read of the same location: must be stable.
  // An unstable/X-propagated read (e.g. an unbacked or mis-decoded region)
  // could plausibly return different bit patterns on successive samples
  // even without an outright bus abort; two-sample stability is the
  // integration-visible proxy for "real backing data, not X/Z" available
  // from bare-metal software (no waveform/testbench visibility here).
  cache_civac(ROM_TOP_DWORD);
  dw2 = rd64(ROM_TOP_DWORD);
  errors += check_eq("MEGASOC-BOOT-005", "ROM dword @0xFFF8 stable across reads",
                     dw1, dw2);

  // Byte-lane reassembly of the low 32 bits must match the word read at the
  // same address: proves narrow (byte) and wide (dword) CPU_AXI accesses at
  // the very top of the window decode to the same ROM location/data path,
  // same evidence pattern as ADDR-001's byte-lane reassembly check.
  cache_civac(ROM_TOP_DWORD);
  reasm = (uint32_t)rd8(ROM_TOP_DWORD)
        | ((uint32_t)rd8(ROM_TOP_DWORD + 1) << 8)
        | ((uint32_t)rd8(ROM_TOP_DWORD + 2) << 16)
        | ((uint32_t)rd8(ROM_TOP_DWORD + 3) << 24);
  errors += check_eq("MEGASOC-BOOT-005", "ROM bytes @0xFFF8..0xFFFB reassembled vs dword low word",
                     (uint32_t)(dw1 & 0xFFFFFFFFUL), reasm);

  // Characterization print (suspected issue S7): record exactly what the
  // top of the CPU-visible 64 KB window returns. This is NOT compared
  // against a fixed expected value — no project-owned source documents the
  // boot image's actual content at this offset — but is printed verbatim so
  // it can be cross-checked against the generated bootrom.v image content
  // if/when that becomes available.
  printf("[MEGASOC-BOOT-005] S7 characterization: ROM dword @0x%08lx = 0x%016llx"
         " (window=%luKB, documented image=%luKB, upper %luKB of image is"
         " outside this CPU-visible window per spec S7)\n",
         (unsigned long)ROM_TOP_DWORD, (unsigned long long)dw1,
         (unsigned long)(ROM_WINDOW_BYTES / 1024UL),
         (unsigned long)(ROM_IMAGE_BYTES / 1024UL),
         (unsigned long)((ROM_IMAGE_BYTES - ROM_WINDOW_BYTES) / 1024UL));

  printf("[MEGASOC-BOOT-005] ROM window top read completed without abort (OKAY)\n");
  return errors;
}

// --------------------------------------------------------------------------
int main(void)
{
  int errors = 0;

  UartStdOutInit();

  printf("MegaSoC ROM window bounds test (tc_rom_window_bounds)\n");
  printf("vplan: MEGASOC-BOOT-005\n");

  errors += test_rom_window_bottom();
  errors += test_rom_window_top();

  printf("Total number of rom window bounds errors: %d\n", errors);

  if (errors == 0) {
    TEST_PASS();
  } else {
    TEST_FAIL();
  }

  return 0;
}
