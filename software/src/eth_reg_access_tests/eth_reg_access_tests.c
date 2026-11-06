//==============================================================================
// eth_reg_access_tests.c -- MegaSoC Ethernet subsystem register access test
//
// INC_ETH builds only. vplan MEGASOC-ETH-001.
//
// Checks, through the NIC-400 ETH_S APB target at 0x0102_0000:
//   1. Reset values of the OpenCores ethmac registers.
//   2. Read/write of each writable register (implemented-bit masks), and
//      that TX_BD_NUM ignores values above 0x80.
//   3. Byte-strobe writes (STRB -> APB PSTRB) land in the right byte lane.
//   4. Buffer-descriptor RAM (0x400-0x7FF) read/write at both ends.
//   5. Sub-block decode inside the window: checksum unit ID at +0x2000,
//      PTP block at +0x1000 completes without an abort. (The unused pages
//      +0x3000-0xFFFF are covered by eth_decode_tests.)
// TXEN/RXEN are never set, so the MAC DMA stays idle throughout. Every
// register touched is put back to its reset value at the end.
// Any external abort on these accesses is counted and fails the test.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "ethmac.h"
#include <stdio.h>
#include <stdint.h>

static const char *V = "MEGASOC-ETH-001";

static volatile uint32_t g_abort_count = 0;
void abort_handler(void) { g_abort_count++; }   // resume at the next instruction

// what/sub: check name and qualifier (sub may be "")
static int check2(const char *what, const char *sub, uint32_t exp, uint32_t got)
{
  if (exp != got) {
    printf("[%s] FAIL %s %s: expected 0x%08x got 0x%08x\n", V, what, sub, (unsigned)exp, (unsigned)got);
    return 1;
  }
#ifdef TEST_VERBOSITY_HIGH
  printf("[%s] ok   %s %s = 0x%08x\n", V, what, sub, (unsigned)got);
#endif
  return 0;
}
#define check(what, exp, got) check2((what), "", (exp), (got))

typedef struct { const char *name; unsigned long off; uint32_t rst; uint32_t mask; } reg_t;

// mask = implemented read/write bits (OpenCores ethmac spec rev 1.19);
// 0 = not covered by the read/write pass
static const reg_t regs[] = {
  { "MODER",      ETH_MODER,      ETH_MODER_RST,      0x0001FFFCUL },  // TXEN/RXEN excluded
  { "INT_SOURCE", ETH_INT_SOURCE, 0x00000000UL,       0 },
  { "INT_MASK",   ETH_INT_MASK,   ETH_INT_MASK_RST,   0x0000007FUL },
  { "IPGT",       ETH_IPGT,       ETH_IPGT_RST,       0x0000007FUL },
  { "IPGR1",      ETH_IPGR1,      ETH_IPGR1_RST,      0x0000007FUL },
  { "IPGR2",      ETH_IPGR2,      ETH_IPGR2_RST,      0x0000007FUL },
  { "PACKETLEN",  ETH_PACKETLEN,  ETH_PACKETLEN_RST,  0xFFFFFFFFUL },
  { "COLLCONF",   ETH_COLLCONF,   ETH_COLLCONF_RST,   0x000F003FUL },
  { "TX_BD_NUM",  ETH_TX_BD_NUM,  ETH_TX_BD_NUM_RST,  0 },             // own check below
  { "CTRLMODER",  ETH_CTRLMODER,  ETH_CTRLMODER_RST,  0x00000007UL },
  { "MIIMODER",   ETH_MIIMODER,   ETH_MIIMODER_RST,   0x000001FFUL },
  { "MIIADDRESS", ETH_MIIADDRESS, 0x00000000UL,       0x00001F1FUL },
  { "MIITX_DATA", ETH_MIITX_DATA, 0x00000000UL,       0x0000FFFFUL },
  { "MAC_ADDR0",  ETH_MAC_ADDR0,  0x00000000UL,       0xFFFFFFFFUL },
  { "MAC_ADDR1",  ETH_MAC_ADDR1,  0x00000000UL,       0x0000FFFFUL },
  { "HASH0",      ETH_HASH0,      0x00000000UL,       0xFFFFFFFFUL },
  { "HASH1",      ETH_HASH1,      0x00000000UL,       0xFFFFFFFFUL },
};
#define NREGS (sizeof(regs) / sizeof(regs[0]))

static int test_reset_values(void)
{
  int e = 0;
  printf("Test: reset values\n");
  for (unsigned i = 0; i < NREGS; i++)
    e += check2(regs[i].name, "reset", regs[i].rst, eth_rd(regs[i].off));
  return e;
}

