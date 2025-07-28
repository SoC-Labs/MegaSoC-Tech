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
//-----------------------------------------------------------------------------
//Purpose : Multi CPU boot, stack initilization 
//
//-----------------------------------------------------------------------------


// Following function initializes the stack for different cpus in
// multi-cluster, multi-cpu environment 

#include "system_defines.h"
#include "clus_pe_cnt.h"


     .section stack_multi_cpu_init
     .text
     .align 8


     .global  __stack_multi_cpu_init
     .type __stack_multi_cpu_init, @function

#ifndef  MASTER_CLUS_REF
#define MASTER_CLUS_REF 0 
#endif

#ifndef MASTER_CPU_REF
#define MASTER_CPU_REF 0
#endif

#ifndef MASTER_THREAD_REF
#define MASTER_THREAD_REF 0
#endif

#ifndef PE_STACK_SIZE
#define PE_STACK_SIZE 10
#endif

__stack_multi_cpu_init :

#ifdef STAGE2_TRANS
// Init VBAR for EL2

    LDR x0, =monitor_vectors       
    MSR VBAR_EL2, x0             // EL3 sets vector base address

// Enable page table transaltion 2
   LDR     x0, =stage2_pgtbl1
   MSR     VTTBR_EL2, x0    
   MOV     x1, #0x00FF      
   MSR     MAIR_EL2, x1     
   LDR     x1, =0x00102F60  
   MSR     VTCR_EL2, x1      

   ISB  


// Enable VM bit 
   MOV     x0, #0x0
   ORR     x0, x0, #(1 << 31)
   ORR     x0, x0, #0x1
   MSR     HCR_EL2, x0


// EL1 non-secure, EL2 present
    MRS     x0, scr_el3 
    MOV     x1, #0xC01  
    ORR     x0, x0, x1  
    MSR     scr_el3, x0

    bl enable_mmu_el1

// Switch to EL1 
    .global  Image$$ARM_LIB_STACK$$ZI$$Limit  
    LDR     x0, =Image$$ARM_LIB_STACK$$ZI$$Limit
    MSR SP_EL1, x0

    ADR x0, el1_ns_entry
    MSR ELR_EL3, x0 
    MOV x0, #0x5 
    MSR SPSR_EL3, x0 
    ISB 
    ERET 

            

el1_ns_entry:

    bl enable_caches_el1
#endif            


#ifdef __ARMCC_VERSION__
    // Get top of stack from scatter file 
    .global  Image$$ARM_LIB_STACK$$ZI$$Limit  
    LDR     x0, =Image$$ARM_LIB_STACK$$ZI$$Limit
#else
    LDR     x0, =__StackTop
#endif

    /* Get cluster info */
    MRS x1, MPIDR_EL1

    /* Cluster ID = w2 */
    AND w2, w1, #0xFF0000
    LSR w2, w2, #16

    /* Core ID = w3 */
    AND w3, w1, #0xF00
    UBFX w3,w3,#8,#4

    /* Thread ID = w4 */
    AND w4, w1, #0xFF

    CMP w2, #0
    B.EQ CLUS_0

    CMP w2, #1
    B.EQ CLUS_1

    CMP w2, #2
    B.EQ CLUS_2

    CMP w2, #3
    B.EQ CLUS_3


    CMP w2, #4
    B.EQ CLUS_4

    CMP w2, #5
    B.EQ CLUS_5

    CMP w2, #6
    B.EQ CLUS_6

    CMP w2, #7
    B.EQ CLUS_7
   

    CMP w2, #8
    B.EQ CLUS_8

    CMP w2, #9
    B.EQ CLUS_9

    CMP w2, #10
    B.EQ CLUS_10

    CMP w2, #11
    B.EQ CLUS_11


    CMP w2, #12
    B.EQ CLUS_12

    CMP w2, #13
    B.EQ CLUS_13

    CMP w2, #14
    B.EQ CLUS_14

    CMP w2, #15
    B.EQ CLUS_15


    CMP w2, #16
    B.EQ CLUS_16

    CMP w2, #17
    B.EQ CLUS_17

    CMP w2, #18
    B.EQ CLUS_18

    CMP w2, #19
    B.EQ CLUS_19


    CMP w2, #20
    B.EQ CLUS_20

    CMP w2, #21
    B.EQ CLUS_21

    CMP w2, #22
    B.EQ CLUS_22

    CMP w2, #23
    B.EQ CLUS_23


    CMP w2, #24

    B.EQ CLUS_24

    CMP w2, #25
    B.EQ CLUS_25

    CMP w2, #26
    B.EQ CLUS_26

    CMP w2, #27
    B.EQ CLUS_27


    CMP w2, #28
    B.EQ CLUS_28

    CMP w2, #29
    B.EQ CLUS_29

    CMP w2, #30
    B.EQ CLUS_30

    CMP w2, #31
    B.EQ CLUS_31

    CMP w2, #32
    B.EQ CLUS_32

    CMP w2, #33
    B.EQ CLUS_33

    CMP w2, #34
    B.EQ CLUS_34

    CMP w2, #35
    B.EQ CLUS_35


    CMP w2, #36
    B.EQ CLUS_36

    CMP w2, #37
    B.EQ CLUS_37

    CMP w2, #38
    B.EQ CLUS_38

    CMP w2, #39
    B.EQ CLUS_39
   

    CMP w2, #40
    B.EQ CLUS_40

    CMP w2, #41
    B.EQ CLUS_41

    CMP w2, #42
    B.EQ CLUS_42

    CMP w2, #43
    B.EQ CLUS_43


    CMP w2, #44
    B.EQ CLUS_44

    CMP w2, #45
    B.EQ CLUS_45

    CMP w2, #46
    B.EQ CLUS_46

    CMP w2, #47
    B.EQ CLUS_47


    CMP w2, #48
    B.EQ CLUS_48

    CMP w2, #49
    B.EQ CLUS_49

    CMP w2, #50
    B.EQ CLUS_50

    CMP w2, #51
    B.EQ CLUS_51


    CMP w2, #52
    B.EQ CLUS_52

    CMP w2, #53
    B.EQ CLUS_53

    CMP w2, #54
    B.EQ CLUS_54

    CMP w2, #55
    B.EQ CLUS_55


    CMP w2, #56

    B.EQ CLUS_56

    CMP w2, #57
    B.EQ CLUS_57

    CMP w2, #58
    B.EQ CLUS_58

    CMP w2, #59
    B.EQ CLUS_59


    CMP w2, #60
    B.EQ CLUS_60

    CMP w2, #61
    B.EQ CLUS_61

    CMP w2, #62
    B.EQ CLUS_62

    CMP w2, #63
    B.EQ CLUS_63
        

