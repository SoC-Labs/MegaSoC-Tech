//==============================================================================
// dma350_m2m_tests.c -- MegaSoC DMA-350 memory-to-memory copies
//
// INC_DMA builds only. vplan MEGASOC-DMA350-004 (ROM/SRAM copies),
// MEGASOC-DMA350-005 (DRAM copies), MEGASOC-DMA350-006 (sizes, unaligned starts,
// element sizes, no writes outside the destination).
//
// Every case is a 1D command (XTYPE continue, increment 1), normal
// non-cacheable memory attributes, on channel (case number % 4):
//   - ROM -> SRAM, 4 KB, byte and 64-bit elements (checked against a CPU
//     read of the ROM)
//   - SRAM -> SRAM, 1 B .. 4 KB, source/destination offsets 0..7
//   - SRAM -> SRAM, 16/32/64-bit elements, aligned
//   - SRAM -> DRAM, DRAM -> SRAM, DRAM -> DRAM, up to 16 KB (crosses many
//     1 KB burst boundaries), aligned and unaligned
// For each case: STAT_DONE set, STAT_ERR clear, CH_ERRINFO 0, the channel
// back to idle, the destination equal to the source, and 16 guard bytes
// before and after the destination unchanged.
// The DMA is not cache coherent: source and destination are cleaned and
// invalidated before the copy, the destination is invalidated again before
// it is checked.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "dma350.h"
#include <stdio.h>
#include <stdint.h>
#ifdef INC_LPDDR4
#include "lpddr4.h"
#endif

static const char *V4 = "MEGASOC-DMA350-004";
static const char *V5 = "MEGASOC-DMA350-005";
static const char *V6 = "MEGASOC-DMA350-006";

// Buffers: SRAM free area above RW/ZI + stack + heap (0x0080_0000-0x0080_7FFF),
// DRAM window base 0x8000_0000
#define ROM_BASE    0x00000000UL
#define SRAM_SRC    0x00808000UL     // 8 KB
#define SRAM_DST    0x0080A000UL     // 8 KB
#define DRAM_SRC    0x80020000UL     // 64 KB
#define DRAM_DST    0x80030000UL     // 64 KB
#define GUARD       16UL
#define GUARD_BYTE  0xEEU
#define POLL_ITERS  400000U

typedef struct {
  const char   *v;
  const char   *name;
  unsigned long src_base;
  unsigned long dst_base;
  unsigned      src_off;
  unsigned      dst_off;
  uint32_t      bytes;
  unsigned      transize;
} dma_case_t;

