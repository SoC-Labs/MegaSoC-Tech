//==============================================================================
// eth_loopback_tests.c -- MegaSoC Ethernet DMA + MII loopback test
//
// INC_ETH builds only. vplan MEGASOC-ETH-002 (data path), MEGASOC-ETH-003
// (DMA byte order).
//
// The bench PHY model loops every transmitted frame straight back to the
// receiver (TXD -> RXD, TX_EN -> RX_DV, 25 MHz, full duplex). Each case:
// TX buffer descriptor -> MAC DMA reads the frame from memory over ETH_M ->
// MII -> loopback -> MAC DMA writes it to the RX buffer over ETH_M -> RX
// descriptor. The CPU then compares the received bytes with the sent ones.
//   Case A  TX SRAM -> RX SRAM   (promiscuous)
//   Case B  TX DRAM -> RX DRAM   (promiscuous)
//   Case C  TX SRAM -> RX DRAM   (promiscuous)
//   Case D  DMA byte order. A loopback compare cannot see byte order (any
//           swap on TX is undone on RX), so this case turns promiscuous
//           mode off: the MAC only accepts a frame whose destination
//           address, as it arrives on the wire, equals MAC_ADDR0/1. The
//           frame is sent twice, once with its bytes in memory in wire
//           order (what a little-endian CPU's network stack writes) and
//           once with the bytes swapped inside each 32-bit word. Exactly
//           one of the two must be received. The test requires wire order
//           = memory order and fails with a SWAP_DMA_BYTES message
//           otherwise. The bench log line "ETH_PHY_MODEL: TX frame ..."
//           shows the wire bytes for each frame.
// The MAC DMA is not cache coherent: buffers are cleaned+invalidated before
// the DMA runs and invalidated again before the CPU reads RX data.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "ethmac.h"
#include <stdio.h>
#include <stdint.h>
#ifdef INC_LPDDR4
#include "lpddr4.h"
#endif

static const char *V = "MEGASOC-ETH-002";
static const char *VB = "MEGASOC-ETH-003";

// Buffers: SRAM free area above RW/ZI + stack + heap (0x0080_0000-0x0080_7FFF),
// DRAM window base 0x8000_0000
#define SRAM_TX   0x00808000UL
#define SRAM_RX   0x00809000UL
#define DRAM_TX   0x80010000UL
#define DRAM_RX   0x80011000UL
#define BUF_SPAN  0x800UL

#define FRAME_LEN 116U           // multiple of 4 (case D swaps whole words)
#define POLL_ITERS 400000U
#define FILTER_POLL_ITERS 40000U // after TXB: a filtered frame never arrives

static const uint8_t mac_a[6]  = { 0x02, 0x4D, 0x53, 0x6F, 0x43, 0x01 };  // locally administered
static const uint8_t mac_sa[6] = { 0x02, 0x4D, 0x53, 0x6F, 0x43, 0x02 };

static uint8_t wire[FRAME_LEN];

#define MODER_BASE (ETH_MODER_PAD | ETH_MODER_CRCEN | ETH_MODER_FULLD)

static void build_frame(void)
{
  for (unsigned i = 0; i < 6; i++) { wire[i] = mac_a[i]; wire[6 + i] = mac_sa[i]; }
  wire[12] = 0x88; wire[13] = 0xB5;                  // local experimental EtherType
  for (unsigned i = 14; i < FRAME_LEN; i++) wire[i] = (uint8_t)(i * 7U + 3U);
}

// swap=0: memory = wire order; swap=1: bytes reversed inside each 32-bit word
static void put_frame(unsigned long addr, int swap)
{
  volatile uint8_t *p = (volatile uint8_t *)addr;
  for (unsigned i = 0; i < FRAME_LEN; i++)
    p[i] = swap ? wire[(i & ~3U) + (3U - (i & 3U))] : wire[i];
}

static void fill(unsigned long addr, uint8_t v, unsigned len)
{
  volatile uint8_t *p = (volatile uint8_t *)addr;
  for (unsigned i = 0; i < len; i++) p[i] = v;
}

typedef struct { uint32_t isrc, txbd, rxbd; int got_rx; } xfer_t;

static xfer_t run_one(uint32_t moder, unsigned long tx, unsigned long rx, uint32_t rx_wait)
{
  xfer_t r;
  eth_cache_civac_range(tx, BUF_SPAN);
  eth_cache_civac_range(rx, BUF_SPAN);
  eth_start_one(moder, (uint32_t)tx, FRAME_LEN, (uint32_t)rx);

  r.isrc = eth_wait_int(ETH_INT_TXB | ETH_INT_TXE, POLL_ITERS);
  if (r.isrc & (ETH_INT_TXB | ETH_INT_TXE))
    r.isrc = eth_wait_int(ETH_INT_RXB | ETH_INT_RXE | ETH_INT_BUSY, rx_wait) | r.isrc;
  r.txbd = eth_bd_ctrl(0);
  r.rxbd = eth_bd_ctrl(1);
  r.got_rx = (r.isrc & ETH_INT_RXB) && !(r.rxbd & ETH_RXBD_E);
  eth_mac_stop();
  eth_cache_civac_range(rx, BUF_SPAN);
  return r;
}

