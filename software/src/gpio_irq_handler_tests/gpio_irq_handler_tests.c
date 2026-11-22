//==============================================================================
// gpio_irq_handler_tests.c -- MegaSoC GPIO combined interrupt, pad stimulus to
//                             real CPU IRQ handler (tc_gpio_irq_handler)
//
// vplan: MEGASOC-IRQ-002 (gpio_combined_irqs, P0)
//   "GPIO0 combined interrupt routes to SPI 37 and GPIO1 combined to SPI 38,
//    each observed by a CPU handler after stimulating a GPIO input."
//
// Why a new test (2026-11-20): the two existing tests each prove half of it.
//   gpio_irq_tests                  pad edge -> GPIO -> SPI 37/38 pending, but
//                                   polled with PSTATE.I masked (no handler).
//   gic_vector_table_delivery_tests real IRQ exception -> handler for SPI 37,
//                                   but the SPI is pended by a GICD_ISPENDR
//                                   write, not by a GPIO pin.
//   This test joins the two: a real pin edge, the real GPIO combined line,
//   the GIC, the CPU IRQ exception, and the C handler, on both instances.
//
// Stimulus (megasoc_tb.sv tran loopbacks, same as gpio_irq_tests):
//   GPIO0: P0[0] output -> tran -> P0[8] input, SPI 37 (GPIO0_COMB_INTR)
//   GPIO1: P1[0] output -> tran -> P1[3] input, SPI 38 (GPIO1_COMB_INTR)
//          P1[3:0] alt-function is cleared first (GPIO1 resets with
//          ALTERNATE_FUNC_DEFAULT 16'hFFFF) and restored at the end.
//   P1[8:14] (EXTIO/ADP link) is never touched.
//
// Handler: registered with gic_install_handler(); reached only through
//   vect_64.s -> el1_irq_handler -> gic400.c irq_handler() -> this function.
//   The GPIO combined line stays high until INTSTATUS is cleared, so the
//   handler clears INTSTATUS (W1C) and waits for it to read 0 before it
//   returns; irq_handler() then writes GICC_EOIR. A runaway guard disables
//   the SPI if the handler is entered more than RUNAWAY_LIMIT times, so a
//   stuck line ends in a FAIL verdict, not a hung simulation.
//
// Checks per instance (each failure message carries [MEGASOC-IRQ-002]):
//   pre-unmask    INTSTATUS clear, SPI not pending/active, handler not run
//   delivery      handler entered within IRQ_WAIT_MAX_ITERS WFI loops,
//                 exactly once, GICC_IAR id == expected SPI, SPI active
//                 inside the handler, INTSTATUS input bit set on entry,
//                 INTSTATUS cleared by the handler
//   after return  SPI not active, not pending, GICC_IAR does not return it
//   negative      with PSTATE.I masked, a second edge pends the SPI but the
//                 handler is not entered; W1C clears it and the SPI
//                 de-pends without an ICPENDR write
//
// Coverage markers (printed, one per instance): GPIOIRQ:HANDLER:<SPI>
//
// Knobs: TEST_VERBOSITY=1 (-DTEST_VERBOSITY_HIGH),
//        IRQ_WAIT_MAX_ITERS=<n> (bounded WFI wait cap)
// printf lines kept short (c_print buffer, BUG-22).
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "gic400.h"
#include "irq.h"
#include <stdio.h>
#include <stdint.h>

#define V "MEGASOC-IRQ-002"

// -- CMSDK GPIO register offsets (ARM DDI 0479C, megasoc_spec.md 5.11) -------
#define GPIO_DATAOUT_OFF     0x004UL
#define GPIO_OUTENSET_OFF    0x010UL
#define GPIO_OUTENCLR_OFF    0x014UL
#define GPIO_ALTFUNCSET_OFF  0x018UL
#define GPIO_ALTFUNCCLR_OFF  0x01CUL
#define GPIO_INTENSET_OFF    0x020UL
#define GPIO_INTENCLR_OFF    0x024UL
#define GPIO_INTTYPESET_OFF  0x028UL
#define GPIO_INTTYPECLR_OFF  0x02CUL
#define GPIO_INTPOLSET_OFF   0x030UL
#define GPIO_INTPOLCLR_OFF   0x034UL
#define GPIO_INTSTATUS_OFF   0x038UL

// -- GIC-400 (same offsets as gic_vector_table_delivery_tests.c) ------------
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
#define GICC_IAR_OFF         0x0CUL
#define GICC_EOIR_OFF        0x10UL
#define CTLR_ENABLE_BIT      0x1UL
#define IAR_ID_MASK          0x3FFUL
#define SPURIOUS_ID          1023UL

#ifndef IRQ_WAIT_MAX_ITERS
#define IRQ_WAIT_MAX_ITERS 2000000UL
#endif
#define RUNAWAY_LIMIT        4
#define CLEAR_POLL_MAX       1000U

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

// >= 2 GPIO input synchroniser stages + propagation to the GIC
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

