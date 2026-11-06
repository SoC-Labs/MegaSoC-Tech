//==============================================================================
// eth_irq_tests.c -- MegaSoC Ethernet interrupts to the GIC
//
// INC_ETH builds only. vplan MEGASOC-ETH-004 (MAC interrupt), MEGASOC-ETH-005
// (checksum-unit interrupt).
//
// Wiring under test (megasoc_tech_wrapper.sv): MAC int_o -> CPU_IRQS[95] ->
// GIC interrupt ID 127; checksum unit cksum_int_o -> CPU_IRQS[96] -> GIC ID
// 128. Both are level interrupts. Frames come from the bench MII loopback.
//   1. Idle: neither ID pending.
//   2. INT_MASK = TXB|RXB, send a frame: INT_SOURCE TXB/RXB set, ID 127
//      pending. With the distributor and CPU interface enabled, GICC_IAR
//      returns 127; EOI. Writing TXB|RXB to INT_SOURCE (write-1-to-clear)
//      clears the sources and ID 127 stops being pending.
//   3. INT_MASK = 0, send a frame: TXB/RXB set in INT_SOURCE but ID 127
//      stays idle. Setting INT_MASK.RXB then makes it pending (the source
//      is still set); clearing INT_SOURCE drops it again.
//   4. Checksum unit: CTRL = EN | IRQ_EN_FRAME, send a frame: FRAME_COUNT
//      goes up, IRQ_STATUS.frame set, ID 128 pending. Reading LAST_SEQNUM
//      pops the snapshot FIFO, the FIFO empties and ID 128 drops. The unit
//      runs in a FIFO mode: the reset mode (LAST_ONLY) has no pop.
// GIC state touched (enable/pending for 127/128, PMR, GICD_CTLR, GICC_CTLR)
// is saved first and restored at the end. GICD_IPRIORITYRn is not written
// (reset priority is used), as in sdio_irq_end_to_end_tests.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "ethmac.h"
#include <stdio.h>
#include <stdint.h>
#ifdef INC_LPDDR4
#include "lpddr4.h"
#endif

static const char *V = "MEGASOC-ETH-004";
static const char *VC = "MEGASOC-ETH-005";

#define GICD_BASE GIC_DISTRIBUTOR_BASE
#define GICC_BASE GIC_CPU_INTERFACE_BASE
#define GICD_CTLR       0x000UL
#define GICD_ISENABLER  0x100UL
#define GICD_ICENABLER  0x180UL
#define GICD_ISPENDR    0x200UL
#define GICD_ICPENDR    0x280UL
#define GICD_ICACTIVER  0x380UL
#define GICC_CTLR       0x00UL
#define GICC_PMR        0x04UL
#define GICC_IAR        0x0CUL
#define GICC_EOIR       0x10UL
#define SPURIOUS_ID     1023U

#define SRAM_TX   0x00808000UL
#define SRAM_RX   0x00809000UL
#define FRAME_LEN 64U
#define POLL_ITERS 400000U
#define GIC_POLL   2000U    // level changes reach the GIC within a few cycles

static inline void wr32(unsigned long a, uint32_t v) { *(volatile uint32_t *)a = v; __asm__ volatile ("dsb sy" ::: "memory"); }
static inline uint32_t rd32(unsigned long a) { return *(volatile uint32_t *)a; }

static uint32_t gicd_bit(unsigned long off, unsigned id)
{ return (rd32(GICD_BASE + off + 4UL * (id / 32U)) >> (id % 32U)) & 1U; }
static void gicd_set(unsigned long off, unsigned id)
{ wr32(GICD_BASE + off + 4UL * (id / 32U), 1UL << (id % 32U)); }

