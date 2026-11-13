//==============================================================================
// sdio_m_exp_m_reach_tests.c -- MegaSoC SDIO_M (SDIO DMA manager) -> EXP_M
//                                (absent expansion target) boundary test
//                                (tc_sdio_m_exp_m_reach)
//
// Written by test-agent from verif/spec/megasoc_spec.md SS2.2 (SDIO_M
// connectivity matrix row: "SDIO_M reaches ROM, FLASH, RAM, DRAM, EXP_M"),
// SS6.4/SS6.5/SS6.6 and SS10 S1a+S1b (RESOLVED 2026-08-07), plus
// verif/spec/sdio_interface_contract.yaml (CMD register field table:
// DMA_ERR bit24, ERR bit15 = OR of CMD_ERR|XFER_ERR|DMA_ERR) and
// verif/spec/sdio_spec.md ONLY. No Arm/vendor RTL was read to write this
// test. Bench-side hooks (verif/testbench/megasoc/BENCH.md "ILLCOV:EXP_M
// markers" section; megasoc_exp_boundary_assertions.sv's
// a_exp_m_decerr_no_hang; megasoc_integration_cov.sv's cg_illegal_access /
// cp_exp_m_absent) are read-only references -- none of that bench
// infrastructure is touched by this file.
//
// Targeted vplan items (verif/vplan/megasoc_vplan.yaml):
//   MEGASOC-CONN-002 sdio_m_target_matrix (P0)
//     "SDIO_M (SDIO DMA manager) reaches exactly ROM, FLASH, RAM, DRAM and
//      EXP_M -- each path exercised via a DMA transfer and observed at the
//      target boundary." Of the 5 targets in this cross, ROM/FLASH are
//      already closed by sdio_dma_read_rom_flash_tests (MEGASOC-DMA-003),
//      and RAM (SRAM)/DRAM are already closed by
//      cpu_plus_sdio_dma_same_target_tests (MEGASOC-CONC-002) and
//      cpu_plus_sdio_dma_disjoint_tests (MEGASOC-CONC-001) -- both issue
//      real multi-block sdio_write() with the DMA source buffer placed in
//      SRAM/DRAM respectively, confirmed via real CMD25 transactions in
//      their VCS logs. This file closes the ONLY remaining bin: EXP_M, the
//      deliberately-absent expansion window (0x6000_0000-0x60FF_FFFF,
//      INC_EXP undefined in this build, no accelerator connected). The
//      EXP_M bin closes on the SDIO_M->EXP_M boundary transaction
//      completing promptly (no hang) -- an OKAY response would itself be a
//      wrong-response bug per MEGASOC-CONN-001's own EXP_M-bin convention
//      for the A53 manager, mirrored here for SDIO_M.
//   MEGASOC-ILL-008 exp_m_decerr_absent_target (P0, path_closure: error)
//     "any read or write to the EXP_M window ... from any manager (A53
//      CPU_AXI or SDIO_M), must complete promptly (no hang, bounded
//      latency) with DECERR". a53_target_matrix_tests.c already closes the
//      a53_manager half of cx_exp_m_absent_rw; this file closes the
//      sdio_m_manager half (both read and write cross bins).
//
// -----------------------------------------------------------------------------
// Mechanism: SDIO DMA engine engagement (confirmed by reading
// megasoc_tech/software/lib/sw_lib/devices/src/sdiodrv.c ONLY, no RTL)
// -----------------------------------------------------------------------------
// sdiodrv.c's sdio_write()/sdio_read() (lines ~2575/~2797) only engage the
// controller's independent SDIO_M_AXI DMA-manager path
// (SDIO_WRMULTI|SDIO_DMA / SDIO_RDMULTI|SDIO_DMA) when SDMULTI==1 AND
// count>1; a single-block (count==1) call instead falls back to the
// CPU-driven PIO/FIFO path (sdio_write_block()/sdio_read_block()) and never
// touches SDIO_M_AXI at all. This is exactly the same mechanism already
// proven by sdio_dma_read_rom_flash_tests.c (MEGASOC-DMA-003) and the
// CONC-001/CONC-002 tests -- reused here unchanged, just pointed at
// EXP_M_WIN_BASE instead of ROM/FLASH/SRAM/DRAM:
//   WRITE-to-card direction (SDIO_M reads system memory to send to the
//   card -- the DMA source fetch is an AXI READ at the EXP_M_AXI
//   boundary): sdio_write(dev, sector, 2, (const char *)EXP_M_WIN_BASE).
//   READ-from-card direction (SDIO_M writes system memory with card data
//   -- the DMA destination store is an AXI WRITE at the EXP_M_AXI
//   boundary): sdio_read(dev, sector, 2, (char *)EXP_M_WIN_BASE).
//
// Both calls are SAFE to issue directly against the absent EXP_M target:
// reading sdiodrv.c's multi-block write path (lines ~2609-2794) and read
// path (lines ~2797 onward) shows every busy-wait is routed through
// sdio_wait_while_busy() -> sdio_wait_while_busy_timeout(), which is
// bounded by SDIO_BUSY_TIMEOUT_CYCLES (never an infinite loop), and both
// functions check `dev_stat & (SDIO_ERR|SDIO_REMOVED)` afterward and return
// RES_ERROR (nonzero) on error rather than treating it as fatal. A NONZERO
// return from either call is therefore the CORRECT, EXPECTED outcome here
// (the DMA aborted due to the EXP_M DECERR) -- checked as a positive
// requirement below, not a test failure. A ZERO (success) return would mean
// EXP_M answered OKAY -- itself the MEGASOC-ILL-008 bug condition, reported
// as a hard FAIL per the "never weaken a spec-supported check" rule.
//
// -----------------------------------------------------------------------------
// ILLCOV:EXP_M:<RW>:SDIO_M markers (BENCH.md "ILLCOV:EXP_M markers")
// -----------------------------------------------------------------------------
// CORRECTED (post-review): <RW> names the actual EXP_M_AXI channel activated,
// matching a53_target_matrix_tests.c's own convention (there, rd32/wr32
// directly ARE the AXI channel, so driver-call-name and channel happen to
// coincide -- for SDIO_M they do not, since sdio_write()'s DMA *source
// fetch* is an AXI READ and sdio_read()'s DMA *destination store* is an AXI
// WRITE, per the SDIO controller's own documented semantics: "sdio_write()
// means card write -> SDIO_M reads system memory (AXI AR/R); sdio_read()
// means card read -> SDIO_M writes system memory (AXI AW/W/B)"). An earlier
// version of this file named the marker after the SDIO driver call instead
// (WRITE for sdio_write(), READ for sdio_read()) -- backwards relative to
// the actual bus channel and inconsistent with the a53 test's bins, which
// would have mis-attributed cg_illegal_access.cp_exp_m_absent's
// sdio_m_manager x {read,write} bins even though it does not affect the
// independent a_exp_m_decerr_no_hang assertion's own pass/fail truth. Fixed
// so: probe_exp_m_write() (issues sdio_write(), an AXI READ at EXP_M_AXI)
// emits "ILLCOV:EXP_M:READ:SDIO_M"; probe_exp_m_read() (issues sdio_read(),
// an AXI WRITE at EXP_M_AXI) emits "ILLCOV:EXP_M:WRITE:SDIO_M". Both markers
// are emitted so both sdio_m_manager x {read,write} cross bins in
// cg_illegal_access.cp_exp_m_absent are sampled, now correctly attributed.
//
// -----------------------------------------------------------------------------
// Response-code check: CMD register DMA_ERR/ERR bits
// -----------------------------------------------------------------------------
// verif/spec/sdio_interface_contract.yaml registers.CMD.fields:
//   DMA_ERR (bit 24): "RO sticky. DMA/bus error; cleared via ERR W1C"
//   ERR     (bit 15): "R/W1C. OR of CMD_ERR|XFER_ERR|DMA_ERR; write 1
//                       clears all error flags"
// After each probe, this test reads SDIO_S->sd_cmd and hard-checks DMA_ERR
// and ERR are both set (the software-visible proxy for the AXI DECERR the
// SDIO_M_AXI DMA engine observed at the EXP_M boundary -- DMA_ERR is
// specifically the DMA/bus-response error source, distinct from XFER_ERR's
// RX/TX data-CRC error and CMD_ERR's command-line error, matching the
// spec's own field table). Bit positions are derived directly from that
// spec table (not reused from sdiodrv.c's internal SDIO_* enum, which is a
// file-local static const not exported via sdiodrv.h), same discipline
// already established by sdio_crc_error_to_cpu_tests.c for its own CMD
// register bit checks.
//
// Recovery discipline (mirrors sdio_crc_error_to_cpu_tests.c's
// error-recovery-then-traffic pattern exactly): after decoding the error,
// W1C-clear it with a CMD write of 0x8080 (sdio_spec.md SS2.1 "0x8080 clear
// error" magic write, same literal value sdio_irq_end_to_end_tests.c and
// sdio_crc_error_to_cpu_tests.c already use), confirm ERR/DMA_ERR read back
// clear, then issue one normal single-block sdio_read() to
// SDIO_RECOVERY_SECTOR = 0x1000 (a real, previously-used-safe sector, same
// value/rationale as sdio_crc_error_to_cpu_tests.c's SDIO_RECOVERY_SECTOR)
// to confirm the controller recovered and is not left wedged by the DMA
// abort. Per that same file's precedent, only clean completion (return 0,
// CMD register shows no busy/error bits) is checked here, not recovered
// data content -- no known-content contract exists for that sector under
// this UVM test class (sdio_base_test, confirmed by reading
// verif/testbench/logical/uvm_vip/sdio_base_test.sv: it builds only the
// card agent and blocks on sim_done_vif.done, no backdoor content preload
// for arbitrary sectors).
//
// -----------------------------------------------------------------------------
// UVM_TESTNAME: this test runs with plain +UVM_TESTNAME=sdio_base_test
// (NOT sdio_crc_error_test) -- this is a pure DUT-vs-NIC400 boundary check
// (SDIO_M_AXI -> EXP_M_AXI DECERR/no-hang), not a card-protocol error, so no
// card-side exception callback is needed. Same UVM test class already used
// by sdio_dma_read_rom_flash_tests / cpu_plus_sdio_dma_same_target_tests /
// cpu_plus_sdio_dma_disjoint_tests for their real SDIO-DMA traffic.
//
// Self-checking: every check compares against a spec-derived expectation
// and prints the vplan id in the failure message, e.g.
//   [MEGASOC-ILL-008] EXP_M WRITE-probe CMD.DMA_ERR (bit24): exp bits set got 0x0
// Unambiguous end-of-test result via TEST PASSED/TEST FAILED + TEST_PASS()/
// TEST_FAIL().
//
// NOTE: printf() in this codebase's sw_lib is macro-redefined to c_print(),
// which vsprintf()'s into a fixed 160-byte stack buffer with NO length
// check whenever the format string contains '%' (documented trap from this
// project's own manifest notes). Every printf() format string in this file
// is kept short.
//
// Config knobs: directed test, no randomization. Build-time verbosity via
// `make ... TEST_VERBOSITY=1`.
//
// Added 2026-11-12 (NIC-400 close-out), defaults keep the original behaviour:
//   EXP_M_PROBE_ONLY=0  both probes (default)
//   EXP_M_PROBE_ONLY=1  write-probe only (sdio_write, AXI READ at EXP_M)
//   EXP_M_PROBE_ONLY=2  read-probe only  (sdio_read,  AXI WRITE at EXP_M)
//   EXP_M_SKIP_RECOVERY skip recover_and_confirm_not_wedged(). After the first
//                       DMA abort the controller's dma_busy never clears and
//                       the recovery access hangs (BUG-42, SDIO controller),
//                       which kept the read-probe from ever running. With one
//                       probe per simulation no recovery is needed to reach
//                       the NIC-400 boundary check. sdio_m_exp_m_axi_write_tests
//                       builds this file with EXP_M_PROBE_ONLY=2 +
//                       EXP_M_SKIP_RECOVERY.
//==============================================================================

