//-----------------------------------------------------------------------------
// MegaSoC DMA-350 register map and helpers (memory-to-memory use)
// A joint work commissioned on behalf of SoC Labs, under Arm Academic Access license.
//
// Copyright (C) 2026, SoC Labs (www.soclabs.org)
//-----------------------------------------------------------------------------
// Only present in INC_DMA builds. Written from the public Arm CoreLink
// DMA-350 Technical Reference Manual (102482_0000_03, chapter 6
// "Programmers model"); only the registers and fields these tests use.
//
// Configuration in MegaSoC: 4 channels, 64-bit data, 44-bit address, no
// triggers, no streams, no GPO, no TrustZone support (every channel is
// Non-secure). Extended features
// (2D/wrap/template) on channels 0 and 1 only.
//
// Register window DMA_S, 64 KB at 0x0103_0000. The DMA decodes the first
// 8 KB; the rest of the window reads 0 and ignores writes (MegaSoC wrapper).
//   +0x0000  security configuration frame (no TrustZone: not used)
//   +0x0100  Secure control frame (no TrustZone: not used)
//   +0x0200  Non-secure control frame (NSEC_*)
//   +0x0F00  information frame (build configuration, IDs)
//   +0x1000 + 0x100*n  channel n frame (CH_*)
// Use 32-bit accesses only: the DMA answers a write with partial byte
// strobes with an APB error. Privileged accesses only (unprivileged ones
// read as zero); the bare-metal tests run privileged.
//
// Bus manager DMA_M reaches ROM, FLASH, RAM (SRAM), PERIPHERAL and DRAM.
// DMA reads of FLASH work (checked by dma350_irq_tests). Any other address
// ends with a bus error (DECERR).
//
// Interrupts (CPU_IRQS bit -> GIC interrupt ID = bit + 32), level:
//   channel n interrupt   CPU_IRQS[97+n] -> GIC ID 129+n  (n = 0..3)
//   combined Non-secure   CPU_IRQS[101]  -> GIC ID 133
//
// The DMA is not cache coherent with the A53: clean+invalidate source and
// destination buffers before a transfer, and invalidate the destination
// again before the CPU reads it (dma350_cache_civac_range).
//-----------------------------------------------------------------------------

#ifndef MEGASOC_DMA350_H
#define MEGASOC_DMA350_H

#include <stdint.h>

#define DMA350_BASE             0x01030000UL
#define DMA350_DECODE_SIZE      0x2000UL     // DMA registers; rest of the 64 KB window reads 0
#define DMA350_NUM_CH           4U

#define DMA350_GIC_ID_CH(n)     (129U + (unsigned)(n))
#define DMA350_GIC_ID_COMB      133U

// ---- Frames -----------------------------------------------------------------
#define DMA350_NSEC_BASE        (DMA350_BASE + 0x0200UL)
#define DMA350_INFO_BASE        (DMA350_BASE + 0x0F00UL)
#define DMA350_CH_BASE(n)       (DMA350_BASE + 0x1000UL + 0x100UL * (unsigned long)(n))

// ---- Channel registers (offset from DMA350_CH_BASE(n)) ----------------------
#define DMA_CH_CMD              0x00UL
#define DMA_CH_STATUS           0x04UL
#define DMA_CH_INTREN           0x08UL
#define DMA_CH_CTRL             0x0CUL
#define DMA_CH_SRCADDR          0x10UL
#define DMA_CH_SRCADDRHI        0x14UL
#define DMA_CH_DESADDR          0x18UL
#define DMA_CH_DESADDRHI        0x1CUL
#define DMA_CH_XSIZE            0x20UL
#define DMA_CH_XSIZEHI          0x24UL
#define DMA_CH_SRCTRANSCFG      0x28UL
#define DMA_CH_DESTRANSCFG      0x2CUL
#define DMA_CH_XADDRINC         0x30UL
#define DMA_CH_YADDRSTRIDE      0x34UL
#define DMA_CH_FILLVAL          0x38UL
#define DMA_CH_YSIZE            0x3CUL
#define DMA_CH_LINKATTR         0x70UL
#define DMA_CH_AUTOCFG          0x74UL
#define DMA_CH_LINKADDR         0x78UL
#define DMA_CH_LINKADDRHI       0x7CUL
#define DMA_CH_ERRINFO          0x90UL
#define DMA_CH_IIDR             0xC8UL
#define DMA_CH_AIDR             0xCCUL
#define DMA_CH_ISSUECAP         0xE8UL
#define DMA_CH_BUILDCFG0        0xF8UL
#define DMA_CH_BUILDCFG1        0xFCUL

// CH_CMD (write 1 to set)
#define DMA_CMD_ENABLECMD       (1UL << 0)
#define DMA_CMD_CLEARCMD        (1UL << 1)
#define DMA_CMD_DISABLECMD      (1UL << 2)
#define DMA_CMD_STOPCMD         (1UL << 3)

