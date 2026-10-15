//-----------------------------------------------------------------------------
// MegaSoC LPDDR4 DRAM driver (INC_LPDDR4 builds only)
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Brings the LPDDR4 subsystem up from reset (controller registers, PHY init
// sequence, DFI handshake, DRAM init) and gives tests the register and DRAM
// access helpers they share.
//
// Opt-in: not part of the common library list. An LPDDR4 test adds it in its
// makefile, together with the folder of the generated PHY init sequence:
//   APP_SRC       := my_test.c $(SW_LIB_PATH)/devices/src/lpddr4.c
//   APP_C_INC_DIR += $(LPDDR4_PROJECT_DIR)/sw/libs/phyinit
//   APP_C_FLAGS   += -DLPDDR4_SKIP_TRAINING=$(LPDDR4_SKIP_TRAINING)
//
// Register access (MegaSoC peripheral window, see megasoc_tech_wrapper.sv):
//   controller / reset-control register R : LPDDR4_CTRL_BASE + R
//   PHY register N                         : LPDDR4_PHY_BASE + (N << 2)
//-----------------------------------------------------------------------------
#ifndef LPDDR4_H
#define LPDDR4_H

#include <stdint.h>

#define LPDDR4_CTRL_BASE   0x41000000UL     // controller + reset control (APB3)
#define LPDDR4_PHY_BASE    0x41400000UL     // PHY (APB4)
#define DRAM_BASE          0x80000000UL     // 2 Gb device = 256 MB, aliases above
#define LPDDR4_DRAM_SIZE   (256UL << 20)
#define LPDDR4_CACHE_LINE  64               // A53 cache line = one DRAM line fill

// 1: skip-training PHY init, the controller initialises the DRAM (default,
// set by the test makefile); 0: PHY training firmware.
#ifndef LPDDR4_SKIP_TRAINING
#define LPDDR4_SKIP_TRAINING 1
#endif

// Bounded polls. Each poll is an APB read through the AHB-to-APB bridge (about
// 1-2 us of simulated time), so 5000 polls is several ms.
#define LPDDR4_POLL_LIMIT  5000

// STAT.operating_mode values
#define LPDDR4_MODE_INIT          0
#define LPDDR4_MODE_NORMAL        1
#define LPDDR4_MODE_POWER_DOWN    2
#define LPDDR4_MODE_SELF_REFRESH  3

// Bring-up. Returns 0 when the DRAM is ready for traffic, 1 on any timeout or
// register readback mismatch (details are printed). Leaves auto-refresh,
// power-down, self-refresh and DRAM clock stop enabled and AXI port 0 on.
int      lpddr4_init(void);
// 1 if the generated PHY init sequence was found at build time
int      lpddr4_phyinit_present(void);
// Poll timeouts + register readback mismatches seen so far
int      lpddr4_error_count(void);

// Controller / reset-control registers (byte offset R from LPDDR4_CTRL_BASE)
void     lpddr4_ctrl_write(uint32_t reg, uint32_t data);
uint32_t lpddr4_ctrl_read(uint32_t reg);
void     lpddr4_ctrl_bit_set(uint32_t reg, uint32_t bit);
void     lpddr4_ctrl_bit_clear(uint32_t reg, uint32_t bit);
// Poll until (value & mask) == want; 0 on success, 1 on timeout (counted)
int      lpddr4_poll_ctrl(const char *what, uint32_t reg, uint32_t mask, uint32_t want);
// STAT.operating_mode (LPDDR4_MODE_*)
unsigned lpddr4_op_mode(void);

// DRAM through the CPU. The DRAM window is cacheable: after writing, clean +
// invalidate the range before reading it back from DRAM.
void     lpddr4_cache_clean_invalidate(uintptr_t addr, uintptr_t len);
static inline void     dram_wr64(uintptr_t off, uint64_t v) { *(volatile uint64_t *)(DRAM_BASE + off) = v; }
static inline uint64_t dram_rd64(uintptr_t off)            { return *(volatile uint64_t *)(DRAM_BASE + off); }
// Print one data mismatch (first 10 per tag/counter); returns 1 for counting
int      lpddr4_report_mismatch(const char *tag, int errs, uintptr_t off, uint64_t got, uint64_t exp);

// DRAM power domain (PPU at slot 5 of the power-control bus). Low power is
// functional retention: both low-power channels quiescent (AXI port stopped,
// DRAM in self-refresh), contents kept, no reset on exit.
// lpddr4_lowpower_enter() turns on DRAM low-power mode: the domain goes to
// retention whenever the AXI port is idle and wakes automatically on any DRAM
// access (CPU, speculative fetch, DMA, debugger), which then completes after
// the self-refresh exit (~2 us). lpddr4_lowpower_exit() turns the mode off
// and keeps the domain ON.
#define LPDDR4_PPU_BASE    0x01205000UL
#define LPDDR4_LP_ACCEPTED 0
#define LPDDR4_LP_TIMEOUT  2
int      lpddr4_lowpower_enter(void);      // LPDDR4_LP_ACCEPTED once in retention
int      lpddr4_lowpower_exit(void);       // 0 when back on, LPDDR4_LP_TIMEOUT otherwise
// Dynamic transition events since the last call (and cleared):
// bit 1 = a transition completed, bit 2 = a transition was denied
#define LPDDR4_PPU_EV_ACCEPT 0x2
#define LPDDR4_PPU_EV_DENY   0x4
unsigned lpddr4_ppu_events(void);
#define LPDDR4_PPU_ON       8               // PPU power status values used here
#define LPDDR4_PPU_FUNC_RET 7
unsigned lpddr4_ppu_state(void);           // PPU power status (LPDDR4_PPU_*)

#endif
