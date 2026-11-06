//-----------------------------------------------------------------------------
// MegaSoC Ethernet subsystem (ethmac_subsystem_apb) register map and helpers
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Only present in INC_ETH builds. Register window ETH_S, 64 KB at
// 0x0102_0000, decoded inside the block on address bits [15:12]:
//   +0x0000  MAC (OpenCores ethmac rel27): registers 0x000-0x0FF, buffer
//            descriptors 0x400-0x7FF (128 x 8 bytes; TX BDs first, RX BDs
//            from index TX_BD_NUM)
//   +0x1000  HA1588 PTP timestamping unit
//   +0x2000  RX checksum unit
//   +0x3000-0xFFFF  unused (accesses complete, read 0)
// MAC DMA = AHB master ETH_M; it reaches the SRAM and DRAM windows only.
//
// Interrupts (CPU_IRQS bit -> GIC interrupt ID = bit + 32):
//   MAC int_o             CPU_IRQS[95] -> GIC ID 127
//   checksum cksum_int_o  CPU_IRQS[96] -> GIC ID 128
//
// Register and descriptor layouts follow the OpenCores ethmac
// specification (rev 1.19).
//-----------------------------------------------------------------------------

#ifndef MEGASOC_ETHMAC_H
#define MEGASOC_ETHMAC_H

#include <stdint.h>

#define ETH_BASE            0x01020000UL
#define ETH_MAC_BASE        (ETH_BASE + 0x0000UL)
#define ETH_PTP_BASE        (ETH_BASE + 0x1000UL)
#define ETH_CKSUM_BASE      (ETH_BASE + 0x2000UL)

#define ETH_GIC_ID          127U
#define ETH_CKSUM_GIC_ID    128U

// ---- MAC registers (offset from ETH_MAC_BASE) ------------------------------
#define ETH_MODER           0x00UL
#define ETH_INT_SOURCE      0x04UL
#define ETH_INT_MASK        0x08UL
#define ETH_IPGT            0x0CUL
#define ETH_IPGR1           0x10UL
#define ETH_IPGR2           0x14UL
#define ETH_PACKETLEN       0x18UL
#define ETH_COLLCONF        0x1CUL
#define ETH_TX_BD_NUM       0x20UL
#define ETH_CTRLMODER       0x24UL
#define ETH_MIIMODER        0x28UL
#define ETH_MIICOMMAND      0x2CUL
#define ETH_MIIADDRESS      0x30UL
#define ETH_MIITX_DATA      0x34UL
#define ETH_MIIRX_DATA      0x38UL
#define ETH_MIISTATUS       0x3CUL
#define ETH_MAC_ADDR0       0x40UL   // [31:24] byte 2 ... [7:0] byte 5
#define ETH_MAC_ADDR1       0x44UL   // [15:8] byte 0, [7:0] byte 1
#define ETH_HASH0           0x48UL
#define ETH_HASH1           0x4CUL
#define ETH_TXCTRL          0x50UL
#define ETH_BD_BASE         0x400UL

// Reset values
#define ETH_MODER_RST       0x0000A000UL
#define ETH_INT_MASK_RST    0x00000000UL
#define ETH_IPGT_RST        0x00000012UL
#define ETH_IPGR1_RST       0x0000000CUL
#define ETH_IPGR2_RST       0x00000012UL
#define ETH_PACKETLEN_RST   0x00400600UL
#define ETH_COLLCONF_RST    0x000F003FUL
#define ETH_TX_BD_NUM_RST   0x00000040UL
#define ETH_CTRLMODER_RST   0x00000000UL
#define ETH_MIIMODER_RST    0x00000064UL

// MODER
#define ETH_MODER_RXEN      (1UL << 0)
#define ETH_MODER_TXEN      (1UL << 1)
#define ETH_MODER_NOPRE     (1UL << 2)
#define ETH_MODER_BRO       (1UL << 3)
#define ETH_MODER_IAM       (1UL << 4)
#define ETH_MODER_PRO       (1UL << 5)
#define ETH_MODER_IFG       (1UL << 6)
#define ETH_MODER_LOOPBCK   (1UL << 7)
#define ETH_MODER_NOBCKOF   (1UL << 8)
#define ETH_MODER_EXDFREN   (1UL << 9)
#define ETH_MODER_FULLD     (1UL << 10)
#define ETH_MODER_DLYCRCEN  (1UL << 12)
#define ETH_MODER_CRCEN     (1UL << 13)
#define ETH_MODER_HUGEN     (1UL << 14)
#define ETH_MODER_PAD       (1UL << 15)
#define ETH_MODER_RECSMALL  (1UL << 16)

// INT_SOURCE / INT_MASK (INT_SOURCE is write-1-to-clear)
#define ETH_INT_TXB         (1UL << 0)
#define ETH_INT_TXE         (1UL << 1)
#define ETH_INT_RXB         (1UL << 2)
#define ETH_INT_RXE         (1UL << 3)
#define ETH_INT_BUSY        (1UL << 4)
#define ETH_INT_TXC         (1UL << 5)
#define ETH_INT_RXC         (1UL << 6)
#define ETH_INT_ALL         0x7FUL