// Poll until ID `id` pending == want (bounded). Returns final pending bit.
static uint32_t wait_pending(unsigned id, uint32_t want)
{
  uint32_t p = gicd_bit(GICD_ISPENDR, id);
  for (uint32_t i = 0; i < GIC_POLL && p != want; i++) p = gicd_bit(GICD_ISPENDR, id);
  return p;
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

static void send_frame(void)
{
  volatile uint8_t *p = (volatile uint8_t *)SRAM_TX;
  for (unsigned i = 0; i < FRAME_LEN; i++) p[i] = (i < 6) ? 0xFF : (uint8_t)(i ^ 0x5A);  // broadcast DA
  eth_cache_civac_range(SRAM_TX, 0x100);
  eth_cache_civac_range(SRAM_RX, 0x100);
  // TXEN/RXEN stay on after the frame: INT_SOURCE is cleared only by the test
  eth_wr(ETH_TX_BD_NUM, 1);
  eth_bd_wr(1, ETH_RXBD_E | ETH_RXBD_IRQ | ETH_RXBD_WR, (uint32_t)SRAM_RX);
  eth_bd_wr(0, (FRAME_LEN << 16) | ETH_TXBD_RD | ETH_TXBD_IRQ | ETH_TXBD_WR | ETH_TXBD_PAD | ETH_TXBD_CRC,
            (uint32_t)SRAM_TX);
  eth_wr(ETH_MODER, ETH_MODER_PAD | ETH_MODER_CRCEN | ETH_MODER_FULLD | ETH_MODER_PRO |
                    ETH_MODER_TXEN | ETH_MODER_RXEN);
}

// Wait for both TXB and RXB in INT_SOURCE
static uint32_t wait_txb_rxb(void)
{
  uint32_t s = 0;
  for (uint32_t i = 0; i < POLL_ITERS; i++) {
    s = eth_rd(ETH_INT_SOURCE);
    if ((s & (ETH_INT_TXB | ETH_INT_RXB)) == (ETH_INT_TXB | ETH_INT_RXB) || (s & (ETH_INT_TXE | ETH_INT_RXE | ETH_INT_BUSY)))
      break;
  }
  return s;
}

static int test_mac_irq(void)
{
  int e = 0;
  uint32_t s, id;

  printf("Test: MAC interrupt -> GIC ID %u\n", ETH_GIC_ID);
  e += check(V, "ID 127 pending at idle", 0, gicd_bit(GICD_ISPENDR, ETH_GIC_ID));
  e += check(V, "ID 128 pending at idle", 0, gicd_bit(GICD_ISPENDR, ETH_CKSUM_GIC_ID));

  // --- unmasked
  eth_mac_stop();
  eth_wr(ETH_INT_MASK, ETH_INT_TXB | ETH_INT_RXB);
  send_frame();
  s = wait_txb_rxb();
  e += check(V, "INT_SOURCE TXB|RXB after frame", ETH_INT_TXB | ETH_INT_RXB, s & 0x7F);
  e += check(V, "ID 127 pending (INT_MASK=TXB|RXB)", 1, wait_pending(ETH_GIC_ID, 1));

  gicd_set(GICD_ISENABLER, ETH_GIC_ID);
  id = rd32(GICC_BASE + GICC_IAR) & 0x3FFU;
  e += check(V, "GICC_IAR returns ID 127", ETH_GIC_ID, id);
  if (id != SPURIOUS_ID) wr32(GICC_BASE + GICC_EOIR, id);
  gicd_set(GICD_ICENABLER, ETH_GIC_ID);

  eth_wr(ETH_INT_SOURCE, ETH_INT_TXB | ETH_INT_RXB);           // write 1 to clear
  e += check(V, "INT_SOURCE after write-1-to-clear", 0, eth_rd(ETH_INT_SOURCE) & (ETH_INT_TXB | ETH_INT_RXB));
  e += check(V, "ID 127 pending after clear", 0, wait_pending(ETH_GIC_ID, 0));

  // --- masked
  eth_mac_stop();
  eth_wr(ETH_INT_MASK, 0);
  send_frame();
  s = wait_txb_rxb();
  e += check(V, "INT_SOURCE TXB|RXB with INT_MASK=0", ETH_INT_TXB | ETH_INT_RXB, s & 0x7F);
  {
    uint32_t p = 0;
    for (uint32_t i = 0; i < GIC_POLL / 4U; i++) p |= gicd_bit(GICD_ISPENDR, ETH_GIC_ID);
    e += check(V, "ID 127 never pending while masked", 0, p);
  }
  eth_wr(ETH_INT_MASK, ETH_INT_RXB);
  e += check(V, "ID 127 pending after INT_MASK.RXB set", 1, wait_pending(ETH_GIC_ID, 1));
  eth_wr(ETH_INT_SOURCE, ETH_INT_ALL);
  e += check(V, "ID 127 pending after clear", 0, wait_pending(ETH_GIC_ID, 0));

  eth_mac_stop();
  eth_wr(ETH_INT_MASK, 0);
  return e;
}

static int test_cksum_irq(void)
{
  int e = 0;
  uint32_t fc0, s;

  printf("Test: checksum unit interrupt -> GIC ID %u\n", ETH_CKSUM_GIC_ID);
  eth_cksum_wr(ETH_CKSUM_CTRL, ETH_CKSUM_CTRL_CLR_COUNTERS | ETH_CKSUM_CTRL_CLR_FIFO | ETH_CKSUM_CTRL_CLR_OVF);
  // FIFO mode: in the reset mode (LAST_ONLY) there is no pop and the frame
  // interrupt cannot be cleared by reading LAST_SEQNUM
  eth_cksum_wr(ETH_CKSUM_CTRL, ETH_CKSUM_CTRL_EN | ETH_CKSUM_CTRL_MODE_DROP_NEW | ETH_CKSUM_CTRL_IRQ_FRAME);
  fc0 = eth_cksum_rd(ETH_CKSUM_FRAME_COUNT);
  e += check(VC, "ID 128 pending before frame", 0, gicd_bit(GICD_ISPENDR, ETH_CKSUM_GIC_ID));

  eth_mac_stop();
  send_frame();
  s = wait_txb_rxb();
  e += check(VC, "frame looped back (INT_SOURCE RXB)", ETH_INT_RXB, s & ETH_INT_RXB);
  for (uint32_t i = 0; i < GIC_POLL && eth_cksum_rd(ETH_CKSUM_FRAME_COUNT) == fc0; i++) { }
  e += check(VC, "FRAME_COUNT +1", fc0 + 1U, eth_cksum_rd(ETH_CKSUM_FRAME_COUNT));
  e += check(VC, "IRQ_STATUS.frame", 1, eth_cksum_rd(ETH_CKSUM_IRQ_STATUS) & 1U);
  e += check(VC, "ID 128 pending", 1, wait_pending(ETH_CKSUM_GIC_ID, 1));
  e += check(V,  "ID 127 not pending (INT_MASK=0)", 0, gicd_bit(GICD_ISPENDR, ETH_GIC_ID));

  (void)eth_cksum_rd(ETH_CKSUM_LAST_FLAGS);                   // documented two-read sequence
  (void)eth_cksum_rd(ETH_CKSUM_LAST_SEQNUM);                  // pop
  e += check(VC, "IRQ_STATUS.frame after pop", 0, eth_cksum_rd(ETH_CKSUM_IRQ_STATUS) & 1U);
  e += check(VC, "ID 128 pending after pop", 0, wait_pending(ETH_CKSUM_GIC_ID, 0));

  eth_cksum_wr(ETH_CKSUM_CTRL, ETH_CKSUM_CTRL_CLR_COUNTERS | ETH_CKSUM_CTRL_CLR_FIFO | ETH_CKSUM_CTRL_CLR_OVF);
  eth_cksum_wr(ETH_CKSUM_CTRL, 0);
  eth_mac_stop();
  return e;
}

int main(void)
{
  int e = 0;
  uint32_t en127, en128, pmr, dctlr, cctlr;

  UartStdOutInit();
  printf("MegaSoC Ethernet interrupt test (%s, %s)\n", V, VC);

#ifdef INC_LPDDR4
  if (lpddr4_init() != 0) { printf("[%s] FAIL lpddr4_init\n", V); TEST_FAIL(); return 1; }
#endif

  en127 = gicd_bit(GICD_ISENABLER, ETH_GIC_ID);
  en128 = gicd_bit(GICD_ISENABLER, ETH_CKSUM_GIC_ID);
  pmr   = rd32(GICC_BASE + GICC_PMR);
  dctlr = rd32(GICD_BASE + GICD_CTLR);
  cctlr = rd32(GICC_BASE + GICC_CTLR);

  gicd_set(GICD_ICENABLER, ETH_GIC_ID);
  gicd_set(GICD_ICENABLER, ETH_CKSUM_GIC_ID);
  wr32(GICC_BASE + GICC_PMR, 0xFF);
  wr32(GICD_BASE + GICD_CTLR, dctlr | 1U);
  wr32(GICC_BASE + GICC_CTLR, cctlr | 1U);

  e += test_mac_irq();
  e += test_cksum_irq();

  // restore
  gicd_set(GICD_ICENABLER, ETH_GIC_ID);
  gicd_set(GICD_ICENABLER, ETH_CKSUM_GIC_ID);
  gicd_set(GICD_ICACTIVER, ETH_GIC_ID);
  gicd_set(GICD_ICACTIVER, ETH_CKSUM_GIC_ID);
  if (en127) gicd_set(GICD_ISENABLER, ETH_GIC_ID);
  if (en128) gicd_set(GICD_ISENABLER, ETH_CKSUM_GIC_ID);
  wr32(GICC_BASE + GICC_PMR, pmr);
  wr32(GICC_BASE + GICC_CTLR, cctlr);
  wr32(GICD_BASE + GICD_CTLR, dctlr);
  eth_wr(ETH_MODER, ETH_MODER_RST);
  eth_wr(ETH_TX_BD_NUM, ETH_TX_BD_NUM_RST);

  printf("Total number of eth_irq errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