CLUS_0:
     MOV w7, #NUM_PE_CLUS_0
     B CAL_STACK

CLUS_1:
     MOV w7, #NUM_PE_CLUS_1
     B CAL_STACK

CLUS_2:
     MOV w7, #NUM_PE_CLUS_2
     B CAL_STACK

CLUS_3:
     MOV w7, #NUM_PE_CLUS_3
     B CAL_STACK

CLUS_4:
     MOV w7, #NUM_PE_CLUS_4
     B CAL_STACK

CLUS_5:
     MOV w7, #NUM_PE_CLUS_5
     B CAL_STACK

CLUS_6:
     MOV w7, #NUM_PE_CLUS_6
     B CAL_STACK

CLUS_7:
     MOV w7, #NUM_PE_CLUS_7
     B CAL_STACK

CLUS_8:
     MOV w7, #NUM_PE_CLUS_8
     B CAL_STACK

CLUS_9:
     MOV w7, #NUM_PE_CLUS_9
     B CAL_STACK

CLUS_10:
     MOV w7, #NUM_PE_CLUS_10
     B CAL_STACK

CLUS_11:
     MOV w7, #NUM_PE_CLUS_11
     B CAL_STACK

CLUS_12:
     MOV w7, #NUM_PE_CLUS_12
     B CAL_STACK

CLUS_13:
     MOV w7, #NUM_PE_CLUS_13
     B CAL_STACK

CLUS_14:
     MOV w7, #NUM_PE_CLUS_14
     B CAL_STACK

CLUS_15:
     MOV w7, #NUM_PE_CLUS_15
     B CAL_STACK

CLUS_16:
     MOV w7, #NUM_PE_CLUS_16
     B CAL_STACK

CLUS_17:
     MOV w7, #NUM_PE_CLUS_17
     B CAL_STACK

CLUS_18:
     MOV w7, #NUM_PE_CLUS_18
     B CAL_STACK

CLUS_19:
     MOV w7, #NUM_PE_CLUS_19
     B CAL_STACK

CLUS_20:
     MOV w7, #NUM_PE_CLUS_20
     B CAL_STACK

CLUS_21:
     MOV w7, #NUM_PE_CLUS_21
     B CAL_STACK

CLUS_22:
     MOV w7, #NUM_PE_CLUS_22
     B CAL_STACK

CLUS_23:
     MOV w7, #NUM_PE_CLUS_23
     B CAL_STACK


CLUS_24:
     MOV w7, #NUM_PE_CLUS_24
     B CAL_STACK

CLUS_25:
     MOV w7, #NUM_PE_CLUS_25
     B CAL_STACK

CLUS_26:
     MOV w7, #NUM_PE_CLUS_26
     B CAL_STACK

CLUS_27:
     MOV w7, #NUM_PE_CLUS_27
     B CAL_STACK

CLUS_28:
     MOV w7, #NUM_PE_CLUS_28
     B CAL_STACK

CLUS_29:
     MOV w7, #NUM_PE_CLUS_29
     B CAL_STACK

CLUS_30:
     MOV w7, #NUM_PE_CLUS_30
     B CAL_STACK

CLUS_31:
     MOV w7, #NUM_PE_CLUS_31
     B CAL_STACK

