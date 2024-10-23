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

#include "global_defines.h"

#ifndef HOST_CHASSIS_CTRL_DEF_H
#define HOST_CHASSIS_CTRL_DEF_H

#define HOST_CHASSIS_CTRL_RES0_SIZE  3
#define HOST_CHASSIS_CTRL_RES1_SIZE  1
#define HOST_CHASSIS_CTRL_RES2_SIZE  1
#define HOST_CHASSIS_CTRL_RES3_SIZE  1
#define HOST_CHASSIS_CTRL_RES4_SIZE  109
#define HOST_CHASSIS_CTRL_RES5_SIZE  63
#define HOST_CHASSIS_CTRL_RES6_SIZE  2
#define HOST_CHASSIS_CTRL_RES7_SIZE  52
#define HOST_CHASSIS_CTRL_RES8_SIZE  62
#define HOST_CHASSIS_CTRL_RES9_SIZE  189
#define HOST_CHASSIS_CTRL_RES10_SIZE 1
#define HOST_CHASSIS_CTRL_RES11_SIZE 2
#define HOST_CHASSIS_CTRL_RES12_SIZE 2
#define HOST_CHASSIS_CTRL_RES13_SIZE 2
#define HOST_CHASSIS_CTRL_RES14_SIZE 2
#define HOST_CHASSIS_CTRL_RES15_SIZE 59
#define HOST_CHASSIS_CTRL_RES16_SIZE 307
#define HOST_CHASSIS_CTRL_RES19_SIZE 103

typedef union{
  struct
  {
    uint32_t CRYPTODISABLE:1;
    uint32_t RESERVED:31;
  } B;
  uint32_t W;
} CLUSTER_CONFIG_Type;

typedef union{
  struct
  {
    uint32_t CFGEND:1;
    uint32_t CFGTE:1;
    uint32_t VINITHI:1;
    uint32_t AA64nAA32:1;
    uint32_t RESERVED:28;
  } B;
  uint32_t W;
} PEn_CONFIG_Type;

typedef union{
  struct
  {
    uint32_t RESERVED:2;
    uint32_t RVBAR31_2:30;
  } B;
  uint32_t W;
} PEn_RVBARADDR_LW_Type;

typedef union{
  struct
  {
    uint32_t RVBAR43_32:12;
    uint32_t RESERVED:20;
  } B;
  uint32_t W;
} PEn_RVBARADDR_UP_Type;

typedef union{
  struct
  {
    uint32_t POR:1;
    uint32_t nSRST:1;
    uint32_t SDC:1;
    uint32_t HOST:1;
    uint32_t RESERVED:28;
  } B;
  uint32_t W;
} HOST_RST_SYN_Type;

typedef union{
  struct
  {
    uint32_t BOOT_MSK:4;
    uint32_t RESERVED:28;
  } B;
  uint32_t W;
} HOST_CPU_BOOT_MSK_Type;

typedef union{
  struct
  {
    uint32_t PWR_REQ:1;
    uint32_t MEM_RET_REQ:1;
    uint32_t RESERVED:30;
  } B;
  uint32_t W;
} HOST_CPU_CLUS_PWR_REQ_Type;

typedef union{
  struct
  {
    uint32_t CORE0_WAKEUP:1;
    uint32_t CORE1_WAKEUP:1;
    uint32_t CORE2_WAKEUP:1;
    uint32_t CORE3_WAKEUP:1;
    uint32_t RESERVED:28;
  } B;
  uint32_t W;
} HOST_CPU_WAKEUP_Type;

typedef union{
  struct
  {
    uint32_t CPUWAIT:1;
    uint32_t RST_REQ:1;
    uint32_t RESERVED:30;
  } B;
  uint32_t W;
} EXT_SYSn_RST_CTRL_Type;

typedef union{
  struct
  {
    uint32_t RESERVED0:1;
    uint32_t RST_ACK:2;
    uint32_t RESERVED1:29;
  } B;
  uint32_t W;
} EXT_SYSn_RST_ST_Type;