// CH_STATUS: STAT_* write 1 to clear; INTR_* = STAT_* & INTREN_*
#define DMA_INTR_DONE           (1UL << 0)
#define DMA_INTR_ERR            (1UL << 1)
#define DMA_INTR_DISABLED       (1UL << 2)
#define DMA_INTR_STOPPED        (1UL << 3)
#define DMA_STAT_DONE           (1UL << 16)
#define DMA_STAT_ERR            (1UL << 17)
#define DMA_STAT_DISABLED       (1UL << 18)
#define DMA_STAT_STOPPED        (1UL << 19)
#define DMA_STAT_W1C_ALL        (DMA_STAT_DONE | DMA_STAT_ERR | DMA_STAT_DISABLED | DMA_STAT_STOPPED)

// CH_INTREN
#define DMA_INTREN_DONE         (1UL << 0)
#define DMA_INTREN_ERR          (1UL << 1)
#define DMA_INTREN_DISABLED     (1UL << 2)
#define DMA_INTREN_STOPPED      (1UL << 3)

// CH_CTRL
#define DMA_CTRL_TRANSIZE(s)    ((uint32_t)(s) & 0x7U)          // bytes = 1 << s
#define DMA_CTRL_CHPRIO(p)      (((uint32_t)(p) & 0xFU) << 4)
#define DMA_CTRL_XTYPE_CONTINUE (1UL << 9)
#define DMA_CTRL_DONETYPE_CMD   (1UL << 21)                     // STAT_DONE at end of command
#define DMA_TRANSIZE_8          0U
#define DMA_TRANSIZE_16         1U
#define DMA_TRANSIZE_32         2U
#define DMA_TRANSIZE_64         3U

// CH_SRCTRANSCFG / CH_DESTRANSCFG
#define DMA_TRANSCFG_MAXBURSTLEN(n) (((uint32_t)(n) & 0xFU) << 16)  // beats - 1
#define DMA_TRANSCFG_NONSEC     (1UL << 10)                     // tied 1 (no TrustZone)
#define DMA_MEMATTR_DEVICE      0x00U                           // Device-nGnRnE (reset value)
#define DMA_MEMATTR_NORMAL_NC   0x44U                           // Normal, inner+outer non-cacheable
// Normal non-cacheable, non-shareable, bursts up to 16 beats
#define DMA_TRANSCFG_MEM        (DMA_TRANSCFG_MAXBURSTLEN(15) | DMA_TRANSCFG_NONSEC | DMA_MEMATTR_NORMAL_NC)

// CH_ERRINFO (valid while STAT_ERR is set). A bus error stops the command
// at once; the channel goes to DISABLED with STAT_ERR set.
#define DMA_ERRINFO_BUSERR      (1UL << 0)
#define DMA_ERRINFO_CFGERR      (1UL << 1)
#define DMA_ERRINFO_AXIRDRESPERR (1UL << 16)   // read error response
#define DMA_ERRINFO_AXIWRRESPERR (1UL << 17)   // write error response

// Channel register reset values (TRM table 6-3)
#define DMA_CH_CTRL_RST         0x00200200UL
#define DMA_CH_TRANSCFG_RST     0x000F0400UL

// ---- Non-secure control registers (offset from DMA350_NSEC_BASE) -----------
#define DMA_NSEC_CHINTRSTATUS0  0x00UL       // bit n: channel n interrupt
#define DMA_NSEC_STATUS         0x08UL
#define DMA_NSEC_CTRL           0x0CUL
#define DMA_NSEC_CTRL_INTREN_ANYCHINTR (1UL << 0)
#define DMA_NSEC_STATUS_INTR_ANYCHINTR (1UL << 0)

// ---- Information registers (offset from DMA350_INFO_BASE) -------------------
#define DMA_INFO_BUILDCFG0      0xB0UL
#define DMA_INFO_BUILDCFG1      0xB4UL
#define DMA_INFO_BUILDCFG2      0xB8UL
#define DMA_INFO_IIDR           0xC8UL
#define DMA_INFO_AIDR           0xCCUL
#define DMA_INFO_PIDR4          0xD0UL
#define DMA_INFO_PIDR0          0xE0UL
#define DMA_INFO_PIDR1          0xE4UL
#define DMA_INFO_PIDR2          0xE8UL
#define DMA_INFO_PIDR3          0xECUL
#define DMA_INFO_CIDR0          0xF0UL
#define DMA_INFO_CIDR1          0xF4UL
#define DMA_INFO_CIDR2          0xF8UL
#define DMA_INFO_CIDR3          0xFCUL

// ---- Access helpers ---------------------------------------------------------
static inline void dma_wr(unsigned long a, uint32_t v)
{
  *(volatile uint32_t *)a = v;
  __asm__ volatile ("dsb sy" ::: "memory");
}
static inline uint32_t dma_rd(unsigned long a) { return *(volatile uint32_t *)a; }

