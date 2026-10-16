//-----------------------------------------------------------------------------
// MegaSoC LPDDR4 low-power (Q-channel) test (INC_LPDDR4 builds only)
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Checks the DRAM power domain end to end: the DRAM PPU's request reaches both
// LPDDR4 low-power interfaces through the Q-channel adapters, the controller
// stops its AXI port and puts the DRAM into self-refresh, any DRAM access
// wakes the domain again (hardware wake-up), and nothing is reset or lost.
//
//   L1  PPU starts ON; DRAM brought up; data pattern written
//   L2  enter low power: PPU in FUNC_RET, controller STAT in self-refresh;
//       stay a while; exit: PPU ON, controller out of self-refresh; data
//       intact
//   L3  four more enter/exit cycles, data checked after each
//   L4  normal DRAM traffic after the cycles (T1-style block)
//   L5  wake on access: with low power on and the domain asleep, read DRAM
//       without leaving low power first -- the read must complete with the
//       right data (the PPU wakes the domain), and the domain must go back
//       to sleep by itself once idle
//   L6  traffic with low power on: writes and reads with idle gaps of
//       varying length, so the domain keeps falling asleep and waking,
//       including requests racing new accesses; every access must complete
//       with the right data
//
// The controller's own automatic self-refresh and power-down are switched off
// for the test, so self-refresh is only ever entered through the low-power
// request.
//-----------------------------------------------------------------------------

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "system_level_functions.h"
#include "CMSDK.h"
#include "lpddr4.h"
#include <stdio.h>
#include <stdint.h>

#define LINES     16
#define LP_US     20        // time spent in low power per L2/L3 cycle
#define L6_ROUNDS 24

static uintptr_t line_off(int k) { return ((uintptr_t)(500 + k / 8) << 14) | ((uintptr_t)(k % 8) << 11) | 0x100; }
static uint64_t  pattern(uintptr_t off, int w, unsigned round) { return 0xd0000000000000ULL ^ ((uint64_t)off << 8) ^ ((uint64_t)round << 48) ^ (uint64_t)w; }

static void flush_line(int k) { lpddr4_cache_clean_invalidate(DRAM_BASE + line_off(k), LPDDR4_CACHE_LINE); }

static void write_line(int k, unsigned round)
{
  for (int w = 0; w < 8; w++) dram_wr64(line_off(k) + 8 * w, pattern(line_off(k), w, round));
  flush_line(k);
}

static int check_line(const char *tag, int k, unsigned round)
{
  int errs = 0;
  for (int w = 0; w < 8; w++) {
    uint64_t got = dram_rd64(line_off(k) + 8 * w), exp = pattern(line_off(k), w, round);
    if (got != exp) errs += lpddr4_report_mismatch(tag, errs, line_off(k) + 8 * w, got, exp);
  }
  flush_line(k);
  return errs;
}

static void write_lines(unsigned round) { for (int k = 0; k < LINES; k++) write_line(k, round); }
static int  check_lines(const char *tag, unsigned round)
{
  int errs = 0;
  for (int k = 0; k < LINES; k++) errs += check_line(tag, k, round);
  return errs;
}

// CMSDK timer 0 on SYS_CLK (100 MHz): free-running down-counter
static void timer_start(void)
{
  CMSDK_TIMER0->CTRL = 0; CMSDK_TIMER0->RELOAD = 0xFFFFFFFFu; CMSDK_TIMER0->VALUE = 0xFFFFFFFFu;
  CMSDK_TIMER0->CTRL = CMSDK_TIMER_CTRL_EN_Msk;
}
static uint32_t timer_ns(void) { return (0xFFFFFFFFu - CMSDK_TIMER0->VALUE) * 10u; }
static void wait_us(uint32_t us) { timer_start(); while (timer_ns() < us * 1000u) ; }

// Time one 64-bit read of a line that is not in the cache
static uint32_t timed_read(int k, uint64_t *v)
{
  timer_start();
  *v = dram_rd64(line_off(k));
  __asm__ volatile ("dsb sy" ::: "memory");
  return timer_ns();
}