typedef union{
  struct
  {
    uint32_t WAKEUP_EN:1;
    uint32_t REFCLK_REQ:1;
    uint32_t DBGTOP_PWR_REQ:1;
    uint32_t SYSTOP_PWR_REQ:3;
    uint32_t RESERVED:26;
  } B;
  uint32_t W;
} CHS_PWR_REQ_Type;

typedef union{
  struct
  {
    uint32_t RESERVED0:2;
    uint32_t DBGTOP_PWR_ST:1;
    uint32_t SYSTOP_PWR_ST:3;
    uint32_t RESERVED1:26;
  } B;
  uint32_t W;
} CHS_PWR_ST_Type;

typedef union{
  struct
  {
    uint32_t HOST_FW_LOCK:1;
    uint32_t INT_RTR_LOCK:1;
    uint32_t HOST_CPU0_LOCK:1;
    uint32_t HOST_CPU1_LOCK:1;
    uint32_t HOST_CPU2_LOCK:1;
    uint32_t HOST_CPU3_LOCK:1;
    uint32_t HOST_GIC_LOCK:1;
    uint32_t HOST_CHS_LOCK:1;
    uint32_t RESERVED:23;
    uint32_t WOR:1;
  } B;
  uint32_t W;
} HOST_SYS_LCTRL_Type;

typedef union{
  struct
  {
    uint32_t CLKSELECT:8;
    uint32_t CLKSELECT_CUR:8;
    uint32_t RESERVED:16;
  } B;
  uint32_t W;
} HOSTCPUCLK_CTRL_Type;

typedef union{
  struct
  {
    uint32_t CLKDIV:5;
    uint32_t RESERVED0:11;
    uint32_t CLKDIV_CUR:5;
    uint32_t RESERVED1:11;
  } B;
  uint32_t W;
} HOSTCPUCLK_DIV0_Type;

typedef union{
  struct
  {
    uint32_t CLKDIV:5;
    uint32_t RESERVED0:11;
    uint32_t CLKDIV_CUR:5;
    uint32_t RESERVED1:11;
  } B;
  uint32_t W;
} HOSTCPUCLK_DIV1_Type;

typedef union{
  struct
  {
    uint32_t CLKSELECT:8;
    uint32_t CLKSELECT_CUR:8;
    uint32_t RESERVED:16;
  } B;
  uint32_t W;
} GICCLK_CTRL_Type;

typedef union{
  struct
  {
    uint32_t CLKDIV:5;
    uint32_t RESERVED0:11;
    uint32_t CLKDIV_CUR:5;
    uint32_t RESERVED1:11;
  } B;
  uint32_t W;
} GICCLK_DIV0_Type;

typedef union{
  struct
  {
    uint32_t CLKSELECT:8;
    uint32_t CLKSELECT_CUR:8;
    uint32_t RESERVED:8;
    uint32_t ENTRY_DELAY:8;
  } B;
  uint32_t W;
} ACLK_CTRL_Type;

typedef union{
  struct
  {
    uint32_t CLKDIV:5;
    uint32_t RESERVED0:11;
    uint32_t CLKDIV_CUR:5;
    uint32_t RESERVED1:11;
  } B;
  uint32_t W;
} ACLK_DIV0_Type;

typedef union{
  struct
  {
    uint32_t CLKSELECT:8;
    uint32_t CLKSELECT_CUR:8;
    uint32_t RESERVED:8;
    uint32_t ENTRY_DELAY:8;
  } B;
  uint32_t W;
} CTRLCLK_CTRL_Type;

typedef union{
  struct
  {
    uint32_t CLKDIV:5;
    uint32_t RESERVED0:11;
    uint32_t CLKDIV_CUR:5;
    uint32_t RESERVED1:11;
  } B;
  uint32_t W;
} CTRLCLK_DIV0_Type;

