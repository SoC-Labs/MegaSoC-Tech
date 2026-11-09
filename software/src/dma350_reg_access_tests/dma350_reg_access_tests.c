//==============================================================================
// dma350_reg_access_tests.c -- MegaSoC DMA-350 register access
//
// INC_DMA builds only. vplan MEGASOC-DMA350-001 (identity and build
// configuration), MEGASOC-DMA350-002 (channel register reset values and
// read/write), MEGASOC-DMA350-003 (register window decode).
//
// Expected values come from the public DMA-350 TRM (102482_0000_03) and the
// MegaSoC configuration (4 channels, 64-bit data, 44-bit address, no
// triggers, no TrustZone).
//   1. Information frame: IIDR/PIDR/CIDR identify an Arm DMA-350 r0p0;
//      DMA_BUILDCFG0/1/2 match the configuration.
//   2. Every channel: reset values of the command registers; channel build
//      configuration (64-bit data, 44-bit address, no triggers, no streams).
//   3. Every channel: write/read-back of SRCADDR, DESADDR, the 44-bit
//      address high words, XSIZE, XADDRINC, FILLVAL, INTREN; then CLEARCMD
//      returns them to reset values.
//   4. Window: DMA_S is 64 KB but the DMA decodes 8 KB. Reads above 8 KB
//      return 0 without an abort, and writes there do not reach the channel
//      registers (no aliasing).
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "dma350.h"
#include <stdio.h>
#include <stdint.h>

static const char *V1 = "MEGASOC-DMA350-001";
static const char *V2 = "MEGASOC-DMA350-002";
static const char *V3 = "MEGASOC-DMA350-003";

#define CLEAR_POLL 2000U

static int check(const char *v, const char *what, uint32_t exp, uint32_t got)
{
  if (exp != got) {
    printf("[%s] FAIL %s: expected 0x%x got 0x%x\n", v, what, (unsigned)exp, (unsigned)got);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] ok   %s = 0x%x\n", v, what, (unsigned)got);
#endif
  return 0;
}

static int test_info(void)
{
  int e = 0;
  uint32_t b0, b1, b2;

  printf("Test: identity and build configuration\n");
  e += check(V1, "IIDR (DMA-350 r0p0, Arm)", 0x3A00043BUL, dma_info_rd(DMA_INFO_IIDR));
  e += check(V1, "PIDR0", 0xA0, dma_info_rd(DMA_INFO_PIDR0) & 0xFF);
  e += check(V1, "PIDR1", 0xB3, dma_info_rd(DMA_INFO_PIDR1) & 0xFF);
  e += check(V1, "PIDR2 [3:0] (JEDEC, designer)", 0xB, dma_info_rd(DMA_INFO_PIDR2) & 0xF);
  e += check(V1, "PIDR4 (1 x 4 KB, continuation 4)", 0x14, dma_info_rd(DMA_INFO_PIDR4) & 0xFF);
  e += check(V1, "CIDR0", 0x0D, dma_info_rd(DMA_INFO_CIDR0) & 0xFF);
  e += check(V1, "CIDR1", 0xF0, dma_info_rd(DMA_INFO_CIDR1) & 0xFF);
  e += check(V1, "CIDR2", 0x05, dma_info_rd(DMA_INFO_CIDR2) & 0xFF);
  e += check(V1, "CIDR3", 0xB1, dma_info_rd(DMA_INFO_CIDR3) & 0xFF);

  b0 = dma_info_rd(DMA_INFO_BUILDCFG0);
  b1 = dma_info_rd(DMA_INFO_BUILDCFG1);
  b2 = dma_info_rd(DMA_INFO_BUILDCFG2);
  printf("[%s] DMA_BUILDCFG0=0x%08x BUILDCFG1=0x%08x BUILDCFG2=0x%08x\n", V1,
         (unsigned)b0, (unsigned)b1, (unsigned)b2);
  e += check(V1, "BUILDCFG0.NUM_CHANNELS (4 channels)", 3, (b0 >> 4) & 0x3F);
  e += check(V1, "BUILDCFG0.ADDR_WIDTH (44 bits)", 43, (b0 >> 10) & 0x3F);
  e += check(V1, "BUILDCFG0.DATA_WIDTH (64 bits)", 3, (b0 >> 16) & 0x7);
  e += check(V1, "BUILDCFG0.CHID_WIDTH", 0, (b0 >> 20) & 0x1F);
  e += check(V1, "BUILDCFG1.NUM_TRIGGER_IN", 0, b1 & 0x1FF);
  e += check(V1, "BUILDCFG1.NUM_TRIGGER_OUT", 0, (b1 >> 9) & 0x7F);
  e += check(V1, "BUILDCFG2.HAS_TZ (no TrustZone)", 0, (b2 >> 8) & 1U);
  return e;
}

