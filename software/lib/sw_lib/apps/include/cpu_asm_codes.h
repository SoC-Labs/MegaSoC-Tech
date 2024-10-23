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
 *------------------------------------------------------------------------------
 * Purpose : V8/V8.2 CPU ASM Functions Defines
 *
 * -----------------------------------------------------------------------------
*/

#ifndef __CPUINFO_H__
#define __CPUINFO_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include <stdint.h>

void enable_caches(void);
void enable_caches_el1(void);
void invalidate_caches(void);
void invalidateclean_cacheline(uint32_t *ptr);
uint32_t get_cpu_core_number(void);
void spin_lock_init(volatile unsigned short *mutex);
void spin_lock_s(volatile unsigned short *mutex);
void spin_unlock_s(volatile unsigned short *mutex);
void power_down_cpu_core(void);
void enable_smp(void);
void cpu_ret_control(uint32_t val);
void l2_retention(void);
void cpu_warm_reset_AArch64(uint32_t);
uint32_t get_cluster_id(void);
uint32_t get_cpu_thread_number(void);
void cluster_pwr_cntrl(uint32_t val);
void cluster_pwr_down(uint32_t val);
void switch_el1_ns(uint32_t stack_addr, uint32_t el1_exc_addr);
void tlb_invalidate_by_va(uint32_t *ptr);
uint32_t get_core_id(void);

#ifdef __cplusplus
}
#endif

#endif // __CPUINFO_H__