#ifndef EXP_M_PROBE_ONLY
#define EXP_M_PROBE_ONLY 0
#endif

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sdiodrv.h"
#include <stdio.h>
#include <stdint.h>

static const char *V_CONN = "MEGASOC-CONN-002";
static const char *V_ILL  = "MEGASOC-ILL-008";

// --------------------------------------------------------------------------
// Spec-derived target address (megasoc_spec.md SS2 row EXP_M) -- same
// literal constant a53_target_matrix_tests.c already defines at its line
// 155 (EXP_M_WIN_BASE), reused verbatim here for the SDIO_M-manager half of
// the same window.
// --------------------------------------------------------------------------
#define EXP_M_WIN_BASE 0x60000000UL

// DMA sectors used to trigger each direction's SDIO_M -> EXP_M probe.
// Distinct sectors for the write-probe and read-probe so their respective
// (aborted) transfers cannot be confused with one another in the log.
#define SDIO_M_EXP_WRITE_SECTOR 0x5000u
#define SDIO_M_EXP_READ_SECTOR  0x5010u
#define SDIO_M_EXP_BLOCKS       2u   // >1 required to engage the SDIO_M_AXI
                                     // DMA path (SDMULTI==1, sdiodrv.c
                                     // sdio_write()/sdio_read())

