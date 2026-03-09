#include <stdint.h>

#define DRAM_PHY_ANIB0_BASE DRAM_PHY_CFG_BASE
#define DRAM_PHY_ANIB1_BASE DRAM_PHY_CFG_BASE+0x1000UL
#define DRAM_PHY_ANIB2_BASE DRAM_PHY_CFG_BASE+0x2000UL

#define DRAM_PHY_DBYTE0_BASE DRAM_PHY_CFG_BASE+0x010000UL


typedef struct {
    volatile uint32_t RESERVED0[26];        // PHY address: 0x00-19 SYS address: 0x00-64
    volatile uint32_t MtestMuxSel;          // PHY address: 0x1A    SYS address: 0x68
    volatile uint32_t RESERVED1[12];        // PHY address: 0x1B-26 SYS address: 0x6C-98
    volatile uint32_t AForceDrvCont;        // PHY address: 0x27    SYS address: 0x9C
    volatile uint32_t AForceTriCont;        // PHY address: 0x28    SYS address: 0xA0
    volatile uint32_t RESERVED2[26];        // PHY address: 0x29-42 SYS address: 0xA4-108
    volatile uint32_t ATxImpedance;         // PHY address: 0x43    SYS address: 0x10C
    volatile uint32_t RESERVED3[15];        // PHY address: 0x44-52 SYS address: 0x110-148
    volatile uint32_t ATestPrbsErr;         // PHY address: 0x53    SYS address: 0x14C
    volatile uint32_t RESERVED4;            // PHY address: 0x54    SYS address: 0x150
    volatile uint32_t ATxSlewRate;          // PHY address: 0x55    SYS address: 0x154
    volatile uint32_t ATestPrbsErrCnt;      // PHY address: 0x56    SYS address: 0x158
    volatile uint32_t RESERVED5[41];        // PHY address: 0x57-7F SYS address: 0x15C-1FC
    volatile uint32_t ATxDly_p0;            // PHY address: 0x80    SYS address: 0x200
    volatile uint32_t RESERVED6[1048575];   // PHY address: 0x81-10007F SYS address: 0x204-4001FC
    volatile uint32_t ATxDly_p1;            // PHY address: 0x100080  SYS address: 0x400200
    volatile uint32_t RESERVED7[1048575];   // PHY address: 0x100081-0x20007F SYS address: 0x400204-0x8001FC
    volatile uint32_t ATxDly_p2;            // PHY address: 0x200080  SYS address: 0x800200
    volatile uint32_t RESERVED8[1048575];   // PHY address: 0x100081-0x20007F SYS address: 0x400204-0x8001FC
    volatile uint32_t ATxDly_p3;            // PHY address: 0x300080  SYS address: 0xC00200

} SNPS_DDRPHY_ANIB_TypeDef;

// DWC_DDRPHYA_ANIBj_Pk 0x0000 + j<<12
#define SNPS_DDR_PHY_ANIB0 ((SNPS_DDRPHY_ANIB_TypeDef *) (DRAM_PHY_ANIB0_BASE))
#define SNPS_DDR_PHY_ANIB1 ((SNPS_DDRPHY_ANIB_TypeDef *) (DRAM_PHY_ANIB1_BASE))
#define SNPS_DDR_PHY_ANIB2 ((SNPS_DDRPHY_ANIB_TypeDef *) (DRAM_PHY_ANIB2_BASE))

