//==============================================================================
// gpio_reset_and_reserved_offset_tests.c -- MegaSoC GPIO0/GPIO1 reset-path +
//                                            error/exception-path boundary
//                                            test (tc_gpio_reset_readback /
//                                            tc_gpio_reserved_offset_characterize)
//
// Written by test-agent from verif/spec/megasoc_spec.md §5.11 "Reset-path
// properties" / "Error/exception-path properties" (added 2026-09-23) and
// verif/spec/megasoc_interface_contract.yaml
// gpio_programmers_model.control_response_properties.gpio_reset_readback /
// .gpio_reserved_offset_response_vendor_internal ONLY. GPIO0/GPIO1 wrap the
// vendor cmsdk_ahb_gpio IP (verif/spec/megasoc_ip_blackboxes.yaml
// rtl_access: forbidden) -- no RTL was read for either instance; every
// register offset, reset value, and reserved-offset range below is sourced
// from the spec's register map (megasoc_spec.md §5.11) and interface
// contract text quoted above.
//
// Bundles both new 2026-09-23 GPIO 8-path-closure vplan items into one
// binary, following the same bundling convention already established by
// peripheral_reserved_offset_tests.c (5 items) and qspi_illegal_access_tests.c
// (2 items) -- both are GPIO-specific items being added together.
//
// Targeted vplan items (verif/vplan/megasoc_vplan.yaml):
//   MEGASOC-GPIO-005  gpio_reset_readback                          -- PARTIAL
//                      (12/13 documented registers x GPIO0/GPIO1 FULL; DATA
//                       register x both instances is a confirmed-reproducible
//                       AHB bus wedge on this exact register -- see the
//                       dedicated finding writeup above
//                       gpio_reset_readback_probe() below -- reported, not
//                       worked around, not faked as passing).
//   MEGASOC-ILL-016   gpio_reserved_offset_response_characterization
//                      -- FULL per its own blocked_on_vendor_doc scope
//                         (bounded-completion pass/fail asserted; HRESP/
//                         HREADYOUT/HRDATA VALUE recorded, not asserted,
//                         per new ambiguity A26).
//
// -----------------------------------------------------------------------------
// Ordering requirement (MEGASOC-GPIO-005): tc_gpio_reset_readback runs FIRST
// -----------------------------------------------------------------------------
// tc_gpio_reset_readback runs FIRST in this binary, before any other
// sub-test, and before any write to any GPIO0/GPIO1 register anywhere in
// this file -- required by MEGASOC-GPIO-005's own "first read after HRESETn
// deassertion, before any write has occurred" acceptance criterion. Same
// precedent as tc_rtc_reset_visibility in peripheral_reserved_offset_tests.c
// ("runs FIRST"). Checked: megasoc_tech/software/lib contains no reference
// to GPIO_0_BASE/GPIO_1_BASE (0x4001_0000/0x4002_0000) anywhere outside the
// sys_memory_map.h macro definitions themselves (grep confirmed) -- no
// platform/library startup code touches either GPIO instance before main(),
// so this binary's own first GPIO0/GPIO1 access genuinely observes
// post-HRESETn-deassertion reset state.
//
// -----------------------------------------------------------------------------
// MEGASOC-ILL-016 design note: abort_handler() override, same idiom as
// MEGASOC-ILL-014 in peripheral_reserved_offset_tests.c
// -----------------------------------------------------------------------------
// GPIO0/GPIO1's gpio0_hresp/gpio1_hresp and gpio0_hreadyout/gpio1_hreadyout
// connect DIRECTLY to the vendor cmsdk_ahb_gpio instance's own HRESP/
// HREADYOUT output ports -- NOT tied off, unlike WATCHDOG/Dual Timer/PL031
// RTC's hardwired PSLVERR=0/PREADY=1 (interface contract
// gpio_reserved_offset_response_vendor_internal, new ambiguity A26). This
// places GPIO in the same "not tied off" disposition family as CMSDK
// Timer0/1 (MEGASOC-ILL-014's own design note, quoted here verbatim for the
// identical reasoning): if the vendor IP asserts HRESP for a
// reserved-offset access AND that response code reaches the CPU as a
// synchronous external abort, an unguarded rd32() would trap without ever
// returning, silently failing the whole binary at that point. The
// abort_handler() override (same idiom as qspi_illegal_access_tests.c /
// nic_decerr_tests.c / peripheral_reserved_offset_tests.c, wired in via
// megasoc_tech/software/lib/sw_lib/apps/src/vect_64.s -> sync_abort_handler
// -> abort_handler(), advances ELR_EL3 by 4 and eret's -- project-owned
// software exception infra, not RTL) makes the read safely completable
// either way, and the abort-count delta is used purely as OBSERVED EVIDENCE
// (recorded, not asserted) of whether HRESP fired, exactly matching
// MEGASOC-ILL-016's own "record (do not assert)" disposition per A26.
//
// -----------------------------------------------------------------------------
// HRDATA-recording gap closure (MEGASOC-ILL-016)
// -----------------------------------------------------------------------------
// BENCH.md (2026-09-23 changelog, "HRDATA gap (GPIO0/GPIO1, reported not
// guessed)") documents that no gpio0_hrdata/gpio1_hrdata wrapper wire is
// named anywhere in verif/spec/megasoc_interface_contract.yaml, so
// harness-agent could not add a hardware HRDATA covergroup for this item --
// and explicitly notes this "can be closed entirely in software (the AHB
// read-data value is already visible to the C test issuing the
// reserved-offset access, needing no bench hook at all)". This test closes
// that half exactly that way: the reserved-offset probe's own rd32() return
// value is printed (recorded, not asserted -- per A26) below as the
// documented software-side closure of the HRDATA evidence dimension; the
// HRESP/HREADYOUT evidence dimension is separately closed in hardware by
// cg_gpio_reserved_offset_observed in
// verif/testbench/megasoc/megasoc_periph_reserved_offset_boundary.sv
// (sampling gpio0_hresp/gpio0_hreadyout/gpio1_hresp/gpio1_hreadyout every
// pclk cycle, both bins always legal, no assertion -- BENCH.md 2026-09-23).
//
// Self-checking: every check compares against a spec-derived expectation
// and prints the vplan id in the failure message, e.g.
//   [MEGASOC-GPIO-005] OUTENSET GPIO0 reset readback: exp 0x0 got ...
// Unambiguous end-of-test result via TEST_PASS()/TEST_FAIL().
//
// Config knobs: TEST_VERBOSITY=1 -> -DTEST_VERBOSITY_HIGH (verbose per-access
// prints).
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include <stdio.h>
#include <stdint.h>

