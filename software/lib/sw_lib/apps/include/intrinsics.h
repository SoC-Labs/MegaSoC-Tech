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
 * Purpose : INTRINSICS for gcc and RCVT 
 *
 * -----------------------------------------------------------------------------
*/

#ifndef _INTRINSICS_H_
#define _INTRINSICS_H_

/* GCC Specific Intrinsics */
#ifdef __GCC_COMPILER__


#define __nop() __asm__ __volatile__ ( "    nop\n" )

#define __dsb(TYPE) __asm__ __volatile__ ("    dsb %0" \
                                        : \
                                        : "I" (TYPE) )

#define __dmb(TYPE) __asm__ __volatile__ ("    dmb %0" \
                                        : \
                                        : "I" (TYPE) )

#define __isb(TYPE) __asm__ __volatile__ ("    isb %0" \
                                        : \
                                        : "I" (TYPE) )


#define __sev()  __asm__ __volatile__ ( "    sev\n" )
#define __sevl() __asm__ __volatile__ ( "    sevl\n" )

#define __ASM __asm__

#else 

/* RVCT Specific Intrinsics */
#define __ASM asm

#endif


#endif
