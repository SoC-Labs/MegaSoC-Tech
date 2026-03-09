#include <stdint.h>
#include "sys_memory_map.h"

/* Operating Mode Status Register */
typedef union{
    struct{
        uint32_t operating_mode:4;
        uint32_t selfref_type:2;
        uint32_t Rsvd:2;
        uint32_t selfref_state:2;
        uint32_t Rsvd1:2;
        uint32_t selfref_cam_not_empty:1;
        uint32_t Rsvd2:19;
    } B;
    uint32_t W;
} DDRC_STAT_Type;

/* Low Power Control Register */
typedef union{
    struct {
        uint32_t selfref_en:1;
        uint32_t powerdown_en:1;
        uint32_t deeppowerdown_en:1;
        uint32_t en_dfi_dram_clk_disable:1;
        uint32_t mpsm_en:1;
        uint32_t selfref_sw:1;
        uint32_t stay_in_selfref:1;
        uint32_t dis_cam_drain_selfref:1;
        uint32_t lpddr4_sr_allowed:1;
        uint32_t Rsvd:23;
    } B;
    uint32_t W;
} DDRC_PWRCTRL_Type;

 /* Refresh Control Register 3 */
typedef union{
    struct{
        uint32_t dis_auto_refresh:1;
        uint32_t refresh_update_level:1;
        uint32_t Rsvd:2;
        uint32_t refresh_mode:3;
        uint32_t Rsvd1:9;
        uint32_t rank_dis_refresh:16;
    } B;
    uint32_t W;
} DDRC_RFSHCTL3_Type;

/* SDRAM Initialisation register 0 */
typedef union{
    struct{
        volatile uint32_t pre_cke_x1024:12;
        volatile uint32_t resvd:4;
        volatile uint32_t post_cke_x1024:10;
        volatile uint32_t resvd2:4;
        volatile uint32_t skip_dram_init:2;
    } B;
    uint32_t W;
} DDRC_INIT0_Type;

/* Software Register Programming Control Enable */
typedef union{
    struct{
        uint32_t sw_done:1;
        uint32_t Rsvd:31;
    } B;
    uint32_t W;
} DDRC_SWCTL_Type;

/* Software Register Programming Control Status */
typedef union{
    struct{
        uint32_t sw_done_ack:1;
        uint32_t Rsvd:31;
    } B;
    uint32_t W;
} DDRC_SWSTAT_Type;

/* DFI Miscellaneous Control Register */
typedef union{
    struct{
        uint32_t dfi_init_complete_en:1;
        uint32_t phy_dbi_mode:1;
        uint32_t dfi_data_cs_polarity:1;
        uint32_t share_dfi_dram_clk_disable:1;
        uint32_t ctl_idle_en:1;
        uint32_t dfi_init_start:1;
        uint32_t dis_dyn_adr_tri:1;
        uint32_t lp_optimized_write:1;
        uint32_t dfi_frequency:5;
        uint32_t Rsvd:19;        
    } B;
    uint32_t W;
} DDRC_DFIMISC_Type;

typedef union{
    struct{
        uint32_t dfi_init_complete:1;
        uint32_t dfi_lp_ack:1;
        uint32_t Rsvd:30;
    } B;
    uint32_t W;
} DDRC_DFISTAT_Type;