static int test_channel_reset(unsigned ch)
{
  int e = 0;
  uint32_t c0, c1;

  e += check(V2, "CH_CMD reset", 0, dma_ch_rd(ch, DMA_CH_CMD));
  e += check(V2, "CH_STATUS reset", 0, dma_ch_rd(ch, DMA_CH_STATUS));
  e += check(V2, "CH_INTREN reset", 0, dma_ch_rd(ch, DMA_CH_INTREN));
  e += check(V2, "CH_CTRL reset", DMA_CH_CTRL_RST, dma_ch_rd(ch, DMA_CH_CTRL));
  e += check(V2, "CH_SRCADDR reset", 0, dma_ch_rd(ch, DMA_CH_SRCADDR));
  e += check(V2, "CH_DESADDR reset", 0, dma_ch_rd(ch, DMA_CH_DESADDR));
  e += check(V2, "CH_XSIZE reset", 0, dma_ch_rd(ch, DMA_CH_XSIZE));
  e += check(V2, "CH_SRCTRANSCFG reset", DMA_CH_TRANSCFG_RST, dma_ch_rd(ch, DMA_CH_SRCTRANSCFG));
  e += check(V2, "CH_DESTRANSCFG reset", DMA_CH_TRANSCFG_RST, dma_ch_rd(ch, DMA_CH_DESTRANSCFG));
  e += check(V2, "CH_XADDRINC reset", 0, dma_ch_rd(ch, DMA_CH_XADDRINC));
  e += check(V2, "CH_LINKADDR reset", 0, dma_ch_rd(ch, DMA_CH_LINKADDR));
  e += check(V2, "CH_ERRINFO reset", 0, dma_ch_rd(ch, DMA_CH_ERRINFO));
  e += check(V2, "CH_IIDR", 0x3A00043BUL, dma_ch_rd(ch, DMA_CH_IIDR));

  c0 = dma_ch_rd(ch, DMA_CH_BUILDCFG0);
  c1 = dma_ch_rd(ch, DMA_CH_BUILDCFG1);
  printf("[%s] ch%u CH_BUILDCFG0=0x%08x CH_BUILDCFG1=0x%08x\n", V2, ch, (unsigned)c0, (unsigned)c1);
  e += check(V2, "CH_BUILDCFG0.DATA_WIDTH (64 bits)", 3, (c0 >> 22) & 0x7);
  e += check(V2, "CH_BUILDCFG0.ADDR_WIDTH (44 bits)", 43, (c0 >> 16) & 0x3F);
  e += check(V2, "CH_BUILDCFG1.HAS_TRIGIN", 0, (c1 >> 5) & 1U);
  e += check(V2, "CH_BUILDCFG1.HAS_TRIGOUT", 0, (c1 >> 6) & 1U);
  e += check(V2, "CH_BUILDCFG1.HAS_STREAM", 0, (c1 >> 11) & 1U);
  e += check(V2, "CH_BUILDCFG1.HAS_CMDLINK", 1, (c1 >> 8) & 1U);
  e += check(V2, "CH_BUILDCFG1.HAS_2D (channels 0,1 only)", (ch < 2) ? 1U : 0U, (c1 >> 2) & 1U);
  return e;
}

// Extended features (2D, wrap, fill, template) on channels 0 and 1 only
static int has_ext(unsigned ch) { return ch < 2U; }

