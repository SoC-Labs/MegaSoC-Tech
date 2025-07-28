/*------------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Limited or its affiliates.
 *
 *        (C) COPYRIGHT 2018-2021 Arm Limited or its affiliates.
 *            ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Limited or its affiliates.
 *
 *      Release Information : SSE710-r0p0-00rel0
 *
 * ------------------------------------------------------------------------------*/

/** @file cpu_asm_codes.c
 *  @brief Helper functions to query/configure various features of the ARMv8/8.2 CORE 
 *
 *   These functions are used in various tests to enable MMU/CACHES, invalidate caches, 
 *   power down core, switch core to EL2 and generate exclusive access using spin lock 
 *
 * 
 */

#include <arm_acle.h>
#include "cpu_asm_codes.h"
#include "system.h"
#include "intrinsics.h"

/** @brief synchronous external abort handler, default functionality is TEST_FAIL(), unless overriden by test owner <br>
 *  @return void 
 */

__attribute__((weak)) void sync_ext_abort_handler(void) 
{
  TEST_FAIL();

}

/** @brief Test Execution entry point for all slave CPU's, default functionality is execute __wfe, unless overriden by test owner <br>
 * @return void 
 */

__attribute__((weak)) void cpu_test(void) 
{ 
  c_print_str("&\n");
  while(1) {
    __wfe();
  }
}

/** @brief Identify Main Core ID Register, function reads MIDR register and returns Main Core ID <br>
 * @return uint32_t
 */

uint32_t get_core_id(void)
{
   uint32_t cpu_core_id;
   __ASM volatile ( "MRS %x[output], midr_el1 \n\t"
                  "MOV x1, 0xffff \n\t"
                  "AND %x[output], %x[output], x1 \n\t"        :[output] "=r" (cpu_core_id)::);

   return cpu_core_id;
}

/** @brief Identify Core number, function reads MPIDR register and returns Core number <br>
 * @return uint32_t
 */

uint32_t get_cpu_core_number(void)
{
   uint32_t cpu_core_no;

#ifndef __V8_ARCH__
  __ASM volatile ( "MRS %x[output], mpidr_el1 \n\t"
                 "MOV x1, 0xf00 \n\t"
                 "AND %x[output], %x[output], x1 \n\t"
                 "UBFX %w[output],%w[output],#8,#4\n\t"    :[output] "=r" (cpu_core_no)::);
#else
  __ASM volatile ( "MRS %x[output], mpidr_el1 \n\t"
                 "MOV x1, 0xf \n\t"
                 "AND %x[output], %x[output], x1 \n\t"        :[output] "=r" (cpu_core_no)::);
#endif

  return cpu_core_no;
}

/** @brief cpu thread number, function reads MPIDR register and returns thread number in multi_threaded CPU <br>
 * @return uint32_t 
 */

uint32_t get_cpu_thread_number(void)
{
  uint32_t cpu_thread_no;
  __ASM volatile ( "MRS %x[output], mpidr_el1 \n\t"
                 "MOV x1, 0xff \n\t"
                 "AND %x[output], %x[output], x1 \n\t" :[output] "=r" (cpu_thread_no)::);
  return cpu_thread_no;
}

/** @brief  cluster id, function reads MPIDR register and returns cluster number <br>
 * @return uint32_t
 */

uint32_t get_cluster_id(void)
{
    uint32_t cluster_id;

#ifndef __V8_ARCH__
    __ASM volatile ("mrs %x[output], mpidr_el1                   \n\t"
                  "lsr %x[output], %x[output], #16             \n\t"
                  "and %x[output], %x[output], #0xFF\n\t" : [output] "=r" (cluster_id)::);
#else
    __ASM volatile ("mrs %x[output], mpidr_el1                   \n\t"
                  "lsr %x[output], %x[output], #8             \n\t"
                  "and %x[output], %x[output], #0xFF\n\t" : [output] "=r" (cluster_id)::);
#endif

    return cluster_id;
}

/** @brief  socket id, function reads MPIDR register and returns socket number <br>
 * @return uint32_t
 */

uint32_t get_socket_id(void)
{
    uint64_t socket_id;

    __ASM volatile ("mrs %x[output], mpidr_el1                   \n\t"
                  "lsr %x[output], %x[output], #32             \n\t"
                  "and %x[output], %x[output], #0xFF\n\t" : [output] "=r" (socket_id)::);
    return socket_id;
}

/** @brief  enable_mmu, function to enable_mmu in el3 <br>
 * @return void
 */