typedef union{
  struct
  {
    uint32_t CLKSELECT:8;
    uint32_t CLKSELECT_CUR:8;
    uint32_t RESERVED:8;
    uint32_t ENTRY_DELAY:8;
  } B;
  uint32_t W;
} DBGCLK_CTRL_Type;

typedef union{
  struct
  {
    uint32_t CLKDIV:5;
    uint32_t RESERVED0:11;
    uint32_t CLKDIV_CUR:5;
    uint32_t RESERVED1:11;
  } B;
  uint32_t W;
} DBGCLK_DIV0_Type;

typedef union{
  struct
  {
    uint32_t CLKSELECT:8;
    uint32_t CLKSELECT_CUR:8;
    uint32_t RESERVED:16;
  } B;
  uint32_t W;
} HOSTUARTCLK_CTRL_Type;

typedef union{
  struct
  {
    uint32_t CLKDIV:5;
    uint32_t RESERVED0:11;
    uint32_t CLKDIV_CUR:5;
    uint32_t RESERVED1:11;
  } B;
  uint32_t W;
} HOSTUARTCLK_DIV0_Type;

typedef union{
  struct
  {
    uint32_t CLKSELECT:8;
    uint32_t CLKSELECT_CUR:8;
    uint32_t RESERVED:8;
    uint32_t ENTRY_DELAY:8;
  } B;
  uint32_t W;
} REFCLK_CTRL_Type;

typedef union{
  struct
  {
    uint32_t RESERVED0:1;
    uint32_t GICCLK_FORCE_ST:1;
    uint32_t ACLK_FORCE_ST:1;
    uint32_t CTRLCLK_FORCE_ST:1;
    uint32_t DBGCLK_FORCE_ST:1;
    uint32_t RESERVED1:27;
  } B;
  uint32_t W;
} CLKFORCE_ST_Type;

typedef union{
  struct
  {
    uint32_t RESERVED0:1;
    uint32_t GICCLK_FORCE_SET:1;
    uint32_t ACLK_FORCE_SET:1;
    uint32_t CTRL_CLK_FORCE_SET:1;
    uint32_t DBGCLK_FORCE_SET:1;
    uint32_t RESERVED1:27;
  } B;
  uint32_t W;
} CLKFORCE_SET_Type;

typedef union{
  struct
  {
    uint32_t RESERVED0:1;
    uint32_t GICCLK_FORCE_CLR:1;
    uint32_t ACLK_FORCE_CLR:1;
    uint32_t CTRL_CLK_FORCE_CLR:1;
    uint32_t DBGCLK_FORCE_CLR:1;
    uint32_t RESERVED1:27;
  } B;
  uint32_t W;
} CLKFORCE_CLR_Type;

typedef union{
  struct
  {
    uint32_t SYSPLLLOCK_ST:1;
    uint32_t CPUPLLLOCK_ST:1;
    uint32_t RESERVED:30;
  } B;
  uint32_t W;
} PLL_ST_Type;

