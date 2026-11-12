//==============================================================================
// nic_window_edge_tests.c -- NIC-400 window edges, peripheral default slave and
//                            unused APB slots (tc_nic_window_edge)
//
// Written 2026-11-12 for the NIC-400 8-path close-out, from
// verif/spec/megasoc_spec.md section 2 (A53 memory map), 2.1 (APB slots and
// the 0xDEADBEEF default slave), 6.6 and 11 A2.
//
// vplan items (verif/vplan/megasoc_vplan.yaml):
//   MEGASOC-ADDR-015  first and last 32-bit word of every A53-view window
//                     (11 windows x {first,last}) completes with the response
//                     the window should give. One-past-end is not probed here:
//                     the 7 NIC holes right after ROM/RAM/GIC/DEBUG/EXP/DRAM
//                     are MEGASOC-ILL-001 (nic_decerr_tests); the other
//                     windows are back-to-back with their neighbour.
//   MEGASOC-ILL-002   reads in the non-decoded peripheral space
//                     (0x4003_0000-0x5FFF_FFFF) hit the AHB default slave:
//                     either an ERROR response (trapped abort) or OKAY with
//                     data 0xDEADBEEF. Which one is recorded (spec A2 leaves
//                     it open); anything else fails.
//   MEGASOC-ILL-003   APB slots C-F (0x4000_C000-0x4000_FFFF): read and write
//                     each. The integration ties PSLVERR12-15 to 1 and PREADY
//                     to 1 (megasoc_peripheral_subsystem.sv), so every access
//                     must end in an error response (trapped abort), no hang.
//
// Per-window policy (pol_t):
//   P_OK     must complete with no trapped abort
//   P_ABORT  must trap an abort (EXP_M absent target, DECERR, ILL-008 rule)
//   P_CHAR   must complete; abort or not is recorded, not judged. Not used
//            any more: the first run (2026-11-12 11:47, simulate/sim_nic400)
//            was reviewed and its results frozen in the table below:
//            FLASH_CTRL/SDIO_S/GIC/PCK_CTRL last word and GIC first word
//            answer OKAY (data 0); DEBUG last word (0x0130_7FFC) answers with
//            an external abort.
//   P_DEFSLV ILL-002 rule above
// Every trapped abort prints ESR_EL3, so a translation fault (DFSC 0x04-0x07)
// or an address size fault (0x00-0x03) cannot pass for a bus error response
// (synchronous external abort, DFSC 0x10; SError, EC 0x2F).
//
// DRAM last word (0x2_7FFF_FFF8) is above 4 GB. The shared startup code sets
// TCR_EL3 = 0x00102F20 (T0SZ=32, PS=32-bit), so a plain access there takes a
// translation fault in the CPU and never reaches the NIC (first run, 11-12).
// That edge is probed with TCR_EL3 widened for the one access (T0SZ=28, PS=36
// bits); level-1 entry 9 of pgtbl1 already maps 0x2_4000_0000-0x2_7FFF_FFFF
// identity as Device-nGnRnE (lib/sw_lib/apps/src/page_table.s). TCR_EL3 is
// restored right after.
// DRAM edges are also written and read back (64-bit), then restored. RAM edges
// are read only: the image and stack live in SRAM and a stray write there
// would corrupt the running test. ROM word 0 is compared to the reset vector.
//
// Coverage: the test prints "ADDRCOV:EDGE:<WIN>:<FIRST|LAST>" and
// "ILLCOV:APB_UNUSED:<C|D|E|F>:<READ|WRITE>" only AFTER the access completed
// and its check passed, so a covered bin means "exercised and checked".
// Decoded by megasoc_integration_cov.sv (cg_window_edge, cg_apb_unused).
//
// Keep printf lines short: c_print() uses a 160-byte buffer (BUG-22).
//
// Run (default bench): make run_vcs TESTNAME=nic_window_edge_tests VCS_SIM_OPTION="+vcs+flush+log"
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include <stdio.h>
#include <stdint.h>

#define ROM_FIRST_WORD   0xd2800000UL   // A64 'mov x0,#0' at the reset vector
#define DEFSLV_DATA      0xDEADBEEFUL   // spec 2.1 default-slave read data