typedef struct{
    volatile uint32_t  MSTR;    //0x0 Master Register 0
    volatile DDRC_STAT_Type  STAT;    //0x4 Operating Mode Status Register
    volatile uint32_t  MSTR1;   //0x8 Master Register 1
    volatile uint32_t  RESERVED0; //0xC RESERVED
    volatile uint32_t  MRCTRL0;     //0x10 Mode Register Read/Write Control Register 0
    volatile uint32_t  MRCTRL1;     //0x14 Mode Register Read/Write Control Register 1
    volatile uint32_t  MRSTAT;  //0x18 Mode Register Read/Write Status Register
    volatile uint32_t  MRCTRL2;     //0x1c Mode Register Read/Write Control Register 2
    volatile uint32_t  DERATEEN;    //0x20 Temperature Derate Enable Register
    volatile uint32_t  DERATEINT;   //0x24 Temperature Derate Interval Register
    volatile uint32_t  MSTR2;   //0x28 Master Register 2
    volatile uint32_t  DERATECTL;   //0x2c Temperature Derate Control Register
    volatile DDRC_PWRCTRL_Type  PWRCTL;  //0x30 Low Power Control Register
    volatile uint32_t  PWRTMG;  //0x34 Low Power Timing Register
    volatile uint32_t  HWLPCTL;     //0x38 Hardware Low Power Control Register
    volatile uint32_t  HWFFCCTL;    //0x3c Hardware Fast Frequency Change Control Register
    volatile uint32_t  HWFFCSTAT;   //0x40 Hardware Fast Frequency Change Status Register
    volatile uint32_t  HWFFCEX_RANK1;   //0x44 Hardware Fast Frequency Change Function Extended for RANK1 Register
    volatile uint32_t  HWFFCEX_RANK2;   //0x48 Hardware Fast Frequency Change Function Extended for RANK2 Register
    volatile uint32_t  HWFFCEX_RANK3;   //0x4c Hardware Fast Frequency Change Function Extended for RANK3 Register
    volatile uint32_t  RFSHCTL0;    //0x50 Refresh Control Register 0
    volatile uint32_t  RFSHCTL1;    //0x54 Refresh Control Register 1
    volatile uint32_t  RFSHCTL2;    //0x58 Refresh Control Register 2
    volatile uint32_t  RFSHCTL4;    //0x5c Refresh Control Register 4
    volatile DDRC_RFSHCTL3_Type RFSHCTL3;  //0x60 Refresh Control Register 3
    volatile uint32_t  RFSHTMG; //0x64 Refresh Timing Register
    volatile uint32_t  RFSHTMG1;    //0x68 Refresh Timing Register 1
    volatile uint32_t  RESERVED1;       //0x6C
    volatile uint32_t  ECCCFG0;         //0x70 ECC Configuration Register 0
    volatile uint32_t  ECCCFG1;         //0x74 ECC Configuration Register 1
    volatile uint32_t  ECCSTAT;         //0x78 SECDED ECC Status Register
    volatile uint32_t  ECCCTL;          //0x7c ECC Clear Register
    volatile uint32_t  ECCERRCNT;       //0x80 ECC Error Counter Register
    volatile uint32_t  ECCCADDR0;       //0x84 ECC Corrected Error Address Register 0
    volatile uint32_t  ECCCADDR1;       //0x88 ECC Corrected Error Address Register 1
    volatile uint32_t  ECCCSYN0;        //0x8c ECC Corrected Syndrome Register 0
    volatile uint32_t  ECCCSYN1;        //0x90 ECC Corrected Syndrome Register 1
    volatile uint32_t  ECCCSYN2;        //0x94 ECC Corrected Syndrome Register 2
    volatile uint32_t  ECCBITMASK0;     //0x98 ECC Corrected Data Bit Mask Register 0
    volatile uint32_t  ECCBITMASK1;     //0x9c ECC Corrected Data Bit Mask Register 1
    volatile uint32_t  ECCBITMASK2;     //0xa0 ECC Corrected Data Bit Mask Register 2
    volatile uint32_t  ECCUADDR0;       //0xa4 ECC Uncorrected Error Address Register 0
    volatile uint32_t  ECCUADDR1;       //0xa8 ECC Uncorrected Error Address Register 1
    volatile uint32_t  ECCUSYN0;        //0xac ECC Uncorrected Syndrome Register 0
    volatile uint32_t  ECCUSYN1;        //0xb0 ECC Uncorrected Syndrome Register 1
    volatile uint32_t  ECCUSYN2;        //0xb4 ECC Uncorrected Syndrome Register 2
    volatile uint32_t  ECCPOISONADDR0;  //0xb8 ECC Data Poisoning Address Register 0
    volatile uint32_t  ECCPOISONADDR1;  //0xbc ECC Data Poisoning Address Register 1
    volatile uint32_t  CRCPARCTL0;      //0xc0 CRC Parity Control Register 0
    volatile uint32_t  CRCPARCTL1;      //0xc4 CRC Parity Control Register 1
    volatile uint32_t  CRCPARCTL2;      //0xc8 CRC Parity Control Register 2
    volatile uint32_t  CRCPARSTAT;      //0xcc CRC Parity Status Register
    volatile DDRC_INIT0_Type  INIT0;    //0xd0 SDRAM Initialization Register 0
    volatile uint32_t  INIT1;           //0xd4 SDRAM Initialization Register 1
    volatile uint32_t  INIT2;           //0xd8 SDRAM Initialization Register 2
    volatile uint32_t  INIT3;           //0xdc SDRAM Initialization Register 3
    volatile uint32_t  INIT4;           //0xe0 SDRAM Initialization Register 4
    volatile uint32_t  INIT5;           //0xe4 SDRAM Initialization Register 5
    volatile uint32_t  INIT6;           //0xe8 SDRAM Initialization Register 6
    volatile uint32_t  INIT7;           //0xec SDRAM Initialization Register 7
    volatile uint32_t  DIMMCTL;         //0xf0 DIMM Control Register
    volatile uint32_t  RANKCTL;         //0xf4 Rank Control Register
    volatile uint32_t  RANKCTL1;        //0xf8 Rank Control Register 1
    volatile uint32_t  CHCTL;           //0xfc Channel Control Register
    volatile uint32_t  DRAMTMG0;        //0x100 SDRAM Timing Register 0
    volatile uint32_t  DRAMTMG1;        //0x104 SDRAM Timing Register 1
    volatile uint32_t  DRAMTMG2;        //0x108 SDRAM Timing Register 2
    volatile uint32_t  DRAMTMG3;        //0x10c SDRAM Timing Register 3
    volatile uint32_t  DRAMTMG4;        //0x110 SDRAM Timing Register 4
    volatile uint32_t  DRAMTMG5;        //0x114 SDRAM Timing Register 5
    volatile uint32_t  DRAMTMG6;        //0x118 SDRAM Timing Register 6
    volatile uint32_t  DRAMTMG7;        //0x11c SDRAM Timing Register 7
    volatile uint32_t  DRAMTMG8;        //0x120 SDRAM Timing Register 8
    volatile uint32_t  DRAMTMG9;        //0x124 SDRAM Timing Register 9
    volatile uint32_t  DRAMTMG10;       //0x128 SDRAM Timing Register 10
    volatile uint32_t  DRAMTMG11;       //0x12c SDRAM Timing Register 11
    volatile uint32_t  DRAMTMG12;       //0x130 SDRAM Timing Register 12
    volatile uint32_t  DRAMTMG13;       //0x134 SDRAM Timing Register 13
    volatile uint32_t  DRAMTMG14;       //0x138 SDRAM Timing Register 14
    volatile uint32_t  DRAMTMG15;       //0x13c SDRAM Timing Register 15
    volatile uint32_t  DRAMTMG16;       //0x140 SDRAM Timing Register 16
    volatile uint32_t  DRAMTMG17;       //0x144 SDRAM Timing Register 17
    volatile uint32_t  RESERVED2[2];    // 0x148-14C
    volatile uint32_t  RFSHTMG_HET;     //0x150 Refresh Timing Register Heterogeneous
    volatile uint32_t  RESERVED3[7];     //0x154 - 0x16C
    volatile uint32_t  MRAMTMG0;        //0x170 MRAM Timing Register 0
    volatile uint32_t  MRAMTMG1;        //0x174 MRAM Timing Register 1
    volatile uint32_t  MRAMTMG4;        //0x178 MRAM Timing Register 4
    volatile uint32_t  MRAMTMG9;        //0x17c MRAM Timing Register 9
    volatile uint32_t  ZQCTL0;          //0x180 ZQ Control Register 0
    volatile uint32_t  ZQCTL1;          //0x184 ZQ Control Register 1
    volatile uint32_t  ZQCTL2;          //0x188 ZQ Control Register 2
    volatile uint32_t  ZQSTAT;          //0x18c ZQ Status Register
    volatile uint32_t  DFITMG0;         //0x190 DFI Timing Register 0
    volatile uint32_t  DFITMG1;         //0x194 DFI Timing Register 1
    volatile uint32_t  DFILPCFG0;       //0x198 DFI Low Power Configuration Register 0
    volatile uint32_t  DFILPCFG1;       //0x19c DFI Low Power Configuration Register 1
    volatile uint32_t  DFIUPD0;         //0x1a0 DFI Update Register 0
    volatile uint32_t  DFIUPD1;         //0x1a4 DFI Update Register 1
    volatile uint32_t  DFIUPD2;         //0x1a8 DFI Update Register 2
    volatile uint32_t  RESERVED4;       //0x1ac
    volatile DDRC_DFIMISC_Type  DFIMISC;         //0x1b0 DFI Miscellaneous Control Register
    volatile uint32_t  DFITMG2;         //0x1b4 DFI Timing Register 2
    volatile uint32_t  DFITMG3;         //0x1b8 DFI Timing Register 3
    volatile DDRC_DFISTAT_Type  DFISTAT;         //0x1bc DFI Status Register
    volatile uint32_t  DBICTL;          //0x1c0 DM/DBI Control Register
    volatile uint32_t  DFIPHYMSTR;      //0x1c4 DFI PHY Master
    volatile uint32_t  RESERVED5[14];   //0x1C8-1FC
    volatile uint32_t  ADDRMAP0;        //0x200 Address Map Register 0
    volatile uint32_t  ADDRMAP1;        //0x204 Address Map Register 1
    volatile uint32_t  ADDRMAP2;        //0x208 Address Map Register 2
    volatile uint32_t  ADDRMAP3;        //0x20c Address Map Register 3
    volatile uint32_t  ADDRMAP4;        //0x210 Address Map Register 4
    volatile uint32_t  ADDRMAP5;        //0x214 Address Map Register 5
    volatile uint32_t  ADDRMAP6;        //0x218 Address Map Register 6
    volatile uint32_t  ADDRMAP7;        //0x21c Address Map Register 7
    volatile uint32_t  ADDRMAP8;        //0x220 Address Map Register 8
    volatile uint32_t  ADDRMAP9;        //0x224 Address Map Register 9
    volatile uint32_t  ADDRMAP10;       //0x228 Address Map Register 10
    volatile uint32_t  ADDRMAP11;       //0x22c Address Map Register 11
    volatile uint32_t  RESERVED6[4];    //0x230-23C
    volatile uint32_t  ODTCFG;          //0x240 ODT Configuration Register
    volatile uint32_t  ODTMAP;          // 0x244 ODT/Rank Map Register
    volatile uint32_t  RESERVED7[2];
    volatile uint32_t  SCHED;           // 0x250 Scheduler Control Register
    volatile uint32_t  SCHED1;          // 0x254 Scheduler Control Register 1
    volatile uint32_t  SCHED2;          // 0x258 Scheduler Control Register 2
    volatile uint32_t  PERFHPR1;        // 0x25c High Priority Read CAM Register 1
    volatile uint32_t  RESERVED8;
    volatile uint32_t  PERFLPR1;        // 0x264 Low Priority Read CAM Register 1
    volatile uint32_t  RESERVED9;
    volatile uint32_t  PERFWR1;         // 0x26c Write CAM Register 1
    volatile uint32_t  SCHED3;          // 0x270 Scheduler Control Register 3
    volatile uint32_t  SCHED4;          // 0x274 Scheduler Control Register 4
    volatile uint32_t  SCHED5;          // 0x278 Scheduler Control Register 5
    volatile uint32_t  RESERVED10;
    volatile uint32_t  DQMAP0;          // 0x280 DQ Map Register 0
    volatile uint32_t  DQMAP1;          // 0x284 DQ Map Register 1
    volatile uint32_t  DQMAP2;          // 0x288 DQ Map Register 2
    volatile uint32_t  DQMAP3;          // 0x28c DQ Map Register 3
    volatile uint32_t  DQMAP4;          // 0x290 DQ Map Register 4
    volatile uint32_t  DQMAP5;          // 0x294 DQ Map Register 5
    volatile uint32_t  RESERVED11[26];   // 0x298 - 0x2fc
    volatile uint32_t  DBG0;            // 0x300 Debug Register 0
    volatile uint32_t  DBG1;            // 0x304 Debug Register 1
    volatile uint32_t  DBGCAM;          // 0x308 CAM Debug Register
    volatile uint32_t  DBGCMD;          // 0x30c Command Debug Register
    volatile uint32_t  DBGSTAT;         // 0x310 Status Debug Register
    volatile uint32_t  RESERVED12;
    volatile uint32_t  DBGCAM1;         // 0x318 CAM Debug Register 1
    volatile uint32_t  RESERVED13;
    volatile DDRC_SWCTL_Type SWCTL;     // 0x320 Software Register Programming Control Enable
    volatile DDRC_SWSTAT_Type  SWSTAT;  // 0x324 Software Register Programming Control Status
    volatile uint32_t  SWCTLSTATIC;     // 0x328 Static Registers Write Enable
    volatile uint32_t  RESERVED14;
    volatile uint32_t  OCPARCFG0;       // 0x330 On-Chip Parity Configuration Register 0
    volatile uint32_t  OCPARCFG1;       // 0x334 On-Chip Parity Configuration Register 1
    volatile uint32_t  OCPARSTAT0;      // 0x338 On-Chip Parity Status Register 0
    volatile uint32_t  OCPARSTAT1;      // 0x33c On-Chip Parity Status Register 1
    volatile uint32_t  OCPARSTAT2;      // 0x340 On-Chip Parity Status Register 2
    volatile uint32_t  OCPARSTAT3;      // 0x344 On-Chip Parity Read Data Log Register 0
    volatile uint32_t  OCPARSTAT4;      // 0x348 On-Chip Parity Write Address Log Register 0
    volatile uint32_t  OCPARSTAT5;      // 0x34c On-Chip Parity Write Address Log Register 1
    volatile uint32_t  OCPARSTAT6;      // 0x350 On-Chip Parity Read Address Log Register 0
    volatile uint32_t  OCPARSTAT7;      // 0x354 On-Chip Parity Read Address Log Register 1
    volatile uint32_t  OCECCCFG0;       // 0x358 On-Chip ECC Configuration Register 0
    volatile uint32_t  OCECCCFG1;       // 0x35c On-Chip ECC Configuration Register 1
    volatile uint32_t  OCECCSTAT0;      // 0x360 On-Chip ECC Status Register 0
    volatile uint32_t  OCECCSTAT1;      // 0x364 On-Chip ECC Status Register 1
    volatile uint32_t  OCECCSTAT2;      // 0x368 On-Chip ECC Status Register 2
    volatile uint32_t  POISONCFG;       // 0x36c AXI Poison Configuration Register
    volatile uint32_t  POISONSTAT;      // 0x370 AXI Poison Status Register
    volatile uint32_t  ADVECCINDEX;     // 0x374 Advanced ECC Index Register
    volatile uint32_t  ADVECCSTAT;      // 0x378 Advanced ECC Status Register
    volatile uint32_t  ECCPOISONPAT0;   // 0x37c ECC Poison Pattern 0 Register
    volatile uint32_t  ECCPOISONPAT1;   // 0x380 ECC Poison Pattern 1 Register
    volatile uint32_t  ECCPOISONPAT2;   // 0x384 ECC Poison Pattern 2 Register
    volatile uint32_t  ECCAPSTAT;       // 0x388 Address protection within ECC Status Register
    volatile uint32_t  RESERVED15[5];   // 0x38C - 0x39C
    volatile uint32_t  CAPARPOISONCTL;  // 0x3a0 CA parity poison contrl Register
    volatile uint32_t  CAPARPOISONSTAT; // 0x3a4 CA parity poison status Register
    volatile uint32_t  RESERVED16[2];
    volatile uint32_t  DYNBSMSTAT;      // 0x3b0 Dynamic BSM Status Register
    volatile uint32_t  RESERVED17;
    volatile uint32_t  CRCPARCTL3;      // 0x3b8 CRC Parity Control Register 3
    volatile uint32_t  RESERVED18;
    volatile uint32_t  REGPARCFG;       // 0x3c0 Register Parity Configuration Register
    volatile uint32_t  REGPARSTAT;      // 0x3c4 Register Parity Status Register
    volatile uint32_t  RESERVED19[2];
    volatile uint32_t  RCDINIT1;        // 0x3d0 Control Word setting Register RCDINIT1
    volatile uint32_t  RCDINIT2;        // 0x3d4 Control Word setting Register RCDINIT2
    volatile uint32_t  RCDINIT3;        // 0x3d8 Control Word setting Register RCDINIT3
    volatile uint32_t  RCDINIT4;        // 0x3dc Control Word setting Register RCDINIT4
    volatile uint32_t  OCCAPCFG;        // 0x3e0 On-Chip command/Address Protection Configuration Register
    volatile uint32_t  OCCAPSTAT;       // 0x3e4 On-Chip command/Address Protection Status Register
    volatile uint32_t  OCCAPCFG1;       // 0x3e8 On-Chip command/Address Protection Configuration Register 1
    volatile uint32_t  OCCAPSTAT1;      // 0x3ec On-Chip command/Address Protection Status Register 1
    volatile uint32_t  DERATESTAT;      // 0x3f0 Temperature Derate Status Register
} SNPS_MCTL2_DDRC_TypeDef;