// -- Handler state ----------------------------------------------------------
static volatile unsigned long g_gpio_base;   // instance under test
static volatile uint32_t      g_in_bit;      // its input pin
static volatile int           g_calls;
static volatile int           g_id;
static volatile int           g_active_in_handler;
static volatile uint32_t      g_intstatus_on_entry;
static volatile uint32_t      g_intstatus_after_clear;
static volatile int           g_sync_abort_count;

static void gpio_comb_handler(int num, int source)
{
  unsigned int n;
  (void)source;
  g_calls++;
  g_id = num;
  g_active_in_handler = (int)gicd_read_bit(GICD_ISACTIVER_OFF, (unsigned int)num);
  g_intstatus_on_entry = rd32(g_gpio_base + GPIO_INTSTATUS_OFF);

  // Clear the source so the level line drops before irq_handler() EOIs.
  wr32(g_gpio_base + GPIO_INTSTATUS_OFF, g_in_bit);
  for (n = 0; n < CLEAR_POLL_MAX; n++) {
    if (!(rd32(g_gpio_base + GPIO_INTSTATUS_OFF) & g_in_bit)) break;
  }
  gpio_settle();
  g_intstatus_after_clear = rd32(g_gpio_base + GPIO_INTSTATUS_OFF) & g_in_bit;

  if (g_calls >= RUNAWAY_LIMIT)      // stuck line: stop the storm, FAIL later
    gicd_set_bit(GICD_ICENABLER_OFF, (unsigned int)num);
}

// abort_handler() override (same idiom as gic_vector_table_delivery_tests.c):
// counts any synchronous/external abort so it becomes a FAIL, not a hang.
void abort_handler(void) { g_sync_abort_count++; }

// -- GPIO pin setup / teardown ---------------------------------------------
static void gpio_quiesce(unsigned long base, uint32_t out_bit, uint32_t in_bit)
{
  wr32(base + GPIO_INTENCLR_OFF,   in_bit);
  wr32(base + GPIO_DATAOUT_OFF,    rd32(base + GPIO_DATAOUT_OFF) & ~out_bit);
  wr32(base + GPIO_OUTENCLR_OFF,   out_bit | in_bit);
  wr32(base + GPIO_INTTYPECLR_OFF, in_bit);
  wr32(base + GPIO_INTPOLCLR_OFF,  in_bit);
  gpio_settle();
  wr32(base + GPIO_INTSTATUS_OFF,  in_bit);
}

static void drive(unsigned long base, uint32_t out_bit, int high)
{
  uint32_t d = rd32(base + GPIO_DATAOUT_OFF);
  wr32(base + GPIO_DATAOUT_OFF, high ? (d | out_bit) : (d & ~out_bit));
  gpio_settle();
}