static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }

static int check_eq(const char *vplan_id, const char *what,
                    unsigned long long expected, unsigned long long got)
{
  if (expected != got) {
    printf("[%s] %s: exp 0x%llx got 0x%llx\n", vplan_id, what, expected, got);
    return 1;
  }
  return 0;
}

/* -- abort_handler() override, same pattern as qspi_illegal_access_tests.c /
 *    nic_decerr_tests.c / peripheral_reserved_offset_tests.c -- used ONLY by
 *    the MEGASOC-ILL-016 sub-test below (GPIO's reserved-offset HRESP is a
 *    genuine vendor-internal pass-through, not tied off), but installed for
 *    the whole binary (MEGASOC-GPIO-005's reset-readback reads are all
 *    in-range documented registers and cannot trap). */
static volatile uint32_t g_abort_count = 0;

void abort_handler(void)
{
  g_abort_count++;
#ifdef TEST_VERBOSITY_HIGH
  printf("[MEGASOC-ILL-016] abort_handler: trapped abort #%u\n", (unsigned)g_abort_count);
#endif
}

/*==============================================================================
 * Shared GPIO0/GPIO1 register offsets (ARM DDI 0479C / megasoc_spec.md §5.11
 * register map)
 *============================================================================*/
#define GPIO_DATA_OFF        0x0000UL
#define GPIO_DATAOUT_OFF     0x0004UL
#define GPIO_OUTENSET_OFF    0x0010UL
#define GPIO_OUTENCLR_OFF    0x0014UL
#define GPIO_ALTFUNCSET_OFF  0x0018UL
#define GPIO_ALTFUNCCLR_OFF  0x001CUL
#define GPIO_INTENSET_OFF    0x0020UL
#define GPIO_INTENCLR_OFF    0x0024UL
#define GPIO_INTTYPESET_OFF  0x0028UL
#define GPIO_INTTYPECLR_OFF  0x002CUL
#define GPIO_INTPOLSET_OFF   0x0030UL
#define GPIO_INTPOLCLR_OFF   0x0034UL
#define GPIO_INTSTATUS_OFF   0x0038UL

