//==============================================================================
// eth_mdio_tests.c -- MegaSoC Ethernet MII management (MDIO) test
//
// INC_ETH builds only. vplan MEGASOC-ETH-006.
//
// The MAC's MII management block talks to the bench PHY model
// (verif/testbench/logical/megasoc_eth_phy_model.sv, PHY address 1) over the
// chip's ETH_MDC / ETH_MDIO pins. Checks:
//   1. MIIMODER clock divider write (MDC = SYS_CLK / CLKDIV, <= 2.5 MHz).
//   2. Reads of the PHY identifier and status registers (2, 3, 1) and the
//      reset values of BMCR (0) and ANAR (4).
//   3. Write + read-back of ANAR; BMCR reset bit self-clears; PHYID1 is
//      read-only.
//   4. A read from an address with no PHY returns 0xFFFF (pull-up).
//   5. The same reads with the preamble suppressed (MIIMODER.MIINOPRE).
//   6. Scan mode (MIICOMMAND.SCANSTAT) on BMSR: MIISTATUS.NVALID clears and
//      LINKFAIL = 0 (bench PHY reports link up).
// PHY registers and MAC registers are put back at the end.
//==============================================================================

#include "uart_stdout.h"
#include "system.h"
#include "ethmac.h"
#include <stdio.h>
#include <stdint.h>

static const char *V = "MEGASOC-ETH-006";

#define PHY      ETH_BENCH_PHY_ADDR
#define NO_PHY   2U
#define MIIMODER_CLKDIV   0x28UL          // 100 MHz / 40 = 2.5 MHz
#define MIIMODER_NOPRE    (1UL << 8)

static int check(const char *what, uint32_t exp, uint32_t got)
{
  if (exp != got) {
    printf("[%s] FAIL %s: expected 0x%04x got 0x%04x\n", V, what, (unsigned)exp, (unsigned)got);
    return 1;
  }
  printf("[%s] ok   %s = 0x%04x\n", V, what, (unsigned)got);
  return 0;
}

static int mdio_rd_check(const char *what, unsigned phy, unsigned reg, uint16_t exp)
{
  uint16_t v = 0;
  if (eth_mdio_read(phy, reg, &v)) {
    printf("[%s] FAIL %s: MDIO read timed out (MIISTATUS 0x%x)\n", V, what,
           (unsigned)eth_rd(ETH_MIISTATUS));
    return 1;
  }
  return check(what, exp, v);
}

static int mdio_wr(const char *what, unsigned phy, unsigned reg, uint16_t val)
{
  if (eth_mdio_write(phy, reg, val)) {
    printf("[%s] FAIL %s: MDIO write timed out (MIISTATUS 0x%x)\n", V, what,
           (unsigned)eth_rd(ETH_MIISTATUS));
    return 1;
  }
  return 0;
}

static int read_set(const char *tag)
{
  int e = 0;
  printf("Test: PHY register reads (%s)\n", tag);
  e += mdio_rd_check("PHYID1 (reg 2)", PHY, 2, ETH_BENCH_PHY_ID1);
  e += mdio_rd_check("PHYID2 (reg 3)", PHY, 3, ETH_BENCH_PHY_ID2);
  e += mdio_rd_check("BMSR   (reg 1)", PHY, 1, 0x786D);
  e += mdio_rd_check("no PHY at addr 2", NO_PHY, 2, 0xFFFF);
  return e;
}

int main(void)
{
  int e = 0;
  UartStdOutInit();
  printf("MegaSoC Ethernet MDIO test (%s)\n", V);

  // 1. clock divider
  eth_wr(ETH_MIIMODER, MIIMODER_CLKDIV);
  e += check("MIIMODER", MIIMODER_CLKDIV, eth_rd(ETH_MIIMODER));
  e += check("MIISTATUS idle", 0, eth_rd(ETH_MIISTATUS) & ETH_MIISTAT_BUSY);

  // 2. reads
  e += read_set("with preamble");
  e += mdio_rd_check("BMCR   (reg 0) reset", PHY, 0, 0x3100);
  e += mdio_rd_check("ANAR   (reg 4) reset", PHY, 4, 0x01E1);

  // 3. writes
  printf("Test: PHY register writes\n");
  e += mdio_wr("ANAR write", PHY, 4, 0x0DE1);
  e += mdio_rd_check("ANAR read-back", PHY, 4, 0x0DE1);
  e += mdio_wr("ANAR write 2", PHY, 4, 0x0021);
  e += mdio_rd_check("ANAR read-back 2", PHY, 4, 0x0021);
  e += mdio_wr("BMCR write reset|ANEN", PHY, 0, 0x9000);
  e += mdio_rd_check("BMCR reset bit self-clears", PHY, 0, 0x1000);
  e += mdio_wr("PHYID1 write (read-only)", PHY, 2, 0x1234);
  e += mdio_rd_check("PHYID1 unchanged", PHY, 2, ETH_BENCH_PHY_ID1);

  // 5. no preamble
  eth_wr(ETH_MIIMODER, MIIMODER_CLKDIV | MIIMODER_NOPRE);
  e += read_set("preamble suppressed");
  eth_wr(ETH_MIIMODER, MIIMODER_CLKDIV);

  // 6. scan BMSR
  printf("Test: scan mode on BMSR\n");
  eth_wr(ETH_MIIADDRESS, ETH_MIIADDR(PHY, 1));
  eth_wr(ETH_MIICOMMAND, ETH_MIICMD_SCANSTAT);
  {
    uint32_t s = ETH_MIISTAT_NVALID, i;
    for (i = 0; i < 200000U; i++) {
      s = eth_rd(ETH_MIISTATUS);
      if (!(s & ETH_MIISTAT_NVALID)) break;
    }
    e += check("MIISTATUS.NVALID cleared by scan", 0, s & ETH_MIISTAT_NVALID);
    e += check("MIISTATUS.LINKFAIL (link up)", 0, s & ETH_MIISTAT_LINKFAIL);
    e += check("MIIRX_DATA from scan = BMSR", 0x786D, eth_rd(ETH_MIIRX_DATA) & 0xFFFF);
  }
  eth_wr(ETH_MIICOMMAND, 0);
  if (eth_mdio_wait(0)) { printf("[%s] FAIL scan did not stop\n", V); e++; }

  // restore
  e += mdio_wr("ANAR restore", PHY, 4, 0x01E1);
  e += mdio_wr("BMCR restore", PHY, 0, 0x3100);
  eth_wr(ETH_MIIADDRESS, 0);
  eth_wr(ETH_MIITX_DATA, 0);
  eth_wr(ETH_MIIMODER, ETH_MIIMODER_RST);

  printf("Total number of eth_mdio errors: %d\n", e);
  if (e == 0) TEST_PASS(); else TEST_FAIL();
  return 0;
}
