//==============================================================================
// rom_image_integrity_tests.c -- MegaSoC Boot ROM 8-path close-out
//                                (tc_rom_image_integrity)
//
// Written 2026-11-10 from verif/spec/megasoc_spec.md section 6.1.1 and the
// vplan items below. Black-box: software-visible behaviour at CPU_AXI only.
// The DATA ORACLE for the whole-window compare is the bench checker
// verif/testbench/megasoc/megasoc_rom_boundary_checker.sv, which compares
// every ROM_AXI read beat against system/src/bootrom/bintxt/bootrom.bintxt.
// Run with +rom_check, otherwise only the software-side checks below apply:
//
//   make run_vcs TESTNAME=rom_image_integrity_tests VCS_SIM_OPTION="+vcs+flush+log +rom_check"
//
// vplan items:
//   MEGASOC-BOOT-008 rom_full_image_integrity
//     Reads every 64-bit word of 0x0000-0xFFF8 through the CPU. Software
//     checks: no abort, word 0 low half == 0xd2800000, and the top line read
//     twice is stable. Image size and checksums are printed, never compared
//     (they are build-specific). Bench: per-beat compare vs bootrom.bintxt,
//     image and zero-fill regions, 8192/8192 dwords seen.
//   MEGASOC-BOOT-018 rom_window_end_bursts
//     Accesses at the top of the window that make the CPU issue multi-beat
//     bursts ending at 0xFFFF (line fill of 0xFFC0, LDP of 0xFFF0) and a
//     critical-word-first (WRAP) line fill in the last 4 KB. Software
//     compares each result with the sweep. Bench: no burst crosses
//     0x1_0000 or 4 KB; burst types are counted. Whether WRAP actually
//     appears depends on the ROM page attributes (cacheable or not); the
//     bench summary records what was seen.
//   MEGASOC-BOOT-013 rom_one_past_end_decerr
//     Read and write at 0x0001_0000 must abort (DECERR from the NIC default
//     slave, same abort-count method as nic_decerr_tests). A read at 0xFFF8
//     right after must still succeed with the same data. Bench: ROM_AXI sees
//     no AR/AW at or above 0x1_0000.
//   MEGASOC-BOOT-007 rom_write_response_characterization (CPU leg only)
//     Writes to 0x0, the last non-zero image dword, and 0xFFF8, each
//     followed by a cache clean so the write leaves the CPU. Pass/fail: the
//     ROM content read back afterwards is unchanged. Characterisation: abort
//     count and ESR_EL3 per write (a permission fault means the MMU blocked
//     the write before the bus). Bench: one B per AW, BRESP recorded.
//     The ADP and SDIO DMA legs are not covered by this test.
//     The writes run LAST so a hang there leaves the earlier results logged.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include <stdio.h>
#include <stdint.h>

#define ROM_WIN_BASE    0x00000000UL
#define ROM_TOP_LINE    0x0000FFC0UL
#define ROM_TOP_DWORD   0x0000FFF8UL
#define ROM_ONE_PAST    0x00010000UL
#define ROM_FIRST_WORD  0xd2800000UL     // spec 6.1 step 3: A64 'mov x0,#0'
#define ROM_WRAP_LINE   0x0000F040UL     // a line inside the last 4 KB
#define LINE_BYTES      64UL