/*==============================================================================
 * MEGASOC-GPIO-005: gpio_reset_readback
 *
 * UPDATE 2026-11-20: DATA is now read (13/13), see the DATA block at the end
 * of gpio_reset_readback_probe(). The "AHB bus wedge" write-up below is kept
 * as history; the likely cause was undriven pads returning X, not a bus
 * stall, and the bench now pulls those pads low.
 *
 * Runs FIRST -- see file header ordering note. Reads only; no write to any
 * GPIO0/GPIO1 register occurs anywhere in this sub-test, preserving
 * post-reset state for the readback check itself.
 *
 * Register x instance table drives both the check_eq() assertion (register
 * VALUE == 0x0) and the GPIOCOV:RESET:<REGISTER>:<GPIO0|GPIO1> software
 * coverage marker (megasoc_integration_cov.sv cg_gpio_reset.cp_register
 * cross cp_instance, BENCH.md "GPIOCOV markers" 2026-09-23).
 *
 * -----------------------------------------------------------------------
 * DATA-register post-reset read: confirmed-reproducible AHB bus wedge
 * (real-run finding, 2026-09-23) -- reported, not worked around
 * -----------------------------------------------------------------------
 * A plain rd32() of GPIO0's DATA register (offset 0x0000) in true
 * post-HRESETn state (before any GPIO register write of any kind, and
 * before any other GPIO register has ever been accessed) does not return:
 * two independent real VCS runs of this file were each killed after the
 * simv process had burned 15+ minutes of wall-clock at ~95-100% CPU with
 * zero further UART/ADP output past the DATA read call, on a bench where
 * every other GPIO register test (gpio_decode_tests, gpio_p0/p1_loopback_
 * tests, gpio_irq_tests, gpio_edge_irq_tests) completes in well under two
 * minutes. Run 1 read DATA first among the 13 (see git history of this
 * file); Run 2 moved DATA to be read LAST, after the other 12 documented
 * registers had already been read and asserted 0x0 successfully (all 12
 * printed their GPIOCOV marker with zero check_eq failures) -- DATA still
 * wedged, confirming the trigger is the DATA register address itself, not
 * read ordering. DATAOUT (offset 0x0004, read immediately before DATA in
 * Run 2) completed normally and read back 0x0 as expected -- the wedge is
 * specific to DATA, not a general DATA/DATAOUT-pair issue.
 *
 * This independently reproduces, in a new untested scenario (true
 * first-ever post-reset access), a hazard already flagged generically by a
 * prior test-agent pass: gpio_decode_tests.c's own header comment
 * (MEGASOC-ADDR-011) states "broad word probing of GPIO DATA/DATAOUT can
 * wedge the simulation before software can report a verdict" and
 * deliberately defers all DATA/DATAOUT access to a later test. No
 * currently-passing GPIO test in this bench issues a DATA read as the
 * first-ever GPIO0/GPIO1 access post-reset with zero prior register writes
 * -- gpio_p0_loopback_tests.c's check_loopback() only ever reads DATA
 * AFTER already writing OUTENCLR/DATAOUT first (gpio_save()/check_loopback()
 * write order), which is the one combination this test cannot exercise
 * without violating MEGASOC-GPIO-005's own "before any write" acceptance
 * criterion for DATA's reset value.
 *
 * Unlike MEGASOC-ILL-014/016's abort_handler() idiom, there is no software
 * safety net for this class of stall: an AHB HREADY-never-asserts wedge
 * traps nothing (no synchronous abort is ever raised -- the LDR simply
 * never retires), so it cannot be time-bounded from software at all (the
 * same honest limit already documented in this project for
 * gic_and_uart_boundary_read_hangs.md and the MEGASOC-IRQ-008 PL031 RTC
 * poll stall). Issuing this read unconditionally in the committed test
 * would hang this binary (and any regression run of it) forever, so
 * DATA is INTENTIONALLY EXCLUDED from the blocking-read loop below for
 * both GPIO0 and GPIO1 (symmetric precaution -- GPIO1 was not
 * independently re-confirmed given the wall-clock cost already spent on
 * GPIO0, but both instances wrap the identical vendor cmsdk_ahb_gpio IP
 * per megasoc_spec.md §5.11). No GPIOCOV:RESET:DATA:<inst> marker is
 * emitted for either instance -- emitting it without ever performing the
 * read would be a false coverage claim, not an honest gap. This is
 * reported as a genuine, reproducible integration-level finding requiring
 * RTL/waveform-level escalation (harness-agent/design-agent territory,
 * outside this test-agent's read boundary), not guessed at or routed
 * around with a weakened check.
 *============================================================================*/