typedef struct {
    volatile uint32_t  PSTAT;           // 0x3fc Port Status Register
    volatile uint32_t  PCCFG;           // 0x400 Port Common Configuration Register
    volatile uint32_t  PCFGR_n;         // (for n = 0; n <= 15)” 0x404 + 0xb0*n Port n Configuration Read Register
    volatile uint32_t  PCFGW_n;         // (for n = 0; n <= 15)” 0x408 + 0xb0*n Port n Configuration Write Register
    volatile uint32_t  PCFGC_n;         // (for n = 0; n <= 15)” 0x40c + 0xb0*n Port n Common Configuration Register
    volatile uint32_t  PCFGIDMASKCH;    // m_n (for m,n = 0; m,n <=15)” 0x410 + 0xb0*n +0x8*m Port n Channel m Configuration ID Mask Register
    volatile uint32_t  PCFGIDVALUECH;   // m_n (for m,n = 0; m,n <=15)” 0x414 + 0xb0*n +0x8*m Port n Channel m Configuration ID Value Register
    volatile uint32_t  PCTRL_n;         // (for n = 0; n <= 15)” 0x490 + 0xb0*n Port n Control Register
    volatile uint32_t  PCFGQOS0_n;      // (for n = 0; n <= 15)” 0x494 + 0xb0*n Port n Read QoS Configuration Register 0
    volatile uint32_t  PCFGQOS1_n;      // (for n = 0; n <= 15)” 0x498 + 0xb0*n Port n Read QoS Configuration Register 1
    volatile uint32_t  PCFGWQOS0_n;     // (for n = 0; n <= 15)” 0x49c + 0xb0*n Port n Write QoS Configuration Register 0
    volatile uint32_t  PCFGWQOS1_n;     // (for n = 0; n <= 15)” 0x4a0 + 0xb0*n Port n Write QoS Configuration Register 1
    volatile uint32_t  SARBASEn;        // (for n = 0; n <= 3)” 0xf04 + 0x8*n SAR Base Address Register n
    volatile uint32_t  SARSIZEn;        // (for n = 0; n <= 3)” 0xf08 + 0x8*n SAR Size Register n
    volatile uint32_t  SBRCTL;          // 0xf24 Scrubber Control Register
    volatile uint32_t  SBRSTAT;         // 0xf28 Scrubber Status Register
    volatile uint32_t  SBRWDATA0; // 0xf2c Scrubber Write Data Pattern 0
    volatile uint32_t  SBRWDATA1; // 0xf30 Scrubber Write Data Pattern 1
    volatile uint32_t  PDCH; // 0xf34 Port Data Channel
    volatile uint32_t  SBRSTART0; // 0xf38 Scrubber Start Address Mask Register 0
    volatile uint32_t  SBRSTART1; // 0xf3c Scrubber Start Address Mask Register 1
    volatile uint32_t  SBRRANGE0; // 0xf40 Scrubber Address Range Mask Register 0
    volatile uint32_t  SBRRANGE1; // 0xf44 Scrubber Address Range Mask Register 1
    volatile uint32_t  SBRSTART0DCH1; // 0xf48 Scrubber Start Address Mask Register 0 for Data Channel 1
    volatile uint32_t  SBRSTART1DCH1; // 0xf4c Scrubber Start Address Mask Register 1 for Data Channel 1
    volatile uint32_t  SBRRANGE0DCH1; // 0xf50 Scrubber Address Range Mask Register 0 for Data Channel 1
    volatile uint32_t  SBRRANGE1DCH1; // 0xf54 Scrubber Address Range Mask Register 1 for Data Channel 1
} SNPS_MCTL2_MP_Typedef;

typedef struct {
    volatile uint32_t  UMCTL2_VER_NUMBER; // 0xff0 UMCTL2 Version Number Register
    volatile uint32_t  UMCTL2_VER_TYPE; // 0xff4 UMCTL2 Version Type Registe
} SNPS_MCTL2_ID_TypeDef;

typedef struct{
    volatile uint32_t  RESET;
    volatile uint32_t  PWROK;
    volatile uint32_t  CORE_RSTn;
} DDR_RESET_CTRL_TypeDef;

#define SNPS_MCTL2_DDRC ((SNPS_MCTL2_DDRC_TypeDef *) (DRAM_CFG_BASE))
#define SNPS_MCTL2_ID ((SNPS_MCTL2_ID_TypeDef *)    (DRAM_CFG_BASE+0xFF0UL)) 
#define DDR_RESET_CTRL ((DDR_RESET_CTRL_TypeDef *)  (DRAM_CFG_BASE+0x6000UL))