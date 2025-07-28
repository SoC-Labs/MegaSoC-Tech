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
// Purpose : v8 cortex-A  class processor 1st stage boot
//
// -----------------------------------------------------------------------------
            
            
            .section SECURE_ROM_BOOT, "ax"
            .balign 8

                        
            .global    Image$$ARM_LIB_STACK$$ZI$$Limit
            .global    __main
            .global   monitor_vectors    
            // .global   __stack_multi_cpu_init 

            .weak monitor_vectors
// ------------------------------------------------------------------------------
// Core initialisation from reset state
// ------------------------------------------------------------------------------
            .global app_bl1_entry
            .type app_bl1_entry, @function



app_bl1_entry:

            MOV     x0,#0x0
            MOV     x1,x0
            MOV     x2,x0
            MOV     x3,x0
            MOV     x4,x0
            MOV     x5,x0
            MOV     x6,x0
            MOV     x7,x0
            MOV     x8,x0
            MOV     x9,x0
            MOV     x10,x0
            MOV     x11,x0
            MOV     x12,x0
            MOV     x13,x0
            MOV     x14,x0
            MOV     x15,x0
            MOV     x16,x0
            MOV     x17,x0
            MOV     x18,x0
            MOV     x19,x0
            MOV     x20,x0
            MOV     x21,x0
            MOV     x22,x0
            MOV     x23,x0
            MOV     x24,x0
            MOV     x25,x0
            MOV     x26,x0
            MOV     x27,x0
            MOV     x28,x0
            MOV     x29,x0
            MOV     x30,x0
            
            MSR     SP_EL0,x0
            MSR     SP_EL1,x0
            MSR     SP_EL2,x0
            MOV     sp,x0
            MSR     ELR_EL1,x0
            MSR     ELR_EL2,x0
            MSR     ELR_EL3,x0
            MSR     SPSR_EL1,x0
            MSR     SPSR_EL2,x0
            MSR     SPSR_EL3,x0


//===================================================================
// Set Vector Base Address Register (VBAR) to point to this application's vector table
//===================================================================
            LDR x0, =monitor_vectors       
            MSR VBAR_EL3, x0             // EL3 sets vector base address
        
//===================================================================
// IRQ/FIQ/External Abort and SError Interrupt Routing taken in Monitor mode
//===================================================================
            MRS     x0, scr_el3
            ORR     x0, x0, #0xe
            MSR     scr_el3, x0
 
            MRS     x1, CPACR_EL1
            ORR     x1, x1, #0xf00000         //// co-pro access for VFP/Neon
            MSR     CPACR_EL1, x1
 
            MRS     x1, CPTR_EL3
            AND     x1, x1, # ~ ( 1 << 10 )   //// clear TFP bit
            MSR     CPTR_EL3, x1

//===================================================================
// Clear the PSTATE.A fpr enabling SError Aborts (Posion Error)   
//===================================================================
            
            MSR     DAIFCLR, #0x4
            ISB
            

//===================================================================
// Enable NEON and initialize the register bank
//===================================================================
            MRS     x0, ID_AA64PFR0_EL1
            SBFX    x5, x0, #16, #4         // Extract the floating-point field

            MOV     x1, #(0x3 << 20)
            MSR     cpacr_el1, x1
            MRS     x1, cptr_el3

            BIC     x1, x1, #(0x1 << 10)      // Ensure that CPTR_EL3.TFP is clear
            MSR     cptr_el3, x1
            ISB     sy
#ifndef NOFP
            FMOV    d0,  xzr
            FMOV    d1,  xzr
            FMOV    d2,  xzr
            FMOV    d3,  xzr
            FMOV    d4,  xzr
            FMOV    d5,  xzr
            FMOV    d6,  xzr
            FMOV    d7,  xzr
            FMOV    d8,  xzr
            FMOV    d9,  xzr
            FMOV    d10, xzr
            FMOV    d11, xzr
            FMOV    d12, xzr
            FMOV    d13, xzr
            FMOV    d14, xzr
            FMOV    d15, xzr
            FMOV    d16, xzr
            FMOV    d17, xzr
            FMOV    d18, xzr
            FMOV    d19, xzr
            FMOV    d20, xzr
            FMOV    d21, xzr
            FMOV    d22, xzr
            FMOV    d23, xzr
            FMOV    d24, xzr
            FMOV    d25, xzr
            FMOV    d26, xzr
            FMOV    d27, xzr
            FMOV    d28, xzr
            FMOV    d29, xzr
            FMOV    d30, xzr
            FMOV    d31, xzr
#endif            
mmu_cache_setup:
            
            bl enable_mmu
            B __stack_multi_cpu_init

            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP
            NOP

