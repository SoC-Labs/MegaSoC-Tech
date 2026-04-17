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

#ifndef __SYSTEM_LEVEL_FUNCTIONS_H__
#define __SYSTEM_LEVEL_FUNCTIONS_H__

// Every "register" should have a separate memory line
// 64 bit aligned wait offsets for Secure Enclave
#define CPUSYNC_SECENC_SE_OFFSET      0x0
#define CPUSYNC_SECENC_ES0_OFFSET     0x8
#define CPUSYNC_SECENC_ES1_OFFSET     0x10
#define CPUSYNC_SECENC_HS0_OFFSET     0x18
// 64 bit aligned wait offsets for Secure Enclave
#define CPUSYNC_EXTSYS0_SE_OFFSET     0x20
#define CPUSYNC_EXTSYS0_ES0_OFFSET    0x28
#define CPUSYNC_EXTSYS0_ES1_OFFSET    0x30
#define CPUSYNC_EXTSYS0_HS0_OFFSET    0x38
// 64 bit aligned wait offsets for Secure Enclave
#define CPUSYNC_EXTSYS1_SE_OFFSET     0x40
#define CPUSYNC_EXTSYS1_ES0_OFFSET    0x48
#define CPUSYNC_EXTSYS1_ES1_OFFSET    0x50
#define CPUSYNC_EXTSYS1_HS0_OFFSET    0x58
// 64 bit aligned wait offsets for Secure Enclave
#define CPUSYNC_HOST0_SE_OFFSET       0x60
#define CPUSYNC_HOST0_ES0_OFFSET      0x68
#define CPUSYNC_HOST0_ES1_OFFSET      0x70
#define CPUSYNC_HOST0_HS0_OFFSET      0x78

//Include for Common memory tester functions
#include <stdio.h>
#include <inttypes.h>
#include "stdlib.h"

#include "system.h"
// CPU_CM3 should be defined in the makfile by adding -DSYNC_CPU_CM3_<0,1>, -DSYNC_CPU_CM0, -DSYNC_CPU_HOST to the C flags
#if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1) || defined(SYNC_CPU_CM0)
  #include "mem_map.h"
#endif
#if defined(SYNC_CPU_HOST)
  #include "sys_memory_map.h"
#endif

/////////////////////////////////////
// CPU synchronising functions
/////////////////////////////////////

// Each CPU has its own set of "registers" for each other masters that can wait for it CPUSYNC_*_OFFSET
// Each line has the following structure:
// Self "registers" determines that the functionality activated or not:
//    CPUSYNC_SECENC_SE_OFFSET = 0x1 -> Secure Enclave CPUSync functionality is ACTIVATED
//    CPUSYNC_SECENC_SE_OFFSET = 0x0 -> Secure Enclave CPUSync functionality is DEACTIVATED
// Other "registers" can tell that another master is waiting for the current CPU:
//    CPUSYNC_SECENC_ES0_OFFSET = 0x1 -> External System 0 is waiting for Secure Enclave
//    CPUSYNC_SECENC_ES0_OFFSET = 0x0 -> External System 0 is NOT waiting for Secure Enclave

// Init function to use by the CM0 when after reset
#if defined(SYNC_CPU_CM0)
  void      CPUSync_Init();
#endif

// Wait functions can be called by each CPU to wait for another
// A CPU can not call a wait function to itself (does not exists because of the defines)
// Each CPU registers itself as "currently waiting" at the target CPU's proper "register"
// When the wait is finished the "currently waiting" status is cleared by the CPU it was waiting for
// by calling CPUSync_Done()
#if !defined(SYNC_CPU_CM0)
  void      CPUSync_WaitForSecEnc();
#endif
#if !defined(SYNC_CPU_CM3_0)
  void      CPUSync_WaitForExtSys0();
#endif
#if !defined(SYNC_CPU_CM3_1)
  void      CPUSync_WaitForExtSys1();
#endif
#if !defined(SYNC_CPU_HOST)
  void      CPUSync_WaitForHost0();
#endif

// Use this on the CPU which is working to indicate job done
// If the working CPU has finished then it clears all its status "register" to let other waiting CPUs continue
void      CPUSync_Done();

// Activate functionality
// Activate syncing functionality, by default it is active
void      CPUSync_Activate();
// DeActivate syncing functionality, by default it is active
void      CPUSync_DeActivate();

// Status functions
// Get the currently waiting CPUs vector:
// [0] - SECENC  is '1' wating or '0' not
// [1] - EXTSYS0 is '1' wating or '0' not
// [2] - EXTSYS1 is '1' wating or '0' not
// [3] - HOST0   is '1' wating or '0' not
// Self bits are always '0'. It is impossible for a CPU to wait for itself
int      get_CPUSync_SecEncWaitList();
int      get_CPUSync_ExtSys0WaitList();
int      get_CPUSync_ExtSys1WaitList();
int      get_CPUSync_Host0WaitList();

/////////////////////////////////////
// Common memory tester functions
/////////////////////////////////////
#ifndef HW_REG_BYTE
#define HW_REG_BYTE(base,offset)            (*(volatile uint8_t *)(uintptr_t)((base) + (offset)))
#endif
#ifndef HW_REG_WORD
#define HW_REG_WORD(base,offset)            (*(volatile uint32_t *)(uintptr_t)((base) + (offset)))
#endif
#ifndef HW_REG_HALF
#define HW_REG_HALF(base,offset)    (*(volatile unsigned short int *)(uintptr_t)((base) + (offset)))
#endif
uint32_t DataBusWalking(uint32_t BaseAddr, uint32_t Offset);
uint32_t AddressWalking(uint32_t BaseAddr, uint32_t TopAddr,uint32_t StartPos, uint32_t* RbAddr);
uint32_t AddressWalkingP(uint32_t BaseAddr, uint32_t TopAddr, uint32_t BackupAddr, uint32_t Preserve, uint32_t StartPos, uint32_t* RbAddr);
uint32_t UnalignedAccess(uint32_t BaseAddr, uint32_t TopAddr, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr);
uint32_t UnalignedAccessP(uint32_t BaseAddr, uint32_t TopAddr, uint32_t BackupAddr, uint32_t Preserve, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr);
uint32_t WordHalfWordAccess(uint32_t BaseAddr, uint32_t TopAddr, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr);
uint32_t WordHalfWordAccessP(uint32_t BaseAddr, uint32_t TopAddr, uint32_t BackupAddr, uint32_t Preserve, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr);

void system_sw_reset(void); //SW RESET Control helper function

#endif //__SYSTEM_LEVEL_FUNCTIONS_H__




