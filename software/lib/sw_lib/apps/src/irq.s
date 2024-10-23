//; ------------------------------------------------------------------------------
//; The confidential and proprietary information contained in this file may
//; only be used by a person authorised under and to the extent permitted
//; by a subsisting licensing agreement from Arm Limited or its affiliates.
//;
//;        (C) COPYRIGHT 2018-2021 Arm Limited or its affiliates.
//;            ALL RIGHTS RESERVED
//;
//; This entire notice must be reproduced on all copies of this file
//; and copies of this file may only be made by a person if such person is
//; permitted to do so under the terms of a subsisting license agreement
//; from Arm Limited or its affiliates.
//;
//;      Release Information : SSE710-r0p0-00rel0
//;
//; ------------------------------------------------------------------------------
//; Purpose : Low level helper functions for Exception handling
//;
//; -----------------------------------------------------------------------------

//    ; Lowlevel routines systembench environment
     .section irq_helper
     .text
     .align 8

     .global  call_wfi
     .global  enable_irq
     .global  enable_fiq
     .global  disable_irq

     .type call_wfi, @function
call_wfi:
    WFI
    RET
    

     .type enable_irq, @function
enable_irq:
    MSR     DAIFCLR, #0x2
    RET

    .type enable_fiq, @function
enable_fiq:
    MSR     DAIFCLR, #0x1
    RET


   .type disable_irq, @function
disable_irq:  
    MSR     DAIFSET, #0x2
    RET


