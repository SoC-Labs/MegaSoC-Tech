//==============================================================================
// dma350_irq_tests.c -- MegaSoC DMA-350 interrupts and bus errors
//
// INC_DMA builds only. vplan MEGASOC-DMA350-007 (channel interrupts),
// MEGASOC-DMA350-008 (combined interrupt), MEGASOC-DMA350-009 (bus errors and
// recovery), MEGASOC-DMA350-010 (DMA reads of the QSPI flash).
//
// Wiring under test (megasoc_tech_wrapper.sv): irq_channel[n] ->
// CPU_IRQS[97+n] -> GIC ID 129+n; irq_comb_nonsec -> CPU_IRQS[101] -> GIC
// ID 133. Level interrupts.
//   1. Idle: IDs 129-133 not pending.
//   2. Each channel, CH_INTREN = DONE, combined interrupt disabled: a 64 B
//      copy sets INTR_DONE, NSEC_CHINTRSTATUS0 bit n and makes ID 129+n
//      pending; no other DMA ID pending. Writing STAT_DONE (write 1 to
//      clear) drops it. GICC_IAR returns 129+n once.
//   3. CH_INTREN = 0: the copy completes (STAT_DONE) but no ID is pending.
//   4. NSEC_CTRL.INTREN_ANYCHINTR = 1: a channel 2 copy makes 131 and 133
//      pending; GICC_IAR (only 133 enabled) returns 133; clearing drops both.
//   5. Bus errors (CH_INTREN = ERR): read from the unmapped hole
//      0x0104_0000 and write to it. STAT_ERR, ERRINFO BUSERR + read/write
//      response flag, ID 129+n pending; clearing drops it.
//   6. FLASH read (0x0040_0000 -> SRAM): completes with STAT_DONE and no
//      error, and the copy equals a CPU read of the flash. (2026-11-06 run:
//      the NIC's "secure" setting on FLASH does not refuse DMA reads; the
//      first expectation, a bus error, was wrong.)
//   7. Recovery: after each error the same channel completes a good copy.
// GIC state touched (enable/pending/active for 129-133, PMR, GICD_CTLR,
// GICC_CTLR) is saved and restored. GICD_IPRIORITYRn is not written.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "dma350.h"
#include <stdio.h>
#include <stdint.h>

static const char *V7  = "MEGASOC-DMA350-007";
static const char *V8  = "MEGASOC-DMA350-008";
static const char *V9  = "MEGASOC-DMA350-009";
static const char *V10 = "MEGASOC-DMA350-010";

#define GICD_BASE GIC_DISTRIBUTOR_BASE
#define GICC_BASE GIC_CPU_INTERFACE_BASE
#define GICD_CTLR       0x000UL
#define GICD_ISENABLER  0x100UL
#define GICD_ICENABLER  0x180UL
#define GICD_ISPENDR    0x200UL
#define GICD_ICACTIVER  0x380UL
#define GICC_CTLR       0x00UL
#define GICC_PMR        0x04UL
#define GICC_IAR        0x0CUL
#define GICC_EOIR       0x10UL
#define SPURIOUS_ID     1023U

#define FIRST_ID        129U
#define LAST_ID         133U

#define SRAM_SRC   0x00808000UL
#define SRAM_DST   0x00809000UL
#define HOLE_ADDR  0x01040000UL    // unmapped on DMA_M: DECERR
#define FLASH_ADDR 0x00400000UL    // QSPI flash window
#define COPY_BYTES 64U
#define POLL_ITERS 200000U
#define GIC_POLL   2000U

static uint32_t gicd_bit(unsigned long off, unsigned id)
{ return (dma_rd(GICD_BASE + off + 4UL * (id / 32U)) >> (id % 32U)) & 1U; }
static void gicd_set(unsigned long off, unsigned id)
{ dma_wr(GICD_BASE + off + 4UL * (id / 32U), 1UL << (id % 32U)); }

static uint32_t wait_pending(unsigned id, uint32_t want)
{
  uint32_t p = gicd_bit(GICD_ISPENDR, id);
  for (uint32_t i = 0; i < GIC_POLL && p != want; i++) p = gicd_bit(GICD_ISPENDR, id);
  return p;
}

// Pending bits of IDs 129-133 as a 5-bit mask (bit 0 = ID 129)
static uint32_t dma_pending_mask(void)
{
  uint32_t m = 0;
  for (unsigned id = FIRST_ID; id <= LAST_ID; id++) m |= gicd_bit(GICD_ISPENDR, id) << (id - FIRST_ID);
  return m;
}

static int check(const char *v, const char *what, uint32_t exp, uint32_t got)
{
  if (exp != got) {
    printf("[%s] FAIL %s: expected 0x%x got 0x%x\n", v, what, (unsigned)exp, (unsigned)got);
    return 1;
  }
  printf("[%s] ok   %s = 0x%x\n", v, what, (unsigned)got);
  return 0;
}