// MIICOMMAND / MIISTATUS / MIIADDRESS
#define ETH_MIICMD_SCANSTAT (1UL << 0)
#define ETH_MIICMD_RSTAT    (1UL << 1)
#define ETH_MIICMD_WCTRLDATA (1UL << 2)
#define ETH_MIISTAT_LINKFAIL (1UL << 0)
#define ETH_MIISTAT_BUSY    (1UL << 1)
#define ETH_MIISTAT_NVALID  (1UL << 2)
#define ETH_MIIADDR(fiad, rgad) ((((uint32_t)(rgad) & 0x1FU) << 8) | ((uint32_t)(fiad) & 0x1FU))

// Buffer descriptor word 0: [31:16] length, [15:0] control/status
#define ETH_TXBD_RD         (1UL << 15)
#define ETH_TXBD_IRQ        (1UL << 14)
#define ETH_TXBD_WR         (1UL << 13)
#define ETH_TXBD_PAD        (1UL << 12)
#define ETH_TXBD_CRC        (1UL << 11)
#define ETH_TXBD_UR         (1UL << 8)
#define ETH_TXBD_RL         (1UL << 3)
#define ETH_TXBD_LC         (1UL << 2)
#define ETH_TXBD_DF         (1UL << 1)
#define ETH_TXBD_CS         (1UL << 0)
#define ETH_TXBD_ERRS       (ETH_TXBD_UR | ETH_TXBD_RL | ETH_TXBD_LC | ETH_TXBD_CS)

#define ETH_RXBD_E          (1UL << 15)
#define ETH_RXBD_IRQ        (1UL << 14)
#define ETH_RXBD_WR         (1UL << 13)
#define ETH_RXBD_CF         (1UL << 8)
#define ETH_RXBD_M          (1UL << 7)
#define ETH_RXBD_OR         (1UL << 6)
#define ETH_RXBD_IS         (1UL << 5)
#define ETH_RXBD_DN         (1UL << 4)
#define ETH_RXBD_TL         (1UL << 3)
#define ETH_RXBD_SF         (1UL << 2)
#define ETH_RXBD_CRC        (1UL << 1)
#define ETH_RXBD_LC         (1UL << 0)
#define ETH_RXBD_ERRS       (ETH_RXBD_OR | ETH_RXBD_IS | ETH_RXBD_DN | ETH_RXBD_TL | \
                             ETH_RXBD_SF | ETH_RXBD_CRC | ETH_RXBD_LC)

// ---- RX checksum unit (offset from ETH_CKSUM_BASE) -------------------------
#define ETH_CKSUM_ID            0x00UL   // RO 0x434B534D ("CKSM")
#define ETH_CKSUM_CTRL          0x04UL
#define ETH_CKSUM_STATUS        0x08UL
#define ETH_CKSUM_FRAME_COUNT   0x0CUL
#define ETH_CKSUM_LAST_FLAGS    0x20UL
#define ETH_CKSUM_LAST_SEQNUM   0x24UL   // reading pops the snapshot FIFO
#define ETH_CKSUM_IRQ_STATUS    0x28UL
#define ETH_CKSUM_ID_VALUE      0x434B534DUL
#define ETH_CKSUM_CTRL_EN           (1UL << 0)
// CTRL[2:1] mode. LAST_ONLY (reset value) always shows the newest frame:
// no software pop, and once a frame has arrived the frame interrupt stays set
// until IRQ_EN_FRAME is cleared. The FIFO modes queue frames and reading
// LAST_SEQNUM pops one; the frame interrupt clears when the FIFO is empty.
#define ETH_CKSUM_CTRL_MODE_LAST_ONLY (0UL << 1)
#define ETH_CKSUM_CTRL_MODE_DROP_NEW  (1UL << 1)
#define ETH_CKSUM_CTRL_MODE_DROP_OLD  (2UL << 1)
#define ETH_CKSUM_CTRL_IRQ_FRAME    (1UL << 8)
#define ETH_CKSUM_CTRL_IRQ_BAD      (1UL << 9)
#define ETH_CKSUM_CTRL_IRQ_OVF      (1UL << 10)
#define ETH_CKSUM_CTRL_CLR_COUNTERS (1UL << 16)
#define ETH_CKSUM_CTRL_CLR_FIFO     (1UL << 17)
#define ETH_CKSUM_CTRL_CLR_OVF      (1UL << 18)

// ---- Bench PHY (megasoc_eth_phy_model) -------------------------------------
#define ETH_BENCH_PHY_ADDR  1U
#define ETH_BENCH_PHY_ID1   0x5A5AU
#define ETH_BENCH_PHY_ID2   0xC3C0U

// ---- Access helpers ---------------------------------------------------------
static inline void eth_wr(unsigned long off, uint32_t v)
{ *(volatile uint32_t *)(ETH_MAC_BASE + off) = v; __asm__ volatile ("dsb sy" ::: "memory"); }
static inline uint32_t eth_rd(unsigned long off)
{ return *(volatile uint32_t *)(ETH_MAC_BASE + off); }