struct gpio_reg_entry {
  unsigned long offset;
  const char   *name;       /* register token for [MEGASOC-GPIO-005] prints */
  const char   *cov_token;  /* exact GPIOCOV:RESET:<REGISTER> token, BENCH.md */
};

/* The 12 documented registers confirmed SAFE to read in true post-reset
 * state (DATA excluded -- see finding above). DATAOUT is read last among
 * these 12 since it is the other half of the originally-flagged
 * DATA/DATAOUT hazard pair and was only actually exercised (safely) in
 * this position during the real run that produced the finding above. */
static const struct gpio_reg_entry k_gpio_reset_regs[12] = {
  { GPIO_OUTENSET_OFF,   "OUTENSET",   "OUTENSET"   },
  { GPIO_OUTENCLR_OFF,   "OUTENCLR",   "OUTENCLR"   },
  { GPIO_ALTFUNCSET_OFF, "ALTFUNCSET", "ALTFUNCSET" },
  { GPIO_ALTFUNCCLR_OFF, "ALTFUNCCLR", "ALTFUNCCLR" },
  { GPIO_INTENSET_OFF,   "INTENSET",   "INTENSET"   },
  { GPIO_INTENCLR_OFF,   "INTENCLR",   "INTENCLR"   },
  { GPIO_INTTYPESET_OFF, "INTTYPESET", "INTTYPESET" },
  { GPIO_INTTYPECLR_OFF, "INTTYPECLR", "INTTYPECLR" },
  { GPIO_INTPOLSET_OFF,  "INTPOLSET",  "INTPOLSET"  },
  { GPIO_INTPOLCLR_OFF,  "INTPOLCLR",  "INTPOLCLR"  },
  { GPIO_INTSTATUS_OFF,  "INTSTATUS",  "INTSTATUS"  },
  { GPIO_DATAOUT_OFF,    "DATAOUT",    "DATAOUT"    },
};

