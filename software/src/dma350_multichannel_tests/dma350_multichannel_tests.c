//==============================================================================
// dma350_multichannel_tests.c -- MegaSoC DMA-350, 4 channels plus CPU traffic
//
// INC_DMA builds only. vplan MEGASOC-DMA350-011 (concurrent channels with CPU
// traffic on the same targets).
//
// All four channels are started back to back and run together:
//   ch0  SRAM -> SRAM   4 KB
//   ch1  SRAM -> DRAM   4 KB
//   ch2  DRAM -> SRAM   4 KB
//   ch3  DRAM -> DRAM   8 KB
// While they run, the CPU writes and reads its own SRAM and DRAM areas
// through the bus (cache clean+invalidate after the writes, invalidate
// before the reads), so the NIC sees DMA and CPU traffic on both targets.
// Round A: priorities 15/8/4/0, bursts up to 16 beats, byte elements.
// Round B: equal priorities, single-beat bursts (most arbitration points),
//          64-bit elements.
// Each round: every channel ends with STAT_DONE and no error, every
// destination equals its source, and the CPU data is intact.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "dma350.h"
#include <stdio.h>
#include <stdint.h>
#ifdef INC_LPDDR4
#include "lpddr4.h"
#endif

static const char *V = "MEGASOC-DMA350-011";

#define POLL_ITERS  800000U
#define CPU_SRAM    0x0080D000UL     // 8 KB
#define CPU_DRAM    0x80050000UL     // 8 KB
#define CPU_BYTES   0x2000UL
#define CPU_PASSES  4U

typedef struct { unsigned long src, dst; uint32_t bytes; } xfer_t;

static const xfer_t xf[DMA350_NUM_CH] = {
  { 0x00808000UL, 0x0080A000UL, 4096 },   // ch0 SRAM -> SRAM
  { 0x00809000UL, 0x80041000UL, 4096 },   // ch1 SRAM -> DRAM
  { 0x80042000UL, 0x0080B000UL, 4096 },   // ch2 DRAM -> SRAM
  { 0x80043000UL, 0x80046000UL, 8192 },   // ch3 DRAM -> DRAM
};

static const unsigned prio[DMA350_NUM_CH] = { 15U, 8U, 4U, 0U };

static inline uint8_t pattern(uint32_t i, uint32_t seed) { return (uint8_t)((i * 13U + seed) ^ (i >> 7)); }

// CPU traffic on one area: write, clean to memory, invalidate, read back.
static uint32_t cpu_traffic(unsigned long base, uint32_t seed)
{
  volatile uint32_t *p = (volatile uint32_t *)base;
  uint32_t bad = 0;
  for (uint32_t i = 0; i < CPU_BYTES / 4U; i++) p[i] = (uint32_t)((i * 0x9E3779B9U) ^ seed);
  dma350_cache_civac_range(base, CPU_BYTES);
  for (uint32_t i = 0; i < CPU_BYTES / 4U; i++) if (p[i] != ((uint32_t)((i * 0x9E3779B9U) ^ seed))) bad++;
  return bad;
}

static int run_round(const char *name, int equal_prio, unsigned transize, unsigned maxburst, uint32_t seed)
{
  int e = 0;
  uint32_t st[DMA350_NUM_CH], busy_seen = 0, cpu_bad = 0;

  printf("Test: round %s\n", name);
  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) {
    volatile uint8_t *s = (volatile uint8_t *)xf[ch].src;
    volatile uint8_t *d = (volatile uint8_t *)xf[ch].dst;
    for (uint32_t i = 0; i < xf[ch].bytes; i++) { s[i] = pattern(i, seed + ch); d[i] = 0; }
    dma350_cache_civac_range(xf[ch].src, xf[ch].bytes);
    dma350_cache_civac_range(xf[ch].dst, xf[ch].bytes);
    dma350_ch_setup_m2m(ch, xf[ch].src, xf[ch].dst, xf[ch].bytes >> transize, transize,
                        equal_prio ? 8U : prio[ch], 0);
    dma_ch_wr(ch, DMA_CH_SRCTRANSCFG, DMA_TRANSCFG_MAXBURSTLEN(maxburst) | DMA_TRANSCFG_NONSEC | DMA_MEMATTR_NORMAL_NC);
    dma_ch_wr(ch, DMA_CH_DESTRANSCFG, DMA_TRANSCFG_MAXBURSTLEN(maxburst) | DMA_TRANSCFG_NONSEC | DMA_MEMATTR_NORMAL_NC);
  }

  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) dma350_ch_start(ch);
  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) busy_seen += (uint32_t)(dma350_ch_busy(ch) != 0);
  printf("[%s] channels busy right after start: %u of 4\n", V, (unsigned)busy_seen);

  for (unsigned pass = 0; pass < CPU_PASSES; pass++) {
    cpu_bad += cpu_traffic(CPU_SRAM, seed ^ (pass << 8));
    cpu_bad += cpu_traffic(CPU_DRAM, ~seed ^ (pass << 8));
  }

  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) st[ch] = dma350_ch_wait(ch, POLL_ITERS);

  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) {
    volatile uint8_t *s = (volatile uint8_t *)xf[ch].src;
    volatile uint8_t *d = (volatile uint8_t *)xf[ch].dst;
    uint32_t bad = 0;
    dma350_cache_civac_range(xf[ch].dst, xf[ch].bytes);
    for (uint32_t i = 0; i < xf[ch].bytes; i++) if (d[i] != s[i]) bad++;
    if ((st[ch] & 0x80000000UL) || !(st[ch] & DMA_STAT_DONE) || (st[ch] & DMA_STAT_ERR) || bad) {
      printf("[%s] FAIL round %s ch%u: status 0x%08x errinfo 0x%08x, %u bad bytes\n", V, name, ch,
             (unsigned)st[ch], (unsigned)dma_ch_rd(ch, DMA_CH_ERRINFO), (unsigned)bad);
      e++;
    } else {
      printf("[%s] ok   round %s ch%u %u bytes\n", V, name, ch, (unsigned)xf[ch].bytes);
    }
    dma_ch_wr(ch, DMA_CH_STATUS, DMA_STAT_W1C_ALL);
  }
  if (cpu_bad) { printf("[%s] FAIL round %s: %u bad CPU words\n", V, name, (unsigned)cpu_bad); e++; }
  else printf("[%s] ok   round %s CPU traffic intact\n", V, name);
  return e;
}

int main(void)
{
  int e = 0;

  UartStdOutInit();
  printf("MegaSoC DMA-350 multichannel test (%s)\n", V);

#ifdef INC_LPDDR4
  if (lpddr4_init() != 0) { printf("[%s] FAIL lpddr4_init\n", V); TEST_FAIL(); return 1; }
#endif

  e += run_round("A (priorities 15/8/4/0, 16-beat bursts, bytes)", 0, DMA_TRANSIZE_8, 15, 0x31);
  e += run_round("B (equal priority, 1-beat bursts, 64-bit)",       1, DMA_TRANSIZE_64, 0, 0x7C);

  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) (void)dma350_ch_clear(ch, 2000U);

  printf("Total number of dma350_multichannel errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
