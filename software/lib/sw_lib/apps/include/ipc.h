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
 * Purpose : Common Header File
 *
 * -----------------------------------------------------------------------------
*/

#ifndef __IPC_H__
#define __IPC_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Function prototype - get cpu id */
extern unsigned get_cpuid(void);

/* Function prototype - instruction synchronization barrier */
extern void instr_sync_barrier(void);

/* Function prototype - data synchronization barrier */
extern void data_sync_barrier(void);

/* Function prototype - checks for uniprocessor system  */
extern int is_uniprocessor(void);

/* Function prototype - wait for event */
extern void wait_for_event(void);

/* Function prototype - wait for event */
extern void call_wfe(void);

/* Function prototype - send event  */
extern void send_event(void);

/* Function prototype - call nop  */
extern void call_nop(void);

/* -----------------------------------------------------------------------------
 * End of ipc.h
 * -----------------------------------------------------------------------------
 */


#ifdef __cplusplus
}
#endif


#endif /* __IPC_H__ */