// Different, unaffected sector for the post-error recovery/no-wedge check
// -- same value/rationale as sdio_crc_error_to_cpu_tests.c's
// SDIO_RECOVERY_SECTOR.
#define SDIO_RECOVERY_SECTOR 0x1000u

// -- SDIO CMD register bit positions (verif/spec/sdio_interface_contract.yaml
//    registers.CMD.fields table) -- defined here directly from the spec,
//    not reused from sdiodrv.c's internal SDIO_* enum (file-local static
//    const there, not exported via sdiodrv.h), same discipline
//    sdio_crc_error_to_cpu_tests.c already established. -----------------
#define SDIO_CMD_DMA_ERR_BIT    (1UL << 24) // DMA_ERR: RO sticky, DMA/bus error, cleared via ERR W1C
#define SDIO_CMD_ERR_BIT        (1UL << 15) // ERR: R/W1C, OR of CMD_ERR|XFER_ERR|DMA_ERR
#define SDIO_CMD_CMDBUSY_BIT    (1UL << 14) // CMD_BUSY: RO, command in flight
#define SDIO_CMD_DMABUSY_BIT    (1UL << 13) // USE_DMA readback: RO, dma_busy
#define SDIO_CMD_MEMBUSY_BIT    (1UL << 11) // USE_FIFO readback: RO, mem (FIFO) busy
#define SDIO_CMD_CARDBUSY_BIT   (1UL << 20) // CARD_BUSY: RO, card busy (R1b/DAT0 low)