// One enter/stay/exit cycle; returns the number of errors
static int lp_cycle(const char *tag)
{
  int e = 0;
  int r = lpddr4_lowpower_enter();
  unsigned ppu_lp = lpddr4_ppu_state(), mode_lp = lpddr4_op_mode();
  if (r != LPDDR4_LP_ACCEPTED)  { printf("  ERROR [%s] low-power entry timed out\n", tag); e++; }
  if (ppu_lp != LPDDR4_PPU_FUNC_RET) { printf("  ERROR [%s] PPU state %u in low power (expected %u)\n", tag, ppu_lp, LPDDR4_PPU_FUNC_RET); e++; }
  if (mode_lp != LPDDR4_MODE_SELF_REFRESH) { printf("  ERROR [%s] controller mode %u in low power (expected self-refresh)\n", tag, mode_lp); e++; }
  if (r == LPDDR4_LP_ACCEPTED) wait_us(LP_US);
  int x = lpddr4_lowpower_exit();
  unsigned ppu_on = lpddr4_ppu_state(), mode_on = lpddr4_op_mode();
  if (x != 0)                  { printf("  ERROR [%s] exit from low power timed out\n", tag); e++; }
  if (ppu_on != LPDDR4_PPU_ON) { printf("  ERROR [%s] PPU state %u after exit (expected %u)\n", tag, ppu_on, LPDDR4_PPU_ON); e++; }
  if (mode_on == LPDDR4_MODE_SELF_REFRESH) { printf("  ERROR [%s] controller still in self-refresh after exit\n", tag); e++; }
  printf("  [%s] enter: %s, PPU %u, mode %u | exit: PPU %u, mode %u\n", tag,
         r == LPDDR4_LP_ACCEPTED ? "accepted" : "TIMEOUT", ppu_lp, mode_lp, ppu_on, mode_on);
  return e;
}

// L5: an access while the domain is asleep wakes it
static int test_wake_on_access(void)
{
  int e = 0;
  uint64_t v;
  write_lines(5);

  uint32_t t_on = timed_read(0, &v);                 // reference: domain ON
  flush_line(0);
  if (v != pattern(line_off(0), 0, 5)) { printf("  ERROR [L5] reference read wrong\n"); e++; }

  if (lpddr4_lowpower_enter() != LPDDR4_LP_ACCEPTED) { printf("  ERROR [L5] did not enter low power\n"); return e + 1; }
  unsigned mode_sleep = lpddr4_op_mode();
  (void)lpddr4_ppu_events();                          // clear the entry event

  uint32_t t_lp = timed_read(1, &v);                 // access while asleep
  unsigned ev = lpddr4_ppu_events();
  if (v != pattern(line_off(1), 0, 5)) { printf("  ERROR [L5] read while asleep returned 0x%08x%08x\n", (unsigned)(v >> 32), (unsigned)v); e++; }
  if (!(ev & LPDDR4_PPU_EV_ACCEPT))   { printf("  ERROR [L5] no PPU transition seen for the access (events 0x%x)\n", ev); e++; }
  if (mode_sleep != LPDDR4_MODE_SELF_REFRESH) { printf("  ERROR [L5] controller mode %u before the access (expected self-refresh)\n", mode_sleep); e++; }

  e += check_lines("L5-data", 5);                    // many accesses, each may wake the domain

  // back to sleep on its own once idle
  int polls = 0;
  while (lpddr4_ppu_state() != LPDDR4_PPU_FUNC_RET && polls++ < LPDDR4_POLL_LIMIT) ;
  unsigned ppu_idle = lpddr4_ppu_state(), mode_idle = lpddr4_op_mode();
  if (ppu_idle != LPDDR4_PPU_FUNC_RET) { printf("  ERROR [L5] domain did not go back to sleep when idle (PPU %u)\n", ppu_idle); e++; }
  if (mode_idle != LPDDR4_MODE_SELF_REFRESH) { printf("  ERROR [L5] controller mode %u when idle again (expected self-refresh)\n", mode_idle); e++; }

  if (lpddr4_lowpower_exit() != 0) e++;
  printf("  read latency: %u ns awake, %u ns asleep (wake-up included); events 0x%x; idle again: PPU %u, mode %u\n",
         (unsigned)t_on, (unsigned)t_lp, ev, ppu_idle, mode_idle);
  return e;
}

