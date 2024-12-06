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



#define GIC_BASE                            0x01100000UL
#define GIC_DISTRIBUTOR_BASE                0x01101000UL
#define GIC_CPU_INTERFACE_BASE              0x01102000UL
#define GIC_VIRTUAL_INTERFACE_CONTROL_BASE  0x01104000UL
#define GIC_VIRTUAL_CPU_INTERFACE_BASE      0x01106000UL

#define PERIPHERAL_BASE                     0x40000000UL
#define SYS_UART0_BASE                      PERIPHERAL_BASE
#define TIMER0_BASE                         0x40001000UL
#define SYS_USRT0_BASE                      0x40002000UL

#define DAP_DBG_BASE                        0x60000000UL

#define PASS_CODE 0xBEEF1AC0
#define FAIL_CODE 0xDEADBEEF
#define HOST_CXDT_CODE   0xFFFFFFFF
#define INTR_RTR_NUM_SII 96 
#define INTR_RTR_NUM_ICI 4 
#define INTR_RTR_LDE_LEVEL 2
#define NUM_CLUSTERS 1
#define NUM_CORES 1 


// ******** Contains REDEFINES for TOP level defines to use in IL context **********
// ************* Include Intgeration Layer includes always as last *****************
#include "il_mem_map_includes.h"
// *********************************************************************************