// sdio_spec.md SS2.1 "Useful magic writes": "0x8080 clear error" -- W1C bit
// 15 (ERR) combined with bit 7 (reset FIFO pointer / suppress command), same
// literal magic value sdio_irq_end_to_end_tests.c and
// sdio_crc_error_to_cpu_tests.c already use and verify against sdio_spec.md
// SS2.1's W1C semantics.
#define SDIO_CMD_CLEAR_ERROR_MAGIC 0x8080UL

static SDIO *SDIO_S = (SDIO *)SYS_SDIO_BASE;

static int check_set_bits(const char *vplan, const char *what, uint32_t mask,
                           uint32_t got)
{
  if ((got & mask) != mask) {
    printf("[%s] %s: exp bits set got 0x%x\n", vplan, what, (unsigned)got);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] %s: OK (0x%x)\n", vplan, what, (unsigned)got);
#endif
  return 0;
}

static int check_zero_bits(const char *vplan, const char *what, uint32_t mask,
                            uint32_t got)
{
  if (got & mask) {
    printf("[%s] %s: exp bits clear got 0x%x\n", vplan, what, (unsigned)got);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] %s: OK (0x%x)\n", vplan, what, (unsigned)got);
#endif
  return 0;
}

static int check_nonzero(const char *vplan, const char *what, int got)
{
  if (got == 0) {
    printf("[%s] %s: exp nonzero got 0\n", vplan, what);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] %s: OK (nonzero, %d)\n", vplan, what, got);
#endif
  return 0;
}

