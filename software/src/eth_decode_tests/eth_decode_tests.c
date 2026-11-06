//==============================================================================
// eth_decode_tests.c -- MegaSoC Ethernet address decode, both directions
//
// INC_ETH builds only. vplan MEGASOC-ETH-007 (ETH_S window), MEGASOC-ETH-008
// (ETH_M reach).
//
// 1. ETH_S, CPU -> 0x0102_0000-0x0102_FFFF: every access completes OKAY (no
//    external abort): MAC, PTP and checksum pages, and the unused pages
//    +0x3000-0xFFFF, which read 0 and ignore writes.
// 2. ETH_M, MAC DMA -> NIC: the MAC's master port reaches SRAM and DRAM only.
//    Positive control: a frame transmitted from SRAM.
//    Boundary probe: TX buffer pointer 0x007F_FFF0, the last 16 bytes of the
//    QSPI flash window (CPU-only), length 32. The MAC does not handle DMA
//    error responses: it advances the address but only counts the length
//    down on an acknowledge, and nothing but a length of 0 (or a TX abort)
//    ends the read loop -- clearing TXEN does not. So the DMA reads the 4
//    flash words (4 error responses, bench "ETH_M_MON: error response"
//    lines), carries on into SRAM at 0x0080_0000 (acknowledged), and sends
//    a frame made of SRAM's first 8 words. The check: the frame equals SRAM
//    0x0080_0000-0x0080_001F (either byte order) and contains no flash word.
//    A pointer further from a reachable region would leave the DMA sweeping
//    the address space for a long time (found 2026-11-06 with a ROM pointer),
//    so the probe is placed just below SRAM on purpose.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "ethmac.h"
#include <stdio.h>
#include <stdint.h>
#ifdef INC_LPDDR4
#include "lpddr4.h"
#endif

static const char *VS = "MEGASOC-ETH-007";
static const char *VM = "MEGASOC-ETH-008";

#define SRAM_TX     0x00808000UL
#define SRAM_RX     0x00809000UL
#define SRAM_BASE   0x00800000UL
#define FLASH_TOP16 0x007FFFF0UL       // last 16 bytes of the QSPI flash window
#define PROBE_LEN   32U
#define POLL_ITERS  400000U

static volatile uint32_t g_abort_count = 0;
void abort_handler(void) { g_abort_count++; }

static int check(const char *v, const char *what, uint32_t exp, uint32_t got)
{
  if (exp != got) {
    printf("[%s] FAIL %s: expected 0x%08x got 0x%08x\n", v, what, (unsigned)exp, (unsigned)got);
    return 1;
  }
  printf("[%s] ok   %s = 0x%08x\n", v, what, (unsigned)got);
  return 0;
}

static inline uint32_t rd(unsigned long a) { return *(volatile uint32_t *)a; }
static inline void wr(unsigned long a, uint32_t v) { *(volatile uint32_t *)a = v; __asm__ volatile ("dsb sy" ::: "memory"); }
static inline uint32_t bswap(uint32_t x)
{ return (x >> 24) | ((x >> 8) & 0xFF00U) | ((x << 8) & 0xFF0000U) | (x << 24); }

static int test_eth_s_window(void)
{
  int e = 0;
  uint32_t a0 = g_abort_count;
  static const unsigned long unused[] = { 0x3000UL, 0x4000UL, 0x8000UL, 0xC000UL, 0xFFFCUL };

  printf("Test: ETH_S window 0x0102_0000-0x0102_FFFF (CPU accesses complete OKAY)\n");
  e += check(VS, "MAC MODER (+0x0000)", ETH_MODER_RST, rd(ETH_BASE + ETH_MODER));
  (void)rd(ETH_PTP_BASE);
  e += check(VS, "checksum ID (+0x2000)", ETH_CKSUM_ID_VALUE, rd(ETH_CKSUM_BASE + ETH_CKSUM_ID));
  for (unsigned i = 0; i < 5; i++) {
    printf("[%s] probing unused +0x%04lx (a hang here = the window does not answer unused pages)\n",
           VS, unused[i]);
    e += check(VS, "unused page reads 0", 0, rd(ETH_BASE + unused[i]));
    wr(ETH_BASE + unused[i], 0xA5A5A5A5UL);
    e += check(VS, "unused page after write", 0, rd(ETH_BASE + unused[i]));
  }
  e += check(VS, "MAC MODER unchanged", ETH_MODER_RST, rd(ETH_BASE + ETH_MODER));
  e += check(VS, "external aborts in the window", a0, g_abort_count);
  return e;
}

