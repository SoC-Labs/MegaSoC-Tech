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

#include "system.h"
#include "platform.h"
#include "intrinsics.h"

// This function handles the aarch64 exec. state async abort
void sync_ext_abort_handler(void) 
{
  printf("Sync_ext_abort_handler\n");
   abort_handler();
}

// Enable instruction cache in aarch64 state (EL3)
void enable_icaches(void){

  __ASM volatile ( "ic iallu                \n\t"           //; Invalidate I cache and BTAC
                   "dsb sy                  \n\t"
                   "isb                     \n\t"
                   "mrs     x0, sctlr_el3   \n\t"
                   "mov     x1, #0x1000     \n\t"           //; Turn on caches
                   "orr     x0, x0, x1      \n\t"
                   "msr     SCTLR_EL3, x0   \n\t"
                   "isb                     \n\t"  ::: "x0", "x1");

}

// Disable  caches in aarch64 state (EL3)
void disable_caches(void){
 __ASM volatile(
    "mrs     x0, sctlr_el3   \n\t"
    "mov     x1, #0x1004     \n\t"  //; Turn on caches
    "BIC     x0, x0, x1      \n\t"
    "msr     SCTLR_EL3, x0   \n\t"
    "ISB              \n\t"  ::: "x0", "x1");
}

/**-----------------------------------------------------------------------------
 Function name : disable_smp
 Input Parameters  : void
 Return Type : void
 Descritpion : Disables SMP bit, only applicable for v8 Cores
------------------------------------------------------------------------------*/
void disable_smp(void)
{
  __ASM volatile  ( "mov     x1, #0x40            \n\t" 
                    "mrs     x0, S3_1_c15_c2_1    \n\t"
                    "bic     x0, x0, x1           \n\t"
                    "msr     S3_1_c15_c2_1, x0    \n\t" :::"x0", "x1"); 
}

void invalidate_l2_caches(void){
//    invalidate_caches();
//Programmer's Guide for ARMv8-A, 
//Example 11-3 Cleaning to Point of Coherency
__ASM volatile  ( "MRS X0, CLIDR_EL1       \n\t"
                  "AND W3, W0, #0x07000000 \n\t"  // Get 2 x Level of Coherence
                  "LSR W3, W3, #23         \n\t"
                  "CBZ W3, Finished        \n\t"
                  "MOV W10, #0             \n\t"  // W10 = 2 x cache level
                  "MOV W8, #1              \n\t"  // W8 = constant 0b1
                  "Loop1: ADD W2, W10, W10, LSR #1 \n\t"// Calculate 3 x cache level
                  "LSR W1, W0, W2          \n\t"  // extract 3-bit cache type for this level
                  "AND W1, W1, #0x7        \n\t"
                  "CMP W1, #2              \n\t"
                  "BLT Skip               \n\t"  // No data or unified cache at this level
                  "MSR CSSELR_EL1, X10     \n\t"  // Select this cache level
                  "ISB                     \n\t"  // Synchronize change of CSSELR
                  "MRS X1, CCSIDR_EL1      \n\t"  // Read CCSIDR
                  "AND W2, W1, #7          \n\t"  // W2 = log2(linelen)-4
                  "ADD W2, W2, #4          \n\t"  // W2 = log2(linelen)
                  "UBFX W4, W1, #3, #10    \n\t"  // W4 = max way number, right aligned
                  "CLZ W5, W4              \n\t"  /* W5 = 32-log2(ways), bit position of way in DC  operand */
                  "LSL W9, W4, W5          \n\t"  /* W9 = max way number, aligned to position in DC operand */
                  "LSL W16, W8, W5         \n\t"  // W16 = amount to decrement way number per iteration
                  "Loop2: UBFX W7, W1, #13, #15  \n\t"  // W7 = max set number, right aligned
                  "LSL W7, W7, W2          \n\t"  /* W7 = max set number, aligned to position in DC    operand    */
                  "LSL W17, W8, W2         \n\t"  // W17 = amount to decrement set number per iteration
                  "Loop3: ORR W11, W10, W9 \n\t"  // W11 = combine way number and cache number...
                  "ORR W11, W11, W7        \n\t"  // ... and set number for DC operand
                  "DC CSW, X11             \n\t"  // Do data cache clean by set and way
                  "SUBS W7, W7, W17        \n\t"  // Decrement set number
                  "BGE Loop3               \n\t"
                  "SUBS X9, X9, X16        \n\t"  // Decrement way number
                  "BGE Loop2               \n\t"
                  "Skip:  ADD W10, W10, #2 \n\t"  // Increment 2 x cache level
                  "CMP W3, W10             \n\t"
                  "DSB sy                  \n\t"  /* Ensure completion of previous cache maintenance operation */
                  "BGT Loop1               \n\t"
                  "Finished:               \n\t":::"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12");
}


uint32_t get_sctlr_val(void){
  uint32_t sctlr_val;
  __ASM volatile("mrs  %x[output], sctlr_el3\n\t"
                 :[output] "=r" (sctlr_val)::);

  return sctlr_val;
}




void write_sctlr_val(uint32_t sctlr_val){

  __ASM volatile("msr SCTLR_EL3, %x[input] \n\t"
                 "dsb SY                   \n\t"
                 "isb                      \n\t"  ::[input] "r" (sctlr_val):);

}

/**-----------------------------------------------------------------------------
   Function name : clear_OS_LOCK
   Input Parameters  : void
   Return Type : void
   Descritpion : Set OS lock to 0
   ------------------------------------------------------------------------------*/
void clear_OS_LOCK(void){

  __ASM volatile  ( "MOV     x0, 0x0 \n\t"
                    "MSR     OSLAR_EL1, x0    \n\t" ::: "x0");

}

