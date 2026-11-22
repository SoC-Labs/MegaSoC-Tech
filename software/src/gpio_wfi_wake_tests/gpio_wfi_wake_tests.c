//==============================================================================
// gpio_wfi_wake_tests.c -- MegaSoC GPIO interrupt wakes the CPU from WFI
//                          (tc_gpio_wfi_wake)
//
// vplan: MEGASOC-GPIO-008 (gpio_irq_wakes_cpu_from_wfi)
//
// The pin edge must arrive while the core is asleep, so software cannot
// make it (every bench loopback is software-driven). The bench does:
//   1. software configures P0[15] as a rising-edge GPIO0 interrupt input
//      (P0[7], its tran partner, also left as an input), installs a real
//      handler on SPI 37 (GPIO0 combined), unmasks PSTATE.I, prints
//      "GPIOWAKE:ARM:P0_15" and enters WFI;
//   2. megasoc_gpio_clk_wake_checker (bench) waits until the Cortex-A53
//      STANDBYWFI output has been high for a while, drives P0[15] high, and
//      checks the core leaves WFI;
//   3. software, back from WFI, checks the handler ran once with SPI 37 and
//      cleared INTSTATUS, then prints "GPIOWAKE:DONE"; the bench releases the
//      pin and prints MEGASOC_GPIO_WAKE: RESULT=PASS|FAIL. A bench failure
//      prints MEGASOC_GPIO_ASSERT:, which fails the run in the regression parser.
//
// Software-side checks carry [MEGASOC-GPIO-008]. Knobs: TEST_VERBOSITY=1,
// IRQ_WAIT_MAX_ITERS=<n>. printf lines kept short (BUG-22).
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "gic400.h"
#include "irq.h"
#include <stdio.h>
#include <stdint.h>

#define V "MEGASOC-GPIO-008"

#define GPIO_DATAOUT_OFF     0x004UL
#define GPIO_OUTENCLR_OFF    0x014UL
#define GPIO_INTENSET_OFF    0x020UL
#define GPIO_INTENCLR_OFF    0x024UL
#define GPIO_INTTYPESET_OFF  0x028UL
#define GPIO_INTTYPECLR_OFF  0x02CUL
#define GPIO_INTPOLSET_OFF   0x030UL
#define GPIO_INTPOLCLR_OFF   0x034UL
#define GPIO_INTSTATUS_OFF   0x038UL

#define GICD_BASE GIC_DISTRIBUTOR_BASE
#define GICC_BASE GIC_CPU_INTERFACE_BASE
#define GICD_CTLR_OFF        0x000UL
#define GICD_ISENABLER_OFF   0x100UL
#define GICD_ICENABLER_OFF   0x180UL
#define GICD_ISPENDR_OFF     0x200UL
#define GICD_ICPENDR_OFF     0x280UL
#define GICD_ISACTIVER_OFF   0x300UL
#define GICD_ICACTIVER_OFF   0x380UL
#define GICC_CTLR_OFF        0x00UL
#define GICC_PMR_OFF         0x04UL
#define CTLR_ENABLE_BIT      0x1UL

#define WAKE_IN_BIT          0x8000u     // P0[15]
#define WAKE_PARTNER_BIT     0x0080u     // P0[7] (tran P0[7]<->P0[15])

#ifndef IRQ_WAIT_MAX_ITERS
#define IRQ_WAIT_MAX_ITERS 64UL          // WFI exits; expect 1
#endif
#define RUNAWAY_LIMIT 4
#define CLEAR_POLL_MAX 1000U

