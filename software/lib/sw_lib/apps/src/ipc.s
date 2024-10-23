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
//------------------------------------------------------------------------------
// Purpose : Lowlevel routines for Inter-Processor communication
// -----------------------------------------------------------------------------
     .section helpers_mp
     .text
     .align 8


     .global wait_for_event
     .global call_wfe
     .global call_nop
     .global send_event
   
     .global instr_sync_barrier
     .global data_sync_barrier
    
     .type wait_for_event, @function
wait_for_event :     
         WFE
         RET

     .type call_wfe, @function
call_wfe :     
         WFE
         RET

     .type  call_nop, @function
call_nop :       
         NOP
         RET
 
     .type send_event, @function 
send_event :
         SEV
         RET

     
       .type instr_sync_barrier, @function
instr_sync_barrier :
         ISB SY
         RET

        .type data_sync_barrier, @function
data_sync_barrier :
         DSB SY
         RET