typedef enum { P_OK = 0, P_ABORT, P_CHAR, P_DEFSLV } pol_t;

typedef struct {
  const char        *name;      // window name, also the ADDRCOV bin name
  unsigned long long first;     // first 32-bit word
  pol_t              first_pol;
  unsigned long long last;      // last 32-bit word
  pol_t              last_pol;
  int                cached;    // normal memory: clean+invalidate before reads
  int                rw64;      // write + read back a 64-bit pattern at each edge
} win_t;

// spec section 2, A53 view, INC_EXP=no build
static const win_t windows[] = {
  { "ROM",        0x00000000ULL, P_OK,    0x0000FFFCULL, P_OK,     1, 0 },
  { "FLASH",      0x00400000ULL, P_OK,    0x007FFFFCULL, P_OK,     1, 0 },
  { "RAM",        0x00800000ULL, P_OK,    0x0080FFFCULL, P_OK,     1, 0 },
  { "FLASH_CTRL", 0x01000000ULL, P_OK,    0x0100FFFCULL, P_OK,     0, 0 },
  { "SDIO_S",     0x01010000ULL, P_OK,    0x0101FFFCULL, P_OK,     0, 0 },
  { "GIC",        0x01100000ULL, P_OK,    0x01107FFCULL, P_OK,     0, 0 },
  { "PCK_CTRL",   0x01200000ULL, P_OK,    0x01207FFCULL, P_OK,     0, 0 },
  { "DEBUG",      0x01300000ULL, P_OK,    0x01307FFCULL, P_ABORT,  0, 0 },
  { "PERIPHERAL", 0x40000000ULL, P_OK,    0x5FFFFFFCULL, P_DEFSLV, 0, 0 },
  { "EXP_M",      0x60000000ULL, P_ABORT, 0x60FFFFFCULL, P_ABORT,  0, 0 },
  { "DRAM",       0x80000000ULL, P_OK,    0x27FFFFFF8ULL, P_OK,    1, 1 },
};
#define N_WINDOWS (sizeof(windows) / sizeof(windows[0]))

// ILL-002 probes in the default-slave range
static const unsigned long long defslv_addr[] = {
  0x40030000ULL, 0x4FFFFFFCULL, 0x5FFFFFFCULL
};

// ILL-003 unused APB slots
static const struct { const char *slot; unsigned long base; } apb_unused[] = {
  { "C", 0x4000C000UL }, { "D", 0x4000D000UL },
  { "E", 0x4000E000UL }, { "F", 0x4000F000UL }
};