static inline uint64_t rd64(unsigned long a)
{ uint64_t v; __asm__ volatile ("ldr %0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline void wr64(unsigned long a, uint64_t v)
{ __asm__ volatile ("str %0, [%1]" : : "r"(v), "r"(a) : "memory"); }
static inline void wr32(unsigned long a, uint32_t v)
{ __asm__ volatile ("str %w0, [%1]" : : "r"(v), "r"(a) : "memory"); }
static inline void rd_pair(unsigned long a, uint64_t *lo, uint64_t *hi)
{
  uint64_t x, y;
  __asm__ volatile ("ldp %0, %1, [%2]" : "=&r"(x), "=&r"(y) : "r"(a) : "memory");
  *lo = x; *hi = y;
}

static inline void barrier_sy(void)
{
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}

// Clean+invalidate one line to the point of coherency so the next access
// really crosses CPU_AXI (valid whether or not the region is cacheable).
static inline void cache_civac(unsigned long a)
{
  __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
  barrier_sy();
}

static void short_settle(uint32_t loops)
{
  for (volatile uint32_t i = 0; i < loops; i++) { }
}

// --------------------------------------------------------------------------
// abort_handler() override (strong symbol, same mechanism as
// nic_decerr_tests.c): counts trapped aborts and records ESR_EL3; the
// library handler advances ELR_EL3 by 4 and returns.
// --------------------------------------------------------------------------
static volatile uint32_t g_abort_count = 0;
static volatile uint64_t g_last_esr = 0;

void abort_handler(void)
{
  uint64_t esr;
  __asm__ volatile ("mrs %0, esr_el3" : "=r"(esr));
  g_last_esr = esr;
  g_abort_count++;
}

static int check_u64(const char *id, const char *what, uint64_t exp, uint64_t got)
{
  if (exp != got) {
    printf("[%s] %s: expected 0x%016llx got 0x%016llx\n", id, what,
           (unsigned long long)exp, (unsigned long long)got);
    return 1;
  }
  return 0;
}

static int check_no_abort(const char *id, const char *what, uint32_t before)
{
  if (g_abort_count != before) {
    printf("[%s] %s: unexpected abort (count %u -> %u, ESR_EL3 0x%llx)\n",
           id, what, (unsigned)before, (unsigned)g_abort_count,
           (unsigned long long)g_last_esr);
    return 1;
  }
  return 0;
}

// Values kept from the sweep for later comparisons.
static uint64_t g_top[8];          // 0xFFC0..0xFFF8
static uint64_t g_wrapline[8];     // ROM_WRAP_LINE..+0x38
static unsigned long g_last_nz = 0;

// --------------------------------------------------------------------------
// MEGASOC-BOOT-008: whole-window sweep
// --------------------------------------------------------------------------
static int test_full_window_sweep(void)
{
  int errors = 0;
  uint32_t before = g_abort_count;
  uint64_t x = 0, s = 0, v;
  unsigned long a, line;
  unsigned nz = 0;

  printf("Test: MEGASOC-BOOT-008 whole ROM window sweep 0x0000-0xFFF8\n");

  for (line = ROM_WIN_BASE; line <= ROM_TOP_LINE; line += LINE_BYTES) {
    cache_civac(line);
    for (a = line; a < line + LINE_BYTES; a += 8) {
      v = rd64(a);
      x ^= v;
      s += v;
      if (v != 0) { g_last_nz = a; nz++; }
      if (line == ROM_TOP_LINE)  g_top[(a - line) / 8] = v;
      if (line == ROM_WRAP_LINE) g_wrapline[(a - line) / 8] = v;
    }
  }
  errors += check_no_abort("MEGASOC-BOOT-008", "sweep", before);

  cache_civac(ROM_WIN_BASE);
  errors += check_u64("MEGASOC-BOOT-008", "ROM word @0x0 (low half)",
                      ROM_FIRST_WORD, rd32(ROM_WIN_BASE));

  // The top line must read back the same on a second pass.
  cache_civac(ROM_TOP_LINE);
  for (a = 0; a < 8; a++)
    errors += check_u64("MEGASOC-BOOT-008", "top line re-read",
                        g_top[a], rd64(ROM_TOP_LINE + a * 8));

  // Kept short: c_print() formats into a 160-byte stack buffer (BUG-22/BUG-32).
  printf("[MEGASOC-BOOT-008] sweep done: 8192 dwords, %u non-zero\n", nz);
  printf("[MEGASOC-BOOT-008] last non-zero dword @0x%05lx, image ends at 0x%05lx\n",
         g_last_nz, g_last_nz + 8);
  printf("[MEGASOC-BOOT-008] XOR 0x%016llx SUM 0x%016llx (not compared)\n",
         (unsigned long long)x, (unsigned long long)s);
  printf("[MEGASOC-BOOT-008] word-by-word compare vs bootrom.bintxt is done by the bench (+rom_check)\n");
  return errors;
}

// --------------------------------------------------------------------------
// MEGASOC-BOOT-018: bursts at the top of the window
// --------------------------------------------------------------------------
static int test_window_end_bursts(void)
{
  int errors = 0;
  uint32_t before = g_abort_count;
  uint64_t lo, hi;

  printf("Test: MEGASOC-BOOT-018 bursts at the top of the ROM window\n");

  // (a) line-aligned access to the last line: a cacheable region gives a
  //     line fill that ends exactly at 0xFFFF.
  cache_civac(ROM_TOP_LINE);
  errors += check_u64("MEGASOC-BOOT-018", "line fill @0xFFC0", g_top[0], rd64(ROM_TOP_LINE));

  // (b) critical-word-first access inside the last line (WRAP if cacheable).
  cache_civac(ROM_TOP_LINE);
  errors += check_u64("MEGASOC-BOOT-018", "critical word @0xFFE8", g_top[5], rd64(0xFFE8UL));
  errors += check_u64("MEGASOC-BOOT-018", "same line @0xFFC8 after wrap fill", g_top[1], rd64(0xFFC8UL));

  // (c) 16-byte LDP of the last two dwords: a 2-beat burst on the 64-bit
  //     ROM_AXI ending at 0xFFFF when uncached.
  cache_civac(ROM_TOP_LINE);
  rd_pair(0xFFF0UL, &lo, &hi);
  errors += check_u64("MEGASOC-BOOT-018", "LDP @0xFFF0 low",  g_top[6], lo);
  errors += check_u64("MEGASOC-BOOT-018", "LDP @0xFFF0 high", g_top[7], hi);

  // (d) critical-word-first access in a line inside the last 4 KB.
  cache_civac(ROM_WRAP_LINE);
  errors += check_u64("MEGASOC-BOOT-018", "critical word @0xF068", g_wrapline[5], rd64(ROM_WRAP_LINE + 0x28));
  errors += check_u64("MEGASOC-BOOT-018", "same line @0xF040", g_wrapline[0], rd64(ROM_WRAP_LINE));

  errors += check_no_abort("MEGASOC-BOOT-018", "window-end bursts", before);
  printf("[MEGASOC-BOOT-018] burst types and 4 KB / 0x1_0000 boundary checks: see bench MEGASOC_ROM_CHECK SUMMARY\n");
  return errors;
}

// --------------------------------------------------------------------------
// MEGASOC-BOOT-013: one past the end of the window
// --------------------------------------------------------------------------
static int test_one_past_end(void)
{
  int errors = 0;
  uint32_t before;
  uint32_t v;

  printf("Test: MEGASOC-BOOT-013 one-past-end access @0x%08lx\n", ROM_ONE_PAST);

  // Positive control: a legal ROM read does not abort (validates the method).
  before = g_abort_count;
  cache_civac(ROM_WIN_BASE);
  (void)rd32(ROM_WIN_BASE);
  barrier_sy();
  if (check_no_abort("MEGASOC-BOOT-013", "positive control read @0x0", before)) {
    printf("[MEGASOC-BOOT-013] abort-count method invalid, one-past-end result not trustworthy\n");
    errors++;
  }

  before = g_abort_count;
  v = rd32(ROM_ONE_PAST);
  barrier_sy();
  short_settle(200);
  if (g_abort_count == before) {
    printf("[MEGASOC-BOOT-013] READ @0x%08lx: no abort, returned 0x%08x (expected DECERR)\n", ROM_ONE_PAST, (unsigned)v);
    errors++;
  } else {
    printf("[MEGASOC-BOOT-013] READ @0x%08lx: abort trapped, ESR_EL3 0x%llx, OK\n",
           ROM_ONE_PAST, (unsigned long long)g_last_esr);
  }

  before = g_abort_count;
  wr32(ROM_ONE_PAST, 0xDEADC0DEUL);
  barrier_sy();
  short_settle(200);
  if (g_abort_count == before) {
    printf("[MEGASOC-BOOT-013] WRITE @0x%08lx: no abort (expected DECERR)\n", ROM_ONE_PAST);
    errors++;
  } else {
    printf("[MEGASOC-BOOT-013] WRITE @0x%08lx: abort trapped, ESR_EL3 0x%llx, OK\n",
           ROM_ONE_PAST, (unsigned long long)g_last_esr);
  }

  // The last legal dword still reads OKAY with the same data.
  before = g_abort_count;
  cache_civac(ROM_TOP_LINE);
  errors += check_u64("MEGASOC-BOOT-013", "last legal dword @0xFFF8 after one-past-end",
                      g_top[7], rd64(ROM_TOP_DWORD));
  errors += check_no_abort("MEGASOC-BOOT-013", "read @0xFFF8", before);
  return errors;
}

// --------------------------------------------------------------------------
// MEGASOC-BOOT-007 (CPU leg): writes to the ROM window
// --------------------------------------------------------------------------
static int rom_write_one(unsigned long a)
{
  int errors = 0;
  uint32_t before;
  uint64_t pre, post;

  cache_civac(a & ~(LINE_BYTES - 1));
  pre = rd64(a);

  before = g_abort_count;
  printf("[MEGASOC-BOOT-007] CPU write 0x%016llx @0x%05lx ...\n",
         (unsigned long long)~pre, a);
  wr64(a, ~pre);
  cache_civac(a & ~(LINE_BYTES - 1));   // push a cached write out to the bus
  short_settle(400);
  if (g_abort_count != before)
    printf("[MEGASOC-BOOT-007]   write @0x%05lx: %u abort(s), last ESR_EL3 0x%llx (EC 0x%02llx, ISS 0x%llx)\n",
           a, (unsigned)(g_abort_count - before), (unsigned long long)g_last_esr,
           (unsigned long long)((g_last_esr >> 26) & 0x3F),
           (unsigned long long)(g_last_esr & 0x1FFFFFF));
  else
    printf("[MEGASOC-BOOT-007]   write @0x%05lx: no abort\n", a);

  cache_civac(a & ~(LINE_BYTES - 1));
  post = rd64(a);
  errors += check_u64("MEGASOC-BOOT-007", "ROM content unchanged after write", pre, post);
  return errors;
}

static int test_rom_writes(void)
{
  int errors = 0;
  printf("Test: MEGASOC-BOOT-007 CPU writes to the ROM window (content must not change)\n");
  errors += rom_write_one(ROM_WIN_BASE);
  if (g_last_nz != ROM_WIN_BASE && g_last_nz != ROM_TOP_DWORD)
    errors += rom_write_one(g_last_nz);
  errors += rom_write_one(ROM_TOP_DWORD);
  printf("[MEGASOC-BOOT-007] AW/B pairing and BRESP codes: see bench MEGASOC_ROM_CHECK SUMMARY\n");
  return errors;
}

// --------------------------------------------------------------------------
int main(void)
{
  int errors = 0;

  UartStdOutInit();

  printf("MegaSoC Boot ROM image integrity test (tc_rom_image_integrity)\n");
  printf("vplan: MEGASOC-BOOT-008, MEGASOC-BOOT-018, MEGASOC-BOOT-013, MEGASOC-BOOT-007 (CPU leg)\n");

  errors += test_full_window_sweep();
  errors += test_window_end_bursts();
  errors += test_one_past_end();
  printf("Errors before ROM writes: %d\n", errors);
  errors += test_rom_writes();

  printf("Total number of rom image integrity errors: %d\n", errors);
  printf("Total trapped aborts: %u\n", (unsigned)g_abort_count);

  if (errors == 0) {
    TEST_PASS();
  } else {
    TEST_FAIL();
  }
  return 0;
}