void enable_mmu(void)
{
  __ASM volatile ( "ldr     x0, =pgtbl1      \n\t"
                 "msr     TTBR0_EL3, x0    \n\t"
                 "mov     x1, #0x00FF      \n\t"            // Normal & Device memory attributes
                 "msr     mair_el3, x1     \n\t"
                 "ldr     x1, =0x00102F20  \n\t"
                 "msr     tcr_el3, x1      \n\t"
                 "ic      iallu            \n\t"
                 "tlbi    alle3            \n\t"
                 "mrs     x0, sctlr_el3    \n\t"
                 "mov     x1, #0x0001      \n\t"            // Turn on MMU
                 "orr     x0, x0, x1       \n\t"
                 "msr     SCTLR_EL3, x0    \n\t"
                 "isb                      \n\t" :::"x0", "x1");   
}

/** @brief  enable_mmu in el1, function to enable_mmu in el1 <br>
 * @return void
 */

void enable_mmu_el1(void)
{
  __ASM volatile ( "ldr     x0, =pgtbl1      \n\t"
                 "msr     TTBR0_EL1, x0    \n\t"
                 "mov     x1, #0x00FF      \n\t"            // Normal & Device memory attributes
                 "msr     mair_el1, x1     \n\t"
                 "ldr     x1, =0x00102F20  \n\t"
                 "msr     tcr_el1, x1      \n\t"
                 "ic      iallu            \n\t"
                 "mrs     x0, sctlr_el1    \n\t"
                 "mov     x1, #0x0001      \n\t"            // Turn on MMU
                 "orr     x0, x0, x1       \n\t"
                 "msr     SCTLR_EL1, x0    \n\t"
                 "isb                      \n\t" :::"x0", "x1");   
}

/** @brief  enable_caches, function to enable caches <br>
 * @return void
 */

void enable_caches(void)
{
  __ASM volatile ( "ic iallu                \n\t"           //; Invalidate I cache and BTAC
                 "dsb sy                  \n\t"
                 "isb                     \n\t"
                 "mrs     x0, sctlr_el3   \n\t"
                 "mov     x1, #0x1004     \n\t"           //; Turn on caches
                 "orr     x0, x0, x1      \n\t"
                 "msr     SCTLR_EL3, x0   \n\t"
                 "isb                     \n\t"  ::: "x0", "x1");
}

/** @brief  enable_caches_el1, function to enable caches in el1 <br>
 * @return void
 */

void enable_caches_el1(void)
{
  __ASM volatile ( "ic      iallu           \n\t"
                 "dsb     sy              \n\t"
                 "isb                     \n\t"
                 "MRS     x0, SCTLR_EL1   \n\t"
                 "MOV     X1, #0x1004     \n\t"           //; Turn on caches
                 "ORR     x0, x0, x1      \n\t"
                 "MSR     SCTLR_EL1, x0   \n\t"
                 "ISB                     \n\t"  ::: "x0", "x1");
}

/** @brief  instr_cache_line_invalidate, invalidate instruction cache line by address <br>
 * @return void
 */

void instr_cache_line_invalidate(uint32_t *ptr)
{    
  __ASM volatile ( "ic ivau, %x[input]    \n\t"         
                 "dsb sy                  \n\t" ::[input] "r" (ptr):);       
}

/** @brief  invalidateclean_cacheline, invalidate data cache line by address <br>
 * @return void
 */

void invalidateclean_cacheline(uint32_t *ptr)
{    
  __ASM volatile ( "dc civac, %x[input]    \n\t"           //; IVAC = Invalidate, by Virtual Adress, to point of Coherence
                 "dsb sy                  \n\t" ::[input] "r" (ptr):);        //; Dont want no funky order
}

/** @brief  tlb_invalidate_by_va, invalidate TLB by VA <br>
 * @return void
 */

void tlb_invalidate_by_va(uint32_t *ptr)
{    
  __ASM volatile ( "tlbi vaae1is, %x[input]    \n\t"         
                 "dsb sy                     \n\t"       
                 "isb                        \n\t" ::[input] "r" (ptr):);       
}

/** @brief  invalidate_caches, invalidate  Level-1 Data cache <br>
 * @return void
 */

