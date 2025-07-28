//------------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from Arm Limited or its affiliates.
//
//        (C) COPYRIGHT 2018-2021 Arm Limited or its affiliates.
//            ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from Arm Limited or its affiliates.
//
//      Release Information : SSE710-r0p0-00rel0
//
// -----------------------------------------------------------------------------

#include  <stdint.h>
#include "global_defines.h"

#ifndef SCP_PPU_1_1_DEF_H
#define SCP_PPU_1_1_DEF_H

// Policies for PPU F1
#define DBG_RECOV_F1  10
#define WARM_RST_F1    9
#define ON_F1          8
#define FUNC_RET_F1    7
#define MEM_OFF_F1     6
#define FULL_RET_F1    5
#define LOGIC_RET_F1   4
#define MEM_RET_EMU_F1 3
#define MEM_RET_F1     2
#define OFF_EMU_F1     1
#define OFF_F1         0

/* POWER_POLICY_REGISTER */

typedef union
{
  struct
  {
     uint32_t PWR_POLICY:4;             /* Power mode policy. */
     uint32_t RESERVED0:4;              /* RESERVED */
     uint32_t PWR_DYN_EN:1;             /* Power mode dynamic transition enable. */
     uint32_t RESERVED1:3;              /* RESERVED */
     uint32_t OFF_LOCK_EN:1;            /* Lock enable bit */
     uint32_t RESERVED2:3;              /* RESERVED */
     uint32_t OP_POLICY:4;              /* Operating mode policy. */
     uint32_t RESERVED3:4;              /* RESERVED */
     uint32_t OP_DYN_EN:1;              /* Operating mode dynamic transition enable. */
     uint32_t RESERVED4:7;              /* RESERVED */
  } B;
  uint32_t W;
} PPU_PWRP_Type;

typedef union
{
  struct
  {
     uint32_t PWR_STATUS:4;             /* RO Power mode status. */
     uint32_t RESERVED0:4;              /* RO RESERVED. */
     uint32_t PWR_DYN_STATUS:1;         /* RO Power mode dynamic transition status. */
     uint32_t RESERVED1:3;              /* RO RESERVED. */
     uint32_t OFF_LOCK_STATUS:1;        /* RO OFF lock status.  */
     uint32_t RESERVED2:3;              /* RO RESERVED. */
     uint32_t OP_STATUS:4;              /* RO Operating mode status. */
     uint32_t RESERVED3:4;              /* RO RESERVED. */
    uint32_t OP_DYN_STATUS:1;          /* RO Operating mode dynamic transition status. */
    uint32_t RESERVED4:7;              /* RO RESERVED. */
  } B;
  uint32_t W;
} PPU_PWSR_Type;

typedef union
{
  struct
  {
    uint32_t STA_POLICY_TRN_IRQ_MASK:1;/* RW Static full policy transition completion event mask. */
    uint32_t STA_ACCEPT_IRQ_MASK:1;    /* RW Static transition acceptance event mask. */
    uint32_t STA_DENY_IRQ_MASK:1;      /* RW Static transition denial event mask. */
    uint32_t EMU_ACCEPT_IRQ_MASK:1;    /* RW Emulation transition acceptance event mask. */
    uint32_t EMU_DENY_IRQ_MASK:1;      /* RW Emulation transition denial event mask. */
    uint32_t DYN_POLICY_MIN_IRQ_MASK:1;/* RW Dynamic minimum policy event mask. */
    uint32_t RESERVED:26;              /* RO RESERVED. */
  } B;
  uint32_t W;
} PPU_IMR_Type;

typedef union
{
  struct
  {
    uint32_t UNSPT_POLICY_IRQ_MASK:1;  /* RW Unsupported Policy event mask. */
    uint32_t DYN_ACCEPT_IRQ_MASK:1;    /* RW Dynamic transition acceptance event mask. */
    uint32_t DYN_DENY_IRQ_MASK:1;      /* RW Dynamic transition denial event mask. */
    uint32_t STA_POLICY_PWR_IRQ_MASK:1;/* RW Static power policy transition completion event status. */
    uint32_t STA_POLICY_OP_IRQ_MASK:1; /* RW Static operating policy transition completion event status. */
    uint32_t RESERVED:27;              /* RO RESERVED. */
  } B;
  uint32_t W;
} PPU_AIMR_Type;