static int gpio_reset_readback_probe(unsigned long base, const char *inst_tag)
{
  int errors = 0;
  const char *V = "MEGASOC-GPIO-005";
  unsigned int i;

  for (i = 0u; i < 12u; i++) {
    uint32_t v;
    char what[64];

    /* Diagnostic breadcrumb (short, printed BEFORE the access itself) --
     * lets a partial/killed run pinpoint exactly which register access was
     * in flight if forward progress stalls, without relying on a
     * possibly-slow-to-flush post-access message. Temporary instrumentation
     * for this file's own real-run debugging; harmless to leave in
     * permanently (single short line, same cost class as the GPIOCOV
     * marker already printed per iteration). */
    printf("RD:%s:%s\n", inst_tag, k_gpio_reset_regs[i].name);

    v = rd32(base + k_gpio_reset_regs[i].offset);

    /* Build "<REGISTER> <INST> reset readback" for the failure message,
     * without pulling in a heavier string lib than this project's other
     * software tests already use. */
    int p = 0;
    const char *r = k_gpio_reset_regs[i].name;
    while (*r && p < 40) { what[p++] = *r++; }
    what[p++] = ' ';
    { const char *t = inst_tag; while (*t && p < 55) { what[p++] = *t++; } }
    { const char *s = " reset readback"; while (*s && p < 63) { what[p++] = *s++; } }
    what[p] = '\0';

    /* Reset value is 0x0 for every register except GPIO1's alternate-function
     * register: MegaSoC instantiates GPIO1 with ALTERNATE_FUNC_DEFAULT =
     * 16'hFFFF (megasoc_peripheral_subsystem.sv) so the UART (P1[3:0]) and
     * EXTIO debug link (P1[14:8]) pads work from reset -- the boot ROM needs
     * them. ALTFUNCSET and ALTFUNCCLR both read back that register. */
    {
      unsigned long long expected = 0;
      if (base == GPIO_1_BASE &&
          (k_gpio_reset_regs[i].offset == GPIO_ALTFUNCSET_OFF ||
           k_gpio_reset_regs[i].offset == GPIO_ALTFUNCCLR_OFF))
        expected = 0xFFFFULL;
      errors += check_eq(V, what, expected, v);
    }

    printf("GPIOCOV:RESET:%s:%s\n", k_gpio_reset_regs[i].cov_token, inst_tag);
  }

  /* DATA register (2026-11-20). DATA has no reset value of its own: it reads
   * the synchronised input pads. The 09-23 "hang" was traced to pads that
   * nothing drove (no pull in megasoc_chip_pads.v or the bench), so DATA
   * returned X and the X then reached a compare/printf in software. The
   * bench now has weak pulldowns on every pin with no other driver
   * (megasoc_tb.sv: P0[15:0], P1[7:4], P1[15]), so those bits are a defined
   * 0 after reset. Only those bits are compared: GPIO1 P1[3:0] carries the
   * UART loop and P1[14:8] the EXTIO/ADP link, whose levels are set by
   * those blocks, not by GPIO reset. The raw value is never printed (it may
   * hold X on the masked-off GPIO1 bits). */
  {
    const uint32_t mask = (base == GPIO_1_BASE) ? 0x80F0u : 0xFFFFu;
    uint32_t v;
    printf("RD:%s:DATA\n", inst_tag);
    v = rd32(base + GPIO_DATA_OFF) & mask;
    errors += check_eq(V, (base == GPIO_1_BASE) ? "DATA GPIO1 pulled bits reset readback"
                                                : "DATA GPIO0 reset readback", 0, v);
    printf("GPIOCOV:RESET:DATA:%s\n", inst_tag);
  }

  return errors;
}

static int test_gpio005_reset_readback(void)
{
  int errors = 0;
  const char *V = "MEGASOC-GPIO-005";

  printf("[%s] GPIO0/GPIO1 post-HRESETn reset-readback check "
         "(first GPIO access this run, before any register write)\n", V);

  errors += gpio_reset_readback_probe(GPIO_0_BASE, "GPIO0");
  errors += gpio_reset_readback_probe(GPIO_1_BASE, "GPIO1");

  printf("[%s] errors: %d\n", V, errors);
  return errors;
}

/*==============================================================================
 * MEGASOC-ILL-016: gpio_reserved_offset_response_characterization
 * (blocked_on_vendor_doc for the expected HRESP/HREADYOUT/HRDATA VALUE only;
 *  bounded completion IS a real, asserted pass/fail check -- see file
 *  header)
 *
 * Reserved offset chosen from the 0x003C-0x03FC gap, just past INTSTATUS
 * (0x0038) and before the MASKLOWBYTE window (0x0400-0x07FC): 0x003C. A
 * second reserved offset from the 0x0C00-0x0FFF gap, just past the
 * MASKHIGHBYTE window (0x0800-0x0BFC): 0x0C00. Both gaps are exercised for
 * both GPIO0 and GPIO1 independently, matching the vplan's own "0x003C-
 * 0x03FC or 0x0C00-0x0FFF" text (either/both accepted; this test exercises
 * both for stronger characterization coverage).
 *============================================================================*/
#define GPIO_RESERVED_GAP1_OFFSET 0x003CUL /* just past INTSTATUS (0x0038) */
#define GPIO_RESERVED_GAP2_OFFSET 0x0C00UL /* just past MASKHIGHBYTE (0x0BFC) */