static int test_read_write(void)
{
  static const uint32_t pat[] = { 0xFFFFFFFFUL, 0xA5A5A5A5UL, 0x5A5A5A5AUL, 0x00000000UL };
  int e = 0;
  printf("Test: read/write\n");
  for (unsigned i = 0; i < NREGS; i++) {
    if (!regs[i].mask) continue;
    for (unsigned p = 0; p < 4; p++) {
      uint32_t w = pat[p] & regs[i].mask;     // MODER mask keeps TXEN/RXEN at 0
      eth_wr(regs[i].off, w);
      e += check2(regs[i].name, "write/read", w, eth_rd(regs[i].off) & regs[i].mask);
    }
    eth_wr(regs[i].off, regs[i].rst);
    e += check2(regs[i].name, "restored", regs[i].rst, eth_rd(regs[i].off));
  }

  // TX_BD_NUM: 0..0x80 accepted, larger values ignored
  eth_wr(ETH_TX_BD_NUM, 0x20);
  e += check("TX_BD_NUM wr 0x20", 0x20, eth_rd(ETH_TX_BD_NUM));
  eth_wr(ETH_TX_BD_NUM, 0x80);
  e += check("TX_BD_NUM wr 0x80", 0x80, eth_rd(ETH_TX_BD_NUM));
  eth_wr(ETH_TX_BD_NUM, 0x81);
  e += check("TX_BD_NUM wr 0x81 ignored", 0x80, eth_rd(ETH_TX_BD_NUM));
  eth_wr(ETH_TX_BD_NUM, ETH_TX_BD_NUM_RST);
  e += check("TX_BD_NUM restored", ETH_TX_BD_NUM_RST, eth_rd(ETH_TX_BD_NUM));
  return e;
}

static int test_byte_strobes(void)
{
  int e = 0;
  volatile uint8_t  *b = (volatile uint8_t  *)(ETH_MAC_BASE + ETH_MAC_ADDR0);
  volatile uint16_t *h = (volatile uint16_t *)(ETH_MAC_BASE + ETH_HASH0);
  printf("Test: byte and halfword writes (PSTRB)\n");

  eth_wr(ETH_MAC_ADDR0, 0x11223344UL);
  b[0] = 0xEE; __asm__ volatile ("dsb sy" ::: "memory");
  e += check("MAC_ADDR0 byte 0", 0x112233EEUL, eth_rd(ETH_MAC_ADDR0));
  b[2] = 0xCC; __asm__ volatile ("dsb sy" ::: "memory");
  e += check("MAC_ADDR0 byte 2", 0x11CC33EEUL, eth_rd(ETH_MAC_ADDR0));
  b[3] = 0xDD; __asm__ volatile ("dsb sy" ::: "memory");
  e += check("MAC_ADDR0 byte 3", 0xDDCC33EEUL, eth_rd(ETH_MAC_ADDR0));

  eth_wr(ETH_HASH0, 0x00000000UL);
  h[1] = 0xBEEF; __asm__ volatile ("dsb sy" ::: "memory");
  e += check("HASH0 upper halfword", 0xBEEF0000UL, eth_rd(ETH_HASH0));

  eth_wr(ETH_MAC_ADDR0, 0);
  eth_wr(ETH_HASH0, 0);
  return e;
}

static int test_bd_ram(void)
{
  static const unsigned long offs[] = { 0x400UL, 0x404UL, 0x5F8UL, 0x7F8UL, 0x7FCUL };
  int e = 0;
  printf("Test: buffer descriptor RAM\n");
  for (unsigned i = 0; i < 5; i++) {
    uint32_t w = 0x5EED0000UL ^ (uint32_t)(offs[i] * 0x01010101UL);
    w &= ~(ETH_TXBD_RD | ETH_RXBD_E);   // never hand a BD to the MAC (TXEN/RXEN are off anyway)
    eth_wr(offs[i], w);
    if (eth_rd(offs[i]) != w) {
      printf("[%s] FAIL BD word @+0x%03lx: expected 0x%08x got 0x%08x\n", V, offs[i],
             (unsigned)w, (unsigned)eth_rd(offs[i]));
      e++;
    }
  }
  for (unsigned i = 0; i < 5; i++) eth_wr(offs[i], 0);
  return e;
}

static int test_window_decode(void)
{
  int e = 0;
  uint32_t before = g_abort_count;
  volatile uint32_t v;
  printf("Test: sub-block decode (MAC +0x0000, PTP +0x1000, checksum +0x2000)\n");

  e += check("checksum unit ID (+0x2000)", ETH_CKSUM_ID_VALUE, eth_cksum_rd(ETH_CKSUM_ID));
  e += check("checksum unit CTRL reset", 0, eth_cksum_rd(ETH_CKSUM_CTRL));
  v = *(volatile uint32_t *)(ETH_PTP_BASE + 0x00);           // PTP block responds
  (void)v;
  e += check("MAC MODER unchanged", ETH_MODER_RST, eth_rd(ETH_MODER));
  e += check("no external aborts in the window", before, g_abort_count);
  return e;
}

int main(void)
{
  int errors = 0;
  UartStdOutInit();
  printf("MegaSoC Ethernet register access test (%s)\n", V);

  errors += test_reset_values();
  errors += test_read_write();
  errors += test_byte_strobes();
  errors += test_bd_ram();
  errors += test_window_decode();

  if (g_abort_count) {
    printf("[%s] FAIL %u external abort(s) on ETH_S accesses\n", V, (unsigned)g_abort_count);
    errors++;
  }
  printf("Total number of eth_reg_access errors: %d\n", errors);
  if (errors == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