// L6: traffic with low power on, idle gaps of varying length
static int test_traffic_with_lowpower(void)
{
  int e = 0, accepts = 0, denies = 0;
  if (lpddr4_lowpower_enter() != LPDDR4_LP_ACCEPTED) { printf("  ERROR [L6] did not enter low power\n"); return 1; }
  (void)lpddr4_ppu_events();
  for (unsigned round = 0; round < L6_ROUNDS; round++) {
    int k = round % LINES;
    write_line(k, 60 + round);                         // store (line fill) + write-back
    if (round % 3) wait_us(round % 5);                 // gaps of 0..4 us: sometimes asleep, sometimes not
    e += check_line("L6", k, 60 + round);
    unsigned ev = lpddr4_ppu_events();
    accepts += !!(ev & LPDDR4_PPU_EV_ACCEPT);
    denies  += !!(ev & LPDDR4_PPU_EV_DENY);
  }
  if (lpddr4_lowpower_exit() != 0) e++;
  printf("  %d rounds: %d with completed transitions, %d with a denied transition, %d data error(s)\n",
         L6_ROUNDS, accepts, denies, e);
  if (accepts == 0) { printf("  ERROR [L6] the domain never changed state during the traffic\n"); e++; }
  return e;
}

int main(void)
{
  int l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0;

  printf("MegaSoC LPDDR4 low-power test (%s PHY init)\n", LPDDR4_SKIP_TRAINING ? "skip-training" : "trained");

  // L1
  printf("=== L1: DRAM PPU state at boot ===\n");
  printf("  PPU state %u\n", lpddr4_ppu_state());
  if (lpddr4_ppu_state() != LPDDR4_PPU_ON) { printf("  ERROR [L1] DRAM PPU not ON at boot\n"); l1++; }
  if (lpddr4_init() != 0) {
    printf("ERROR: LPDDR4 initialisation failed (%d error(s))\n", lpddr4_error_count());
    TEST_FAIL();
    return 1;
  }
  // The controller's own idle power saving would put the DRAM into
  // self-refresh / power-down by itself; switch it off so that self-refresh
  // seen here can only come from the low-power request.
  lpddr4_ctrl_bit_clear(0x030, 0);                   // PWRCTL.selfref_en = 0
  lpddr4_ctrl_bit_clear(0x030, 1);                   // PWRCTL.powerdown_en = 0
  write_lines(0);

  // L2
  printf("=== L2: enter / exit functional retention ===\n");
  l2 += lp_cycle("L2");
  l2 += check_lines("L2-data", 0);
  printf("  %d error(s)\n", l2);

  // L3
  printf("=== L3: repeated enter / exit with new data each time ===\n");
  for (unsigned round = 1; round <= 4; round++) {
    char tag[8] = "L3-c0";
    tag[4] = (char)('0' + round);
    write_lines(round);
    l3 += lp_cycle(tag);
    l3 += check_lines(tag, round);
  }
  printf("  %d error(s)\n", l3);

  // L4
  printf("=== L4: normal traffic after low power ===\n");
  for (int i = 0; i < 256; i++) dram_wr64(0x00200000 + 8 * i, 0x5a5a000000000000ULL + i);
  lpddr4_cache_clean_invalidate(DRAM_BASE + 0x00200000, 8 * 256);
  for (int i = 0; i < 256; i++) {
    uint64_t got = dram_rd64(0x00200000 + 8 * i), exp = 0x5a5a000000000000ULL + i;
    if (got != exp) l4 += lpddr4_report_mismatch("L4", l4, 0x00200000 + 8 * i, got, exp);
  }
  printf("  256 words compared, %d mismatch(es)\n", l4);

  // L5
  printf("=== L5: wake on access ===\n");
  l5 = test_wake_on_access();
  printf("  %d error(s)\n", l5);

  // L6
  printf("=== L6: traffic with low power on ===\n");
  l6 = test_traffic_with_lowpower();
  printf("  %d error(s)\n", l6);

  printf("=== LPDDR4 low-power test summary ===\n");
  printf("  L1 boot state       : %s (%d errors)\n", l1 ? "FAIL" : "PASS", l1);
  printf("  L2 enter / exit     : %s (%d errors)\n", l2 ? "FAIL" : "PASS", l2);
  printf("  L3 repeated cycles  : %s (%d errors)\n", l3 ? "FAIL" : "PASS", l3);
  printf("  L4 traffic after    : %s (%d errors)\n", l4 ? "FAIL" : "PASS", l4);
  printf("  L5 wake on access   : %s (%d errors)\n", l5 ? "FAIL" : "PASS", l5);
  printf("  L6 traffic with LP  : %s (%d errors)\n", l6 ? "FAIL" : "PASS", l6);

  if (l1 + l2 + l3 + l4 + l5 + l6 + lpddr4_error_count() == 0) {
    TEST_PASS();
  } else {
    TEST_FAIL();
  }
  return 0;
}