// Run one copy on `ch` and wait for it. Returns CH_STATUS.
static uint32_t copy(unsigned ch, unsigned long src, unsigned long dst, uint32_t intren)
{
  // Cache maintenance only on the SRAM buffers (never on the hole/FLASH)
  if (src == SRAM_SRC) dma350_cache_civac_range(SRAM_SRC, COPY_BYTES);
  dma350_cache_civac_range(SRAM_DST, 0x100);
  dma350_ch_setup_m2m(ch, src, dst, COPY_BYTES, DMA_TRANSIZE_8, 0, intren);
  dma350_ch_start(ch);
  return dma350_ch_wait(ch, POLL_ITERS);
}

static int good_copy(const char *v, unsigned ch, const char *what)
{
  uint32_t st = copy(ch, SRAM_SRC, SRAM_DST, 0);
  int e = check(v, what, DMA_STAT_DONE, st & (DMA_STAT_DONE | DMA_STAT_ERR | 0x80000000UL));
  dma_ch_wr(ch, DMA_CH_STATUS, DMA_STAT_W1C_ALL);
  return e;
}

static int test_channel_irqs(void)
{
  int e = 0;
  uint32_t st, id;

  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) {
    const unsigned gid = DMA350_GIC_ID_CH(ch);
    printf("Test: channel %u done interrupt -> GIC ID %u\n", ch, gid);
    st = copy(ch, SRAM_SRC, SRAM_DST, DMA_INTREN_DONE);
    e += check(V7, "CH_STATUS STAT_DONE|INTR_DONE", DMA_STAT_DONE | DMA_INTR_DONE,
               st & (DMA_STAT_DONE | DMA_STAT_ERR | DMA_INTR_DONE | 0x80000000UL));
    e += check(V7, "NSEC_CHINTRSTATUS0", 1UL << ch, dma_nsec_rd(DMA_NSEC_CHINTRSTATUS0) & 0xF);
    e += check(V7, "channel ID pending", 1, wait_pending(gid, 1));
    e += check(V7, "only this DMA ID pending (129-133 mask)", 1UL << ch, dma_pending_mask());

    gicd_set(GICD_ISENABLER, gid);
    id = dma_rd(GICC_BASE + GICC_IAR) & 0x3FFU;
    e += check(V7, "GICC_IAR returns the channel ID", gid, id);
    if (id != SPURIOUS_ID) dma_wr(GICC_BASE + GICC_EOIR, id);
    gicd_set(GICD_ICENABLER, gid);

    dma_ch_wr(ch, DMA_CH_STATUS, DMA_STAT_DONE);              // write 1 to clear
    e += check(V7, "INTR_DONE after clear", 0, dma_ch_rd(ch, DMA_CH_STATUS) & DMA_INTR_DONE);
    e += check(V7, "channel ID pending after clear", 0, wait_pending(gid, 0));
  }

  printf("Test: CH_INTREN = 0 gives no interrupt\n");
  st = copy(1, SRAM_SRC, SRAM_DST, 0);
  e += check(V7, "STAT_DONE with INTREN=0", DMA_STAT_DONE, st & (DMA_STAT_DONE | DMA_INTR_DONE | 0x80000000UL));
  {
    uint32_t p = 0;
    for (uint32_t i = 0; i < GIC_POLL / 4U; i++) p |= dma_pending_mask();
    e += check(V7, "no DMA ID pending with INTREN=0", 0, p);
  }
  dma_ch_wr(1, DMA_CH_STATUS, DMA_STAT_W1C_ALL);
  return e;
}

static int test_combined_irq(void)
{
  int e = 0;
  uint32_t id;

  printf("Test: combined interrupt -> GIC ID %u\n", DMA350_GIC_ID_COMB);
  dma_nsec_wr(DMA_NSEC_CTRL, DMA_NSEC_CTRL_INTREN_ANYCHINTR);
  (void)copy(2, SRAM_SRC, SRAM_DST, DMA_INTREN_DONE);
  e += check(V8, "NSEC_STATUS.INTR_ANYCHINTR", 1, dma_nsec_rd(DMA_NSEC_STATUS) & DMA_NSEC_STATUS_INTR_ANYCHINTR);
  e += check(V8, "ID 133 pending", 1, wait_pending(DMA350_GIC_ID_COMB, 1));
  e += check(V8, "IDs 131 and 133 pending (mask)", (1UL << 2) | (1UL << 4), dma_pending_mask());

  gicd_set(GICD_ISENABLER, DMA350_GIC_ID_COMB);
  id = dma_rd(GICC_BASE + GICC_IAR) & 0x3FFU;
  e += check(V8, "GICC_IAR returns 133", DMA350_GIC_ID_COMB, id);
  if (id != SPURIOUS_ID) dma_wr(GICC_BASE + GICC_EOIR, id);
  gicd_set(GICD_ICENABLER, DMA350_GIC_ID_COMB);

  dma_ch_wr(2, DMA_CH_STATUS, DMA_STAT_DONE);
  e += check(V8, "ID 133 pending after clear", 0, wait_pending(DMA350_GIC_ID_COMB, 0));
  e += check(V8, "no DMA ID pending after clear", 0, dma_pending_mask());
  dma_nsec_wr(DMA_NSEC_CTRL, 0);
  return e;
}

