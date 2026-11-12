//==============================================================================
// rom_adp_write_tests.c -- MEGASOC-BOOT-007, ADP_AHB leg (tc_rom_write_response)
//
// Written 2026-11-11 from megasoc_spec.md section 6.1.1 Q1 and section 11 A27.
// The ADP debug controller (SoC Labs socdebug_adp_control, an AHB manager on
// the NIC-400) writes to the ROM window and reads it back. The commands come
// from adp.cmd in this directory, which the bench replays into the ADP port
// (flows/makefile.simulate copies it to the sim directory). This C program only
// coordinates with ADP and checks the ROM afterwards.
//
// Handshake through two words in an SRAM area no image uses (bootloader and
// tests stop at 0x0080_8000):
//   MBOX_DONE  0x0080_F004  CPU writes 0, ADP writes ADP_DONE when finished
//   MBOX_READY 0x0080_F000  CPU writes CPU_READY after DONE=0 and the snapshot
// adp.cmd polls MBOX_READY with many short "p" commands (a long poll would
// stop ADP passing console output, stalling the boot ROM banner: deadlock),
// then writes/reads the ROM, writes MBOX_DONE and exits ADP mode.
// ADP takes the access size from the number of hex digits in a command, so
// every address/data/count is written with 8 digits (32-bit). The first run
// (2026-11-11 12:49) used "p 0x100": 16-bit polls that could never match the
// 32-bit ready word; all 400 timed out and ADP went on after them. The result
// was still valid because the CPU had signalled ready long before (see the
// console characters among the P lines in extadp_out.log).
//
// CPU checks (hard pass/fail):
//   - ADP finished within the timeout (MBOX_DONE == ADP_DONE);
//   - the six ROM words ADP wrote still equal the snapshot taken before.
// Bench checks (+rom_check, megasoc_rom_boundary_checker): one B per AW with
// matching BID, BRESP recorded, every ROM_AXI read beat (ADP read-backs too)
// equal to bootrom.bintxt.
// Characterisation (logs/extadp_out.log): ADP echoes "W 0x..." for a write
// with OKAY and "W!0x..." for a write with an AHB ERROR response; same for
// "R". The read at 0x0001_0000 is a positive control for the "!" marker.
//
// Run: make run_vcs TESTNAME=rom_adp_write_tests VCS_SIM_OPTION="+vcs+flush+log +rom_check"
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

static const unsigned long rom_addr[6] = {
  0x0000UL, 0x0004UL,     // first image word pair
  0x99F0UL, 0x99F4UL,     // last image dword of the current build (BOOT-008)
  0xFFF8UL, 0xFFFCUL      // last dword of the 64 KB window
};

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

int main(void)
{
  uint32_t snap[6], now, done = 0;
  uint32_t t = DONE_TIMEOUT;
  int errors = 0;

  // Snapshot and handshake first, before any printf: ADP may be polling and
  // the console only drains between ADP commands.
  for (int i = 0; i < 6; i++) {
    dc_civac(rom_addr[i]);
    dsb_sy();
    snap[i] = rd32(rom_addr[i]);
  }
  wr32(MBOX_DONE, 0u);
  wr32(MBOX_READY, CPU_READY);
  dc_cvac(MBOX_READY);          // same 64 B line as MBOX_DONE; line stays clean
  dsb_sy();

  UartStdOutInit();
  printf("MegaSoC ROM write via ADP test (tc_rom_write_response, ADP leg)\n");
  printf("vplan: MEGASOC-BOOT-007\n");
  printf("[MEGASOC-BOOT-007] CPU ready, waiting for ADP (adp.cmd)\n");

  while (t--) {
    dc_civac(MBOX_DONE);         // line is clean, so this only invalidates
    dsb_sy();
    done = rd32(MBOX_DONE);
    if (done == ADP_DONE) break;
  }
  if (done != ADP_DONE) {
    printf("[MEGASOC-BOOT-007] FAIL: ADP did not finish (MBOX_DONE=0x%08x)\n", (unsigned)done);
    TEST_FAIL();
    while (1) { }
  }
  printf("[MEGASOC-BOOT-007] ADP finished, checking ROM content\n");

  for (int i = 0; i < 6; i++) {
    dc_civac(rom_addr[i]);
    dsb_sy();
    now = rd32(rom_addr[i]);
    if (now != snap[i]) {
      printf("[MEGASOC-BOOT-007] FAIL: ROM @0x%05lx = 0x%08x after ADP write, was 0x%08x\n",
             rom_addr[i], (unsigned)now, (unsigned)snap[i]);
      errors++;
    } else {
      printf("[MEGASOC-BOOT-007] ROM @0x%05lx unchanged (0x%08x)\n", rom_addr[i], (unsigned)now);
    }
  }
  printf("[MEGASOC-BOOT-007] ADP BRESP/RRESP: see W/R echo lines in logs/extadp_out.log\n");
  printf("Total number of rom adp write errors: %d\n", errors);
  if (errors == 0) { TEST_PASS(); } else { TEST_FAIL(); }
  while (1) { }
}