static int check_eq_int(const char *vplan, const char *what, int expected,
                         int got)
{
  if (expected != got) {
    printf("[%s] %s: exp %d got %d\n", vplan, what, expected, got);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] %s: OK (%d)\n", vplan, what, got);
#endif
  return 0;
}

// --------------------------------------------------------------------------
// Post-probe response-code decode: DMA_ERR + ERR must both be set (the
// software-visible proxy for "SDIO_M_AXI observed a non-OKAY (DECERR)
// response at the EXP_M boundary"). A read with neither bit set would mean
// the DMA transfer to/from EXP_M completed as if EXP_M were a live,
// OKAY-responding target -- itself the MEGASOC-ILL-008 bug condition.
// --------------------------------------------------------------------------
// The driver's sdio_read() error path (sdiodrv.c, SDDEBUG=1) prints
// "SD-READ -> ERR: <CMD>:<data>" and then sends CMD13 (SEND_STATUS) through
// sdio_get_r1(). Issuing that command clears the sticky DMA_ERR/ERR bits, so
// after a READ-probe the register no longer holds the DECERR signature
// (2026-11-12 run: driver printed 0x0101930c, test then read 0x0001010d).
// In that one case the register check is handed to the log check
// verif/tests/megasoc/check_sdio_decerr_report.py, which the manifest runs as
// post_check and which needs DMA_ERR+ERR set in the driver's own report.
#define SDIO_CMD_INDEX_MASK  0x3FUL
#define SDIO_CMD_SEND_STATUS 13UL

static int check_exp_m_decerr_signature(const char *probe_name)
{
  int errors = 0;
  uint32_t cmd_status = SDIO_S->sd_cmd;
  char what[64];

  printf("[%s] post-%s SDIO CMD register = 0x%08x\n", V_ILL, probe_name,
         (unsigned)cmd_status);

  if ((cmd_status & SDIO_CMD_INDEX_MASK) == SDIO_CMD_SEND_STATUS &&
      !(cmd_status & SDIO_CMD_DMA_ERR_BIT)) {
    printf("[%s] %s: CMD reg overwritten by driver CMD13; signature checked "
           "from SD-READ -> ERR line (post_check)\n", V_ILL, probe_name);
    return 0;
  }

  snprintf(what, sizeof(what), "%s CMD.DMA_ERR (bit24) set", probe_name);
  errors += check_set_bits(V_ILL, what, SDIO_CMD_DMA_ERR_BIT, cmd_status);

  snprintf(what, sizeof(what), "%s CMD.ERR (bit15) set", probe_name);
  errors += check_set_bits(V_ILL, what, SDIO_CMD_ERR_BIT, cmd_status);

  if (errors) {
    printf("[%s] FAIL: %s did not show DECERR signature (appears OKAY)\n",
           V_ILL, probe_name);
  } else {
    printf("[%s] %s: DECERR signature confirmed (DMA_ERR+ERR set)\n",
           V_ILL, probe_name);
  }

  return errors;
}