static inline void dma_ch_wr(unsigned ch, unsigned long off, uint32_t v) { dma_wr(DMA350_CH_BASE(ch) + off, v); }
static inline uint32_t dma_ch_rd(unsigned ch, unsigned long off) { return dma_rd(DMA350_CH_BASE(ch) + off); }
static inline void dma_nsec_wr(unsigned long off, uint32_t v) { dma_wr(DMA350_NSEC_BASE + off, v); }
static inline uint32_t dma_nsec_rd(unsigned long off) { return dma_rd(DMA350_NSEC_BASE + off); }
static inline uint32_t dma_info_rd(unsigned long off) { return dma_rd(DMA350_INFO_BASE + off); }

// Clean+invalidate data cache lines covering [addr, addr+len) to the point
// of coherency (64-byte lines on the A53).
static inline void dma350_cache_civac_range(unsigned long addr, unsigned long len)
{
  for (unsigned long a = addr & ~63UL; a < addr + len; a += 64UL)
    __asm__ volatile ("dc civac, %0" : : "r"(a) : "memory");
  __asm__ volatile ("dsb sy" ::: "memory");
  __asm__ volatile ("isb" ::: "memory");
}

// Channel idle = ENABLECMD reads 0 (no command running).
static inline int dma350_ch_busy(unsigned ch) { return (int)(dma_ch_rd(ch, DMA_CH_CMD) & DMA_CMD_ENABLECMD); }

// Clear every channel register to its reset value (CLEARCMD stays set until
// done). Returns 0 when cleared, -1 on timeout.
static inline int dma350_ch_clear(unsigned ch, uint32_t poll)
{
  dma_ch_wr(ch, DMA_CH_CMD, DMA_CMD_CLEARCMD);
  for (uint32_t i = 0; i < poll; i++)
    if ((dma_ch_rd(ch, DMA_CH_CMD) & DMA_CMD_CLEARCMD) == 0) return 0;
  return -1;
}

// Program a 1D memory-to-memory copy of `count` elements of (1 << transize)
// bytes from `src` to `dst` (both 44-bit addresses), both sides normal
// non-cacheable memory, address increment 1, no command linking. `intren`
// goes to CH_INTREN. Does not start the channel (see dma350_ch_start).
static inline void dma350_ch_setup_m2m(unsigned ch, uint64_t src, uint64_t dst, uint32_t count,
                                       unsigned transize, unsigned prio, uint32_t intren)
{
  dma_ch_wr(ch, DMA_CH_STATUS,      DMA_STAT_W1C_ALL);
  dma_ch_wr(ch, DMA_CH_SRCADDR,     (uint32_t)src);
  dma_ch_wr(ch, DMA_CH_SRCADDRHI,   (uint32_t)(src >> 32));
  dma_ch_wr(ch, DMA_CH_DESADDR,     (uint32_t)dst);
  dma_ch_wr(ch, DMA_CH_DESADDRHI,   (uint32_t)(dst >> 32));
  dma_ch_wr(ch, DMA_CH_XSIZE,       ((count & 0xFFFFU) << 16) | (count & 0xFFFFU));
  dma_ch_wr(ch, DMA_CH_XSIZEHI,     ((count >> 16) << 16) | (count >> 16));
  dma_ch_wr(ch, DMA_CH_XADDRINC,    (1UL << 16) | 1UL);
  dma_ch_wr(ch, DMA_CH_SRCTRANSCFG, DMA_TRANSCFG_MEM);
  dma_ch_wr(ch, DMA_CH_DESTRANSCFG, DMA_TRANSCFG_MEM);
  dma_ch_wr(ch, DMA_CH_LINKADDR,    0);
  dma_ch_wr(ch, DMA_CH_LINKADDRHI,  0);
  dma_ch_wr(ch, DMA_CH_CTRL,        DMA_CTRL_TRANSIZE(transize) | DMA_CTRL_CHPRIO(prio) |
                                    DMA_CTRL_XTYPE_CONTINUE | DMA_CTRL_DONETYPE_CMD);
  dma_ch_wr(ch, DMA_CH_INTREN,      intren);
}

static inline void dma350_ch_start(unsigned ch) { dma_ch_wr(ch, DMA_CH_CMD, DMA_CMD_ENABLECMD); }

// Wait until STAT_DONE or STAT_ERR is set and the channel is idle. Returns
// the last CH_STATUS value (bit 31 set on timeout).
static inline uint32_t dma350_ch_wait(unsigned ch, uint32_t poll)
{
  uint32_t s = 0;
  for (uint32_t i = 0; i < poll; i++) {
    s = dma_ch_rd(ch, DMA_CH_STATUS);
    if ((s & (DMA_STAT_DONE | DMA_STAT_ERR)) && !dma350_ch_busy(ch)) return s;
  }
  return s | 0x80000000UL;
}

#endif // MEGASOC_DMA350_H
