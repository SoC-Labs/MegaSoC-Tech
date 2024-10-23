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
// Purpose : Exception vector  
// -----------------------------------------------------------------------------

    .section  VECTORS
    .text
    .balign  2048

    .global monitor_vectors


// PADDING 
   .macro pad_zero from=0, to=16
   .word 0
   .if \to-\from
    pad_zero "(\from+1)",\to
   .endif
   .endm

    .macro SAVE_X1_X30
    STP     x29, x30, [sp, #-0x10]!
    STP     x27, x28, [sp, #-0x10]!
    STP     x25, x26, [sp, #-0x10]!
    STP     x23, x24, [sp, #-0x10]!
    STP     x21, x22, [sp, #-0x10]!
    STP     x19, x20, [sp, #-0x10]!
    STP     x17, x18, [sp, #-0x10]!
    STP     x15, x16, [sp, #-0x10]!
    STP     x13, x14, [sp, #-0x10]!
    STP     x11, x12, [sp, #-0x10]!
    STP     x9,  x10, [sp, #-0x10]!
    STP     x7,  x8,  [sp, #-0x10]!
    STP     x5,  x6,  [sp, #-0x10]!
    STP     x3,  x4,  [sp, #-0x10]!
    STP     x1,  x2,  [sp, #-0x10]!
    .endm
    
    .macro RESTORE_X1_X30
    LDP     x1, x2, [sp], #0x10
    LDP     x3, x4, [sp], #0x10
    LDP     x5, x6, [sp], #0x10
    LDP     x7, x8, [sp], #0x10
    LDP     x9, x10, [sp], #0x10
    LDP     x11, x12, [sp], #0x10
    LDP     x13, x14, [sp], #0x10
    LDP     x15, x16, [sp], #0x10
    LDP     x17, x18, [sp], #0x10
    LDP     x19, x20, [sp], #0x10
    LDP     x21, x22, [sp], #0x10
    LDP     x23, x24, [sp], #0x10
    LDP     x25, x26, [sp], #0x10
    LDP     x27, x28, [sp], #0x10
    LDP     x29, x30, [sp], #0x10
    .endm

 //==================================================================
// EL3 VECTOR TABLE
//==================================================================

monitor_vectors :
        B .                           // Current EL 32bits: Synchronous
        .balign 128
        B .                           //IRQ/vIRQ
        .balign 128
        B .                           //FIQ/vFIQ
        .balign 128
        B .      //                    //Error/vError
        .balign 128



        //B .                          // Current EL 64bits: Synchronous
        b sync_abort_handler
        .balign 128
        B el1_irq_handler            // IRQ/vIRQ
        .balign 128
        B el1_irq_handler            //FIQ/vFIQ
        .balign 128
        B sync_abort_handler       //Error/vError

        .balign 128
        B .                          //Lower EL SPx:      //Synchronous
        .balign 128
        B el1_irq_handler            //IRQ/vIRQ
        .balign 128
        B .                          //FIQ/vFIQ
        .balign 128
        B .                          //Error/vError

        .balign 128
        B .                          //Lower EL SP0:      Synchronous
        .balign 128
        B .                          //IRQ/vIRQ
        .balign 128
        B .                          //FIQ/vFIQ
        .balign 128
        B .                          //Error/vError


//
       .type el1_irq_handler, @function
el1_irq_handler:

    SAVE_X1_X30

    mrs     x1, sp_el0
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    
    // save x0 and stack pointer
    stp     x1, x0, [sp, #-0x10]! 

    // save elr and spsr
    stp     x2, x3, [sp, #-0x10]!

    bl      irq_handler

    ldp     x0, x1, [sp], #0x10

    msr     elr_el1, x0
    msr     spsr_el1, x1

    ldp     x1, x0, [sp], #0x10
    msr     sp_el0, x1
    
    RESTORE_X1_X30

    eret


sync_abort_handler : 
   SAVE_X1_X30

    mrs     x1, sp_el0
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    
    // save x0 and stack pointer
    stp     x1, x0, [sp, #-0x10]! 

    // save elr and spsr
    stp     x2, x3, [sp, #-0x10]!

    bl      sync_ext_abort_handler

    ldp     x0, x1, [sp], #0x10

    msr     elr_el1, x0
    msr     spsr_el1, x1

    ldp     x1, x0, [sp], #0x10
    msr     sp_el0, x1

    mrs     x1, elr_el3
    add     x1, x1, #0x4
    msr     elr_el3, x1
   
    RESTORE_X1_X30

    eret
    

//  Padding 
    pad_zero 0,16
    pad_zero 0,16
