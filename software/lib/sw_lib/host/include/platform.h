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

#ifndef __PLATFORM__
#define __PLATFORM__
#include <arm_acle.h>
void sync_ext_abort_handler(void);

void invalidate_l2_caches(void);
void disable_smp(void);
void disable_caches(void);
void enable_icache(void);
uint32_t get_sctlr_val(void);
void write_sctlr_val(uint32_t sctlr_val);
void clear_OS_LOCK(void);
#endif


