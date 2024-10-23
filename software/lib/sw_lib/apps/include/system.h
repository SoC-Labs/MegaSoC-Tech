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
 * Purpose : Application processor system peripheral base, irq and common functions defines
 *
 * -----------------------------------------------------------------------------
 */

#ifndef __SYSTEM_APP_H__ 
#define __SYSTEM_APP_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include <arm_acle.h>
#include "irq.h"
#include "ipc.h"
#include "cpu_asm_codes.h"

/* Simple IO macro 
 *  Given a interger type `base' pointer and `offset', the HW_REG returns de-
 *  referenced pointer. Note that since `base' is 4byte size 32bit system, the   
 *  `offset' needs to be divided by 4 to give correct increment
 */ 
#define HW_REG(base,offset) *(((volatile unsigned int *)((unsigned long long)base)) + (offset >> 2))
#define MEM_RW(base,offset) *(((volatile unsigned int *)((unsigned long long)base)) + (offset >> 2))

#define __WFI() call_wfi()
#define __wfi() call_wfi()

#define __WFE() call_wfe()
#define __wfe() call_wfe()

extern void abort_handler(void);

void TEST_PASS(void);
void TEST_FAIL(void);

int c_print_str(const char * fmt);
void c_print_char(const char ch);

int c_print(const char * fmt, ...);

int access_addr_wdata(unsigned long int base_address, unsigned int num_accesses, unsigned int write_data);


/* Fast Printf */
#define printf c_print

/* -----------------------------------------------------------------------------
 * End of system.h
 * -----------------------------------------------------------------------------
 */
 
#ifdef __cplusplus
}
#endif

#endif  // __SYSTEM_APP_H__