// Transmit PROBE_LEN bytes from `src` (padded to 60 by the MAC), receive the
// loopback into SRAM_RX. Returns 0 if the frame went out and came back.
static int probe_tx(unsigned long src, uint32_t *isrc, uint32_t *rxbd)
{
  volatile uint8_t *q = (volatile uint8_t *)SRAM_RX;
  for (unsigned i = 0; i < 0x80; i++) q[i] = 0xCC;
  eth_cache_civac_range(SRAM_RX, 0x100);
  if (src == SRAM_TX) eth_cache_civac_range(SRAM_TX, 0x100);
  eth_start_one(ETH_MODER_PAD | ETH_MODER_CRCEN | ETH_MODER_FULLD | ETH_MODER_PRO,
                (uint32_t)src, PROBE_LEN, (uint32_t)SRAM_RX);
  *isrc = eth_wait_int(ETH_INT_RXB | ETH_INT_RXE | ETH_INT_TXE | ETH_INT_BUSY, POLL_ITERS);
  *isrc |= eth_rd(ETH_INT_SOURCE);
  *rxbd = eth_bd_ctrl(1);
  eth_mac_stop();
  eth_cache_civac_range(SRAM_RX, 0x100);
  return ((*isrc & ETH_INT_RXB) && !(*rxbd & ETH_RXBD_E)) ? 0 : 1;
}

static int test_eth_m_reach(void)
{
  int e = 0;
  uint32_t isrc, rxbd, ref[PROBE_LEN / 4];
  volatile uint8_t *t = (volatile uint8_t *)SRAM_TX;

  printf("Test: ETH_M reach\n");

  // Positive control: SRAM source, known pattern comes back
  for (unsigned i = 0; i < PROBE_LEN; i++) t[i] = (uint8_t)(0x40 + i);
  if (probe_tx(SRAM_TX, &isrc, &rxbd)) {
    printf("[%s] FAIL positive control SRAM: no loopback (INT_SOURCE 0x%02x, RX BD 0x%08x)\n",
           VM, (unsigned)isrc, (unsigned)rxbd);
    return 1;                                  // the negative checks below would be vacuous
  }
  e += check(VM, "positive control SRAM word 0", rd(SRAM_TX), rd(SRAM_RX));
  e += check(VM, "positive control SRAM word 7", rd(SRAM_TX + 28), rd(SRAM_RX + 28));

  // Flash -> SRAM boundary
  printf("ETH_DECODE: probe 0x%08lx-0x%08lx (expect exactly 4 ETH_M_MON error responses,\n", FLASH_TOP16, FLASH_TOP16 + 0xFUL);
  printf("ETH_DECODE: 0x007ffff0-0x007ffffc, then SRAM from 0x00800000 acknowledged)\n");
  eth_cache_civac_range(SRAM_BASE, 0x40);              // DMA reads SRAM's backing store
  eth_cache_civac_range(FLASH_TOP16, 0x10);
  {
    uint32_t flash[4];
    for (unsigned w = 0; w < 4; w++) flash[w] = rd(FLASH_TOP16 + 4UL * w);
    for (unsigned w = 0; w < PROBE_LEN / 4; w++) ref[w] = rd(SRAM_BASE + 4UL * w);
    if (probe_tx(FLASH_TOP16, &isrc, &rxbd)) {
      printf("[%s] FAIL boundary probe: no frame (INT_SOURCE 0x%02x, RX BD 0x%08x)\n", VM,
             (unsigned)isrc, (unsigned)rxbd);
      e++;
    } else {
      for (unsigned w = 0; w < PROBE_LEN / 4; w++) {
        uint32_t got = rd(SRAM_RX + 4UL * w);
        if (got != ref[w] && got != bswap(ref[w])) {
          printf("[%s] FAIL boundary probe word %u: got 0x%08x, SRAM 0x%08lx holds 0x%08x\n", VM, w,
                 (unsigned)got, SRAM_BASE + 4UL * w, (unsigned)ref[w]);
          e++;
        }
        for (unsigned k = 0; k < 4; k++)
          if (flash[k] != ref[w] && flash[k] != 0 && flash[k] != 0xFFFFFFFFUL &&
              (got == flash[k] || got == bswap(flash[k]))) {
            printf("[%s] FAIL boundary probe word %u = flash word %u (0x%08x): ETH_M reached flash\n",
                   VM, w, k, (unsigned)got);
            e++;
          }
      }
      if (!e) printf("[%s] ok   flash words refused, frame = SRAM 0x%08lx-0x%08lx\n", VM,
                     SRAM_BASE, SRAM_BASE + PROBE_LEN - 1);
    }
  }
  return e;
}

int main(void)
{
  int e = 0;
  UartStdOutInit();
  printf("MegaSoC Ethernet decode test (%s, %s)\n", VS, VM);

#ifdef INC_LPDDR4
  if (lpddr4_init() != 0) { printf("[%s] FAIL lpddr4_init\n", VM); TEST_FAIL(); return 1; }
#endif

  e += test_eth_s_window();
  e += test_eth_m_reach();

  eth_wr(ETH_MODER, ETH_MODER_RST);
  eth_wr(ETH_TX_BD_NUM, ETH_TX_BD_NUM_RST);
  if (g_abort_count) {
    printf("FAIL %u unexpected external abort(s)\n", (unsigned)g_abort_count);
    e++;
  }
  printf("Total number of eth_decode errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