void invalidate_caches(void)
{
  __ASM volatile ( "   mrs     x0, clidr_el1          \n\t"   //; read clidr
                 "   ands    w3, w0, #0x7000000     \n\t"   //; extract loc from clidr
                 "   lsr     w3, w3, #23            \n\t"   //; left align loc bit field
                 "   b.eq    finished               \n\t"   //; if loc is 0, then no need to clean
                 "   mov     w10, #0                \n\t"   //; start clean at cache level 0 (in x10)
                 "loop1:                            \n\t" 
                 "   add     w2, w10, w10, lsr #1   \n\t"   //; work out 3x current cache level
                 "   lsr     w12, w0, w2            \n\t"   //; extract cache type bits from clidr
                 "   and     w12, w12, #7           \n\t"   //; mask of the bits for current cache only
                 "   cmp     w12, #2                \n\t"   //; see what cache we have at this level
                 "   b.lt    skip                   \n\t"   //; skip if no cache, or just i-cache
                 "   msr     csselr_el1, x10        \n\t"   //; select current cache level in cssr
                 "   isb                            \n\t"   //; sync the new context
                 "   mrs     x12, ccsidr_el1        \n\t"   // ; read the new csidr
                 "   and     w2, w12, #7            \n\t"   //; extract the length of the cache lines
                 "   add     w2, w2, #4             \n\t"   //; add 4 (line length offset)
                 "   mov     w6, #0x3ff             \n\t"   //
                 "   ands    w6, w6, w12, lsr #3    \n\t"   //; find maximum number on the way size
                 "   clz     w5, w6                 \n\t"   //; find bit position of way size increment
                 "   mov     w7, #0x7fff            \n\t"
                 "   ands    w7, w7, w12, lsr #13   \n\t"   //; extract max number of the index size
                 "loop2:                            \n\t"  
                 "   mov     w8, w6                 \n\t"   //; create working copy of max way size
                 "loop3:                            \n\t"
                 "   lsl     w9, w8, w5             \n\t" 
                 "   lsl     w13, w7, w2            \n\t"
                 "   orr     w11, w10, w9           \n\t"   //; factor way and cache number into x11
                 "   orr     w11, w11, w13          \n\t"   //; factor index number into x11
                 "   dc      cisw, x11              \n\t"   // ; invalidate by set/way
                 "   subs    w8, w8, #1             \n\t"   //; decrement the way
                 "   b.ge    loop3                  \n\t"
                 "   subs    w7, w7, #1             \n\t"   //; decrement the index
                 "   b.ge    loop2                  \n\t"
                 "skip:                             \n\t"
                 "    //add     w10, w10, #2        \n\t"   //; increment cache number
                 "    //cmp     w3, x10             \n\t"   //
                 "    //b.gt    loop1               \n\t"   //  
                 "finished:                         \n\t"   //
                 "   mov x0, #0                     \n\t"   //
                 "   msr csselr_el1, x0             \n\t"   //
                 "   dsb     sy                     \n\t"   //
                 "   isb                            \n\t" :::"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12");
}

/** @brief  spin_lock_init, initialization of spinlock routine <br>
 * @return void
 */

void spin_lock_init(volatile unsigned short *mutex)
{
  *mutex = 0;
}

/** @brief spin_lock_s, locks the spinlock <br>
 * @return void
 */

void spin_lock_s(volatile unsigned short *mutex)
{
  __ASM volatile ( "    mov     w2, #0x1       \n" 
                 "spin:                      \n\t"
                 "    ldxrh   w1, [%x[input]] \n\t"         //; see what state the lock is in
                 "    cmp     w1, #0x1       \n\t"          //; locked already?
                 "    b.eq    1f             \n\t"
                 "    wfe                    \n\t"          //; wait if it's locked
                 "    b.ne   spin            \n"
                 "1:                         \n\t"
                 "    stxr    w1, w2, [%x[input]]   \n\t"   //; try to lock it, if it's unlocked     
                 "    cbnz    w1, spin       \n\t"          //; did we fail? //; if any failure, loop
                 "    dmb     #0x3           \n\t"::[input] "r" (mutex): "w1", "w2");        //; make sure subsequent accesses appear after the lock
}

/** @brief spin_unlock_s, unlocks the spinlock <br>
 * @return void
 */

void spin_unlock_s(volatile unsigned short *mutex)
{
  __ASM volatile ("dmb     #0x3               \n\t"
                "mov     w1, #0x1           \n\t"
                "strh    w1, [%x[input]]           \n\t"           //; release spin lock
                "dsb     #0x3               \n\t"           //; ensure completion of the store to clear the lock
                "sev                        \n\t" ::[input] "r" (mutex): "w1");           //; unlock any spinlock
}

/** @brief enable_smp, enables SMP bit, only applicable for v8 Cores <br>
 * @return void
 */

void enable_smp(void)
{
#ifdef __V8_ARCH__
  __ASM volatile  ( "mov     x1, #0x40            \n\t" 
                  "mrs     x0, S3_1_c15_c2_1    \n\t"
                  "orr     x0, x0, x1           \n\t"
                  "msr     S3_1_c15_c2_1, x0    \n\t" :::"x0", "x1"); 
#endif
}