static inline uint32_t rd32(unsigned long a)
{ uint32_t v; __asm__ volatile ("ldr %w0, [%1]" : "=r"(v) : "r"(a) : "memory"); return v; }
static inline void wr32(unsigned long a, uint32_t v)
{ __asm__ volatile ("str %w0, [%1]" : : "r"(v), "r"(a) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory"); }
static inline uint64_t read_daif(void)
{ uint64_t v; __asm__ volatile ("mrs %0, daif" : "=r"(v) :: "memory"); return v; }

static inline unsigned long gicd_1bpi_addr(unsigned long off, unsigned int id)
{ return GICD_BASE + off + ((unsigned long)(id / 32U) * 4UL); }
static inline uint32_t gicd_read_bit(unsigned long off, unsigned int id)
{ return (rd32(gicd_1bpi_addr(off, id)) >> (id % 32U)) & 0x1UL; }
static inline void gicd_set_bit(unsigned long off, unsigned int id)
{ wr32(gicd_1bpi_addr(off, id), 1UL << (id % 32U)); }

static void gpio_settle(void)
{
  volatile uint32_t i;
  for (i = 0u; i < 64u; i++) { __asm__ volatile ("dsb sy" ::: "memory"); }
}

static int check_eq(const char *what, unsigned long long exp, unsigned long long got)
{
  if (exp != got) {
    printf("[%s] %s: exp 0x%llx got 0x%llx\n", V, what, exp, got);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] %s: OK 0x%llx\n", V, what, got);
#endif
  return 0;
}

static volatile int      g_calls;
static volatile int      g_id;
static volatile int      g_active_in_handler;
static volatile uint32_t g_intstatus_on_entry;
static volatile uint32_t g_intstatus_after_clear;
static volatile int      g_sync_abort_count;

static void wake_handler(int num, int source)
{
  unsigned int n;
  (void)source;
  g_calls++;
  g_id = num;
  g_active_in_handler = (int)gicd_read_bit(GICD_ISACTIVER_OFF, (unsigned int)num);
  g_intstatus_on_entry = rd32(GPIO_0_BASE + GPIO_INTSTATUS_OFF);
  // edge-latched; the bench keeps P0[15] high until DONE, so a level-mode
  // config would re-fire -- edge mode + W1C is what makes one entry enough
  wr32(GPIO_0_BASE + GPIO_INTSTATUS_OFF, WAKE_IN_BIT);
  for (n = 0; n < CLEAR_POLL_MAX; n++)
    if (!(rd32(GPIO_0_BASE + GPIO_INTSTATUS_OFF) & WAKE_IN_BIT)) break;
  gpio_settle();
  g_intstatus_after_clear = rd32(GPIO_0_BASE + GPIO_INTSTATUS_OFF) & WAKE_IN_BIT;
  if (g_calls >= RUNAWAY_LIMIT)
    gicd_set_bit(GICD_ICENABLER_OFF, (unsigned int)num);
}

void abort_handler(void) { g_sync_abort_count++; }

static void gpio_quiesce(void)
{
  const uint32_t m = WAKE_IN_BIT | WAKE_PARTNER_BIT;
  wr32(GPIO_0_BASE + GPIO_INTENCLR_OFF,   m);
  wr32(GPIO_0_BASE + GPIO_OUTENCLR_OFF,   m);   // both pins inputs: bench drives P0[15]
  wr32(GPIO_0_BASE + GPIO_DATAOUT_OFF,    rd32(GPIO_0_BASE + GPIO_DATAOUT_OFF) & ~m);
  wr32(GPIO_0_BASE + GPIO_INTTYPECLR_OFF, m);
  wr32(GPIO_0_BASE + GPIO_INTPOLCLR_OFF,  m);
  gpio_settle();
  wr32(GPIO_0_BASE + GPIO_INTSTATUS_OFF,  m);
}

int main(void)
{
  int e = 0;
  const unsigned int spi = GPIO0_COMB_INTR;
  unsigned long wfi_exits;
  uint64_t daif0;
  uint32_t sv_en, sv_pmr, sv_dct, sv_cct;

  UartStdOutInit();
  printf("MegaSoC GPIO interrupt wakes CPU from WFI (tc_gpio_wfi_wake)\n");
  printf("vplan: %s\n", V);
  daif0 = read_daif();
  e += check_eq("GPIO0_COMB_INTR == 37", 37, spi);

  sv_en  = gicd_read_bit(GICD_ISENABLER_OFF, spi);
  sv_pmr = rd32(GICC_BASE + GICC_PMR_OFF);
  sv_dct = rd32(GICD_BASE + GICD_CTLR_OFF);
  sv_cct = rd32(GICC_BASE + GICC_CTLR_OFF);

  // GPIO0 P0[15]: rising-edge interrupt input, idle low (bench pulldown)
  gpio_quiesce();
  wr32(GPIO_0_BASE + GPIO_INTTYPESET_OFF, WAKE_IN_BIT);
  wr32(GPIO_0_BASE + GPIO_INTPOLSET_OFF,  WAKE_IN_BIT);
  wr32(GPIO_0_BASE + GPIO_INTSTATUS_OFF,  WAKE_IN_BIT);
  wr32(GPIO_0_BASE + GPIO_INTENSET_OFF,   WAKE_IN_BIT);
  gpio_settle();
  e += check_eq("INTSTATUS clear before sleep", 0,
                rd32(GPIO_0_BASE + GPIO_INTSTATUS_OFF) & WAKE_IN_BIT);

  // GIC: SPI 37 quiet, enabled, PMR open, real handler installed
  gicd_set_bit(GICD_ICENABLER_OFF, spi);
  gicd_set_bit(GICD_ICPENDR_OFF,   spi);
  gicd_set_bit(GICD_ICACTIVER_OFF, spi);
  wr32(GICD_BASE + GICD_CTLR_OFF, sv_dct | CTLR_ENABLE_BIT);
  wr32(GICC_BASE + GICC_CTLR_OFF, sv_cct | CTLR_ENABLE_BIT);
  wr32(GICC_BASE + GICC_PMR_OFF,  0xFFUL);
  gic_install_handler(spi, &wake_handler);
  g_calls = 0; g_id = -1; g_active_in_handler = -1;
  g_intstatus_on_entry = 0; g_intstatus_after_clear = 0xFFFFFFFFu; g_sync_abort_count = 0;
  gicd_set_bit(GICD_ISENABLER_OFF, spi);
  e += check_eq("SPI 37 not pending before sleep", 0, gicd_read_bit(GICD_ISPENDR_OFF, spi));

  // Sleep. The only wake source enabled is SPI 37; the bench makes the edge.
  enable_irq();
  printf("GPIOWAKE:ARM:P0_15\n");
  wfi_exits = 0;
  while (g_calls == 0 && wfi_exits < IRQ_WAIT_MAX_ITERS) { call_wfi(); wfi_exits++; }
  disable_irq();

  printf("[%s] woke: WFI exits %lu, handler calls %d\n", V, wfi_exits, (int)g_calls);
  e += check_eq("woke within IRQ_WAIT_MAX_ITERS WFI exits", 1, wfi_exits < IRQ_WAIT_MAX_ITERS);
  e += check_eq("handler entered exactly once", 1, (unsigned long long)g_calls);
  e += check_eq("GICC_IAR id in handler", spi, (unsigned long long)g_id);
  e += check_eq("SPI active inside handler", 1, (unsigned long long)g_active_in_handler);
  e += check_eq("INTSTATUS P0[15] set on entry", WAKE_IN_BIT, g_intstatus_on_entry & WAKE_IN_BIT);
  e += check_eq("INTSTATUS cleared by handler", 0, g_intstatus_after_clear);
  e += check_eq("no abort taken", 0, (unsigned long long)g_sync_abort_count);
  e += check_eq("SPI not active after return",  0, gicd_read_bit(GICD_ISACTIVER_OFF, spi));
  e += check_eq("SPI not pending after return", 0, gicd_read_bit(GICD_ISPENDR_OFF, spi));

  // Bench releases P0[15] on DONE; pulldown brings it back low.
  printf("GPIOWAKE:DONE\n");
  gpio_settle();

  // Restore
  gpio_quiesce();
  gicd_set_bit(GICD_ICENABLER_OFF, spi);
  gicd_set_bit(GICD_ICPENDR_OFF,   spi);
  gicd_set_bit(GICD_ICACTIVER_OFF, spi);
  if (sv_en) gicd_set_bit(GICD_ISENABLER_OFF, spi);
  wr32(GICC_BASE + GICC_PMR_OFF,  sv_pmr);
  wr32(GICC_BASE + GICC_CTLR_OFF, sv_cct);
  wr32(GICD_BASE + GICD_CTLR_OFF, sv_dct);
  e += check_eq("DAIF restored", daif0, read_daif());

  printf("Total number of gpio wfi wake errors: %d\n", e);
  if (e == 0) { printf("TEST PASSED\n"); TEST_PASS(); }
  else        { printf("TEST FAILED\n"); TEST_FAIL(); }
  while (1) { }
}