typedef union
{
  struct
  {
    uint32_t STA_POLICY_TRN_IRQ:1;     /* RW Static full policy transition completion event status. */
    uint32_t STA_ACCEPT_IRQ:1;         /* RW Static transition acceptance event status. */
    uint32_t STA_DENY_IRQ:1;           /* RW Static transition denial event status. */
    uint32_t EMU_ACCEPT_IRQ:1;         /* RW Emulated transition acceptance event status. */
    uint32_t EMU_DENY_IRQ:1;           /* RW Emulated transition denial event status. */
    uint32_t DYN_POLICY_MIN_IRQ:1;     /* RW Dynamic minimum policy event status. */
    uint32_t RESERVED0:1;              /* RW RESERVED. */
    uint32_t OTHER_IRQ:1;              /* RO Indicates there is an interrupt event pending in the Additional Interrupt Status Register (PPU_AISR)*/
    uint32_t PWR_ACTIVE_EDGE_IRQ:11;   /* RW Indicates which power mode DEVACTIVE inputs caused the input edge event. */
    uint32_t RESERVED1:5;              /* RO RESERVED */
    uint32_t OP_ACTIVE_EDGE_IRQ:8;     /* RW Indicates which operating mode DEVPACTIVE inputs caused the input edge event. */
  } B;
  uint32_t W;
} PPU_ISR_Type;

typedef union
{
  struct
  {
    uint32_t UNSPT_POLICY_IRQ:1;       /* RW Unsupported Policy event status. */
    uint32_t DYN_ACCEPT_IRQ:1;         /* RW Dynamic transition completion event status. */
    uint32_t DYN_DENY_IRQ:1;           /* RW Dynamic transition denial event status. */
    uint32_t STA_POLICY_PWR_IRQ:1;     /* RW Static power policy transition completion event status. */
    uint32_t STA_POLICY_OP_IRQ:1;      /* RW Static operating policy transition completion event status. */
    uint32_t RESERVED:27;              /* RO Reserved. */
  } B;
  uint32_t W;
} PPU_AISR_Type;


typedef union
{
  struct
  {
    uint32_t WARM_RST_DEVREQEN:1;
    uint32_t DBG_RECOV_PORST_EN:1;
    uint32_t RESERVED:30;
  } B;
  uint32_t W;
} PPU_PTCR_Type;


typedef union
{
  struct
  {
    uint32_t PWR_DEVACTIVE_STATUS:11;
    uint32_t RESERVED:13;
    uint32_t OP_DEVACTIVE_STATUS:8;
  } B;
  uint32_t W;
} PPU_DISR_Type;


typedef union
{
  struct
  {
    uint32_t PCSMPACCEPT_STATUS:1;
    uint32_t RESERVED0:7;
    uint32_t DEVACCEPT_STATUS:8;
    uint32_t DEVDENY_STATUS:8;
    uint32_t RESERVED1:8;
  } B;
  uint32_t W;
} PPU_MISR_Type;