static int loop_case(const char *name, unsigned long tx, unsigned long rx)
{
  int e = 0;
  xfer_t r;
  volatile uint8_t *q = (volatile uint8_t *)rx;

  printf("Test: %s (TX 0x%08lx -> RX 0x%08lx, %u bytes)\n", name, tx, rx, FRAME_LEN);
  put_frame(tx, 0);
  fill(rx, 0xCC, BUF_SPAN);
  r = run_one(MODER_BASE | ETH_MODER_PRO, tx, rx, POLL_ITERS);

  if (!(r.isrc & ETH_INT_TXB) || (r.isrc & ETH_INT_TXE) || (r.txbd & (ETH_TXBD_RD | ETH_TXBD_ERRS))) {
    printf("[%s] FAIL %s: transmit (INT_SOURCE 0x%02x, TX BD 0x%08x)\n", V, name,
           (unsigned)r.isrc, (unsigned)r.txbd);
    e++;
  }
  if (!r.got_rx || (r.rxbd & ETH_RXBD_ERRS)) {
    printf("[%s] FAIL %s: receive (INT_SOURCE 0x%02x, RX BD 0x%08x)\n", V, name,
           (unsigned)r.isrc, (unsigned)r.rxbd);
    return e + 1;
  }
  if ((r.rxbd >> 16) != FRAME_LEN + 4U) {          // RX length includes the 4-byte FCS
    printf("[%s] FAIL %s: RX length %u, expected %u\n", V, name,
           (unsigned)(r.rxbd >> 16), FRAME_LEN + 4U);
    e++;
  }
  for (unsigned i = 0; i < FRAME_LEN; i++) {
    if (q[i] != wire[i]) {
      printf("[%s] FAIL %s: byte %u: expected 0x%02x got 0x%02x\n", V, name, i,
             (unsigned)wire[i], (unsigned)q[i]);
      if (++e > 8) break;
    }
  }
  if (q[FRAME_LEN + 4U] != 0xCC) {                 // nothing written past FCS (word aligned)
    printf("[%s] FAIL %s: write past end of frame (0x%02x)\n", V, name, (unsigned)q[FRAME_LEN + 4U]);
    e++;
  }
  if (!e) printf("[%s] %s: %u bytes received intact, RX BD 0x%08x\n", V, name, FRAME_LEN, (unsigned)r.rxbd);
  return e;
}

static int byte_order_case(void)
{
  xfer_t r_mem, r_swp;
  printf("Test: DMA byte order (address filter on, station address %02x:%02x:%02x:%02x:%02x:%02x)\n",
         mac_a[0], mac_a[1], mac_a[2], mac_a[3], mac_a[4], mac_a[5]);

  eth_set_mac_addr(mac_a);

  put_frame(SRAM_TX, 0);
  fill(SRAM_RX, 0xCC, BUF_SPAN);
  r_mem = run_one(MODER_BASE, SRAM_TX, SRAM_RX, FILTER_POLL_ITERS);

  put_frame(SRAM_TX, 1);
  fill(SRAM_RX, 0xCC, BUF_SPAN);
  r_swp = run_one(MODER_BASE, SRAM_TX, SRAM_RX, FILTER_POLL_ITERS);

  eth_wr(ETH_MAC_ADDR0, 0);
  eth_wr(ETH_MAC_ADDR1, 0);

  printf("[%s] frame in wire order in memory:        %s (INT_SOURCE 0x%02x)\n", VB,
         r_mem.got_rx ? "received" : "dropped by address filter", (unsigned)r_mem.isrc);
  printf("[%s] frame byte-swapped per word in memory: %s (INT_SOURCE 0x%02x)\n", VB,
         r_swp.got_rx ? "received" : "dropped by address filter", (unsigned)r_swp.isrc);

  if (!(r_mem.isrc & ETH_INT_TXB) || !(r_swp.isrc & ETH_INT_TXB)) {
    printf("[%s] FAIL byte order: a frame was not transmitted\n", VB);
    return 1;
  }
  if (r_mem.got_rx && !r_swp.got_rx) {
    printf("ETH_BYTE_ORDER: memory order = wire order (correct for the little-endian A53)\n");
    return 0;
  }
  if (!r_mem.got_rx && r_swp.got_rx) {
    printf("ETH_BYTE_ORDER: bytes swapped inside each 32-bit word on the wire\n");
    printf("[%s] FAIL byte order: the DMA sends byte 3 of each word first. A network stack on the\n", VB);
    printf("[%s]      little-endian A53 needs wire order = memory order: set SWAP_DMA_BYTES = 1\n", VB);
    printf("[%s]      on ethmac_subsystem_apb in megasoc_tech_wrapper.sv\n", VB);
    return 1;
  }
  printf("ETH_BYTE_ORDER: inconclusive\n");
  printf("[%s] FAIL byte order: expected exactly one of the two frames to pass the address filter\n", VB);
  return 1;
}

int main(void)
{
  int e = 0;
  UartStdOutInit();
  printf("MegaSoC Ethernet loopback test (%s, %s)\n", V, VB);

#ifdef INC_LPDDR4
  if (lpddr4_init() != 0) {
    printf("[%s] FAIL lpddr4_init\n", V);
    TEST_FAIL();
    return 1;
  }
#endif

  build_frame();
  eth_wr(ETH_INT_MASK, 0);                         // polled; no interrupts
  eth_wr(ETH_INT_SOURCE, ETH_INT_ALL);

  e += loop_case("case A SRAM->SRAM", SRAM_TX, SRAM_RX);
  e += loop_case("case B DRAM->DRAM", DRAM_TX, DRAM_RX);
  e += loop_case("case C SRAM->DRAM", SRAM_TX, DRAM_RX);
  e += byte_order_case();

  eth_mac_stop();
  eth_wr(ETH_MODER, ETH_MODER_RST);
  eth_wr(ETH_TX_BD_NUM, ETH_TX_BD_NUM_RST);

  printf("Total number of eth_loopback errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