typedef struct {
    volatile uint16_t DbyteMiscMode:8; // 0x010000		
    volatile uint16_t TsmByte0:8; // 0x010001		
    volatile uint16_t TrainingParam:8; // 0x010002		
    volatile uint16_t UseDqsEnReplica_p0:8; // 0x010003	
    volatile uint16_t RESERVED0[6]; // 0x010004 - 0x01000E
    volatile uint16_t RxTrainPatternEnable:8; // 0x010010		
    volatile uint16_t TsmByte1:8; // 0x010011		
    volatile uint16_t TsmByte2:8; // 0x010012		
    volatile uint16_t TsmByte3:8; // 0x010013		
    volatile uint16_t TsmByte4:8; // 0x010014
    volatile uint16_t RESERVED1:8; // 0x15
    volatile uint16_t RESERVED2:8; // 0x16		
    volatile uint16_t TestModeConfig:8; // 0x010017		
    volatile uint16_t TsmByte5; // 0x010018		
    volatile uint16_t MtestMuxSel; // 0x01001a
    volatile uint16_t RESERVED3; //0x1c
    volatile uint16_t RESERVED4:8; // 0x1e		
    volatile uint16_t DtsmTrainModeCtrl:8; // 0x01001f		
    volatile uint16_t DFIMRL_p0; // 0x010020
    volatile uint16_t RESERVED5; // 0x22	
    volatile uint16_t AsyncDbyteMode; // 0x010024		
    volatile uint16_t AsyncDbyteTxEn; // 0x010026		
    volatile uint16_t AsyncDbyteTxData; // 0x010028		
    volatile uint16_t AsyncDbyteRxData; // 0x01002a	
    volatile uint16_t RESERVED6[2]; // 0x2C-2E	
    volatile uint16_t VrefDAC1_r0; // 0x010030	
    volatile uint16_t TrainingCntr_r0; // 0x010032
    volatile uint16_t RESERVED7[5]; //0x34 - 3E
    volatile uint16_t VrefDAC0_r0:8; // 0x010040	
    volatile uint16_t TxImpedanceCtrl0_b0_p0:8; // 0x010041
    volatile uint16_t RESERVED8:8; //0x42
    volatile uint16_t DqDqsRcvCntrl_b0_p0; // 0x010043
    volatile uint16_t RESERVED9[2]; //0x44 - 0x46

    volatile uint16_t TxEqualizationMode_p0:8; // 0x010048	
    volatile uint16_t TxImpedanceCtrl1_b0_p0:8; // 0x010049
    volatile uint16_t DqDqsRcvCntrl1:8; // 0x01004a		
    volatile uint16_t TxImpedanceCtrl2_b0_p0:8; // 0x01004b
    volatile uint16_t DqDqsRcvCntrl2_p0:8; // 0x01004c	
    volatile uint16_t TxOdtDrvStren_b0_p0:8; // 0x01004d
    volatile uint16_t RESERVED10[4]; // 0x4e - 54

    volatile uint16_t RxFifoCheckStatus:8; // 0x010056		
    volatile uint16_t RxFifoCheckErrValues:8; // 0x010057		
    volatile uint16_t RxFifoInfo:8; // 0x010058		
    volatile uint16_t RxFifoVisibility:8; // 0x010059		
    volatile uint16_t RxFifoContentsDQ3210:8; // 0x01005a		
    volatile uint16_t RxFifoContentsDQ7654:8; // 0x01005b		
    volatile uint16_t RxFifoContentsDBI:8; // 0x01005c	
    volatile uint16_t RESERVED11:8; // 0x5e	
    volatile uint16_t TxSlewRate_b0_p0:8; // 0x01005f
    volatile uint16_t RESERVED12; // 0x60
    volatile uint16_t TrainingIncDecDtsmEn_r0; // 0x010062	
    volatile uint16_t RESERVED13[2]; // 0x64 - 66

    volatile uint16_t RxPBDlyTg0_r0:8; // 0x010068	
    volatile uint16_t RxPBDlyTg1_r0:8; // 0x010069	
    volatile uint16_t RxPBDlyTg2_r0:8; // 0x01006a	
    volatile uint16_t RxPBDlyTg3_r0:8; // 0x01006b	

    volatile uint16_t RESERVED14[10]; // 0x6C - 0x7E
    volatile uint16_t RxEnDlyTg0_u0_p0:8; // 0x010080
    volatile uint16_t RxEnDlyTg1_u0_p0:8; // 0x010081
    volatile uint16_t RxEnDlyTg2_u0_p0:8; // 0x010082
    volatile uint16_t RxEnDlyTg3_u0_p0:8; // 0x010083

    volatile uint16_t RESERVED15[4]; // 0x84 - 0x8A
    volatile uint16_t RxClkDlyTg0_u0_p0:8; // 0x01008c
    volatile uint16_t RxClkDlyTg1_u0_p0:8; // 0x01008d
    volatile uint16_t RxClkDlyTg2_u0_p0:8; // 0x01008e
    volatile uint16_t RxClkDlyTg3_u0_p0:8; // 0x01008f
    volatile uint16_t RxClkcDlyTg0_u0_p0:8; // 0x010090
    volatile uint16_t RxClkcDlyTg1_u0_p0:8; // 0x010091
    volatile uint16_t RxClkcDlyTg2_u0_p0:8; // 0x010092
    volatile uint16_t RxClkcDlyTg3_u0_p0:8; // 0x010093

    volatile uint16_t RESERVED16[6]; //0x094 - 0x9e
   //volatile uint16_t Dq0LnSel:8; // 0x0100a0		
   //volatile uint16_t Dq1LnSel:8; // 0x0100a1		
   //volatile uint16_t Dq2LnSel:8; // 0x0100a2		
   //volatile uint16_t Dq3LnSel:8; // 0x0100a3		
   //volatile uint16_t Dq4LnSel:8; // 0x0100a4		
   //volatile uint16_t Dq5LnSel:8; // 0x0100a5		
   //volatile uint16_t Dq6LnSel:8; // 0x0100a6		
   //volatile uint16_t Dq7LnSel:8; // 0x0100a7	
    volatile uint8_t  DqLnSel[8]; 

    volatile uint16_t RESERVED17; // 0xa8
    volatile uint16_t PptCtlStatic:8; // 0x0100aa		
    volatile uint16_t PptCtlDyn:8; // 0x0100ab		
    volatile uint16_t PptInfo:8; // 0x0100ac		
    volatile uint16_t PptRxEnEvnt:8; // 0x0100ad		
    volatile uint16_t PptDqsCntInvTrnTg0_p0:8; // 0x0100ae	
    volatile uint16_t PptDqsCntInvTrnTg1_p0:8; // 0x0100af	
    volatile uint16_t RESERVED18:8; // 0xb0
    volatile uint16_t DtsmBlankingCtrl:8; // 0x0100b1		
    volatile uint16_t Tsm0_i0:8; // 0x0100b2	
    volatile uint16_t Tsm1_i0:8; // 0x0100b3	
    volatile uint16_t Tsm2_i0:8; // 0x0100b4	
    volatile uint16_t Tsm3:8; // 0x0100b5		
    volatile uint16_t TxChkDataSelects:8; // 0x0100b6		
    volatile uint16_t DtsmUpThldXingInd:8; // 0x0100b7		
    volatile uint16_t DtsmLoThldXingInd:8; // 0x0100b8		
    volatile uint16_t DbyteAllDtsmCtrl0:8; // 0x0100b9		
    volatile uint16_t DbyteAllDtsmCtrl1:8; // 0x0100ba		
    volatile uint16_t DbyteAllDtsmCtrl2:8; // 0x0100bb		

    volatile uint16_t RESERVED19[2]; // 0xbc - 0xbe
    volatile uint16_t TxDqDlyTg0_r0_p0:8; // 0x0100c0
    volatile uint16_t TxDqDlyTg1_r0_p0:8; // 0x0100c1
    volatile uint16_t TxDqDlyTg2_r0_p0:8; // 0x0100c2
    volatile uint16_t TxDqDlyTg3_r0_p0:8; // 0x0100c3

    volatile uint16_t RESERVED20[6]; // 0xc4 - 0xce
    volatile uint16_t TxDqsDlyTg0_u0_p0:8; // 0x0100d0
    volatile uint16_t TxDqsDlyTg1_u0_p0:8; // 0x0100d1
    volatile uint16_t TxDqsDlyTg2_u0_p0:8; // 0x0100d2
    volatile uint16_t TxDqsDlyTg3_u0_p0:8; // 0x0100d3

    volatile uint16_t RESERVED21[8]; //0xd4 - 0xe2
    volatile uint16_t DxLcdlStatus; // 0x0100e4	
    
    volatile uint16_t RESERVED22[37]; // 0xe6 - 0x12E
    volatile uint16_t VrefDAC1_r1; // 0x010130	
    volatile uint16_t TrainingCntr_r1; // 0x010132	

    volatile uint16_t RESERVED23[6]; //0x134 - 13E
    volatile uint16_t VrefDAC0_r1:8; // 0x010140	
    volatile uint16_t TxImpedanceCtrl0_b1_p0:8; // 0x010141
    volatile uint16_t RESERVED24:8; // 0x142
    volatile uint16_t DqDqsRcvCntrl_b1_p0:8; // 0x010143

    volatile uint16_t RESERVED25[2]; // 0x144 - 146
    volatile uint16_t RESERVED26:8; //0x148
    volatile uint16_t TxImpedanceCtrl1_b1_p0:8; // 0x010149
    volatile uint16_t RESERVED27:8; //0x14a
    volatile uint16_t TxImpedanceCtrl2_b1_p0:8; // 0x01014b
    volatile uint16_t RESERVED28:8; //0x14c
    volatile uint16_t TxOdtDrvStren_b1_p0:8; // 0x01014d

    volatile uint16_t RESERVED29[7]; //0x14e - 0x15c
    volatile uint16_t RESERVED30:8;
    volatile uint16_t TxSlewRate_b1_p0; // 0x01015f
    volatile uint16_t TrainingIncDecDtsmEn_r1; // 0x010162	
    volatile uint16_t RxPBDlyTg0_r1; // 0x010168	
    volatile uint16_t RxPBDlyTg1_r1; // 0x010169	
    volatile uint16_t RxPBDlyTg2_r1; // 0x01016a	
    volatile uint16_t RxPBDlyTg3_r1; // 0x01016b	
    volatile uint16_t RxEnDlyTg0_u1_p0; // 0x010180
    volatile uint16_t RxEnDlyTg1_u1_p0; // 0x010181
    volatile uint16_t RxEnDlyTg2_u1_p0; // 0x010182
    volatile uint16_t RxEnDlyTg3_u1_p0; // 0x010183
    volatile uint16_t RxClkDlyTg0_u1_p0; // 0x01018c
    volatile uint16_t RxClkDlyTg1_u1_p0; // 0x01018d
    volatile uint16_t RxClkDlyTg2_u1_p0; // 0x01018e
    volatile uint16_t RxClkDlyTg3_u1_p0; // 0x01018f
    volatile uint16_t RxClkcDlyTg0_u1_p0; // 0x010190
    volatile uint16_t RxClkcDlyTg1_u1_p0; // 0x010191
    volatile uint16_t RxClkcDlyTg2_u1_p0; // 0x010192
    volatile uint16_t RxClkcDlyTg3_u1_p0; // 0x010193
    volatile uint16_t Tsm0_i1; // 0x0101b2	
    volatile uint16_t Tsm1_i1; // 0x0101b3	
    volatile uint16_t Tsm2_i1; // 0x0101b4	
    volatile uint16_t TxDqDlyTg0_r1_p0; // 0x0101c0
    volatile uint16_t TxDqDlyTg1_r1_p0; // 0x0101c1
    volatile uint16_t TxDqDlyTg2_r1_p0; // 0x0101c2
    volatile uint16_t TxDqDlyTg3_r1_p0; // 0x0101c3
    volatile uint16_t TxDqsDlyTg0_u1_p0; // 0x0101d0
    volatile uint16_t TxDqsDlyTg1_u1_p0; // 0x0101d1
    volatile uint16_t TxDqsDlyTg2_u1_p0; // 0x0101d2
    volatile uint16_t TxDqsDlyTg3_u1_p0; // 0x0101d3
    volatile uint16_t VrefDAC1_r2; // 0x010230	
    volatile uint16_t TrainingCntr_r2; // 0x010232	
    volatile uint16_t VrefDAC0_r2; // 0x010240	
    volatile uint16_t TrainingIncDecDtsmEn_r2; // 0x010262	
    volatile uint16_t RxPBDlyTg0_r2; // 0x010268	
    volatile uint16_t RxPBDlyTg1_r2; // 0x010269	
    volatile uint16_t RxPBDlyTg2_r2; // 0x01026a	
    volatile uint16_t RxPBDlyTg3_r2; // 0x01026b	
    volatile uint16_t Tsm0_i2; // 0x0102b2	
    volatile uint16_t Tsm1_i2; // 0x0102b3	
    volatile uint16_t Tsm2_i2; // 0x0102b4	
    volatile uint16_t TxDqDlyTg0_r2_p0; // 0x0102c0
    volatile uint16_t TxDqDlyTg1_r2_p0; // 0x0102c1
    volatile uint16_t TxDqDlyTg2_r2_p0; // 0x0102c2
    volatile uint16_t TxDqDlyTg3_r2_p0; // 0x0102c3
    volatile uint16_t VrefDAC1_r3; // 0x010330	
    volatile uint16_t TrainingCntr_r3; // 0x010332	
    volatile uint16_t VrefDAC0_r3; // 0x010340	
    volatile uint16_t TrainingIncDecDtsmEn_r3; // 0x010362	
    volatile uint16_t RxPBDlyTg0_r3; // 0x010368	
    volatile uint16_t RxPBDlyTg1_r3; // 0x010369	
    volatile uint16_t RxPBDlyTg2_r3; // 0x01036a	
    volatile uint16_t RxPBDlyTg3_r3; // 0x01036b	
    volatile uint16_t Tsm0_i3; // 0x0103b2	
    volatile uint16_t Tsm1_i3; // 0x0103b3	
    volatile uint16_t Tsm2_i3; // 0x0103b4	
    volatile uint16_t TxDqDlyTg0_r3_p0; // 0x0103c0
    volatile uint16_t TxDqDlyTg1_r3_p0; // 0x0103c1
    volatile uint16_t TxDqDlyTg2_r3_p0; // 0x0103c2
    volatile uint16_t TxDqDlyTg3_r3_p0; // 0x0103c3
    volatile uint16_t VrefDAC1_r4; // 0x010430	
    volatile uint16_t TrainingCntr_r4; // 0x010432	
    volatile uint16_t VrefDAC0_r4; // 0x010440	
    volatile uint16_t TrainingIncDecDtsmEn_r4; // 0x010462	
    volatile uint16_t RxPBDlyTg0_r4; // 0x010468	
    volatile uint16_t RxPBDlyTg1_r4; // 0x010469	
    volatile uint16_t RxPBDlyTg2_r4; // 0x01046a	
    volatile uint16_t RxPBDlyTg3_r4; // 0x01046b	
    volatile uint16_t Tsm0_i4; // 0x0104b2	
    volatile uint16_t Tsm1_i4; // 0x0104b3	
    volatile uint16_t Tsm2_i4; // 0x0104b4	
    volatile uint16_t TxDqDlyTg0_r4_p0; // 0x0104c0
    volatile uint16_t TxDqDlyTg1_r4_p0; // 0x0104c1
    volatile uint16_t TxDqDlyTg2_r4_p0; // 0x0104c2
    volatile uint16_t TxDqDlyTg3_r4_p0; // 0x0104c3
    volatile uint16_t VrefDAC1_r5; // 0x010530	
    volatile uint16_t TrainingCntr_r5; // 0x010532	
    volatile uint16_t VrefDAC0_r5; // 0x010540	
    volatile uint16_t TrainingIncDecDtsmEn_r5; // 0x010562	
    volatile uint16_t RxPBDlyTg0_r5; // 0x010568	
    volatile uint16_t RxPBDlyTg1_r5; // 0x010569	
    volatile uint16_t RxPBDlyTg2_r5; // 0x01056a	
    volatile uint16_t RxPBDlyTg3_r5; // 0x01056b	
    volatile uint16_t Tsm0_i5; // 0x0105b2	
    volatile uint16_t Tsm1_i5; // 0x0105b3	
    volatile uint16_t Tsm2_i5; // 0x0105b4	
    volatile uint16_t TxDqDlyTg0_r5_p0; // 0x0105c0
    volatile uint16_t TxDqDlyTg1_r5_p0; // 0x0105c1
    volatile uint16_t TxDqDlyTg2_r5_p0; // 0x0105c2
    volatile uint16_t TxDqDlyTg3_r5_p0; // 0x0105c3
    volatile uint16_t VrefDAC1_r6; // 0x010630	
    volatile uint16_t TrainingCntr_r6; // 0x010632	
    volatile uint16_t VrefDAC0_r6; // 0x010640	
    volatile uint16_t TrainingIncDecDtsmEn_r6; // 0x010662	
    volatile uint16_t RxPBDlyTg0_r6; // 0x010668	
    volatile uint16_t RxPBDlyTg1_r6; // 0x010669	
    volatile uint16_t RxPBDlyTg2_r6; // 0x01066a	
    volatile uint16_t RxPBDlyTg3_r6; // 0x01066b	
    volatile uint16_t Tsm0_i6; // 0x0106b2	
    volatile uint16_t Tsm1_i6; // 0x0106b3	
    volatile uint16_t Tsm2_i6; // 0x0106b4	
    volatile uint16_t TxDqDlyTg0_r6_p0; // 0x0106c0
    volatile uint16_t TxDqDlyTg1_r6_p0; // 0x0106c1
    volatile uint16_t TxDqDlyTg2_r6_p0; // 0x0106c2
    volatile uint16_t TxDqDlyTg3_r6_p0; // 0x0106c3
    volatile uint16_t VrefDAC1_r7; // 0x010730	
    volatile uint16_t TrainingCntr_r7; // 0x010732	
    volatile uint16_t VrefDAC0_r7; // 0x010740	
    volatile uint16_t TrainingIncDecDtsmEn_r7; // 0x010762	
    volatile uint16_t RxPBDlyTg0_r7; // 0x010768	
    volatile uint16_t RxPBDlyTg1_r7; // 0x010769	
    volatile uint16_t RxPBDlyTg2_r7; // 0x01076a	
    volatile uint16_t RxPBDlyTg3_r7; // 0x01076b	
    volatile uint16_t Tsm0_i7; // 0x0107b2	
    volatile uint16_t Tsm1_i7; // 0x0107b3	
    volatile uint16_t Tsm2_i7; // 0x0107b4	
    volatile uint16_t TxDqDlyTg0_r7_p0; // 0x0107c0
    volatile uint16_t TxDqDlyTg1_r7_p0; // 0x0107c1
    volatile uint16_t TxDqDlyTg2_r7_p0; // 0x0107c2
    volatile uint16_t TxDqDlyTg3_r7_p0; // 0x0107c3
    volatile uint16_t VrefDAC1_r8; // 0x010830	
    volatile uint16_t TrainingCntr_r8; // 0x010832	
    volatile uint16_t VrefDAC0_r8; // 0x010840	
    volatile uint16_t TrainingIncDecDtsmEn_r8; // 0x010862	
    volatile uint16_t RxPBDlyTg0_r8; // 0x010868	
    volatile uint16_t RxPBDlyTg1_r8; // 0x010869	
    volatile uint16_t RxPBDlyTg2_r8; // 0x01086a	
    volatile uint16_t RxPBDlyTg3_r8; // 0x01086b	
    volatile uint16_t Tsm0_i8; // 0x0108b2	
    volatile uint16_t Tsm1_i8; // 0x0108b3	
    volatile uint16_t Tsm2_i8; // 0x0108b4	
    volatile uint16_t TxDqDlyTg0_r8_p0; // 0x0108c0
    volatile uint16_t TxDqDlyTg1_r8_p0; // 0x0108c1
    volatile uint16_t TxDqDlyTg2_r8_p0; // 0x0108c2
    volatile uint16_t TxDqDlyTg3_r8_p0; // 0x0108c3
    volatile uint16_t UseDqsEnReplica_p1; // 0x110003	
    volatile uint16_t DFIMRL_p1; // 0x110020	
    volatile uint16_t TxImpedanceCtrl0_b0_p1; // 0x110041
    volatile uint16_t DqDqsRcvCntrl_b0_p1; // 0x110043
    volatile uint16_t TxEqualizationMode_p1; // 0x110048	
    volatile uint16_t TxImpedanceCtrl1_b0_p1; // 0x110049
    volatile uint16_t TxImpedanceCtrl2_b0_p1; // 0x11004b
    volatile uint16_t DqDqsRcvCntrl2_p1; // 0x11004c	
    volatile uint16_t TxOdtDrvStren_b0_p1; // 0x11004d
    volatile uint16_t TxSlewRate_b0_p1; // 0x11005f
    volatile uint16_t RxEnDlyTg0_u0_p1; // 0x110080
    volatile uint16_t RxEnDlyTg1_u0_p1; // 0x110081
    volatile uint16_t RxEnDlyTg2_u0_p1; // 0x110082
    volatile uint16_t RxEnDlyTg3_u0_p1; // 0x110083
    volatile uint16_t RxClkDlyTg0_u0_p1; // 0x11008c
    volatile uint16_t RxClkDlyTg1_u0_p1; // 0x11008d
    volatile uint16_t RxClkDlyTg2_u0_p1; // 0x11008e
    volatile uint16_t RxClkDlyTg3_u0_p1; // 0x11008f
    volatile uint16_t RxClkcDlyTg0_u0_p1; // 0x110090
    volatile uint16_t RxClkcDlyTg1_u0_p1; // 0x110091
    volatile uint16_t RxClkcDlyTg2_u0_p1; // 0x110092
    volatile uint16_t RxClkcDlyTg3_u0_p1; // 0x110093
    volatile uint16_t PptDqsCntInvTrnTg0_p1; // 0x1100ae	
    volatile uint16_t PptDqsCntInvTrnTg1_p1; // 0x1100af	
    volatile uint16_t TxDqDlyTg0_r0_p1; // 0x1100c0
    volatile uint16_t TxDqDlyTg1_r0_p1; // 0x1100c1
    volatile uint16_t TxDqDlyTg2_r0_p1; // 0x1100c2
    volatile uint16_t TxDqDlyTg3_r0_p1; // 0x1100c3
    volatile uint16_t TxDqsDlyTg0_u0_p1; // 0x1100d0
    volatile uint16_t TxDqsDlyTg1_u0_p1; // 0x1100d1
    volatile uint16_t TxDqsDlyTg2_u0_p1; // 0x1100d2
    volatile uint16_t TxDqsDlyTg3_u0_p1; // 0x1100d3
    volatile uint16_t TxImpedanceCtrl0_b1_p1; // 0x110141
    volatile uint16_t DqDqsRcvCntrl_b1_p1; // 0x110143
    volatile uint16_t TxImpedanceCtrl1_b1_p1; // 0x110149
    volatile uint16_t TxImpedanceCtrl2_b1_p1; // 0x11014b
    volatile uint16_t TxOdtDrvStren_b1_p1; // 0x11014d
    volatile uint16_t TxSlewRate_b1_p1; // 0x11015f
    volatile uint16_t RxEnDlyTg0_u1_p1; // 0x110180
    volatile uint16_t RxEnDlyTg1_u1_p1; // 0x110181
    volatile uint16_t RxEnDlyTg2_u1_p1; // 0x110182
    volatile uint16_t RxEnDlyTg3_u1_p1; // 0x110183
    volatile uint16_t RxClkDlyTg0_u1_p1; // 0x11018c
    volatile uint16_t RxClkDlyTg1_u1_p1; // 0x11018d
    volatile uint16_t RxClkDlyTg2_u1_p1; // 0x11018e
    volatile uint16_t RxClkDlyTg3_u1_p1; // 0x11018f
    volatile uint16_t RxClkcDlyTg0_u1_p1; // 0x110190
    volatile uint16_t RxClkcDlyTg1_u1_p1; // 0x110191
    volatile uint16_t RxClkcDlyTg2_u1_p1; // 0x110192
    volatile uint16_t RxClkcDlyTg3_u1_p1; // 0x110193
    volatile uint16_t TxDqDlyTg0_r1_p1; // 0x1101c0
    volatile uint16_t TxDqDlyTg1_r1_p1; // 0x1101c1
    volatile uint16_t TxDqDlyTg2_r1_p1; // 0x1101c2
    volatile uint16_t TxDqDlyTg3_r1_p1; // 0x1101c3
    volatile uint16_t TxDqsDlyTg0_u1_p1; // 0x1101d0
    volatile uint16_t TxDqsDlyTg1_u1_p1; // 0x1101d1
    volatile uint16_t TxDqsDlyTg2_u1_p1; // 0x1101d2
    volatile uint16_t TxDqsDlyTg3_u1_p1; // 0x1101d3
    volatile uint16_t TxDqDlyTg0_r2_p1; // 0x1102c0
    volatile uint16_t TxDqDlyTg1_r2_p1; // 0x1102c1
    volatile uint16_t TxDqDlyTg2_r2_p1; // 0x1102c2
    volatile uint16_t TxDqDlyTg3_r2_p1; // 0x1102c3
    volatile uint16_t TxDqDlyTg0_r3_p1; // 0x1103c0
    volatile uint16_t TxDqDlyTg1_r3_p1; // 0x1103c1
    volatile uint16_t TxDqDlyTg2_r3_p1; // 0x1103c2
    volatile uint16_t TxDqDlyTg3_r3_p1; // 0x1103c3
    volatile uint16_t TxDqDlyTg0_r4_p1; // 0x1104c0
    volatile uint16_t TxDqDlyTg1_r4_p1; // 0x1104c1
    volatile uint16_t TxDqDlyTg2_r4_p1; // 0x1104c2
    volatile uint16_t TxDqDlyTg3_r4_p1; // 0x1104c3
    volatile uint16_t TxDqDlyTg0_r5_p1; // 0x1105c0
    volatile uint16_t TxDqDlyTg1_r5_p1; // 0x1105c1
    volatile uint16_t TxDqDlyTg2_r5_p1; // 0x1105c2
    volatile uint16_t TxDqDlyTg3_r5_p1; // 0x1105c3
    volatile uint16_t TxDqDlyTg0_r6_p1; // 0x1106c0
    volatile uint16_t TxDqDlyTg1_r6_p1; // 0x1106c1
    volatile uint16_t TxDqDlyTg2_r6_p1; // 0x1106c2
    volatile uint16_t TxDqDlyTg3_r6_p1; // 0x1106c3
    volatile uint16_t TxDqDlyTg0_r7_p1; // 0x1107c0
    volatile uint16_t TxDqDlyTg1_r7_p1; // 0x1107c1
    volatile uint16_t TxDqDlyTg2_r7_p1; // 0x1107c2
    volatile uint16_t TxDqDlyTg3_r7_p1; // 0x1107c3
    volatile uint16_t TxDqDlyTg0_r8_p1; // 0x1108c0
    volatile uint16_t TxDqDlyTg1_r8_p1; // 0x1108c1
    volatile uint16_t TxDqDlyTg2_r8_p1; // 0x1108c2
    volatile uint16_t TxDqDlyTg3_r8_p1; // 0x1108c3
    volatile uint16_t UseDqsEnReplica_p2; // 0x210003	
    volatile uint16_t DFIMRL_p2; // 0x210020	
    volatile uint16_t TxImpedanceCtrl0_b0_p2; // 0x210041
    volatile uint16_t DqDqsRcvCntrl_b0_p2; // 0x210043
    volatile uint16_t TxEqualizationMode_p2; // 0x210048	
    volatile uint16_t TxImpedanceCtrl1_b0_p2; // 0x210049
    volatile uint16_t TxImpedanceCtrl2_b0_p2; // 0x21004b
    volatile uint16_t DqDqsRcvCntrl2_p2; // 0x21004c	
    volatile uint16_t TxOdtDrvStren_b0_p2; // 0x21004d
    volatile uint16_t TxSlewRate_b0_p2; // 0x21005f
    volatile uint16_t RxEnDlyTg0_u0_p2; // 0x210080
    volatile uint16_t RxEnDlyTg1_u0_p2; // 0x210081
    volatile uint16_t RxEnDlyTg2_u0_p2; // 0x210082
    volatile uint16_t RxEnDlyTg3_u0_p2; // 0x210083
    volatile uint16_t RxClkDlyTg0_u0_p2; // 0x21008c
    volatile uint16_t RxClkDlyTg1_u0_p2; // 0x21008d
    volatile uint16_t RxClkDlyTg2_u0_p2; // 0x21008e
    volatile uint16_t RxClkDlyTg3_u0_p2; // 0x21008f
    volatile uint16_t RxClkcDlyTg0_u0_p2; // 0x210090
    volatile uint16_t RxClkcDlyTg1_u0_p2; // 0x210091
    volatile uint16_t RxClkcDlyTg2_u0_p2; // 0x210092
    volatile uint16_t RxClkcDlyTg3_u0_p2; // 0x210093
    volatile uint16_t PptDqsCntInvTrnTg0_p2; // 0x2100ae	
    volatile uint16_t PptDqsCntInvTrnTg1_p2; // 0x2100af	
    volatile uint16_t TxDqDlyTg0_r0_p2; // 0x2100c0
    volatile uint16_t TxDqDlyTg1_r0_p2; // 0x2100c1
    volatile uint16_t TxDqDlyTg2_r0_p2; // 0x2100c2
    volatile uint16_t TxDqDlyTg3_r0_p2; // 0x2100c3
    volatile uint16_t TxDqsDlyTg0_u0_p2; // 0x2100d0
    volatile uint16_t TxDqsDlyTg1_u0_p2; // 0x2100d1
    volatile uint16_t TxDqsDlyTg2_u0_p2; // 0x2100d2
    volatile uint16_t TxDqsDlyTg3_u0_p2; // 0x2100d3
    volatile uint16_t TxImpedanceCtrl0_b1_p2; // 0x210141
    volatile uint16_t DqDqsRcvCntrl_b1_p2; // 0x210143
    volatile uint16_t TxImpedanceCtrl1_b1_p2; // 0x210149
    volatile uint16_t TxImpedanceCtrl2_b1_p2; // 0x21014b
    volatile uint16_t TxOdtDrvStren_b1_p2; // 0x21014d
    volatile uint16_t TxSlewRate_b1_p2; // 0x21015f
    volatile uint16_t RxEnDlyTg0_u1_p2; // 0x210180
    volatile uint16_t RxEnDlyTg1_u1_p2; // 0x210181
    volatile uint16_t RxEnDlyTg2_u1_p2; // 0x210182
    volatile uint16_t RxEnDlyTg3_u1_p2; // 0x210183
    volatile uint16_t RxClkDlyTg0_u1_p2; // 0x21018c
    volatile uint16_t RxClkDlyTg1_u1_p2; // 0x21018d
    volatile uint16_t RxClkDlyTg2_u1_p2; // 0x21018e
    volatile uint16_t RxClkDlyTg3_u1_p2; // 0x21018f
    volatile uint16_t RxClkcDlyTg0_u1_p2; // 0x210190
    volatile uint16_t RxClkcDlyTg1_u1_p2; // 0x210191
    volatile uint16_t RxClkcDlyTg2_u1_p2; // 0x210192
    volatile uint16_t RxClkcDlyTg3_u1_p2; // 0x210193
    volatile uint16_t TxDqDlyTg0_r1_p2; // 0x2101c0
    volatile uint16_t TxDqDlyTg1_r1_p2; // 0x2101c1
    volatile uint16_t TxDqDlyTg2_r1_p2; // 0x2101c2
    volatile uint16_t TxDqDlyTg3_r1_p2; // 0x2101c3
    volatile uint16_t TxDqsDlyTg0_u1_p2; // 0x2101d0
    volatile uint16_t TxDqsDlyTg1_u1_p2; // 0x2101d1
    volatile uint16_t TxDqsDlyTg2_u1_p2; // 0x2101d2
    volatile uint16_t TxDqsDlyTg3_u1_p2; // 0x2101d3
    volatile uint16_t TxDqDlyTg0_r2_p2; // 0x2102c0
    volatile uint16_t TxDqDlyTg1_r2_p2; // 0x2102c1
    volatile uint16_t TxDqDlyTg2_r2_p2; // 0x2102c2
    volatile uint16_t TxDqDlyTg3_r2_p2; // 0x2102c3
    volatile uint16_t TxDqDlyTg0_r3_p2; // 0x2103c0
    volatile uint16_t TxDqDlyTg1_r3_p2; // 0x2103c1
    volatile uint16_t TxDqDlyTg2_r3_p2; // 0x2103c2
    volatile uint16_t TxDqDlyTg3_r3_p2; // 0x2103c3
    volatile uint16_t TxDqDlyTg0_r4_p2; // 0x2104c0
    volatile uint16_t TxDqDlyTg1_r4_p2; // 0x2104c1
    volatile uint16_t TxDqDlyTg2_r4_p2; // 0x2104c2
    volatile uint16_t TxDqDlyTg3_r4_p2; // 0x2104c3
    volatile uint16_t TxDqDlyTg0_r5_p2; // 0x2105c0
    volatile uint16_t TxDqDlyTg1_r5_p2; // 0x2105c1
    volatile uint16_t TxDqDlyTg2_r5_p2; // 0x2105c2
    volatile uint16_t TxDqDlyTg3_r5_p2; // 0x2105c3
    volatile uint16_t TxDqDlyTg0_r6_p2; // 0x2106c0
    volatile uint16_t TxDqDlyTg1_r6_p2; // 0x2106c1
    volatile uint16_t TxDqDlyTg2_r6_p2; // 0x2106c2
    volatile uint16_t TxDqDlyTg3_r6_p2; // 0x2106c3
    volatile uint16_t TxDqDlyTg0_r7_p2; // 0x2107c0
    volatile uint16_t TxDqDlyTg1_r7_p2; // 0x2107c1
    volatile uint16_t TxDqDlyTg2_r7_p2; // 0x2107c2
    volatile uint16_t TxDqDlyTg3_r7_p2; // 0x2107c3
    volatile uint16_t TxDqDlyTg0_r8_p2; // 0x2108c0
    volatile uint16_t TxDqDlyTg1_r8_p2; // 0x2108c1
    volatile uint16_t TxDqDlyTg2_r8_p2; // 0x2108c2
    volatile uint16_t TxDqDlyTg3_r8_p2; // 0x2108c3
    volatile uint16_t UseDqsEnReplica_p3; // 0x310003	
    volatile uint16_t DFIMRL_p3; // 0x310020	
    volatile uint16_t TxImpedanceCtrl0_b0_p3; // 0x310041
    volatile uint16_t DqDqsRcvCntrl_b0_p3; // 0x310043
    volatile uint16_t TxEqualizationMode_p3; // 0x310048
    volatile uint16_t TxImpedanceCtrl1_b0_p3; // 0x310049
    volatile uint16_t TxImpedanceCtrl2_b0_p3; // 0x31004b
    volatile uint16_t DqDqsRcvCntrl2_p3; // 0x31004c
    volatile uint16_t TxOdtDrvStren_b0_p3; // 0x31004d
    volatile uint16_t TxSlewRate_b0_p3; // 0x31005f
    volatile uint16_t RxEnDlyTg0_u0_p3; // 0x310080
    volatile uint16_t RxEnDlyTg1_u0_p3; // 0x310081
    volatile uint16_t RxEnDlyTg2_u0_p3; // 0x310082
    volatile uint16_t RxEnDlyTg3_u0_p3; // 0x310083
    volatile uint16_t RxClkDlyTg0_u0_p3; // 0x31008c
    volatile uint16_t RxClkDlyTg1_u0_p3; // 0x31008d
    volatile uint16_t RxClkDlyTg2_u0_p3; // 0x31008e
    volatile uint16_t RxClkDlyTg3_u0_p3; // 0x31008f
    volatile uint16_t RxClkcDlyTg0_u0_p3; // 0x310090
    volatile uint16_t RxClkcDlyTg1_u0_p3; // 0x310091
    volatile uint16_t RxClkcDlyTg2_u0_p3; // 0x310092
    volatile uint16_t RxClkcDlyTg3_u0_p3; // 0x310093
    volatile uint16_t PptDqsCntInvTrnTg0_p3; // 0x3100ae	
    volatile uint16_t PptDqsCntInvTrnTg1_p3; // 0x3100af	
    volatile uint16_t TxDqDlyTg0_r0_p3; // 0x3100c0
    volatile uint16_t TxDqDlyTg1_r0_p3; // 0x3100c1
    volatile uint16_t TxDqDlyTg2_r0_p3; // 0x3100c2
    volatile uint16_t TxDqDlyTg3_r0_p3; // 0x3100c3
    volatile uint16_t TxDqsDlyTg0_u0_p3; // 0x3100d0
    volatile uint16_t TxDqsDlyTg1_u0_p3; // 0x3100d1
    volatile uint16_t TxDqsDlyTg2_u0_p3; // 0x3100d2
    volatile uint16_t TxDqsDlyTg3_u0_p3; // 0x3100d3
    volatile uint16_t TxImpedanceCtrl0_b1_p3; // 0x310141
    volatile uint16_t DqDqsRcvCntrl_b1_p3; // 0x310143
    volatile uint16_t TxImpedanceCtrl1_b1_p3; // 0x310149
    volatile uint16_t TxImpedanceCtrl2_b1_p3; // 0x31014b
    volatile uint16_t TxOdtDrvStren_b1_p3; // 0x31014d
    volatile uint16_t TxSlewRate_b1_p3; // 0x31015f
    volatile uint16_t RxEnDlyTg0_u1_p3; // 0x310180
    volatile uint16_t RxEnDlyTg1_u1_p3; // 0x310181
    volatile uint16_t RxEnDlyTg2_u1_p3; // 0x310182
    volatile uint16_t RxEnDlyTg3_u1_p3; // 0x310183
    volatile uint16_t RxClkDlyTg0_u1_p3; // 0x31018c
    volatile uint16_t RxClkDlyTg1_u1_p3; // 0x31018d
    volatile uint16_t RxClkDlyTg2_u1_p3; // 0x31018e
    volatile uint16_t RxClkDlyTg3_u1_p3; // 0x31018f
    volatile uint16_t RxClkcDlyTg0_u1_p3; // 0x310190
    volatile uint16_t RxClkcDlyTg1_u1_p3; // 0x310191
    volatile uint16_t RxClkcDlyTg2_u1_p3; // 0x310192
    volatile uint16_t RxClkcDlyTg3_u1_p3; // 0x310193
    volatile uint16_t TxDqDlyTg0_r1_p3; // 0x3101c0
    volatile uint16_t TxDqDlyTg1_r1_p3; // 0x3101c1
    volatile uint16_t TxDqDlyTg2_r1_p3; // 0x3101c2
    volatile uint16_t TxDqDlyTg3_r1_p3; // 0x3101c3
    volatile uint16_t TxDqsDlyTg0_u1_p3; // 0x3101d0
    volatile uint16_t TxDqsDlyTg1_u1_p3; // 0x3101d1
    volatile uint16_t TxDqsDlyTg2_u1_p3; // 0x3101d2
    volatile uint16_t TxDqsDlyTg3_u1_p3; // 0x3101d3
    volatile uint16_t TxDqDlyTg0_r2_p3; // 0x3102c0
    volatile uint16_t TxDqDlyTg1_r2_p3; // 0x3102c1
    volatile uint16_t TxDqDlyTg2_r2_p3; // 0x3102c2
    volatile uint16_t TxDqDlyTg3_r2_p3; // 0x3102c3
    volatile uint16_t TxDqDlyTg0_r3_p3; // 0x3103c0
    volatile uint16_t TxDqDlyTg1_r3_p3; // 0x3103c1
    volatile uint16_t TxDqDlyTg2_r3_p3; // 0x3103c2
    volatile uint16_t TxDqDlyTg3_r3_p3; // 0x3103c3
    volatile uint16_t TxDqDlyTg0_r4_p3; // 0x3104c0
    volatile uint16_t TxDqDlyTg1_r4_p3; // 0x3104c1
    volatile uint16_t TxDqDlyTg2_r4_p3; // 0x3104c2
    volatile uint16_t TxDqDlyTg3_r4_p3; // 0x3104c3
    volatile uint16_t TxDqDlyTg0_r5_p3; // 0x3105c0
    volatile uint16_t TxDqDlyTg1_r5_p3; // 0x3105c1
    volatile uint16_t TxDqDlyTg2_r5_p3; // 0x3105c2
    volatile uint16_t TxDqDlyTg3_r5_p3; // 0x3105c3
    volatile uint16_t TxDqDlyTg0_r6_p3; // 0x3106c0
    volatile uint16_t TxDqDlyTg1_r6_p3; // 0x3106c1
    volatile uint16_t TxDqDlyTg2_r6_p3; // 0x3106c2
    volatile uint16_t TxDqDlyTg3_r6_p3; // 0x3106c3
    volatile uint16_t TxDqDlyTg0_r7_p3; // 0x3107c0
    volatile uint16_t TxDqDlyTg1_r7_p3; // 0x3107c1
    volatile uint16_t TxDqDlyTg2_r7_p3; // 0x3107c2
    volatile uint16_t TxDqDlyTg3_r7_p3; // 0x3107c3
    volatile uint16_t TxDqDlyTg0_r8_p3; // 0x3108c0
    volatile uint16_t TxDqDlyTg1_r8_p3; // 0x3108c1
    volatile uint16_t TxDqDlyTg2_r8_p3; // 0x3108c2
    volatile uint16_t TxDqDlyTg3_r8_p3; // 0x3108c3

} SNPS_DDRPHY_DBYTE_TypeDef;

#define SNPS_DDRPHY_DBYTE0 ((SNPS_DDRPHY_DBYTE_TypeDef *) (DRAM_PHY_DBYTE0_BASE))