static const dma_case_t cases[] = {
  // ROM source
  { "MEGASOC-DMA350-004", "ROM->SRAM 4KB bytes",       ROM_BASE, SRAM_DST, 0, 0, 4096, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-004", "ROM->SRAM 4KB 64-bit",      ROM_BASE, SRAM_DST, 0, 0, 4096, DMA_TRANSIZE_64 },
  { "MEGASOC-DMA350-004", "ROM->SRAM 1000B +3/+5",     ROM_BASE, SRAM_DST, 3, 5, 1000, DMA_TRANSIZE_8  },
  // SRAM -> SRAM sizes and offsets
  { "MEGASOC-DMA350-006", "SRAM->SRAM 1B",             SRAM_SRC, SRAM_DST, 0, 0,    1, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 2B +1/+0",       SRAM_SRC, SRAM_DST, 1, 0,    2, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 3B +0/+7",       SRAM_SRC, SRAM_DST, 0, 7,    3, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 7B +5/+3",       SRAM_SRC, SRAM_DST, 5, 3,    7, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 8B",             SRAM_SRC, SRAM_DST, 0, 0,    8, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 9B +7/+1",       SRAM_SRC, SRAM_DST, 7, 1,    9, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 63B +2/+6",      SRAM_SRC, SRAM_DST, 2, 6,   63, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 64B",            SRAM_SRC, SRAM_DST, 0, 0,   64, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 65B +4/+4",      SRAM_SRC, SRAM_DST, 4, 4,   65, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 1000B +6/+2",    SRAM_SRC, SRAM_DST, 6, 2, 1000, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-004", "SRAM->SRAM 4KB",            SRAM_SRC, SRAM_DST, 0, 0, 4096, DMA_TRANSIZE_8  },
  // element sizes
  { "MEGASOC-DMA350-006", "SRAM->SRAM 2KB 16-bit",     SRAM_SRC, SRAM_DST, 0, 0, 2048, DMA_TRANSIZE_16 },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 2KB 32-bit",     SRAM_SRC, SRAM_DST, 0, 0, 2048, DMA_TRANSIZE_32 },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 2KB 64-bit",     SRAM_SRC, SRAM_DST, 0, 0, 2048, DMA_TRANSIZE_64 },
  { "MEGASOC-DMA350-006", "SRAM->SRAM 40B 32-bit +4/+8",SRAM_SRC, SRAM_DST, 4, 8,  40, DMA_TRANSIZE_32 },
  // DRAM
  { "MEGASOC-DMA350-005", "SRAM->DRAM 4KB",            SRAM_SRC, DRAM_DST, 0, 0, 4096, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-005", "SRAM->DRAM 1001B +3/+5",    SRAM_SRC, DRAM_DST, 3, 5, 1001, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-005", "DRAM->SRAM 4KB",            DRAM_SRC, SRAM_DST, 0, 0, 4096, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-005", "DRAM->SRAM 777B +1/+6",     DRAM_SRC, SRAM_DST, 1, 6,  777, DMA_TRANSIZE_8  },
  { "MEGASOC-DMA350-005", "DRAM->DRAM 16KB 64-bit",    DRAM_SRC, DRAM_DST, 0, 0,16384, DMA_TRANSIZE_64 },
  { "MEGASOC-DMA350-005", "DRAM->DRAM 5003B +7/+0",    DRAM_SRC, DRAM_DST, 7, 0, 5003, DMA_TRANSIZE_8  },
};

static inline uint8_t pattern(uint32_t i, uint32_t seed) { return (uint8_t)((i * 7U + seed) ^ (i >> 8)); }

static int run_case(unsigned n, const dma_case_t *c)
{
  int e = 0;
  const unsigned ch = n % DMA350_NUM_CH;
  const unsigned long src = c->src_base + c->src_off;
  const unsigned long dst = c->dst_base + GUARD + c->dst_off;
  volatile uint8_t *s = (volatile uint8_t *)src;
  volatile uint8_t *d = (volatile uint8_t *)dst;
  uint32_t st, ei, bad = 0, first_bad = 0;
  uint8_t exp0 = 0, got0 = 0;

  // Source pattern (the ROM keeps its own contents)
  if (c->src_base != ROM_BASE)
    for (uint32_t i = 0; i < c->bytes; i++) s[i] = pattern(i, n);
  // Destination plus guard bytes
  for (uint32_t i = 0; i < c->bytes + 2U * GUARD; i++) ((volatile uint8_t *)(dst - GUARD))[i] = GUARD_BYTE;
  dma350_cache_civac_range(src, c->bytes);
  dma350_cache_civac_range(dst - GUARD, c->bytes + 2U * GUARD);

  dma350_ch_setup_m2m(ch, src, dst, c->bytes >> c->transize, c->transize, 0, 0);
  dma350_ch_start(ch);
  st = dma350_ch_wait(ch, POLL_ITERS);
  ei = dma_ch_rd(ch, DMA_CH_ERRINFO);

  dma350_cache_civac_range(dst - GUARD, c->bytes + 2U * GUARD);
  for (uint32_t i = 0; i < c->bytes; i++) {
    if (d[i] != s[i]) {
      if (bad == 0) { first_bad = i; exp0 = s[i]; got0 = d[i]; }
      bad++;
    }
  }
  for (uint32_t i = 0; i < GUARD; i++) {
    if (((volatile uint8_t *)(dst - GUARD))[i] != GUARD_BYTE) bad++;
    if (d[c->bytes + i] != GUARD_BYTE) bad++;
  }

  if ((st & 0x80000000UL) || !(st & DMA_STAT_DONE) || (st & DMA_STAT_ERR) || ei != 0 || bad != 0) {
    printf("[%s] FAIL case %u ch%u %s: status 0x%08x errinfo 0x%08x, %u bad bytes",
           c->v, n, ch, c->name, (unsigned)st, (unsigned)ei, (unsigned)bad);
    if (bad) printf(" (first at +%u: expected 0x%02x got 0x%02x)", (unsigned)first_bad, exp0, got0);
    printf("\n");
    e = 1;
  } else {
    printf("[%s] ok   case %u ch%u %s\n", c->v, n, ch, c->name);
  }

  dma_ch_wr(ch, DMA_CH_STATUS, DMA_STAT_W1C_ALL);
  return e;
}

int main(void)
{
  int e = 0;
  const unsigned ncases = (unsigned)(sizeof(cases) / sizeof(cases[0]));

  UartStdOutInit();
  printf("MegaSoC DMA-350 memory-to-memory test (%s, %s, %s)\n", V4, V5, V6);

#ifdef INC_LPDDR4
  if (lpddr4_init() != 0) { printf("[%s] FAIL lpddr4_init\n", V5); TEST_FAIL(); return 1; }
#endif

  for (unsigned n = 0; n < ncases; n++) e += run_case(n, &cases[n]);

  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) (void)dma350_ch_clear(ch, 2000U);

  printf("Total number of dma350_m2m errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