typedef union
{
  struct
  {
    uint32_t STORED_DEVDENY:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} PPU_STSR_Type;

typedef union
{
  struct
  {
    uint32_t UNLOCK:1;
    uint32_t RESERVED:31;
  } B;
  uint32_t W;
} PPU_UNLK_Type;

typedef union
{
  struct
  {
    uint32_t DEVREQEN:8;
    uint32_t PWR_DEVACTIVEEN:11;
    uint32_t RESERVED:5;
    uint32_t OP_DEVACTIVEEN:8;
  } B;
  uint32_t W;
} PPU_PWCR_Type;

typedef union
{
  struct
  {
    uint32_t DEVACTIVE00_EDGE:2;
    uint32_t DEVACTIVE01_EDGE:2;
    uint32_t DEVACTIVE02_EDGE:2;
    uint32_t DEVACTIVE03_EDGE:2;
    uint32_t DEVACTIVE04_EDGE:2;
    uint32_t DEVACTIVE05_EDGE:2;
    uint32_t DEVACTIVE06_EDGE:2;
    uint32_t DEVACTIVE07_EDGE:2;
    uint32_t DEVACTIVE08_EDGE:2;
    uint32_t DEVACTIVE09_EDGE:2;
    uint32_t DEVACTIVE10_EDGE:2;
    uint32_t RESERVED:10;
  } B;
  uint32_t W;
} PPU_IESR_Type;


typedef union
{
  struct
  {
    uint32_t DEVACTIVE16_EDGE:2;
    uint32_t DEVACTIVE17_EDGE:2;
    uint32_t DEVACTIVE18_EDGE:2;
    uint32_t DEVACTIVE19_EDGE:2;
    uint32_t DEVACTIVE20_EDGE:2;
    uint32_t DEVACTIVE21_EDGE:2;
    uint32_t DEVACTIVE22_EDGE:2;
    uint32_t DEVACTIVE23_EDGE:2;
    uint32_t RESERVED:16;
  } B;
  uint32_t W;
} PPU_OPSR_Type;

typedef union
{
  struct
  {
    uint32_t FUNC_RET_RAM_CFG:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} PPU_FUNRR_Type;

typedef union
{
  struct
  {
    uint32_t FULL_RET_RAM_CFG:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} PPU_FULRR_Type;


typedef union
{
  struct
  {
    uint32_t MEM_RET_RAM_CFG:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} PPU_MEMRR_Type;


typedef union
{
  struct
  {
    uint32_t OFF_DEL:8;
    uint32_t MEM_RET_DEL:8;
    uint32_t LOGIC_RET_DEL:8;
    uint32_t FULL_RET_DEL:8;
  } B;
  uint32_t W;
} PPU_EDTR0_Type;

typedef union
{
  struct
  {
    uint32_t MEM_OFF_DEL:8;
    uint32_t FUNC_RET_DEL:8;
    uint32_t RESERVED:16;
  } B;
  uint32_t W;
} PPU_EDTR1_Type;

typedef union
{
  struct
  {
    uint32_t CLKEN_RST_DLY:8;
    uint32_t ISO_CLKEN_DLY:8;
    uint32_t RST_HWSTAT_DLY:8;
    uint32_t RESERVED:8;
  } B;
  uint32_t W;
} PPU_DCDR0_Type;

typedef union
{
  struct
  {
    uint32_t ISO_RST_DLY:8;
    uint32_t CLKEN_ISO_DLY:8;
    uint32_t RESERVED:16;
  } B;
  uint32_t W;
} PPU_DCDR1_Type;


/*------------ Power Policy Unit typedef -----------*/
typedef struct
{
  __IO  PPU_PWRP_Type                   PPU_PWRP;             /*!< Offset: 0x0 */
  __IO  uint32_t                        PPU_PMER;             /*!< Offset: 0x4 */
  __I   PPU_PWSR_Type                   PPU_PWSR;             /*!< Offset: 0x8 */
  __I   uint32_t                        RESERVED1;            /*!< Offset: 0x0c */

  __I   PPU_DISR_Type                   PPU_DISR;             /*!< Offset: 0x10 */
  __I   PPU_MISR_Type                   PPU_MISR;             /*!< Offset: 0x14 */
  __I   PPU_STSR_Type                   PPU_STSR;             /*!< Offset: 0x18 */
  __IO  PPU_UNLK_Type                   PPU_UNLK;             /*!< Offset: 0x1C */

  __IO  PPU_PWCR_Type                   PPU_PWCR;             /*!< Offset: 0x20 */
  __IO  PPU_PTCR_Type                   PPU_PTCR;             /*!< Offset: 0x24 */
  __I   uint32_t                        RESERVED2[2];         /*!< Offset: 0x28 - 0x2C */

  __IO  PPU_IMR_Type                    PPU_IMR;              /*!< Offset: 0x30 */
  __IO  PPU_AIMR_Type                   PPU_AIMR;             /*!< Offset: 0x34 */
  __IO  PPU_ISR_Type                    PPU_ISR;              /*!< Offset: 0x38 */
  __IO  PPU_AISR_Type                   PPU_AISR;             /*!< Offset: 0x3C */

  __IO  PPU_IESR_Type                   PPU_IESR;             /*!< Offset: 0x40 */
  __IO  PPU_OPSR_Type                   PPU_OPSR;             /*!< Offset: 0x44 */
  __I   uint32_t                        RESERVED3[2];         /*!< Offset: 0x48 - 0x4C */

  __IO  PPU_FUNRR_Type                  PPU_FUNRR;            /*!< Offset: 0x50 */
  __IO  PPU_FULRR_Type                  PPU_FULRR;            /*!< Offset: 0x54 */
  __IO  PPU_MEMRR_Type                  PPU_MEMRR;            /*!< Offset: 0x58 */
  __IO  uint32_t                        RESERVED4;            /*!< Offset: 0x5C */

  __I   uint32_t                        RESERVED5[64];        /*!< Offset: 0x60 - 0x15C */

  __IO  PPU_EDTR0_Type                  PPU_EDTR0;            /*!< Offset: 0x160 */
  __IO  PPU_EDTR1_Type                  PPU_EDTR1;            /*!< Offset: 0x164 */
  __I   uint32_t                        RESERVED6[2];         /*!< Offset: 0x168 - 0x16C */

  __IO  PPU_DCDR0_Type                  PPU_DCDR0;            /*!< Offset: 0x170 */
  __IO  PPU_DCDR1_Type                  PPU_DCDR1;            /*!< Offset: 0x174 */
  __I   uint32_t                        RESERVED7[2];         /*!< Offset: 0x178 - 0x17C */

  __I  uint32_t                         RESERVED8[908];       /*!< Offset: 0x180 - 0xfac */


  __I  uint32_t                         PPU_IDR0;             /*!< Offset: 0xfb0 */
  __I  uint32_t                         PPU_IDR1;             /*!< Offset: 0xfb4 */
  __I  uint32_t                         RESERVED9[2];         /*!< Offset: 0xfb8 - 0xfbc */

  __I  uint32_t                         RESERVED10[2];        /*!< Offset: 0xFC0 - 0xfc4 */
  __I  uint32_t                         PPU_IIDR;             /*!< Offset: 0xfc8 */
  __I  uint32_t                         PPU_AIDR;             /*!< Offset: 0xfcc */

  __I  uint32_t                         PID4;                 /*!< Offset: 0xfd0 */
  __I  uint32_t                         PID5;                 /*!< Offset: 0xfd4 */
  __I  uint32_t                         PID6;                 /*!< Offset: 0xfd8 */
  __I  uint32_t                         PID7;                 /*!< Offset: 0xfdc */

  __I  uint32_t                         PID0;                 /*!< Offset: 0xfe0 */
  __I  uint32_t                         PID1;                 /*!< Offset: 0xfe4 */
  __I  uint32_t                         PID2;                 /*!< Offset: 0xfe8 */
  __I  uint32_t                         PID3;                 /*!< Offset: 0xfec */

  __I  uint32_t                         ID0;                  /*!< Offset: 0xff0 */
  __I  uint32_t                         ID1;                  /*!< Offset: 0xff4 */
  __I  uint32_t                         ID2;                  /*!< Offset: 0xff8 */
  __I  uint32_t                         ID3;                  /*!< Offset: 0xffc */

} SCP_PPU_1_1_TypeDef;

//Address Offsets - needed when Write is done to RO with '__I' type
#define PPU_PWRP_OFFSET 0x00
#define PPU_PMER_OFFSET 0x04
#define PPU_PWSR_OFFSET 0x08

#define PPU_DISR_OFFSET 0x10
#define PPU_MISR_OFFSET 0x14
#define PPU_STSR_OFFSET 0x18
#define PPU_UNLK_OFFSET 0x1C

#define PPU_PWCR_OFFSET 0x20
#define PPU_PTCR_OFFSET 0x24

#endif  // SCP_PPU_1_1_DEF_H

void ppu_set_PWRP (SCP_PPU_1_1_TypeDef*, uint32_t);
void ppu_set_power_policy (SCP_PPU_1_1_TypeDef*, uint32_t );
void ppu_check_power_policy (SCP_PPU_1_1_TypeDef*, uint32_t);
void ppu_clear_irq_mask(SCP_PPU_1_1_TypeDef*);
void ppu_set_irq_mask(SCP_PPU_1_1_TypeDef *ppu);
void ppu_clear_intr(SCP_PPU_1_1_TypeDef*);

#define ROM_PPU_BASE      0x01200000
#define SRAM_PPU_BASE     0x01201000
#define A53_CORE_PPU_BASE 0x01202000
#define A53_NEON_PPU_BASE 0x01203000
#define A53_L2_PPU_BASE   0x01204000

#define ROM_PPU       ((SCP_PPU_1_1_TypeDef *) ROM_PPU_BASE)
#define SRAM_PPU      ((SCP_PPU_1_1_TypeDef *) SRAM_PPU_BASE)
#define A53_CORE_PPU  ((SCP_PPU_1_1_TypeDef *) A53_CORE_PPU_BASE)
#define A53_NEON_PPU  ((SCP_PPU_1_1_TypeDef *) A53_NEON_PPU_BASE)
#define A53_L2_PPU    ((SCP_PPU_1_1_TypeDef *) A53_L2_PPU_BASE)