static int test_channel_rw(unsigned ch)
{
  int e = 0;
  const uint32_t k = 0x11111111UL * (ch + 1U);

  dma_ch_wr(ch, DMA_CH_SRCADDR,   0xA5A5A5A0UL ^ k);
  dma_ch_wr(ch, DMA_CH_DESADDR,   0x5A5A5A58UL ^ k);
  dma_ch_wr(ch, DMA_CH_SRCADDRHI, 0x00000ABCUL);
  dma_ch_wr(ch, DMA_CH_DESADDRHI, 0x00000543UL);
  dma_ch_wr(ch, DMA_CH_XSIZE,     0x12345678UL);
  dma_ch_wr(ch, DMA_CH_XADDRINC,  0x00020001UL);
  dma_ch_wr(ch, DMA_CH_FILLVAL,   0xC0FFEE00UL ^ k);
  dma_ch_wr(ch, DMA_CH_INTREN,    DMA_INTREN_DONE | DMA_INTREN_ERR | DMA_INTREN_DISABLED | DMA_INTREN_STOPPED);

  e += check(V2, "CH_SRCADDR r/w", 0xA5A5A5A0UL ^ k, dma_ch_rd(ch, DMA_CH_SRCADDR));
  e += check(V2, "CH_DESADDR r/w", 0x5A5A5A58UL ^ k, dma_ch_rd(ch, DMA_CH_DESADDR));
  e += check(V2, "CH_SRCADDRHI r/w (bits 43:32)", 0xABC, dma_ch_rd(ch, DMA_CH_SRCADDRHI) & 0xFFF);
  e += check(V2, "CH_DESADDRHI r/w (bits 43:32)", 0x543, dma_ch_rd(ch, DMA_CH_DESADDRHI) & 0xFFF);
  // Channels without wrap/stream support (2, 3): DESXSIZE is read-only and
  // follows SRCXSIZE; FILLVAL is not present (RAZ/WI). TRM 6.5.1.9/6.5.1.15.
  e += check(V2, "CH_XSIZE r/w", has_ext(ch) ? 0x12345678UL : 0x56785678UL, dma_ch_rd(ch, DMA_CH_XSIZE));
  e += check(V2, "CH_XADDRINC r/w", 0x00020001UL, dma_ch_rd(ch, DMA_CH_XADDRINC));
  e += check(V2, "CH_FILLVAL r/w", has_ext(ch) ? (0xC0FFEE00UL ^ k) : 0U, dma_ch_rd(ch, DMA_CH_FILLVAL));
  e += check(V2, "CH_INTREN r/w", 0xF, dma_ch_rd(ch, DMA_CH_INTREN) & 0xF);
  e += check(V2, "channel stays idle", 0, (uint32_t)dma350_ch_busy(ch));

  e += check(V2, "CLEARCMD completes", 0, (uint32_t)dma350_ch_clear(ch, CLEAR_POLL));
  e += check(V2, "CH_SRCADDR after CLEARCMD", 0, dma_ch_rd(ch, DMA_CH_SRCADDR));
  e += check(V2, "CH_XSIZE after CLEARCMD", 0, dma_ch_rd(ch, DMA_CH_XSIZE));
  e += check(V2, "CH_INTREN after CLEARCMD", 0, dma_ch_rd(ch, DMA_CH_INTREN));
  return e;
}

static int test_window(void)
{
  int e = 0;
  uint32_t r = 0;

  printf("Test: register window decode (8 KB of 64 KB)\n");
  // Reads above the DMA's 8 KB: 0, no abort
  r |= dma_rd(DMA350_BASE + 0x2000UL);
  r |= dma_rd(DMA350_BASE + 0x3010UL);       // would alias CH0 SRCADDR at 0x1010
  r |= dma_rd(DMA350_BASE + 0x8F00UL);       // would alias the information frame
  r |= dma_rd(DMA350_BASE + 0xFFFCUL);
  e += check(V3, "reads above 8 KB return 0", 0, r);

  // Writes above 8 KB do not reach the channel registers
  dma_ch_wr(0, DMA_CH_SRCADDR, 0x0BADF00CUL);
  dma_wr(DMA350_BASE + 0x3010UL, 0xDEADBEECUL);
  dma_wr(DMA350_BASE + 0xF010UL, 0xFEEDFACCUL);
  e += check(V3, "CH0 SRCADDR not aliased", 0x0BADF00CUL, dma_ch_rd(0, DMA_CH_SRCADDR));
  dma_ch_wr(0, DMA_CH_SRCADDR, 0);
  // Reserved space inside the 8 KB (e.g. unused channel frames 4-7) is not
  // probed: the TRM leaves accesses there unpredictable.
  return e;
}

int main(void)
{
  int e = 0;

  UartStdOutInit();
  printf("MegaSoC DMA-350 register access test (%s, %s, %s)\n", V1, V2, V3);

  e += test_info();
  for (unsigned ch = 0; ch < DMA350_NUM_CH; ch++) {
    printf("Test: channel %u reset values and read/write\n", ch);
    e += test_channel_reset(ch);
    e += test_channel_rw(ch);
  }
  e += test_window();

  printf("Total number of dma350_reg_access errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