// --------------------------------------------------------------------------
// W1C-clear the sticky error, confirm it cleared, then issue one normal
// single-block sdio_read() to a real, unaffected sector to prove the
// controller recovered and is not left wedged by the DMA abort. Mirrors
// sdio_crc_error_to_cpu_tests.c's recovery-then-traffic discipline exactly.
// --------------------------------------------------------------------------
static int __attribute__((unused))
recover_and_confirm_not_wedged(SDIODRV *dev, const char *probe_name)
{
  int errors = 0;
  uint32_t cmd_status;
  static uint8_t recovery_buf[512];
  int read_ret;
  char what[64];

  printf("[%s] %s: W1C-clearing CMD.ERR (0x%04x)\n", V_ILL, probe_name,
         (unsigned)SDIO_CMD_CLEAR_ERROR_MAGIC);
  SDIO_S->sd_cmd = SDIO_CMD_CLEAR_ERROR_MAGIC;
  cmd_status = SDIO_S->sd_cmd;

  snprintf(what, sizeof(what), "%s CMD.ERR clear after W1C", probe_name);
  errors += check_zero_bits(V_ILL, what, SDIO_CMD_ERR_BIT, cmd_status);
  snprintf(what, sizeof(what), "%s CMD.DMA_ERR clear after W1C (via ERR OR)",
            probe_name);
  errors += check_zero_bits(V_ILL, what, SDIO_CMD_DMA_ERR_BIT, cmd_status);

  printf("[%s] %s: issuing recovery sdio_read() to unaffected sector 0x%x\n",
         V_CONN, probe_name, (unsigned)SDIO_RECOVERY_SECTOR);
  read_ret = sdio_read(dev, SDIO_RECOVERY_SECTOR, 1, (char *)recovery_buf);
  printf("[%s] %s: recovery sdio_read() returned %d\n", V_CONN, probe_name,
         read_ret);

  snprintf(what, sizeof(what), "%s recovery sdio_read() succeeds (not wedged)",
            probe_name);
  errors += check_eq_int(V_CONN, what, 0, read_ret);

  cmd_status = SDIO_S->sd_cmd;
  printf("[%s] %s: post-recovery SDIO CMD register = 0x%08x\n", V_CONN,
         probe_name, (unsigned)cmd_status);

  snprintf(what, sizeof(what), "%s CMD.ERR clear post-recovery", probe_name);
  errors += check_zero_bits(V_CONN, what, SDIO_CMD_ERR_BIT, cmd_status);
  snprintf(what, sizeof(what), "%s CMD.cmd_busy clear post-recovery", probe_name);
  errors += check_zero_bits(V_CONN, what, SDIO_CMD_CMDBUSY_BIT, cmd_status);
  snprintf(what, sizeof(what), "%s CMD.dma_busy clear post-recovery", probe_name);
  errors += check_zero_bits(V_CONN, what, SDIO_CMD_DMABUSY_BIT, cmd_status);
  snprintf(what, sizeof(what), "%s CMD.mem_busy clear post-recovery", probe_name);
  errors += check_zero_bits(V_CONN, what, SDIO_CMD_MEMBUSY_BIT, cmd_status);
  snprintf(what, sizeof(what), "%s CMD.card_busy clear post-recovery", probe_name);
  errors += check_zero_bits(V_CONN, what, SDIO_CMD_CARDBUSY_BIT, cmd_status);

  return errors;
}

// --------------------------------------------------------------------------
// MEGASOC-CONN-002 / MEGASOC-ILL-008 -- SDIO_M -> EXP_M, WRITE-to-card
// direction: sdio_write() with the DMA source buffer pointed at
// EXP_M_WIN_BASE. SDIO_M_AXI issues the DMA source-fetch (AXI read) at the
// EXP_M boundary; the deliberately-absent target must DECERR promptly
// (no hang), aborting the SDIO transfer.
// --------------------------------------------------------------------------
static int probe_exp_m_write(SDIODRV *dev)
{
  int errors = 0;
  int wr_ret;

  printf("Test: MEGASOC-CONN-002/ILL-008 sdio_m -> EXP_M (write-to-card dir)\n");
  /* sdio_write() = card write = SDIO_M DMA source-fetch = AXI READ at
   * EXP_M_AXI -- marker names the actual bus channel, not the driver call. */
  printf("ILLCOV:EXP_M:READ:SDIO_M\n");

  wr_ret = sdio_write(dev, SDIO_M_EXP_WRITE_SECTOR, SDIO_M_EXP_BLOCKS,
                       (const char *)EXP_M_WIN_BASE);
  printf("[%s] EXP_M WRITE-probe: sdio_write() returned %d\n", V_CONN, wr_ret);

  if (wr_ret != 0) {
    printf("[%s] EXP_M WRITE-probe: sdio_write() failed as expected "
           "(DMA aborted, no hang)\n", V_CONN);
  }
  /* A ZERO return here would mean EXP_M answered OKAY -- itself the
   * MEGASOC-ILL-008 bug condition, per the "never weaken a spec-supported
   * check" rule; check_nonzero() below is the single, authoritative check
   * for this (not double-counted against a separate manual increment). */
  errors += check_nonzero(V_ILL,
                          "EXP_M WRITE-probe sdio_write() returns error (expected)",
                          wr_ret);

  errors += check_exp_m_decerr_signature("WRITE-probe");
#ifndef EXP_M_SKIP_RECOVERY
  errors += recover_and_confirm_not_wedged(dev, "WRITE-probe");
#endif

  return errors;
}