typedef union{
  struct
  {
    uint32_t FW_INT_ST:1;
    uint32_t DBGTOP_INT_ST:1;
    uint32_t SYSTOP_INT_ST:1;
    uint32_t CLUSTOP_INT_ST:1;
    uint32_t CORE0_INT_ST:1;
    uint32_t CORE1_INT_ST:1;
    uint32_t CORE2_INT_ST:1;
    uint32_t CORE3_INT_ST:1;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_PPU_INT_ST_Type;

typedef union{
  struct
  {
    uint32_t DES_2:4;
    uint32_t Size:4;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_PID4_Type;

typedef union{
  struct
  {
    uint32_t RESERVED:32;
  } B;
  uint32_t W;
} HOST_CH_PID5_Type;

typedef union{
  struct
  {
    uint32_t RESERVED:32;
  } B;
  uint32_t W;
} HOST_CH_PID6_Type;

typedef union{
  struct
  {
    uint32_t RESERVED:32;
  } B;
  uint32_t W;
} HOST_CH_PID7_Type;

typedef union{
  struct
  {
    uint32_t PART_0:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_PID0_Type;

typedef union{
  struct
  {
    uint32_t PART_1:4;
    uint32_t DES_0:4;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_PID1_Type;

typedef union{
  struct
  {
    uint32_t DES_1:3;
    uint32_t JEDEC:1;
    uint32_t REVISION:4;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_PID2_Type;

typedef union{
  struct
  {
    uint32_t CMOD:4;
    uint32_t REVAD:4;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_PID3_Type;

typedef union{
  struct
  {
    uint32_t PRMBL_0:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_CID0_Type;

typedef union{
  struct
  {
    uint32_t PRMBL_1:4;
    uint32_t CLASS:4;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_CID1_Type;

typedef union{
  struct
  {
    uint32_t PRMBL_2:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_CID2_Type;

typedef union{
  struct
  {
    uint32_t PRMBL_3:8;
    uint32_t RESERVED:24;
  } B;
  uint32_t W;
} HOST_CH_CID3_Type;


typedef struct{
  __IO CLUSTER_CONFIG_Type        CLUSTER_CONFIG;                           /*!< Offset: 0x000 */
  __I  uint32_t                   RESERVED0[HOST_CHASSIS_CTRL_RES0_SIZE];   /*!< Offset: 0x004-0x00C */
  __IO PEn_CONFIG_Type            PE0_CONFIG;                               /*!< Offset: 0x010 */
  __IO PEn_RVBARADDR_LW_Type      PE0_RVBARADDR_LW;                         /*!< Offset: 0x014 */
  __IO PEn_RVBARADDR_UP_Type      PE0_RVBARADDR_UP;                         /*!< Offset: 0x018 */
  __IO uint32_t                   RESERVED1[HOST_CHASSIS_CTRL_RES1_SIZE];   /*!< Offset: 0x01C */
  __IO PEn_CONFIG_Type            PE1_CONFIG;                               /*!< Offset: 0x020 */
  __IO PEn_RVBARADDR_LW_Type      PE1_RVBARADDR_LW;                         /*!< Offset: 0x024 */
  __IO PEn_RVBARADDR_UP_Type      PE1_RVBARADDR_UP;                         /*!< Offset: 0x028 */
  __IO uint32_t                   RESERVED2[HOST_CHASSIS_CTRL_RES2_SIZE];   /*!< Offset: 0x02C */
  __IO PEn_CONFIG_Type            PE2_CONFIG;                               /*!< Offset: 0x030 */
  __IO PEn_RVBARADDR_LW_Type      PE2_RVBARADDR_LW;                         /*!< Offset: 0x034 */
  __IO PEn_RVBARADDR_UP_Type      PE2_RVBARADDR_UP;                         /*!< Offset: 0x038 */
  __IO uint32_t                   RESERVED3[HOST_CHASSIS_CTRL_RES3_SIZE];   /*!< Offset: 0x03C */
  __IO PEn_CONFIG_Type            PE3_CONFIG;                               /*!< Offset: 0x040 */
  __IO PEn_RVBARADDR_LW_Type      PE3_RVBARADDR_LW;                         /*!< Offset: 0x044 */
  __IO PEn_RVBARADDR_UP_Type      PE3_RVBARADDR_UP;                         /*!< Offset: 0x048 */
  __IO uint32_t                   RESERVED4[HOST_CHASSIS_CTRL_RES4_SIZE];   /*!< Offset: 0x04C-0x1FC */
  __I  HOST_RST_SYN_Type          HOST_RST_SYN;                             /*!< Offset: 0x200 */
  __IO uint32_t                   RESERVED5[HOST_CHASSIS_CTRL_RES5_SIZE];   /*!< Offset: 0x204-0x2FC */
  __IO HOST_CPU_BOOT_MSK_Type     HOST_CPU_BOOT_MSK_Type;                   /*!< Offset: 0x300 */
  __IO HOST_CPU_CLUS_PWR_REQ_Type HOST_CPU_CLUS_PWR_REQ;                    /*!< Offset: 0x304 */
  __O  HOST_CPU_WAKEUP_Type       HOST_CPU_WAKEUP;                          /*!< Offset: 0x308 */
  __IO uint32_t                   RESERVED6[HOST_CHASSIS_CTRL_RES1_SIZE];   /*!< Offset: 0x30C */
  __IO EXT_SYSn_RST_CTRL_Type     EXT_SYS0_RST_CTRL;                        /*!< Offset: 0x310 */
  __IO EXT_SYSn_RST_ST_Type       EXT_SYS0_RST_ST;                          /*!< Offset: 0x314 */
  __IO EXT_SYSn_RST_CTRL_Type     EXT_SYS1_RST_CTRL;                        /*!< Offset: 0x318 */
  __IO EXT_SYSn_RST_ST_Type       EXT_SYS1_RST_ST;                          /*!< Offset: 0x31C */
  __IO EXT_SYSn_RST_CTRL_Type     EXT_SYS2_RST_CTRL;                        /*!< Offset: 0x320 */
  __IO EXT_SYSn_RST_ST_Type       EXT_SYS2_RST_ST;                          /*!< Offset: 0x324 */
  __IO EXT_SYSn_RST_CTRL_Type     EXT_SYS3_RST_CTRL;                        /*!< Offset: 0x328 */
  __IO EXT_SYSn_RST_ST_Type       EXT_SYS3_RST_ST;                          /*!< Offset: 0x32C */
  __IO uint32_t                   RESERVED7[HOST_CHASSIS_CTRL_RES7_SIZE];   /*!< Offset: 0x330-0x3FC */
  __IO CHS_PWR_REQ_Type           CHS_PWR_REQ;                              /*!< Offset: 0x400 */
  __I  CHS_PWR_ST_Type            CHS_PWR_ST;                               /*!< Offset: 0x404 */
  __IO uint32_t                   RESERVED8[HOST_CHASSIS_CTRL_RES8_SIZE];   /*!< Offset: 0x408-0x4FC */
  __I  HOST_SYS_LCTRL_Type        HOST_SYS_LCTRL_ST;                        /*!< Offset: 0x500 */
  __O  HOST_SYS_LCTRL_Type        HOST_SYS_LCTRL_SET;                       /*!< Offset: 0x504 */
  __O  HOST_SYS_LCTRL_Type        HOST_SYS_LCTRL_CLR;                       /*!< Offset: 0x508 */
  __IO uint32_t                   RESERVED9[HOST_CHASSIS_CTRL_RES9_SIZE];   /*!< Offset: 0x504-0x7FC */
  __IO HOSTCPUCLK_CTRL_Type       HOSTCPUCLK_CTRL;                          /*!< Offset: 0x800 */
  __IO HOSTCPUCLK_DIV0_Type       HOSTCPUCLK_DIV0;                          /*!< Offset: 0x804 */
  __IO HOSTCPUCLK_DIV1_Type       HOSTCPUCLK_DIV1;                          /*!< Offset: 0x808 */
  __IO uint32_t                   RESERVED10[HOST_CHASSIS_CTRL_RES10_SIZE]; /*!< Offset: 0x80C */
  __IO GICCLK_CTRL_Type           GICCLK_CTRL;                              /*!< Offset: 0x810 */
  __IO GICCLK_DIV0_Type           GICCLK_DIV0;                              /*!< Offset: 0x814 */
  __IO uint32_t                   RESERVED11[HOST_CHASSIS_CTRL_RES11_SIZE]; /*!< Offset: 0x818-0x81C */
  __IO ACLK_CTRL_Type             ACLK_CTRL;                                /*!< Offset: 0x820 */
  __IO ACLK_DIV0_Type             ACLK_DIV0;                                /*!< Offset: 0x824 */
  __IO uint32_t                   RESERVED12[HOST_CHASSIS_CTRL_RES12_SIZE]; /*!< Offset: 0x828-0x82C */
  __IO CTRLCLK_CTRL_Type          CTRLCLK_CTRL;                             /*!< Offset: 0x830 */
  __IO CTRLCLK_DIV0_Type          CTRLCLK_DIV0;                             /*!< Offset: 0x834 */
  __IO uint32_t                   RESERVED13[HOST_CHASSIS_CTRL_RES13_SIZE]; /*!< Offset: 0x838-0x83C */
  __IO DBGCLK_CTRL_Type           DBGCLK_CTRL;                              /*!< Offset: 0x840 */
  __IO DBGCLK_DIV0_Type           DBGCLK_DIV0;                              /*!< Offset: 0x844 */
  __IO uint32_t                   RESERVED17[HOST_CHASSIS_CTRL_RES13_SIZE]; /*!< Offset: 0x848-0x84C */
  __IO HOSTUARTCLK_CTRL_Type      HOSTUARTCLK_CTRL;                         /*!< Offset: 0x850 */
  __IO HOSTUARTCLK_DIV0_Type      HOSTUARTCLK_DIV0;                         /*!< Offset: 0x854 */
  __IO uint32_t                   RESERVED14[HOST_CHASSIS_CTRL_RES14_SIZE]; /*!< Offset: 0x858-0x85C */
  __IO REFCLK_CTRL_Type           REFCLK_CTRL;                              /*!< Offset: 0x860 */
  __IO uint32_t                   RESERVED19[HOST_CHASSIS_CTRL_RES19_SIZE]; /*!< Offset: 0x864-0x9FC */
  __I  CLKFORCE_ST_Type           CLKFORCE_ST;                              /*!< Offset: 0xA00 */
  __O  CLKFORCE_SET_Type          CLKFORCE_SET;                             /*!< Offset: 0xA04 */
  __O  CLKFORCE_CLR_Type          CLKFORCE_CLR;                             /*!< Offset: 0xA08 */
  __IO uint32_t                   RESERVED18[HOST_CHASSIS_CTRL_RES1_SIZE];  /*!< Offset: 0xA0C */
  __I  PLL_ST_Type                PLL_ST;                                   /*!< Offset: 0xA10 */
  __IO uint32_t                   RESERVED15[HOST_CHASSIS_CTRL_RES15_SIZE]; /*!< Offset: 0xA14-0xAFC */
  __I  HOST_PPU_INT_ST_Type       HOST_PPU_INT_ST;                          /*!< Offset: 0xB00 */
  __IO uint32_t                   RESERVED16[HOST_CHASSIS_CTRL_RES16_SIZE]; /*!< Offset: 0xB04-0xFCC */
  __IO HOST_CH_PID4_Type          PID4;                                     /*!< Offset: 0xFD0 */
  __IO HOST_CH_PID5_Type          PID5;                                     /*!< Offset: 0xFD4 */
  __IO HOST_CH_PID6_Type          PID6;                                     /*!< Offset: 0xFD8 */
  __IO HOST_CH_PID7_Type          PID7;                                     /*!< Offset: 0xFDC */
  __IO HOST_CH_PID0_Type          PID0;                                     /*!< Offset: 0xFE0 */
  __IO HOST_CH_PID1_Type          PID1;                                     /*!< Offset: 0xFE4 */
  __IO HOST_CH_PID2_Type          PID2;                                     /*!< Offset: 0xFE8 */
  __IO HOST_CH_PID3_Type          PID3;                                     /*!< Offset: 0xFEC */
  __IO HOST_CH_CID0_Type          CID0;                                     /*!< Offset: 0xFF0 */
  __IO HOST_CH_CID1_Type          CID1;                                     /*!< Offset: 0xFF4 */
  __IO HOST_CH_CID2_Type          CID2;                                     /*!< Offset: 0xFF8 */
  __IO HOST_CH_CID3_Type          CID3;                                     /*!< Offset: 0xFFC */
  
  
} HOST_CHASSIS_CTRL_TypeDef;


#endif /* HOST_CHASSIS_CTRL_DEF_H */
