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
 * Purpose : IRQ Header File
 *
 * -----------------------------------------------------------------------------
*/

#ifndef __IRQ_H__
#define __IRQ_H__

#ifdef __cplusplus
 extern "C" {
#endif 

/* Function prototype - wfi function */
extern void call_wfi(void);

/* Function prototype - enable the irq flag in cpu */
extern void enable_irq(void);

/* Function prototype - disable the irq flag in cpu */
extern void disable_irq(void);

/* -----------------------------------------------------------------------------
 * End of irq.h
 * -----------------------------------------------------------------------------
 */


#ifdef __cplusplus
}
#endif

#endif /* __IRQ_H__ */