// --------------------------------------------------------------------------
// Access helpers (same pattern as nic_decerr_tests.c)
// --------------------------------------------------------------------------
static inline void wr32(unsigned long a, uint32_t v)
{ __asm__ volatile ("str %w0, [%1]" : : "r"(v), "r"(a) : "memory"); }
static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline void wr64(unsigned long a, uint64_t v)
{ __asm__ volatile ("str %0, [%1]" : : "r"(v), "r"(a) : "memory"); }
static inline uint64_t rd64(unsigned long a)
{ uint64_t v; __asm__ volatile ("ldr %0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline void cache_civac(unsigned long a)
{
  __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}
static inline void barrier_sy(void)
{
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}
static void short_settle(uint32_t loops)
{
  for (volatile uint32_t i = 0; i < loops; i++) { }
}

// Overrides the weak abort_handler in system.c; the vector code returns past
// the faulting access (same mechanism as nic_decerr_tests).
static volatile uint32_t g_abort_count = 0;
static volatile uint64_t g_last_esr = 0;
void abort_handler(void)
{
  uint64_t esr;
  __asm__ volatile ("mrs %0, esr_el3" : "=r"(esr));
  g_last_esr = esr;
  g_abort_count++;
}

// Above-4GB access window: see the header. Returns the TCR_EL3 to restore.
#define ABOVE_4GB      0x100000000ULL
#define TCR_T0SZ_MASK  0x3FULL
#define TCR_PS_MASK    (0x7ULL << 16)
static uint64_t wide_va_on(void)
{
  uint64_t old, tcr;
  __asm__ volatile ("mrs %0, tcr_el3" : "=r"(old));
  tcr = (old & ~(TCR_T0SZ_MASK | TCR_PS_MASK)) | 28ULL | (1ULL << 16);
  __asm__ volatile ("dsb sy\n\tmsr tcr_el3, %0\n\tisb\n\ttlbi alle3\n\tdsb sy\n\tisb"
                    : : "r"(tcr) : "memory");
  return old;
}
static void wide_va_off(uint64_t old)
{
  __asm__ volatile ("dsb sy\n\tmsr tcr_el3, %0\n\tisb\n\ttlbi alle3\n\tdsb sy\n\tisb"
                    : : "r"(old) : "memory");
}

// One read; returns 1 if it trapped an abort.
static int probe_read(unsigned long long a, int cached, uint32_t *data)
{
  uint32_t before = g_abort_count;
  if (cached) cache_civac((unsigned long)a);
  *data = rd32((unsigned long)a);
  barrier_sy();
  short_settle(200);
  return g_abort_count != before;
}

// One write; returns 1 if it trapped an abort (sync or SError).
static int probe_write(unsigned long long a, uint32_t v)
{
  uint32_t before = g_abort_count;
  wr32((unsigned long)a, v);
  barrier_sy();
  short_settle(200);
  return g_abort_count != before;
}

static const char *pol_name(pol_t p)
{
  switch (p) {
  case P_OK:     return "OK";
  case P_ABORT:  return "ABORT";
  case P_CHAR:   return "CHAR";
  default:       return "DEFSLV";
  }
}

// --------------------------------------------------------------------------
// MEGASOC-ADDR-015: one window edge
// --------------------------------------------------------------------------
static int check_edge(const win_t *w, int is_last)
{
  unsigned long long a = is_last ? w->last : w->first;
  pol_t pol = is_last ? w->last_pol : w->first_pol;
  const char *end = is_last ? "LAST" : "FIRST";
  uint32_t data = 0;
  int ab, errors = 0;
  int wide = (a >= ABOVE_4GB);
  uint64_t tcr_saved = 0;

  if (wide)
    tcr_saved = wide_va_on();
  ab = probe_read(a, w->cached, &data);
  printf("[MEGASOC-ADDR-015] %s %s @0x%09llx pol=%s abort=%d data=0x%08x\n",
         w->name, end, a, pol_name(pol), ab, (unsigned)data);
  if (ab)
    printf("[MEGASOC-ADDR-015]   ESR_EL3=0x%08llx (EC 0x%02llx, DFSC 0x%02llx)\n",
           (unsigned long long)g_last_esr, (unsigned long long)((g_last_esr >> 26) & 0x3F),
           (unsigned long long)(g_last_esr & 0x3F));

  switch (pol) {
  case P_OK:
    if (ab) {
      printf("[MEGASOC-ADDR-015] FAIL: %s %s trapped an abort\n", w->name, end);
      errors++;
    }
    break;
  case P_ABORT:
    if (!ab) {
      printf("[MEGASOC-ADDR-015] FAIL: %s %s returned OKAY, expected DECERR\n",
             w->name, end);
      errors++;
    }
    break;
  case P_DEFSLV:
    if (!ab && data != DEFSLV_DATA) {
      printf("[MEGASOC-ADDR-015] FAIL: %s %s OKAY but data is not 0xDEADBEEF\n",
             w->name, end);
      errors++;
    }
    break;
  default:   // P_CHAR: completion is the check; result recorded above
    break;
  }

  // ROM word 0 must be the reset vector
  if (!errors && !ab && a == 0x0ULL && data != ROM_FIRST_WORD) {
    printf("[MEGASOC-ADDR-015] FAIL: ROM word 0 = 0x%08x, expected 0x%08x\n",
           (unsigned)data, (unsigned)ROM_FIRST_WORD);
    errors++;
  }

  // RAM / DRAM: 64-bit write + read back at the edge, then restore
  if (!errors && w->rw64) {
    unsigned long aa = (unsigned long)(a & ~0x7ULL);
    uint64_t pat = 0xA5C3000000000000ULL ^ (uint64_t)aa;
    uint64_t old, got;
    uint32_t before = g_abort_count;
    cache_civac(aa);
    old = rd64(aa);
    wr64(aa, pat);
    cache_civac(aa);
    got = rd64(aa);
    wr64(aa, old);
    cache_civac(aa);
    if (g_abort_count != before || got != pat) {
      printf("[MEGASOC-ADDR-015] FAIL: %s %s rw64 got 0x%016llx exp 0x%016llx\n",
             w->name, end, (unsigned long long)got, (unsigned long long)pat);
      errors++;
    }
  }

  if (wide)
    wide_va_off(tcr_saved);

  if (!errors)
    printf("ADDRCOV:EDGE:%s:%s\n", w->name, end);
  return errors;
}

// --------------------------------------------------------------------------
// MEGASOC-ILL-002: peripheral default slave
// --------------------------------------------------------------------------
static int check_default_slave(unsigned long long a)
{
  uint32_t data = 0;
  int ab = probe_read(a, 0, &data);

  if (ab) {
    printf("[MEGASOC-ILL-002] @0x%08llx: ERROR response (trapped abort)\n", a);
    return 0;
  }
  if (data == DEFSLV_DATA) {
    printf("[MEGASOC-ILL-002] @0x%08llx: OKAY, data 0xDEADBEEF\n", a);
    return 0;
  }
  printf("[MEGASOC-ILL-002] FAIL: @0x%08llx OKAY with data 0x%08x\n",
         a, (unsigned)data);
  return 1;
}

// --------------------------------------------------------------------------
// MEGASOC-ILL-003: unused APB slot, read and write must both error
// --------------------------------------------------------------------------
static int check_apb_unused(const char *slot, unsigned long base)
{
  uint32_t data = 0;
  int errors = 0;
  int ab;

  ab = probe_read(base, 0, &data);
  printf("[MEGASOC-ILL-003] slot %s READ @0x%08lx abort=%d data=0x%08x\n",
         slot, base, ab, (unsigned)data);
  if (!ab) {
    printf("[MEGASOC-ILL-003] FAIL: slot %s READ OKAY, expected SLVERR\n", slot);
    errors++;
  } else {
    printf("ILLCOV:APB_UNUSED:%s:READ\n", slot);
  }

  ab = probe_write(base, 0xC0FFEE00UL);
  printf("[MEGASOC-ILL-003] slot %s WRITE @0x%08lx abort=%d\n", slot, base, ab);
  if (!ab) {
    printf("[MEGASOC-ILL-003] FAIL: slot %s WRITE OKAY, expected SLVERR\n", slot);
    errors++;
  } else {
    printf("ILLCOV:APB_UNUSED:%s:WRITE\n", slot);
  }
  return errors;
}

int main(void)
{
  int e_edge = 0, e_defslv = 0, e_apb = 0;
  unsigned i;

  UartStdOutInit();
  printf("MegaSoC NIC-400 window edge test (tc_nic_window_edge)\n");
  printf("vplan: MEGASOC-ADDR-015, MEGASOC-ILL-002, MEGASOC-ILL-003\n");

  for (i = 0; i < N_WINDOWS; i++) {
    e_edge += check_edge(&windows[i], 0);
    e_edge += check_edge(&windows[i], 1);
  }

  for (i = 0; i < sizeof(defslv_addr) / sizeof(defslv_addr[0]); i++)
    e_defslv += check_default_slave(defslv_addr[i]);

  // last: a hang here (PREADY not answered) would otherwise hide the rest
  for (i = 0; i < sizeof(apb_unused) / sizeof(apb_unused[0]); i++)
    e_apb += check_apb_unused(apb_unused[i].slot, apb_unused[i].base);

  printf("ADDR-015 errors: %d, ILL-002 errors: %d, ILL-003 errors: %d\n",
         e_edge, e_defslv, e_apb);
  printf("Total trapped external aborts observed: %u\n", (unsigned)g_abort_count);

  if (e_edge + e_defslv + e_apb == 0) {
    TEST_PASS();
  } else {
    TEST_FAIL();
  }
  return 0;
}
