//-----------------------------------------------------------------------------
// MegaSoC LPDDR4 DRAM bring-up and functional test (INC_LPDDR4 builds only)
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Runs on the A53. Brings the LPDDR4 subsystem up with the shared driver
// (software/lib/sw_lib/devices: lpddr4.h / lpddr4.c), then checks DRAM through
// the CPU at 0x8000_0000: T1 block, T2 address walk, T3 sub-word stores,
// T4 timing stress, T5 refresh / power-down / self-refresh, T6 random traffic.
//
// The DRAM window is cacheable, so every write phase is followed by a clean +
// invalidate of the range before the data is read back from DRAM.
//-----------------------------------------------------------------------------

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "system_level_functions.h"
#include "CMSDK.h"
#include "lpddr4.h"
#include <stdio.h>
#include <stdint.h>

// T1: 2 KB block (one DRAM row) written, flushed to DRAM, read back
static int test_block(void)
{
  const uintptr_t off = 0x0;
  const uint64_t base = 0x1122334455667788ULL;
  const unsigned n = 256;
  int errs = 0;

  printf("=== T1: 2 KB block write/read at 0x%08lx ===\n", (unsigned long)(DRAM_BASE + off));
  for (unsigned i = 0; i < n; i++) dram_wr64(off + 8 * i, base + i);
  lpddr4_cache_clean_invalidate(DRAM_BASE + off, 8 * n);
  for (unsigned i = 0; i < n; i++) {
    uint64_t got = dram_rd64(off + 8 * i);
    if (got != base + i) errs += lpddr4_report_mismatch("T1", errs, off + 8 * i, got, base + i);
  }
  printf("  %u words compared, %d mismatch(es)\n", n, errs);
  return errs;
}

// T2: address walk over the 256 MB device; any aliasing between address bits
// (wrong address map) shows up as an overwritten pattern
static uint64_t walk_pattern(uintptr_t off)
{
  return ((uint64_t)(~off & 0xffffffffUL) << 32) | (off & 0xffffffffUL);
}

static int test_address_walk(void)
{
  uintptr_t offs[48];
  int n = 0, errs = 0;

  offs[n++] = 0x0;
  for (int k = 3; k <= 27; k++) offs[n++] = (uintptr_t)1 << k;                      // every address bit
  for (int b = 0; b < 8; b++)   offs[n++] = ((uintptr_t)5 << 14) | ((uintptr_t)b << 11) | 0x40; // all banks
  offs[n++] = 0x0ffffff8;                                                           // last word

  printf("=== T2: address walk (%d addresses) ===\n", n);
  for (int i = 0; i < n; i++) {
    dram_wr64(offs[i], walk_pattern(offs[i]));
    lpddr4_cache_clean_invalidate(DRAM_BASE + offs[i], 8);
  }
  for (int i = 0; i < n; i++) {
    uint64_t got = dram_rd64(offs[i]), exp = walk_pattern(offs[i]);
    if (got != exp) errs += lpddr4_report_mismatch("T2", errs, offs[i], got, exp);
  }
  printf("  %d addresses compared, %d mismatch(es)\n", n, errs);
  return errs;
}

// T3: byte and half-word stores merged into a word, flushed, read back.
// Note: the cache merges sub-word stores, so DRAM sees whole-line writes here;
// strobed (read-modify-write) DRAM writes need a non-cacheable mapping.
static int test_subword(void)
{
  const uintptr_t off = 0x00100000;
  int errs = 0;

  printf("=== T3: sub-word stores ===\n");
  for (unsigned i = 0; i < 16; i++) dram_wr64(off + 8 * i, 0x0123456789abcdefULL);
  for (unsigned i = 0; i < 16; i++) {
    volatile uint8_t  *b = (volatile uint8_t  *)(DRAM_BASE + off + 8 * i);
    volatile uint16_t *h = (volatile uint16_t *)(DRAM_BASE + off + 8 * i);
    b[i & 7] = (uint8_t)(0xA0 + i);
    if (i & 1) h[3] = (uint16_t)(0x5A00 + i);
  }
  lpddr4_cache_clean_invalidate(DRAM_BASE + off, 8 * 16);
  for (unsigned i = 0; i < 16; i++) {
    uint64_t exp = 0x0123456789abcdefULL;
    exp &= ~(0xFFULL << (8 * (i & 7)));
    exp |= (uint64_t)(0xA0 + i) << (8 * (i & 7));
    if (i & 1) exp = (exp & 0x0000FFFFFFFFFFFFULL) | ((uint64_t)(0x5A00 + i) << 48);
    uint64_t got = dram_rd64(off + 8 * i);
    if (got != exp) errs += lpddr4_report_mismatch("T3", errs, off + 8 * i, got, exp);
  }
  printf("  16 words compared, %d mismatch(es)\n", errs);
  return errs;
}