static inline void eth_cksum_wr(unsigned long off, uint32_t v)
{ *(volatile uint32_t *)(ETH_CKSUM_BASE + off) = v; __asm__ volatile ("dsb sy" ::: "memory"); }
static inline uint32_t eth_cksum_rd(unsigned long off)
{ return *(volatile uint32_t *)(ETH_CKSUM_BASE + off); }

// Buffer descriptor n (0..127): word 0 = len/status, word 1 = buffer pointer
static inline void eth_bd_wr(unsigned n, uint32_t ctrl, uint32_t ptr)
{
  eth_wr(ETH_BD_BASE + 8UL * n + 4UL, ptr);
  eth_wr(ETH_BD_BASE + 8UL * n, ctrl);       // status word last (RD/E hands it over)
}
static inline uint32_t eth_bd_ctrl(unsigned n)
{ return eth_rd(ETH_BD_BASE + 8UL * n); }

// Program the station address, a[0] = first octet on the wire
static inline void eth_set_mac_addr(const uint8_t a[6])
{
  eth_wr(ETH_MAC_ADDR1, ((uint32_t)a[0] << 8) | a[1]);
  eth_wr(ETH_MAC_ADDR0, ((uint32_t)a[2] << 24) | ((uint32_t)a[3] << 16) |
                        ((uint32_t)a[4] << 8)  |  (uint32_t)a[5]);
}

// D-cache maintenance for buffers the MAC DMA reads or writes (ETH_M is not
// cache coherent): clean+invalidate every line of [addr, addr+len).
static inline void eth_cache_civac_range(unsigned long addr, unsigned long len)
{
  for (unsigned long a = addr & ~63UL; a < addr + len; a += 64UL)
    __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}

// Stop TX/RX and clear every pending MAC interrupt source.
static inline void eth_mac_stop(void)
{
  eth_wr(ETH_MODER, eth_rd(ETH_MODER) & ~(ETH_MODER_TXEN | ETH_MODER_RXEN));
  eth_wr(ETH_INT_SOURCE, ETH_INT_ALL);
}

// One-frame transfer setup: TX BD 0 sends `len` bytes from `tx`, RX BD 1
// receives into `rx` (TX_BD_NUM = 1). `moder` supplies the mode bits
// (FULLD, PRO, ...); TXEN and RXEN are added here. The caller handles
// cache maintenance for both buffers before calling.
static inline void eth_start_one(uint32_t moder, uint32_t tx, uint32_t len, uint32_t rx)
{
  eth_mac_stop();
  eth_wr(ETH_TX_BD_NUM, 1);
  eth_bd_wr(1, ETH_RXBD_E | ETH_RXBD_IRQ | ETH_RXBD_WR, rx);
  eth_bd_wr(0, (len << 16) | ETH_TXBD_RD | ETH_TXBD_IRQ | ETH_TXBD_WR |
               ETH_TXBD_PAD | ETH_TXBD_CRC, tx);
  eth_wr(ETH_MODER, moder | ETH_MODER_TXEN | ETH_MODER_RXEN);
}

// Poll INT_SOURCE until any bit of `any` is set or `iters` reads have
// passed. Returns the last INT_SOURCE value.
static inline uint32_t eth_wait_int(uint32_t any, uint32_t iters)
{
  uint32_t s = 0;
  for (uint32_t i = 0; i < iters; i++) {
    s = eth_rd(ETH_INT_SOURCE);
    if (s & any) break;
  }
  return s;
}

// MII management (MDIO) through the MAC. Return 0 on success, -1 on timeout.
// The command bit (RSTAT/WCTRLDATA) clears itself when the MAC starts the
// operation; BUSY then stays set until the frame has finished, so wait for
// the command bit first, then for BUSY.
static inline int eth_mdio_wait(uint32_t cmd)
{
  uint32_t i;
  for (i = 0; i < 200000U && (eth_rd(ETH_MIICOMMAND) & cmd); i++) { }
  for (     ; i < 200000U; i++)
    if (!(eth_rd(ETH_MIISTATUS) & ETH_MIISTAT_BUSY)) return 0;
  return -1;
}
static inline int eth_mdio_read(unsigned phy, unsigned reg, uint16_t *val)
{
  if (eth_mdio_wait(0)) return -1;
  eth_wr(ETH_MIIADDRESS, ETH_MIIADDR(phy, reg));
  eth_wr(ETH_MIICOMMAND, ETH_MIICMD_RSTAT);
  if (eth_mdio_wait(ETH_MIICMD_RSTAT)) return -1;
  *val = (uint16_t)eth_rd(ETH_MIIRX_DATA);
  return 0;
}
static inline int eth_mdio_write(unsigned phy, unsigned reg, uint16_t val)
{
  if (eth_mdio_wait(0)) return -1;
  eth_wr(ETH_MIIADDRESS, ETH_MIIADDR(phy, reg));
  eth_wr(ETH_MIITX_DATA, val);
  eth_wr(ETH_MIICOMMAND, ETH_MIICMD_WCTRLDATA);
  return eth_mdio_wait(ETH_MIICMD_WCTRLDATA);
}

#endif // MEGASOC_ETHMAC_H