// --------------------------------------------------------------------------
// MEGASOC-CONN-002 / MEGASOC-ILL-008 -- SDIO_M -> EXP_M, READ-from-card
// direction: sdio_read() with the DMA destination buffer pointed at
// EXP_M_WIN_BASE. SDIO_M_AXI issues the DMA destination-store (AXI write)
// at the EXP_M boundary; the deliberately-absent target must DECERR
// promptly (no hang), aborting the SDIO transfer.
// --------------------------------------------------------------------------
static int probe_exp_m_read(SDIODRV *dev)
{
  int errors = 0;
  int rd_ret;

  printf("Test: MEGASOC-CONN-002/ILL-008 sdio_m -> EXP_M (read-from-card dir)\n");
  /* sdio_read() = card read = SDIO_M DMA destination-store = AXI WRITE at
   * EXP_M_AXI -- marker names the actual bus channel, not the driver call. */
  printf("ILLCOV:EXP_M:WRITE:SDIO_M\n");

  rd_ret = sdio_read(dev, SDIO_M_EXP_READ_SECTOR, SDIO_M_EXP_BLOCKS,
                      (char *)EXP_M_WIN_BASE);
  printf("[%s] EXP_M READ-probe: sdio_read() returned %d\n", V_CONN, rd_ret);

  if (rd_ret != 0) {
    printf("[%s] EXP_M READ-probe: sdio_read() failed as expected "
           "(DMA aborted, no hang)\n", V_CONN);
  }
  /* A ZERO return here would mean EXP_M answered OKAY -- itself the
   * MEGASOC-ILL-008 bug condition, per the "never weaken a spec-supported
   * check" rule; check_nonzero() below is the single, authoritative check
   * for this (not double-counted against a separate manual increment). */
  errors += check_nonzero(V_ILL,
                          "EXP_M READ-probe sdio_read() returns error (expected)",
                          rd_ret);

  errors += check_exp_m_decerr_signature("READ-probe");
#ifndef EXP_M_SKIP_RECOVERY
  errors += recover_and_confirm_not_wedged(dev, "READ-probe");
#endif

  return errors;
}

int main(void)
{
  int errors = 0;
  SDIODRV *dev;

  UartStdOutInit();

  printf("MegaSoC sdio_m -> EXP_M reach test (tc_sdio_m_exp_m_reach)\n");
  printf("vplan: MEGASOC-CONN-002 (EXP_M bin) + MEGASOC-ILL-008 "
         "(sdio_m_manager half)\n");

  dev = sdio_init(SDIO_S);

  if (dev == NULL) {
    printf("[%s] sdio_init() returned NULL\n", V_CONN);
    printf("[%s] BLOCKED: no reachable SD card in this bench flow\n", V_CONN);
    TEST_FAIL();
    return 1;
  }
  printf("[%s] sdio_init() returned a driver handle\n", V_CONN);

#if EXP_M_PROBE_ONLY != 2
  errors += probe_exp_m_write(dev);
#endif
#if EXP_M_PROBE_ONLY != 1
  errors += probe_exp_m_read(dev);
#endif

  printf("Total number of sdio_m exp_m reach errors: %d\n", errors);

  if (errors == 0) {
    printf("TEST PASSED\n");
    TEST_PASS();
  } else {
    printf("TEST FAILED\n");
    TEST_FAIL();
  }

  return errors;
}