//-----------------------------------------------------------------------------
// T4-T6: ports of the standalone suite. The standalone testbench queues AXI
// transactions directly; here the A53 is the only master, so DRAM traffic is
// cache line fills (reads, 64 B) and write-backs (64 B). To get several
// requests outstanding at once, lines are filled with independent loads and
// written back by one dc civac sweep with a single dsb at the end.
//-----------------------------------------------------------------------------
static void flush_lines(const uintptr_t *offs, int n)
{
  for (int i = 0; i < n; i++)
    __asm__ volatile ("dc civac, %0" : : "r"(DRAM_BASE + offs[i]) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}

// Pull n lines into the cache with independent loads (no load depends on
// another, so the CPU can have several line fills in flight)
static void fill_lines(const uintptr_t *offs, int n)
{
  volatile uint64_t sink = 0;
  for (int i = 0; i < n; i++) sink += dram_rd64(offs[i]);
  (void)sink;
}

static uint64_t t4_data(uintptr_t off) { return 0xb000000000000000ULL | ((uint64_t)off << 8); }

static void line_write(uintptr_t off)
{
  for (int w = 0; w < 8; w++) dram_wr64(off + 8 * w, t4_data(off) + w);
}

static int line_check(const char *tag, uintptr_t off)
{
  int errs = 0;
  for (int w = 0; w < 8; w++) {
    uint64_t got = dram_rd64(off + 8 * w), exp = t4_data(off) + w;
    if (got != exp) errs += lpddr4_report_mismatch(tag, errs, off + 8 * w, got, exp);
  }
  return errs;
}

// Write lines, write them all back in one sweep, fill them back concurrently
// and compare
static int lines_roundtrip(const char *tag, const uintptr_t *offs, int n)
{
  int errs = 0;
  for (int i = 0; i < n; i++) line_write(offs[i]);
  flush_lines(offs, n);
  fill_lines(offs, n);
  for (int i = 0; i < n; i++) errs += line_check(tag, offs[i]);
  flush_lines(offs, n);                          // leave nothing cached for later tests
  return errs;
}

// T4: timing stress (same address pattern as the standalone T4)
//   (a) bank interleave: 16 lines over 8 banks x 2 rows  -> tRRD, tFAW, tRC
//   (b) row thrash: 8 lines alternating 2 rows of bank 6  -> tRP, tRAS, tRC
//   (c) mixed: fills of (a) interleaved with write-allocate fills and
//       write-backs of new rows -> wr2rd, rd2wr
// The memory model's timing checks are the main pass criterion; data is
// compared as well.
#define T4_OFF(row, bank) (((uintptr_t)(row) << 14) | ((uintptr_t)(bank) << 11) | 0x200)

static int test_timing_stress(void)
{
  uintptr_t a[16], b[8], c[8];
  int ea, eb, ec = 0;

  printf("=== T4: timing stress (concurrent line fills / write-backs) ===\n");

  for (int k = 0; k < 16; k++) a[k] = T4_OFF(100 + k / 8, k % 8);
  ea = lines_roundtrip("T4a", a, 16);
  printf("  (a) bank interleave : 16 lines, %d mismatch(es)\n", ea);

  for (int k = 0; k < 8; k++) b[k] = T4_OFF(200 + (k & 1), 6) + 0x40 * (k / 2);
  eb = lines_roundtrip("T4b", b, 8);
  printf("  (b) row thrash      : 8 lines, %d mismatch(es)\n", eb);

  // (c) (a) is in DRAM and not cached; read it while writing new rows
  for (int k = 0; k < 8; k++) c[k] = T4_OFF(300 + k / 4, (k * 3) % 8);
  volatile uint64_t sink = 0;
  for (int k = 0; k < 8; k++) {
    sink += dram_rd64(a[k]);                     // line fill of an (a) line
    line_write(c[k]);                            // write-allocate fill of a (c) line
  }
  (void)sink;
  flush_lines(c, 8);                             // write-backs of (c)
  for (int k = 0; k < 8; k++) ec += line_check("T4c-rd", a[k]);
  flush_lines(a, 8);
  fill_lines(c, 8);
  for (int k = 0; k < 8; k++) ec += line_check("T4c-wr", c[k]);
  flush_lines(c, 8);
  printf("  (c) mixed rd/wr     : 16 lines, %d mismatch(es)\n", ec);

  return ea + eb + ec;
}

// Delays use CMSDK timer 0, free-running down from 0xFFFFFFFF on the
// peripheral clock = SYS_CLK (LPDDR4_TIMER_MHZ ticks per microsecond)
#ifndef LPDDR4_TIMER_MHZ
#define LPDDR4_TIMER_MHZ 100
#endif

static void timer_start(void)
{
  CMSDK_TIMER0->CTRL   = 0;
  CMSDK_TIMER0->RELOAD = 0xFFFFFFFFu;
  CMSDK_TIMER0->VALUE  = 0xFFFFFFFFu;
  CMSDK_TIMER0->CTRL   = CMSDK_TIMER_CTRL_EN_Msk;
}

static uint32_t elapsed_us(void)
{
  return (0xFFFFFFFFu - CMSDK_TIMER0->VALUE) / LPDDR4_TIMER_MHZ;
}

static void wait_us(uint32_t us)
{
  timer_start();
  while (elapsed_us() < us) ;
}


// T5: retention across refresh and self-refresh (standalone T5, without the
// testbench's DFI command counters: refresh timing is left to the memory
// model's checks, self-refresh is seen through STAT.operating_mode).
//   (a) idle 16 us with self-refresh disabled (tREFI 3.9 us -> several REFs)
//   (b) idle 16 us with self-refresh enabled: automatic entry after the
//       PWRTMG idle time (STAT = 3), then the read wakes the controller
//   (c) software-requested self-refresh via PWRCTL.selfref_sw
//   (d) 8 rounds of back-to-back traffic, so refreshes land in the middle of it
// Delays come from the SYS_CLK timer; the testbench prints the SYS_CLK
// frequency (LPDDR4_MON: SYS_CLK period) to set LPDDR4_TIMER_MHZ.
#define T5_OFF(k) (((uintptr_t)(400 + (k) / 8) << 14) | ((uintptr_t)((k) % 8) << 11) | 0x400)

static int t5_verify(const char *tag, const uintptr_t *offs)
{
  int errs = 0;
  fill_lines(offs, 16);
  for (int k = 0; k < 16; k++) errs += line_check(tag, offs[k]);
  flush_lines(offs, 16);
  return errs;
}

static int test_retention(void)
{
  uintptr_t t[16];
  int errs = 0, e;
  unsigned polls;

  printf("=== T5: retention (refresh, self-refresh) ===\n");
  for (int k = 0; k < 16; k++) { t[k] = T5_OFF(k); line_write(t[k]); }
  flush_lines(t, 16);

  // (a) refresh only
  lpddr4_ctrl_bit_clear(0x030, 0);                      // PWRCTL.selfref_en = 0
  wait_us(16);
  e = t5_verify("T5a", t);
  printf("  (a) idle 16us, SR off : mode %u, %d error(s)\n", lpddr4_op_mode(), e);
  errs += e;

  // (b) automatic self-refresh on idle
  lpddr4_ctrl_bit_set(0x030, 0);                        // PWRCTL.selfref_en = 1
  wait_us(16);
  unsigned mode_idle = lpddr4_op_mode();
  volatile uint64_t wake = dram_rd64(t[0]);      // one access wakes the controller;
  (void)wake;                                    // check before it can idle again
  unsigned mode_woken = lpddr4_op_mode();
  __asm__ volatile ("dc civac, %0\n dsb sy" : : "r"(DRAM_BASE + t[0]) : "memory");
  e = t5_verify("T5b", t);
  if (mode_idle != 3)  { printf("  ERROR [T5b] STAT.operating_mode = %u after 16us idle (expected 3)\n", mode_idle); e++; }
  if (mode_woken == 3) { printf("  ERROR [T5b] still in self-refresh after access\n"); e++; }
  printf("  (b) idle 16us, SR on  : mode %u, after access %u, %d error(s)\n", mode_idle, mode_woken, e);
  errs += e;

  // (c) software-requested self-refresh
  e = 0;
  lpddr4_ctrl_bit_clear(0x030, 0);                      // no automatic SR while testing SW SR
  lpddr4_ctrl_bit_set(0x030, 5);                        // PWRCTL.selfref_sw = 1
  polls = 0;
  while (lpddr4_op_mode() != 3 && polls++ < LPDDR4_POLL_LIMIT) ;
  unsigned mode_sw = lpddr4_op_mode();
  wait_us(5);
  lpddr4_ctrl_bit_clear(0x030, 5);                      // PWRCTL.selfref_sw = 0
  polls = 0;
  while (lpddr4_op_mode() == 3 && polls++ < LPDDR4_POLL_LIMIT) ;
  unsigned mode_after = lpddr4_op_mode();
  e += t5_verify("T5c", t);
  if (mode_sw != 3)    { printf("  ERROR [T5c] did not enter self-refresh (mode %u)\n", mode_sw); e++; }
  if (mode_after == 3) { printf("  ERROR [T5c] did not leave self-refresh\n"); e++; }
  printf("  (c) software SR       : entered mode %u, left to mode %u, %d error(s)\n", mode_sw, mode_after, e);
  errs += e;

  // (d) traffic across several refresh intervals (tREFI 3.9 us; one loop is
  // 16 line fills + write-backs, several microseconds)
  e = 0;
  timer_start();
  for (int loop = 0; loop < 8; loop++) e += t5_verify("T5d", t);
  printf("  (d) traffic 8 loops   : %u us (at %d timer ticks/us), %d error(s)\n",
         (unsigned)elapsed_us(), LPDDR4_TIMER_MHZ, e);
  errs += e;

  lpddr4_ctrl_bit_set(0x030, 0);                        // restore PWRCTL.selfref_en = 1
  return errs;
}

// T6: random traffic checked against a shadow copy in SRAM (standalone T6).
// 64 slots of one cache line at random line-aligned offsets over the 256 MB
// device. Each batch does 8 operations on 8 distinct slots -- a check of
// random words, or 8/16/32/64-bit stores at random offsets -- then cleans and
// invalidates those lines, so every operation is a DRAM line fill and every
// store a DRAM write-back. (The cache merges narrow stores, so DRAM sees
// whole-line writes; strobed DRAM writes need a non-cacheable mapping.)
#define T6_SLOTS   64
// Batches of random operations: 20 by default, half the previous 40 (saves
// about 1 ms simulated, ~10 min CPU). Override: make ... LPDDR4_T6_BATCHES=<n>
#ifndef LPDDR4_T6_BATCHES
#define LPDDR4_T6_BATCHES 20
#endif
#define T6_BATCHES LPDDR4_T6_BATCHES
#define T6_OPS      8
#ifndef LPDDR4_SEED
#define LPDDR4_SEED 1
#endif

static uint64_t t6_rng;
static uint64_t t6_rand(void)                    // 64-bit LCG, high bits are good enough
{
  t6_rng = t6_rng * 6364136223846793005ULL + 1442695040888963407ULL;
  return t6_rng >> 17;
}

static int test_random(void)
{
  static uintptr_t slot_off[T6_SLOTS];
  static uint64_t  shadow[T6_SLOTS][8];
  int errs = 0, nchecks = 0, nstores = 0;
  t6_rng = LPDDR4_SEED;

  printf("=== T6: random traffic (seed %d, %d batches x %d ops) ===\n", LPDDR4_SEED, T6_BATCHES, T6_OPS);

  for (int s = 0; s < T6_SLOTS; s++) {
    uintptr_t o; int dup;
    do {
      o = (uintptr_t)(t6_rand() % (256UL << 20)) & ~(uintptr_t)(LPDDR4_CACHE_LINE - 1);
      dup = 0;
      for (int j = 0; j < s; j++) if (slot_off[j] == o) dup = 1;
    } while (dup);
    slot_off[s] = o;
    uint64_t base = t6_rand();
    for (int w = 0; w < 8; w++) { shadow[s][w] = base + w; dram_wr64(o + 8 * w, base + w); }
  }
  flush_lines(slot_off, T6_SLOTS);

  for (int batch = 0; batch < T6_BATCHES; batch++) {
    uintptr_t touched[T6_OPS];
    int slot[T6_OPS];
    for (int i = 0; i < T6_OPS; i++) {
      int dup;
      do {
        slot[i] = t6_rand() % T6_SLOTS;
        dup = 0;
        for (int j = 0; j < i; j++) if (slot[j] == slot[i]) dup = 1;
      } while (dup);
      touched[i] = slot_off[slot[i]];
    }
    for (int i = 0; i < T6_OPS; i++) {
      int s = slot[i];
      uintptr_t o = slot_off[s];
      unsigned w = t6_rand() % 8;
      if (t6_rand() & 1) {                       // check one word
        uint64_t got = dram_rd64(o + 8 * w);
        nchecks++;
        if (got != shadow[s][w]) errs += lpddr4_report_mismatch("T6", errs, o + 8 * w, got, shadow[s][w]);
      } else {                                   // store 1, 2, 4 or 8 bytes
        unsigned sz = 1u << (t6_rand() % 4);
        unsigned b  = (t6_rand() % (8 / sz)) * sz;
        uint64_t v  = t6_rand();
        uint64_t m  = (sz == 8) ? ~0ULL : (((1ULL << (8 * sz)) - 1) << (8 * b));
        uintptr_t a = DRAM_BASE + o + 8 * w + b;
        switch (sz) {
          case 1:  *(volatile uint8_t  *)a = (uint8_t)v;  break;
          case 2:  *(volatile uint16_t *)a = (uint16_t)v; break;
          case 4:  *(volatile uint32_t *)a = (uint32_t)v; break;
          default: *(volatile uint64_t *)a = v;           break;
        }
        shadow[s][w] = (shadow[s][w] & ~m) | ((v << (8 * b)) & m);
        nstores++;
      }
    }
    flush_lines(touched, T6_OPS);
  }

  // Final sweep: every slot must match the shadow
  for (int s = 0; s < T6_SLOTS; s++)
    for (int w = 0; w < 8; w++) {
      uint64_t got = dram_rd64(slot_off[s] + 8 * w);
      if (got != shadow[s][w]) errs += lpddr4_report_mismatch("T6-final", errs, slot_off[s] + 8 * w, got, shadow[s][w]);
    }
  flush_lines(slot_off, T6_SLOTS);
  printf("  %d checks, %d stores, final sweep of %d lines, %d mismatch(es)\n",
         nchecks, nstores, T6_SLOTS, errs);
  return errs;
}

int main(void)
{
  printf("MegaSoC LPDDR4 DRAM test (%s PHY init)\n", LPDDR4_SKIP_TRAINING ? "skip-training" : "trained");

  if (lpddr4_init() != 0) {
    printf("ERROR: LPDDR4 initialisation failed (%d error(s))\n", lpddr4_error_count());
    TEST_FAIL();
    return 1;
  }

  int t1 = test_block();
  int t2 = test_address_walk();
  int t3 = test_subword();
  int t4 = test_timing_stress();
  int t5 = test_retention();
  int t6 = test_random();

  printf("=== LPDDR4 test summary ===\n");
  printf("  T1 block         : %s (%d errors)\n", t1 ? "FAIL" : "PASS", t1);
  printf("  T2 address walk  : %s (%d errors)\n", t2 ? "FAIL" : "PASS", t2);
  printf("  T3 sub-word      : %s (%d errors)\n", t3 ? "FAIL" : "PASS", t3);
  printf("  T4 timing stress : %s (%d errors)\n", t4 ? "FAIL" : "PASS", t4);
  printf("  T5 retention     : %s (%d errors)\n", t5 ? "FAIL" : "PASS", t5);
  printf("  T6 random        : %s (%d errors)\n", t6 ? "FAIL" : "PASS", t6);

  if (t1 + t2 + t3 + t4 + t5 + t6 + lpddr4_error_count() == 0) {
    TEST_PASS();
  } else {
    TEST_FAIL();
  }
  return 0;
}