/** @brief power_down_cpu_core, Power downs individual cores, Only applicable for v8 Cores <br>
 * @return void
 */

void power_down_cpu_core(void)
{
     
  __ASM volatile ("isb                             \n\t"
                "dsb     sy                      \n\t"
                "mov     x1, #0x40               \n\t"   /* Switch the processor from Symmetric Multiprocessing (SMP) mode to Asymmetric Multiprocessing (AMP) */
                "mrs     x0, s3_1_c15_c2_1       \n\t"
                "bic     x0, x0, x1              \n\t"
                "msr     s3_1_c15_c2_1, x0       \n\t"
                "mov     x1, #0x40               \n\t"  /* Set the DBGOSDLR.DLK bit */
                "mrs     x0, osdlr_el1           \n\t"
                "orr     x0, x0, #0x1            \n\t"
                "msr     osdlr_el1, x0           \n\t"
                "isb                             \n\t"
                "dsb sy                          \n\t"
                "wfi                             \n\t" :::"x0", "x1", "x2");
}

/** @brief cpu_ret_control, configures cpu in retention <br>
 * @return void
 */

void cpu_ret_control(uint32_t val)
{
#ifndef __V8_ARCH__    
  __ASM volatile ("msr    s3_0_c15_c2_7, %x[input] \n\t" ::[input] "r" (val));
#else
    /* CPU Retention control with 512 Architecture Timer ticks */
    __ASM volatile ("mov    x0, #0x7                \n\t"
                  "mrs    x1, s3_1_c15_c2_1       \n\t"
                  "orr    x1, x1, x0              \n\t"
                  "msr    s3_1_c15_c2_1, x1       \n\t" :::"x0", "x1");
#endif  
}

/** @brief l2_retention, enables l2 retention, only applicable for v8 Processor <br>
 * @return void
 */

void l2_retention(void)
{
#ifdef __V8_ARCH__     
  __ASM volatile ("mrs x0, s3_1_c11_c0_3        \n\t"
                "orr x0, x0, #0x7             \n\t"  /* 512 Generic Timer ticks required before retention entry */
                "orr x0, x0, #0x7             \n\t"
                "msr s3_1_c11_c0_3, x0        \n\t" :: :"x0");
#endif  
}

/** @brief cpu_warm_reset_AArch64, generates warm reset <br>
 * @return void
 */

void cpu_warm_reset_AArch64(uint32_t val)
{
  /* Generate WARM reset */
  __ASM volatile ("msr rmr_el3, %x[input]      \n\t"
                "isb                        \n\t"
                "wfi                        \n\t" ::[input] "r" (val):);
}

/** @brief cluster_pwr_cntrl, Cluster Power Control Register for L3 data RAM retention <br>
 * @return void
 */

void cluster_pwr_cntrl(uint32_t val)
{
  /* Generate WARM reset */
  __ASM volatile ("msr s3_0_c15_c3_5, %x[input] \n\t"
                "isb                          \n\t" ::[input] "r" (val):);
}


/** @brief cluster_pwr_down, cluster power down Register <br>
 * @return void
 */

void cluster_pwr_down(uint32_t val)
{
  __ASM volatile ("msr s3_0_c15_c3_6, %x[input] \n\t"
                "isb                          \n\t" ::[input] "r" (val):);
}

/** @brief switch_el1_ns, witch EL1 in non-secure mode <br>
 * @return void
 */

void switch_el1_ns(uint32_t stack_addr, uint32_t el1_exc_addr)
{
        __ASM volatile (
                  "    MRS     x1, scr_el3           \n\t"
                  "    MOV     x2, #0xC01            \n\t"
                  "    ORR     x1, x1, x2            \n\t"
                  "    MSR     scr_el3, x1           \n\t"
                  "    MSR     SP_EL1,  %x[input0]   \n\t"
                  "    MSR     ELR_EL3, %x[input1]   \n\t"
                  "    mov     x0, #0x5              \n\t"
                  "    MSR     SPSR_EL3, x0          \n\t"
                  "    MOV     x0, #0x0              \n\t"
                  "    ORR     x0, x0, #(1 << 31)    \n\t"
                  "    MSR     HCR_EL2, x0           \n\t"
                  "    ISB                           \n\t"
                  "    ERET                          \n\t" ::[input0] "r" (stack_addr), [input1] "r" (el1_exc_addr) : "x0", "x1", "x3");

}