CLUS_32:
     MOV w7, #NUM_PE_CLUS_32
     B CAL_STACK

CLUS_33:
     MOV w7, #NUM_PE_CLUS_33
     B CAL_STACK

CLUS_34:
     MOV w7, #NUM_PE_CLUS_34
     B CAL_STACK

CLUS_35:
     MOV w7, #NUM_PE_CLUS_35
     B CAL_STACK

CLUS_36:
     MOV w7, #NUM_PE_CLUS_36
     B CAL_STACK

CLUS_37:
     MOV w7, #NUM_PE_CLUS_37
     B CAL_STACK

CLUS_38:
     MOV w7, #NUM_PE_CLUS_38
     B CAL_STACK

CLUS_39:
     MOV w7, #NUM_PE_CLUS_39
     B CAL_STACK

CLUS_40:
     MOV w7, #NUM_PE_CLUS_40
     B CAL_STACK

CLUS_41:
     MOV w7, #NUM_PE_CLUS_41
     B CAL_STACK

CLUS_42:
     MOV w7, #NUM_PE_CLUS_42
     B CAL_STACK

CLUS_43:
     MOV w7, #NUM_PE_CLUS_43
     B CAL_STACK

CLUS_44:
     MOV w7, #NUM_PE_CLUS_44
     B CAL_STACK

CLUS_45:
     MOV w7, #NUM_PE_CLUS_45
     B CAL_STACK

CLUS_46:
     MOV w7, #NUM_PE_CLUS_46
     B CAL_STACK

CLUS_47:
     MOV w7, #NUM_PE_CLUS_47
     B CAL_STACK

CLUS_48:
     MOV w7, #NUM_PE_CLUS_48
     B CAL_STACK

CLUS_49:
     MOV w7, #NUM_PE_CLUS_49
     B CAL_STACK

CLUS_50:
     MOV w7, #NUM_PE_CLUS_50
     B CAL_STACK

CLUS_51:
     MOV w7, #NUM_PE_CLUS_51
     B CAL_STACK

CLUS_52:
     MOV w7, #NUM_PE_CLUS_52
     B CAL_STACK

CLUS_53:
     MOV w7, #NUM_PE_CLUS_53
     B CAL_STACK

CLUS_54:
     MOV w7, #NUM_PE_CLUS_54
     B CAL_STACK

CLUS_55:
     MOV w7, #NUM_PE_CLUS_55
     B CAL_STACK


CLUS_56:
     MOV w7, #NUM_PE_CLUS_56
     B CAL_STACK

CLUS_57:
     MOV w7, #NUM_PE_CLUS_57
     B CAL_STACK

CLUS_58:
     MOV w7, #NUM_PE_CLUS_58
     B CAL_STACK

CLUS_59:
     MOV w7, #NUM_PE_CLUS_59
     B CAL_STACK

CLUS_60:
     MOV w7, #NUM_PE_CLUS_60
     B CAL_STACK

CLUS_61:
     MOV w7, #NUM_PE_CLUS_61
     B CAL_STACK

CLUS_62:
     MOV w7, #NUM_PE_CLUS_62
     B CAL_STACK

CLUS_63:
     MOV w7, #NUM_PE_CLUS_63
     B CAL_STACK
        
CAL_STACK : 
     /* Multiply Cores NUM * Thread NUM */
     MOV w9, #NUM_THREADS
     MUL w8, w3, w9
   
     /* ADD num threads value */
     ADD w5, w8, w4

     /* Add NUM_PE's till this Cluster */
     ADD w5, w5, w7

     /* Substract that many KB's from STACK BASE */
     SUB x0, x0, x5, LSL #PE_STACK_SIZE

     /* STACK BASE for PE */
     ADD sp, x0, #0x0


     MRS x0, MPIDR_EL1
#ifndef __V8_ARCH__            
     //; Check cluster not zero 
     AND x1, x0, #0xff0000
     LSR x1, x1, #16

     //; Check CPU not zero
     AND x2, x0, #0xf00
     UBFX x2,x2,#8,#4

     //; Check thread not zero 
     AND x3, x0, #0xff 
#else 
     //; Check cluster not zero 
     AND x1, x0, #0xff00
     LSR x1, x1, #8

     //; Check CPU not zero
     AND x2, x0, #0xff
#endif            

     //; Master CPU ref
     CMP x2, #MASTER_CPU_REF
     B.NE slave_cpu_entry

     //; Check Cluster ID
     CMP x1, #MASTER_CLUS_REF
     B.NE slave_cpu_entry

     //; Check thread ID 
     CMP x3, #MASTER_THREAD_REF
     B.NE slave_cpu_entry


// ------------------------------------------------------------------------------
// Master core process
// ----------------------------- 
main_entry:
     //branch to C Library entry 
#ifdef __ARMCC_VERSION__
     BL      __rt_lib_init
#endif
     BL       main
     
                
slave_cpu_entry:         
     //; Enter WFI for slave CPU 
     BL    cpu_test  
     WFI