static int gpio_reserved_offset_probe(const char *inst_tag, unsigned long base,
                                       const char *gap_tag, unsigned long offset)
{
  int errors = 0;
  const char *V = "MEGASOC-ILL-016";
  uint32_t before = g_abort_count;
  uint32_t v;
  int trapped;

  /* Short lines only from here down -- a prior real run of this file hit a
   * multi-minute UART/ADP stall partway through a long, multi-clause
   * evidence-print literal on this exact code path (not correlated with
   * any specific %-conversion or data value; the stall began mid-literal-
   * text, before the %x HRDATA conversion was even reached). Every project
   * test proven reliable in this bench uses short single-purpose print
   * lines; this sub-test now matches that pattern instead of the original
   * long prose lines. */
  printf("RD:%s:%s\n", inst_tag, gap_tag);

  /* The read itself is the bounded-completion check: a single LDR either
   * returns control here (completion, PASS -- whether via a normal
   * response or a safely-recovered synchronous abort) or the CPU does not
   * return at all, which this software cannot itself time-bound further --
   * same honest limit already documented for MEGASOC-ILL-014 in
   * peripheral_reserved_offset_tests.c. Reaching the check_eq() call below
   * IS the assertable "no hang" evidence. */
  v = rd32(base + offset);

  trapped = (g_abort_count != before);

  /* Real, assertable pass/fail: the read completed (we are executing this
   * line at all). Encoded as a trivially-true self-comparison so the check
   * participates in the same check_eq()/errors accounting as every other
   * sub-test in this file -- same idiom as MEGASOC-ILL-014's
   * timer_reserved_probe(). */
  errors += check_eq(V, "reserved-offset access completed (bounded, no hang)",
                     1, 1);

  /* Evidence only -- NOT asserted, per A26/blocked_on_vendor_doc. HRESP/
   * HREADYOUT are independently, continuously recorded in hardware by
   * cg_gpio_reserved_offset_observed (megasoc_periph_reserved_offset_boundary.sv);
   * HRDATA has no documented wrapper wire (BENCH.md "HRDATA gap"), so this
   * printed rd32() return value is the ONLY place the HRDATA content for
   * this specific access gets recorded -- the documented software-side
   * closure of that evidence dimension (see file header). Short, separate
   * lines (not one long combined sentence) -- see the "Short lines only"
   * note above. */
  printf("OBS:%s:%s:trapped=%d\n", inst_tag, gap_tag, trapped);
  printf("OBS:%s:%s:hrdata=0x%08x\n", inst_tag, gap_tag, (unsigned)v);

  printf("ILLCOV:GPIO_RESERVED_CHARACTERIZE:%s:COMPLETED\n", inst_tag);
  return errors;
}

static int test_ill016_gpio_reserved_offset_characterize(void)
{
  int errors = 0;
  const char *V = "MEGASOC-ILL-016";

  printf("[%s] GPIO0/GPIO1 reserved-offset response characterization\n", V);

  errors += gpio_reserved_offset_probe("GPIO0", GPIO_0_BASE, "GAP1_0x003C-0x03FC",
                                        GPIO_RESERVED_GAP1_OFFSET);
  errors += gpio_reserved_offset_probe("GPIO0", GPIO_0_BASE, "GAP2_0x0C00-0x0FFF",
                                        GPIO_RESERVED_GAP2_OFFSET);
  errors += gpio_reserved_offset_probe("GPIO1", GPIO_1_BASE, "GAP1_0x003C-0x03FC",
                                        GPIO_RESERVED_GAP1_OFFSET);
  errors += gpio_reserved_offset_probe("GPIO1", GPIO_1_BASE, "GAP2_0x0C00-0x0FFF",
                                        GPIO_RESERVED_GAP2_OFFSET);

  printf("[%s] errors: %d\n", V, errors);
  return errors;
}

int main(void)
{
  int errors = 0;

  UartStdOutInit();

  printf("MegaSoC GPIO0/GPIO1 reset-readback + reserved-offset boundary test\n");
  printf("vplan: MEGASOC-GPIO-005, MEGASOC-ILL-016\n");

  /* MUST run first -- see file header ordering note (pre-write reset
   * state). */
  errors += test_gpio005_reset_readback();

  errors += test_ill016_gpio_reserved_offset_characterize();

  printf("Total number of gpio_reset_and_reserved_offset_tests errors: %d\n", errors);

  if (errors == 0) {
    printf("TEST PASSED\n");
    TEST_PASS();
  } else {
    printf("TEST FAILED\n");
    TEST_FAIL();
  }

  return 0;
}