static int run_instance(const char *tag, unsigned long base,
                        uint32_t out_bit, uint32_t in_bit, unsigned int spi)
{
  int e = 0;
  unsigned long it;
  uint32_t iar;
  uint32_t sv_en  = gicd_read_bit(GICD_ISENABLER_OFF, spi);
  uint32_t sv_pmr = rd32(GICC_BASE + GICC_PMR_OFF);
  uint32_t sv_dct = rd32(GICD_BASE + GICD_CTLR_OFF);
  uint32_t sv_cct = rd32(GICC_BASE + GICC_CTLR_OFF);

  printf("=== %s: out 0x%04x -> in 0x%04x, SPI %u ===\n",
         tag, (unsigned)out_bit, (unsigned)in_bit, spi);

  // 1. GPIO: output pin low, input pin rising-edge interrupt, status clear
  gpio_quiesce(base, out_bit, in_bit);
  wr32(base + GPIO_OUTENSET_OFF,   out_bit);
  drive(base, out_bit, 0);
  wr32(base + GPIO_INTTYPESET_OFF, in_bit);   // edge
  wr32(base + GPIO_INTPOLSET_OFF,  in_bit);   // rising
  wr32(base + GPIO_INTSTATUS_OFF,  in_bit);
  wr32(base + GPIO_INTENSET_OFF,   in_bit);
  gpio_settle();
  e += check_eq("INTSTATUS clear before edge", 0,
                rd32(base + GPIO_INTSTATUS_OFF) & in_bit);

  // 2. GIC: SPI quiet, distributor + CPU interface on, PMR open, handler in
  gicd_set_bit(GICD_ICENABLER_OFF, spi);
  gicd_set_bit(GICD_ICPENDR_OFF,   spi);
  gicd_set_bit(GICD_ICACTIVER_OFF, spi);
  wr32(GICD_BASE + GICD_CTLR_OFF, sv_dct | CTLR_ENABLE_BIT);
  wr32(GICC_BASE + GICC_CTLR_OFF, sv_cct | CTLR_ENABLE_BIT);
  wr32(GICC_BASE + GICC_PMR_OFF,  0xFFUL);
  gic_install_handler(spi, &gpio_comb_handler);
  e += check_eq("SPI not pending before edge", 0, gicd_read_bit(GICD_ISPENDR_OFF, spi));
  e += check_eq("SPI not active before edge",  0, gicd_read_bit(GICD_ISACTIVER_OFF, spi));

  g_gpio_base = base; g_in_bit = in_bit;
  g_calls = 0; g_id = -1; g_active_in_handler = -1;
  g_intstatus_on_entry = 0; g_intstatus_after_clear = 0xFFFFFFFFu;
  g_sync_abort_count = 0;
  gicd_set_bit(GICD_ISENABLER_OFF, spi);

  // 3. Unmask, then the only stimulus: a rising edge on the output pin
  enable_irq();
  e += check_eq("PSTATE.I unmasked", 0, (read_daif() >> 7) & 0x1ULL);
  drive(base, out_bit, 1);
  it = 0;
  while (g_calls == 0 && it < IRQ_WAIT_MAX_ITERS) { call_wfi(); it++; }
  disable_irq();

  // 4. Delivery checks
  e += check_eq("handler entered (no wait timeout)", 1, it < IRQ_WAIT_MAX_ITERS);
  e += check_eq("handler entered exactly once", 1, (unsigned long long)g_calls);
  e += check_eq("GICC_IAR id in handler", spi, (unsigned long long)g_id);
  e += check_eq("SPI active inside handler", 1, (unsigned long long)g_active_in_handler);
  e += check_eq("INTSTATUS in-bit set on entry", in_bit, g_intstatus_on_entry & in_bit);
  e += check_eq("INTSTATUS cleared by handler", 0, g_intstatus_after_clear);
  e += check_eq("no abort taken", 0, (unsigned long long)g_sync_abort_count);
  e += check_eq("SPI not active after return",  0, gicd_read_bit(GICD_ISACTIVER_OFF, spi));
  e += check_eq("SPI not pending after return", 0, gicd_read_bit(GICD_ISPENDR_OFF, spi));
  iar = rd32(GICC_BASE + GICC_IAR_OFF) & IAR_ID_MASK;
  if (iar != SPURIOUS_ID) {
    wr32(GICC_BASE + GICC_EOIR_OFF, iar);
    e += check_eq("GICC_IAR spurious after return", SPURIOUS_ID, iar);
  }
  if (e == 0) printf("GPIOIRQ:HANDLER:%u\n", spi);

  // 5. Negative control: PSTATE.I masked, second edge pends but no dispatch
  drive(base, out_bit, 0);
  drive(base, out_bit, 1);
  e += check_eq("masked: INTSTATUS set by 2nd edge", in_bit,
                rd32(base + GPIO_INTSTATUS_OFF) & in_bit);
  e += check_eq("masked: SPI pending", 1, gicd_read_bit(GICD_ISPENDR_OFF, spi));
  e += check_eq("masked: handler not entered", 1, (unsigned long long)g_calls);
  wr32(base + GPIO_INTSTATUS_OFF, in_bit);
  gpio_settle();
  e += check_eq("masked: SPI de-pends after W1C", 0, gicd_read_bit(GICD_ISPENDR_OFF, spi));

  // 6. Restore
  gicd_set_bit(GICD_ICENABLER_OFF, spi);
  gicd_set_bit(GICD_ICPENDR_OFF,   spi);
  gicd_set_bit(GICD_ICACTIVER_OFF, spi);
  if (sv_en) gicd_set_bit(GICD_ISENABLER_OFF, spi);
  wr32(GICC_BASE + GICC_PMR_OFF,  sv_pmr);
  wr32(GICC_BASE + GICC_CTLR_OFF, sv_cct);
  wr32(GICD_BASE + GICD_CTLR_OFF, sv_dct);
  gpio_quiesce(base, out_bit, in_bit);

  printf("[%s] %s errors: %d\n", V, tag, e);
  return e;
}

int main(void)
{
  int errors = 0;
  uint64_t daif0;
  uint32_t altfunc1;

  UartStdOutInit();
  printf("MegaSoC GPIO pad -> GIC -> CPU IRQ handler test (tc_gpio_irq_handler)\n");
  printf("vplan: %s\n", V);
  daif0 = read_daif();

  errors += check_eq("GPIO0_COMB_INTR == 37", 37, GPIO0_COMB_INTR);
  errors += check_eq("GPIO1_COMB_INTR == 38", 38, GPIO1_COMB_INTR);

  errors += run_instance("GPIO0", GPIO_0_BASE, 0x0001u, 0x0100u, GPIO0_COMB_INTR);

  altfunc1 = rd32(GPIO_1_BASE + GPIO_ALTFUNCSET_OFF);
  wr32(GPIO_1_BASE + GPIO_ALTFUNCCLR_OFF, 0x000FUL);   // P1[3:0] -> GPIO
  gpio_settle();
  errors += run_instance("GPIO1", GPIO_1_BASE, 0x0001u, 0x0008u, GPIO1_COMB_INTR);
  wr32(GPIO_1_BASE + GPIO_ALTFUNCSET_OFF, altfunc1 & 0x000FUL);
  gpio_settle();

  errors += check_eq("DAIF restored", daif0, read_daif());
  printf("Total number of gpio irq handler errors: %d\n", errors);
  if (errors == 0) { printf("TEST PASSED\n"); TEST_PASS(); }
  else             { printf("TEST FAILED\n"); TEST_FAIL(); }
  while (1) { }
}