// One bus error case on `ch`; `rd` = error on the read side.
static int error_case(const char *v, unsigned ch, const char *name, unsigned long src, unsigned long dst, int rd)
{
  int e = 0;
  const unsigned gid = DMA350_GIC_ID_CH(ch);
  uint32_t st, ei;

  printf("Test: bus error, %s (channel %u)\n", name, ch);
  st = copy(ch, src, dst, DMA_INTREN_ERR);
  ei = dma_ch_rd(ch, DMA_CH_ERRINFO);
  e += check(v, "STAT_ERR|INTR_ERR, no STAT_DONE", DMA_STAT_ERR | DMA_INTR_ERR,
             st & (DMA_STAT_DONE | DMA_STAT_ERR | DMA_INTR_ERR | 0x80000000UL));
  e += check(v, "ERRINFO.BUSERR", 1, ei & DMA_ERRINFO_BUSERR);
  e += check(v, rd ? "ERRINFO.AXIRDRESPERR" : "ERRINFO.AXIWRRESPERR", 1,
             (ei >> (rd ? 16 : 17)) & 1U);
  e += check(v, "channel idle after error", 0, (uint32_t)dma350_ch_busy(ch));
  e += check(v, "channel ID pending", 1, wait_pending(gid, 1));

  dma_ch_wr(ch, DMA_CH_STATUS, DMA_STAT_W1C_ALL);
  e += check(v, "channel ID pending after clear", 0, wait_pending(gid, 0));
  e += good_copy(v, ch, "recovery copy after error");
  return e;
}

// DMA copy FLASH -> SRAM on channel 1; data compared with a CPU read of the
// flash (the CPU path through the NIC to FLASH is already proven by boot).
static int flash_read_case(void)
{
  int e = 0;
  uint32_t st, bad = 0;
  volatile uint8_t *f = (volatile uint8_t *)FLASH_ADDR;
  volatile uint8_t *d = (volatile uint8_t *)SRAM_DST;

  printf("Test: DMA read of the QSPI flash (channel 1)\n");
  for (uint32_t i = 0; i < COPY_BYTES; i++) d[i] = 0xEE;
  st = copy(1, FLASH_ADDR, SRAM_DST, 0);
  e += check(V10, "FLASH->SRAM STAT_DONE, no error", DMA_STAT_DONE,
             st & (DMA_STAT_DONE | DMA_STAT_ERR | 0x80000000UL));
  e += check(V10, "ERRINFO", 0, dma_ch_rd(1, DMA_CH_ERRINFO));
  dma350_cache_civac_range(SRAM_DST, 0x100);
  for (uint32_t i = 0; i < COPY_BYTES; i++) if (d[i] != f[i]) bad++;
  e += check(V10, "copy equals CPU read of flash (bad bytes)", 0, bad);
  dma_ch_wr(1, DMA_CH_STATUS, DMA_STAT_W1C_ALL);
  return e;
}

int main(void)
{
  int e = 0;
  uint32_t en[LAST_ID - FIRST_ID + 1], pmr, dctlr, cctlr;

  UartStdOutInit();
  printf("MegaSoC DMA-350 interrupt and error test (%s, %s, %s, %s)\n", V7, V8, V9, V10);

  for (unsigned id = FIRST_ID; id <= LAST_ID; id++) en[id - FIRST_ID] = gicd_bit(GICD_ISENABLER, id);
  pmr   = dma_rd(GICC_BASE + GICC_PMR);
  dctlr = dma_rd(GICD_BASE + GICD_CTLR);
  cctlr = dma_rd(GICC_BASE + GICC_CTLR);
  for (unsigned id = FIRST_ID; id <= LAST_ID; id++) gicd_set(GICD_ICENABLER, id);
  dma_wr(GICC_BASE + GICC_PMR, 0xFF);
  dma_wr(GICD_BASE + GICD_CTLR, dctlr | 1U);
  dma_wr(GICC_BASE + GICC_CTLR, cctlr | 1U);

  for (uint32_t i = 0; i < COPY_BYTES; i++) ((volatile uint8_t *)SRAM_SRC)[i] = (uint8_t)(i ^ 0xA5U);

  e += check(V7, "no DMA ID pending at idle (129-133 mask)", 0, dma_pending_mask());
  e += test_channel_irqs();
  e += test_combined_irq();
  e += error_case(V9,  0, "read from unmapped 0x0104_0000", HOLE_ADDR, SRAM_DST, 1);
  e += error_case(V9,  3, "write to unmapped 0x0104_0000",  SRAM_SRC, HOLE_ADDR, 0);
  e += flash_read_case();

  // restore
  for (unsigned id = FIRST_ID; id <= LAST_ID; id++) {
    gicd_set(GICD_ICENABLER, id);
    gicd_set(GICD_ICACTIVER, id);
    if (en[id - FIRST_ID]) gicd_set(GICD_ISENABLER, id);
  }
  dma_wr(GICC_BASE + GICC_PMR, pmr);
  dma_wr(GICC_BASE + GICC_CTLR, cctlr);
  dma_wr(GICD_BASE + GICD_CTLR, dctlr);
  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) (void)dma350_ch_clear(ch, 2000U);

  printf("Total number of dma350_irq errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
