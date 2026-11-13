//==============================================================================
// adp_target_matrix_tests.c -- MEGASOC-CONN-003: ADP manager reaches all 11
//                              NIC-400 targets (tc_adp_target_matrix)
//
// Written 2026-11-12 for the NIC-400 8-path close-out, from
// verif/spec/megasoc_spec.md section 2.2 (row ADP: ROM, FLASH, RAM,
// PERIPHERAL, DRAM 0x8000_0000-0xFFFF_FFFF, FLASH_CTRL, DEBUG, GIC, PCK_CTRL,
// SDIO_S, EXP_M via 32-bit AHB).
//
// The ADP debug controller (an AHB manager on the NIC-400) runs adp.cmd from
// this directory (flows/makefile.simulate copies it to the sim directory).
// Handshake and command style are copied from rom_adp_write_tests (see its
// header for why polls are many short 32-bit "p" commands):
//   MBOX_DONE  0x0080_F004  CPU writes 0, ADP writes ADP_DONE at the end
//   MBOX_READY 0x0080_F000  CPU writes CPU_READY after DONE=0
//
// What ADP does, one access per target:
//   ROM        0x0000_0000 read      expect 0xD280_0000 (reset vector)
//   FLASH      0x0040_0000 read      expect OKAY
//   RAM        0x0080_F040 write+read  0xA5A5_0080 (CPU checks it too)
//   FLASH_CTRL 0x0100_0000 read      expect OKAY
//   SDIO_S     0x0101_0000 read      expect OKAY
//   GIC        0x0110_1008 read      GICD_IIDR, expect 0x0200_143B
//   PCK_CTRL   0x0120_0FB0 read      ROM_PPU PPU_IDR0, expect 0x1C33_C301
//   DEBUG      0x0130_0FE0 read      expect OKAY
//   PERIPHERAL 0x4000_0FE0 read      TIMER0 PID0, expect 0x0000_0022
//   EXP_M      0x6000_0000 read      expect ERROR (absent target, DECERR)
//   DRAM       0x8000_0100 write+read  0xA5A5_8000 (CPU checks it too)
//   hole       0x0001_0000 read      expect ERROR (positive control for "!")
//
// Checks:
//   CPU (this program, hard pass/fail): ADP finished; the RAM and DRAM words
//     ADP wrote hold the expected values.
//   Post-sim (verif/tests/megasoc/check_adp_target_matrix.py, hard pass/fail):
//     every ADP echo line in logs/extadp_out.log ("R 0x..." = OKAY,
//     "R!0x..." = ERROR) matches the table above.
//   Bench: a_bounded_response_adp_ahb (MEGASOC-ILL-007) sees every access.
//
// Run: make run_vcs TESTNAME=adp_target_matrix_tests VCS_SIM_OPTION="+vcs+flush+log"
//      python3 verif/tests/megasoc/check_adp_target_matrix.py \
//          simulate/sim/adp_target_matrix_tests/logs/extadp_out.log
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include <stdio.h>
#include <stdint.h>

#define MBOX_READY   0x0080F000UL
#define MBOX_DONE    0x0080F004UL
#define CPU_READY    0x600DB007u
#define ADP_DONE     0xADF1D0E5u
#define DONE_TIMEOUT 20000000u

#define RAM_ADDR     0x0080F040UL
#define RAM_EXPECT   0xA5A50080u
#define DRAM_ADDR    0x80000100UL
#define DRAM_EXPECT  0xA5A58000u

static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline void wr32(unsigned long a, uint32_t v)
{ __asm__ volatile ("str %w0, [%1]" : : "r"(v), "r"(a) : "memory"); }
static inline void dc_civac(unsigned long a)
{ __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory"); }
static inline void dc_cvac(unsigned long a)
{ __asm__ volatile ("dc cvac, %0" : : "r"(a) : "memory"); }
static inline void dsb_sy(void)
{ __asm__ volatile ("dsb sy" ::: "memory"); }

static int check_word(const char *what, unsigned long a, uint32_t expect)
{
  uint32_t got;
  dc_civac(a);
  dsb_sy();
  got = rd32(a);
  if (got != expect) {
    printf("[MEGASOC-CONN-003] FAIL: %s @0x%08lx = 0x%08x, expected 0x%08x\n",
           what, a, (unsigned)got, (unsigned)expect);
    return 1;
  }
  printf("[MEGASOC-CONN-003] %s @0x%08lx = 0x%08x OK\n", what, a, (unsigned)got);
  return 0;
}

int main(void)
{
  uint32_t done = 0;
  uint32_t t = DONE_TIMEOUT;
  int errors = 0;

  // Clear the target words and do the handshake before any printf: ADP may
  // already be polling and the console only drains between ADP commands.
  wr32(RAM_ADDR, 0u);
  wr32(DRAM_ADDR, 0u);
  dc_cvac(RAM_ADDR);
  dc_cvac(DRAM_ADDR);
  wr32(MBOX_DONE, 0u);
  wr32(MBOX_READY, CPU_READY);
  dc_cvac(MBOX_READY);          // same 64 B line as MBOX_DONE
  dsb_sy();

  UartStdOutInit();
  printf("MegaSoC ADP target matrix test (tc_adp_target_matrix)\n");
  printf("vplan: MEGASOC-CONN-003 (+ MEGASOC-ILL-007 ADP_AHB leg)\n");
  printf("[MEGASOC-CONN-003] CPU ready, waiting for ADP (adp.cmd)\n");

  while (t--) {
    dc_civac(MBOX_DONE);
    dsb_sy();
    done = rd32(MBOX_DONE);
    if (done == ADP_DONE) break;
  }
  if (done != ADP_DONE) {
    printf("[MEGASOC-CONN-003] FAIL: ADP did not finish (MBOX_DONE=0x%08x)\n",
           (unsigned)done);
    TEST_FAIL();
    while (1) { }
  }
  printf("[MEGASOC-CONN-003] ADP finished, checking words it wrote\n");

  errors += check_word("RAM word written by ADP", RAM_ADDR, RAM_EXPECT);
  errors += check_word("DRAM word written by ADP", DRAM_ADDR, DRAM_EXPECT);

  printf("[MEGASOC-CONN-003] per-target ADP responses: run check_adp_target_matrix.py\n");
  printf("Total number of adp target matrix errors: %d\n", errors);
  if (errors == 0) { TEST_PASS(); } else { TEST_FAIL(); }
  while (1) { }
}
