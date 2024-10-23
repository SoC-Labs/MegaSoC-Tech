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

#ifndef __SYSTEM_LEVEL_FUNCTIONS_C__
#define __SYSTEM_LEVEL_FUNCTIONS_C__

#include "system_level_functions.h"

#if defined(SYNC_CPU_CM0)
  void CPUSync_Init() {
    for(int i=0x0; i < 0x80; i+=0x4){
      MEM_RW(CPUSYNC_BASE,i) = 0x0;
    }
  }
#endif

#if !defined(SYNC_CPU_CM0)
  void CPUSync_WaitForSecEnc() {
    // wait only if the deactive bit is not set
    if((MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_SE_OFFSET) & 0x1) == 0x0) {
      // first register the current waiting CPU in its proper location
      #if defined(SYNC_CPU_CM0)
        // Not allowed to call for itself
      #elif defined(SYNC_CPU_CM3_0)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES0_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES0_OFFSET) & 0x1)   != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_SE_OFFSET)  & 0x1)   != 0x1)
              ) {}
      #elif defined(SYNC_CPU_CM3_1)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES1_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES1_OFFSET) & 0x1)   != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_SE_OFFSET)  & 0x1)   != 0x1)
              ) {}
      #elif defined (SYNC_CPU_HOST)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_HS0_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_HS0_OFFSET) & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_SE_OFFSET)  & 0x1) != 0x1)
              ) {}
      #endif
    }
  }
#endif

#if !defined(SYNC_CPU_CM3_0)
  void CPUSync_WaitForExtSys0() {
    // wait only if the deactive bit is not set
    if((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES0_OFFSET) & 0x1) == 0x0) {
      // first register the current waiting CPU in its proper location
      #if defined(SYNC_CPU_CM0)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_SE_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_SE_OFFSET)  & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES0_OFFSET) & 0x1) != 0x1)
              ) {}
      #elif defined(SYNC_CPU_CM3_0)
        // Not allowed to call for itself
      #elif defined(SYNC_CPU_CM3_1)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES1_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES1_OFFSET) & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES0_OFFSET) & 0x1) != 0x1)
              ) {}
      #elif defined (SYNC_CPU_HOST)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_HS0_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_HS0_OFFSET) & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES0_OFFSET) & 0x1) != 0x1)
              ){}
      #endif
    }
  }
#endif

#if !defined(SYNC_CPU_CM3_1)
  void CPUSync_WaitForExtSys1() {
    // wait only if the deactive bit is not set
    if((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES1_OFFSET) & 0x1) == 0x0) {
      // first register the current waiting CPU in its proper location
      #if defined(SYNC_CPU_CM0)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_SE_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_SE_OFFSET)  & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES1_OFFSET) & 0x1) != 0x1)
              ) {}
      #elif defined(SYNC_CPU_CM3_0)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES0_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES0_OFFSET) & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES1_OFFSET) & 0x1) != 0x1)
              ) {}
      #elif defined(SYNC_CPU_CM3_1)
        // Not allowed to call for itself
      #elif defined (SYNC_CPU_HOST)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_HS0_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_HS0_OFFSET) & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES1_OFFSET) & 0x1) != 0x1)
              ) {}
      #endif
    }
  }
#endif

#if !defined(SYNC_CPU_HOST)
  void CPUSync_WaitForHost0() {
    // wait only if the deactive bit is not set
    if((MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_HS0_OFFSET) & 0x1) == 0x0) {
      // first register the current waiting CPU in its proper location
      #if defined(SYNC_CPU_CM0)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_SE_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_SE_OFFSET)  & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_HS0_OFFSET) & 0x1) != 0x1)
              ) {}
      #elif defined(SYNC_CPU_CM3_0)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES0_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES0_OFFSET) & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_HS0_OFFSET) & 0x1) != 0x1)
              ) {}
      #elif defined(SYNC_CPU_CM3_1)
        MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES1_OFFSET) = 0x1;
        // wait until it is finished or deactivated
        while (
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES1_OFFSET) & 0x1) != 0x0)  &&
                ((MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_HS0_OFFSET) & 0x1) != 0x1)
              ) {}
      #elif defined (SYNC_CPU_HOST)
        // Not allowed to call for itself
      #endif
    }
  }
#endif

// Clear all pending wait "register" for the actual CPU
void CPUSync_Done() {
#if defined(SYNC_CPU_CM0)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES0_OFFSET)    = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES1_OFFSET)    = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_HS0_OFFSET)    = 0x0;
#elif defined(SYNC_CPU_CM3_0)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_SE_OFFSET)    = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES1_OFFSET)   = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_HS0_OFFSET)   = 0x0;
#elif defined(SYNC_CPU_CM3_1)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_SE_OFFSET)    = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES0_OFFSET)   = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_HS0_OFFSET)   = 0x0;
#elif defined(SYNC_CPU_HOST)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_SE_OFFSET)      = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES0_OFFSET)     = 0x0;
  MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES1_OFFSET)     = 0x0;
#else
  // undefined CPU
#endif
}

// To deactivate write 0x1 to the self "register"
void CPUSync_DeActivate() {
#if defined(SYNC_CPU_CM0)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_SE_OFFSET)   = 0x1;
#elif defined(SYNC_CPU_CM3_0)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES0_OFFSET) = 0x1;
#elif defined(SYNC_CPU_CM3_1)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES1_OFFSET) = 0x1;
#elif defined(SYNC_CPU_HOST)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_HS0_OFFSET)   = 0x1;
#else
  // undefined CPU
#endif
}

// To activate write 0x0 to the self "register" (by default it is activated)
void CPUSync_Activate() {
#if defined(SYNC_CPU_CM0)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_SE_OFFSET)   = 0x0;
#elif defined(SYNC_CPU_CM3_0)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES0_OFFSET) = 0x0;
#elif defined(SYNC_CPU_CM3_1)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES1_OFFSET) = 0x0;
#elif defined(SYNC_CPU_HOST)
  MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_HS0_OFFSET)   = 0x0;
#else
  // undefined CPU
#endif
}

// Get the currently waiting CPUs vector:
// [0] - SECENC  is '1' wating or '0' not
// [1] - EXTSYS0 is '1' wating or '0' not
// [2] - EXTSYS1 is '1' wating or '0' not
// [3] - HOST0   is '1' wating or '0' not
// Self bits are always '0'. It is impossible for a CPU to wait for itself
int      get_CPUSync_SecEncWaitList() {
  return (
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES0_OFFSET) << 1) |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_ES1_OFFSET) << 2) |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_SECENC_HS0_OFFSET) << 3)
         );
}
int      get_CPUSync_ExtSys0WaitList() {
  return (
           MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_SE_OFFSET)        |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_ES1_OFFSET) << 2) |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS0_HS0_OFFSET) << 3)
         );
}
int      get_CPUSync_ExtSys1WaitList() {
  return (
           MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_SE_OFFSET)        |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_ES0_OFFSET) << 1) |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_EXTSYS1_HS0_OFFSET) << 3)
         );
}
int      get_CPUSync_Host0WaitList() {
  return (
           MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_SE_OFFSET)        |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES0_OFFSET) << 1) |
          (MEM_RW(CPUSYNC_BASE,CPUSYNC_HOST0_ES1_OFFSET) << 2)
         );
}

// -----------------------------------------------------------------------------
// Common memory tester functions **********************************************
// -----------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Data Bus Walking, write same address with walking 1's data
//--------------------------------------------------------------------------
uint32_t DataBusWalking(uint32_t BaseAddr, uint32_t Offset) {
  uint32_t pattern = 0x0;
  uint32_t rd_word = 0x0;
  uint32_t ErrCnt  = 0;
  int i            = 0;

  #ifdef TEST_VERBOSITY_HIGH
  c_print("DataBusWalking at Addr=0x%08x\n", BaseAddr + Offset);
  #endif

  for (i=0; i<32; i++) {
    pattern = (1<<i);
    HW_REG_WORD(BaseAddr, Offset) = pattern;
    //c_print("wr(addr=0x%08x)=0x%08x (1 << %2d) \n", BaseAddr + Offset, pattern, i);
    rd_word = HW_REG_WORD(BaseAddr, Offset);
    if (rd_word != pattern) {
      c_print("ERROR: read value not equal write pattern at Addr 0x%08x\n", BaseAddr + Offset);
      ErrCnt++;
    }
  }
  //reset memory content
  HW_REG_WORD(BaseAddr, Offset) = 0x0;

  return ErrCnt;
}


// -------------------------------------------------
// Function for walking ones across address bus (also checks start and end address)
// All writes followed by all reads
// -------------------------------------------------
uint32_t AddressWalking(uint32_t BaseAddr, uint32_t TopAddr, uint32_t StartPos, uint32_t* RbAddr) {
    uint32_t ErrCnt = 0;
    uint32_t i = 0;
    uint8_t Data_8 = 0;
    uint32_t Data_32 = 0;
    uint32_t Actual, addr, addr_actual;
    uint32_t SEED;

    // Generate random seed with SV $urand function:

#if (defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)) && !defined(NO_TBOX)
    TRICKBOX_IL->RND_MAX = 0xFFFF;
    SEED = TRICKBOX_IL->RND_VAL;
    srand(SEED);
#endif

    // Write data to memory. Walk 1 across address bus
    c_print("Walking 1s on addr bus. Range 0x%08x - 0x%08x\n",BaseAddr,TopAddr);

    //check start address
    c_print("Check start address.\n");
    Data_8 = (rand()%0x00FF);
    //c_print("Wr 0x%08x to 0x%08x \n", Data_8, (BaseAddr));
    addr = (BaseAddr);
    HW_REG_BYTE(addr, 0) = Data_8;
    //c_print("Rd 0x%08x from 0x%08x \n",Data_8, (BaseAddr));
    Actual = HW_REG_BYTE(addr, 0);
    if(Actual != Data_8) {
        c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr), Data_8, Actual);
        ErrCnt++;
    }
#if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
    //read back address
    if(RbAddr != NULL)
    {
      addr_actual = *(RbAddr);
      if(addr_actual != addr){
        c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
        ErrCnt++;
        }
    }
#endif

    c_print("Moving 1s.\n");
    for(i=(1<<StartPos); ((BaseAddr|i)+4)<TopAddr; i=(i<<1)) {


        if(0x4>i)
        {
              Data_8 = (rand()%0x00FF);
              //c_print("Wr 0x%08x to 0x%08x \n", Data_8, (BaseAddr|i));
              addr = (BaseAddr|i);
              HW_REG_BYTE(addr, 0) = Data_8;
              //c_print("Rd 0x%08x from 0x%08x \n",Data_8, (BaseAddr|i));
              Actual = HW_REG_BYTE(addr, 0);
              if(Actual != Data_8) {
                  c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr|i), Data_8, Actual);
                  ErrCnt++;
              }
              #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
               //read back address
               if(RbAddr != NULL)
               {
                 addr_actual = *(RbAddr);
                 if(addr_actual != addr){
                   c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
                   ErrCnt++;
                   }
               }
              #endif
        }
        else {
              Data_32 = (rand()%0xFFFF)|(rand()%0xFFFF)<<16;
              //c_print("Wr 0x%08x to 0x%08x \n", Data_32, (BaseAddr|i));
              addr = (BaseAddr|i);
              HW_REG_WORD(addr, 0) = Data_32;
              //c_print("Rd 0x%08x from 0x%08x \n",Data_32, (BaseAddr|i));
              Actual = HW_REG_WORD(addr, 0);
              if(Actual != Data_32) {
                  c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr|i), Data_32, Actual);
                  ErrCnt++;
              }
              #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
              //read back address
              if(RbAddr != NULL)
              {
                addr_actual = *(RbAddr);
                if(addr_actual != addr){
                  c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
                  ErrCnt++;
                  }
              }
              #endif
        }
    }

    //check end address
    c_print("Check end address.\n");
    Data_8 = (rand()%0x00FF);
    //c_print("Wr 0x%08x to 0x%08x \n", Data_8, (TopAddr));
    addr = (TopAddr);
    HW_REG_BYTE(addr, 0) = Data_8;
    //c_print("Rd 0x%08x from 0x%08x \n",Data_8, (TopAddr));
    Actual = HW_REG_BYTE(addr, 0);
    if(Actual != Data_8) {
        c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(TopAddr), Data_8, Actual);
        ErrCnt++;
    }
    #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
    //read back address
    if(RbAddr != NULL)
    {
      addr_actual = *(RbAddr);
      if(addr_actual != addr){
        c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
        ErrCnt++;
        }
    }
    #endif

    return(ErrCnt);
}

// -------------------------------------------------
// Function for walking ones across address bus (protected)
// All writes followed by all reads
// -------------------------------------------------
uint32_t AddressWalkingP(uint32_t BaseAddr, uint32_t TopAddr, uint32_t BackupAddr, uint32_t Preserve, uint32_t StartPos, uint32_t* RbAddr) {
    uint32_t ErrCnt = 0;
    uint32_t i = 0, k = 0;
    uint8_t Data_8 = 0;
    uint32_t Data_32 = 0;
    uint32_t Actual, addr, addr_actual;
    uint32_t SEED;

    // Write data to memory. Walk 1 across address bus
    c_print("Walking 1s on addr bus. Range 0x%08x - 0x%08x.\n",BaseAddr,TopAddr);
    c_print("Backup the targeted mem range\n");


    // Backup the targeted mem range
    if (Preserve == 1) {
        for(i=1, k = 0; (BaseAddr|i)<TopAddr; i=(i<<1), k++) {
            HW_REG_BYTE(BackupAddr, k) = HW_REG_BYTE((BaseAddr|i), 0);
        }
    }

    // Generate random seed with SV $urand function:

#if (defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)) && !defined(NO_TBOX)
    TRICKBOX_IL->RND_MAX = 0xFFFF;
    SEED = TRICKBOX_IL->RND_VAL;
    srand(SEED);
#endif

    // Write data to memory. Walk 1 across address bus
    c_print("Walking 1s on addr bus. Range 0x%08x - 0x%08x\n",BaseAddr,TopAddr);

    //check start address
    c_print("Check start address.\n");
    Data_8 = (rand()%0x00FF);
    //c_print("Wr 0x%08x to 0x%08x \n", Data_8, (BaseAddr));
    addr = (BaseAddr);
    HW_REG_BYTE(addr, 0) = Data_8;
    //c_print("Rd 0x%08x from 0x%08x \n",Data_8, (BaseAddr));
    Actual = HW_REG_BYTE(addr, 0);
    if(Actual != Data_8) {
        c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr), Data_8, Actual);
        ErrCnt++;
    }

    #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
    //read back address
    if(RbAddr != NULL)
    {
      addr_actual = *(RbAddr);
      if(addr_actual != addr){
        c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
        ErrCnt++;
        }
    }
    #endif


    c_print("Moving 1s.\n");
    for(i=(1<<StartPos); ((BaseAddr|i)+4)<TopAddr; i=(i<<1)) {

        if(0x4>i) //byte access
        {
              Data_8 = (rand()%0x00FF);
              //c_print("Wr 0x%08x to 0x%08x \n", Data_8, (BaseAddr|i));
              addr = (BaseAddr|i);
              HW_REG_BYTE(addr, 0) = Data_8;
              //c_print("Rd 0x%08x from 0x%08x \n",Data_8, (BaseAddr|i));
              Actual = HW_REG_BYTE(addr, 0);
              if(Actual != Data_8) {
                  c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr|i), Data_8, Actual);
                  ErrCnt++;
              }
              #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
              //read back address
              if(RbAddr != NULL)
              {
                addr_actual = *(RbAddr);
                if(addr_actual != addr){
                  c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
                  ErrCnt++;
                  }






              }
              #endif
      }
      else
      {  //word access
            Data_32 = (rand()%0xFFFF)|(rand()%0xFFFF)<<16;
            //c_print("Wr 0x%08x to 0x%08x \n", Data_32, (BaseAddr|i));
            addr = (BaseAddr|i);
            HW_REG_WORD(addr, 0) = Data_32;
            //c_print("Rd 0x%08x from 0x%08x \n",Data_32, (BaseAddr|i));
            Actual = HW_REG_WORD(addr, 0);
            if(Actual != Data_32) {
                c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr|i), Data_32, Actual);
                ErrCnt++;
            }
            #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
            //read back address
            if(RbAddr != NULL)
            {
              addr_actual = *(RbAddr);
              if(addr_actual != addr){
                c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
                ErrCnt++;
                }
            }
            #endif

      }
    }

    //check end address
    c_print("Check end address.\n");
    Data_8 = (rand()%0x00FF);
    //c_print("Wr 0x%08x to 0x%08x \n", Data_8, (TopAddr));
    addr = (TopAddr);
    HW_REG_BYTE(addr, 0) = Data_8;
    //c_print("Rd 0x%08x from 0x%08x \n",Data_8, (TopAddr));
    Actual = HW_REG_BYTE(addr, 0);
    if(Actual != Data_8) {
        c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(TopAddr), Data_8, Actual);
        ErrCnt++;
    }
    #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
    //read back address
    if(RbAddr != NULL)
    {
      addr_actual = *(RbAddr);
      if(addr_actual != addr){
        c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
        ErrCnt++;
        }
    }
    #endif


    // Restore the targeted mem range
    if (Preserve == 1) {
        for(i=1, k=0; (BaseAddr|i)<TopAddr; i=(i<<1), k++) {
            HW_REG_BYTE((BaseAddr|i), 0) = HW_REG_BYTE(BackupAddr, k);
        }
    }

    return(ErrCnt);
}

// -------------------------------------------------
// Word and half word accesses
// All writes followed by all reads
// -------------------------------------------------
uint32_t WordHalfWordAccess(uint32_t BaseAddr, uint32_t TopAddr, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr) {
    uint32_t ErrCnt = 0;
    uint32_t i = 0;
    uint16_t Data_16_0 = 0, Data_16_1 = 0;
    uint32_t Data_32 = 0;
    uint32_t Actual_32;
    uint16_t Actual_16_0, Actual_16_1;
    uint32_t Temp_data, addr_actual, addr;
    uint32_t SEED;

    if ((TopAddr-BaseAddr) < MaxStep ) MaxStep = (TopAddr-BaseAddr)%MaxStep;

    // Generate random seed with SV $urand function:

#if (defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)) && !defined(NO_TBOX)
    TRICKBOX_IL->RND_MAX = 0xFFFF;
    SEED = TRICKBOX_IL->RND_VAL;
    srand(SEED);
#endif

    c_print("Write/readback Words and Halfwords. Range 0x%08x - 0x%08x\n",BaseAddr,TopAddr);
    i = (rand()<<2)%MaxStep;
    i = i - (i%4); //aligned
    while((i+8)<(TopAddr-BaseAddr)) {
      Data_16_0 = (rand()%0xFFFF);
      Data_16_1 = (rand()%0xFFFF);
      Data_32 = (rand()%0xFFFF)|(rand()%0xFFFF)<<16;

      HW_REG_HALF(BaseAddr, i) = Data_16_0;   //HALFWORD write
      addr = BaseAddr + i;
      #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
      //read back address
      if(RbAddr != NULL)
      {
        addr_actual = *(RbAddr);
        if(addr_actual != addr){
          c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
          ErrCnt++;
          }
      }
      #endif

      HW_REG_HALF(BaseAddr, i+2) = Data_16_1; //HALFWORD write
      addr = BaseAddr + i +2;
      #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
      //read back address
      if(RbAddr != NULL)
      {
        addr_actual = *(RbAddr);
        if(addr_actual != addr){
          c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
          ErrCnt++;
          }
      }
      #endif

      HW_REG_WORD(BaseAddr, i+4) = Data_32;   //WORD write
      addr = BaseAddr + i +4;
      #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
      //read back address
      if(RbAddr != NULL)
      {
        addr_actual = *(RbAddr);
        if(addr_actual != addr){
          c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
          ErrCnt++;
          }
      }
      #endif

      Actual_32 = HW_REG_WORD(BaseAddr, i); //WORD read
      Temp_data = (Data_16_1<<16)|Data_16_0;
      if(Actual_32 != Temp_data) {
          c_print("ERROR word rd: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr+i), Temp_data, Actual_32);
          ErrCnt++;
      }

      Actual_16_0 = HW_REG_HALF(BaseAddr, i+4); //HALFWORD read
      Actual_16_1 = HW_REG_HALF(BaseAddr, i+6); //HALFWORD read
      Temp_data = (Actual_16_1<<16)|Actual_16_0;
      if(Data_32 != Temp_data) {
          c_print("ERROR half rd: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr+i+4), Data_32, Temp_data);
          ErrCnt++;
      }

      i += (((rand()%0xFFFF)|(rand()%0xFFFF)<<16)%MaxStep)+MinOffset;
      i = i - (i%4); //aligned

    }

    return(ErrCnt);
}

uint32_t WordHalfWordAccessP(uint32_t BaseAddr, uint32_t TopAddr, uint32_t BackupAddr, uint32_t Preserve, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr){
    uint32_t ErrCnt = 0;
    uint32_t i = 0;
    uint16_t Data_16_0 = 0, Data_16_1 = 0;
    uint32_t Data_32 = 0;
    uint32_t Actual_32;
    uint16_t Actual_16_0, Actual_16_1;
    uint32_t Temp_data, addr_actual, addr;
    uint32_t SEED;

    if ((TopAddr-BaseAddr) < MaxStep ) MaxStep = (TopAddr-BaseAddr)%MaxStep;

    // Generate random seed with SV $urand function:

#if (defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)) && !defined(NO_TBOX)
    TRICKBOX_IL->RND_MAX = 0xFFFF;
    SEED = TRICKBOX_IL->RND_VAL;
    srand(SEED);
#endif

    c_print("Write/readback Words and Halfwords. Range 0x%08x - 0x%08x\n",BaseAddr,TopAddr);
    i = (rand()<<2)%MaxStep;
    i = i - (i%4); //aligned
    while((i+8)<(TopAddr-BaseAddr)) {
      Data_16_0 = (rand()%0xFFFF);
      Data_16_1 = (rand()%0xFFFF);
      Data_32 = (rand()%0xFFFF)|(rand()%0xFFFF)<<16;
      if(Preserve)
      {// Backup the targeted mem range
        HW_REG_WORD(BackupAddr, 0) = HW_REG_WORD(BaseAddr, i);
        HW_REG_WORD(BackupAddr, 4) = HW_REG_WORD(BaseAddr, i+4);
      }
      HW_REG_HALF(BaseAddr, i) = Data_16_0;   //HALFWORD write
      addr = BaseAddr + i;
      #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
      //read back address
      if(RbAddr != NULL)
      {
        addr_actual = *(RbAddr);
        if(addr_actual != addr){
          c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
          ErrCnt++;
          }
      }
      #endif

      HW_REG_HALF(BaseAddr, i+2) = Data_16_1; //HALFWORD write
      addr = BaseAddr + i +2;
      #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
      //read back address
      if(RbAddr != NULL)
      {
        addr_actual = *(RbAddr);
        if(addr_actual != addr){
          c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
          ErrCnt++;
          }
      }
      #endif

      HW_REG_WORD(BaseAddr, i+4) = Data_32;   //WORD write
      addr = BaseAddr + i +4;
      #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
      //read back address
      if(RbAddr != NULL)
      {
        addr_actual = *(RbAddr);
        if(addr_actual != addr){
          c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
          ErrCnt++;
          }
      }
      #endif

      Actual_32 = HW_REG_WORD(BaseAddr, i); //WORD read
      Temp_data = (Data_16_1<<16)|Data_16_0;
      if(Actual_32 != Temp_data) {
          c_print("ERROR word rd: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr+i), Temp_data, Actual_32);
          ErrCnt++;
      }

      Actual_16_0 = HW_REG_HALF(BaseAddr, i+4); //HALFWORD read
      Actual_16_1 = HW_REG_HALF(BaseAddr, i+6); //HALFWORD read
      Temp_data = (Actual_16_1<<16)|Actual_16_0;
      if(Data_32 != Temp_data) {
          c_print("ERROR half rd: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr+i+4), Data_32, Temp_data);
          ErrCnt++;
      }
      if(Preserve) {// Restore the targeted mem range
        HW_REG_WORD(BaseAddr, i)   = HW_REG_WORD(BackupAddr, 0);
        HW_REG_WORD(BaseAddr, i+4) = HW_REG_WORD(BackupAddr, 4);
      }
      i += (((rand()%0xFFFF)|(rand()%0xFFFF)<<16)%MaxStep)+MinOffset;
      i = i - (i%4); //aligned

    }

    return(ErrCnt);
}


// -------------------------------------------------
// unaligned word accesses at each boundary
// All word writes followed by all reads one byte at a time
// -------------------------------------------------
uint32_t UnalignedAccess(uint32_t BaseAddr, uint32_t TopAddr, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr) {
    uint32_t ErrCnt = 0;
    uint32_t i = 0;
    uint32_t Data = 0;
    uint32_t Actual, addr, addr_actual;
    uint32_t SEED;

    if ((TopAddr-BaseAddr) < MaxStep ) MaxStep = (TopAddr-BaseAddr)%MaxStep;

    // Generate random seed with SV $urand function:

#if (defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)) && !defined(NO_TBOX)
    TRICKBOX_IL->RND_MAX = 0xFFFF;
    SEED = TRICKBOX_IL->RND_VAL;
    srand(SEED);
#endif

    c_print("Write/readback unaligned words. Range 0x%08x - 0x%08x\n",BaseAddr,TopAddr);
    i = rand()%MaxStep;
    while((i+4)<(TopAddr-BaseAddr)) {
        Data = (rand()%0xFFFF)|(rand()%0xFFFF)<<16;
        //c_print("Wr 0x%08x to 0x%08x \n", Data, (BaseAddr+i));
        HW_REG_WORD(BaseAddr, i) = Data;
        //c_print("Rd 0x%08x from 0x%08x \n", Data, (BaseAddr+i));
        Actual = HW_REG_WORD(BaseAddr, i);
        if(Actual != Data) {
            c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr+i), Data, Actual);
            ErrCnt++;
        }
        addr = BaseAddr + i;
        #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
        //read back address
        if(RbAddr != NULL)
        {
          addr_actual = *(RbAddr);
          if(addr_actual != addr){
            c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
            ErrCnt++;
            }
        }
        #endif
        i += 4+(((rand()%0xFFFF)|(rand()%0xFFFF)<<16)%MaxStep)+MinOffset;


    }

    return(ErrCnt);
}


uint32_t UnalignedAccessP(uint32_t BaseAddr, uint32_t TopAddr, uint32_t BackupAddr, uint32_t Preserve, uint32_t MaxStep, uint32_t MinOffset, uint32_t* RbAddr) {
    uint32_t ErrCnt = 0;
    uint32_t i = 0;
    uint32_t Data = 0;
    uint32_t Actual, addr, addr_actual;
    uint32_t SEED;

    c_print("Write/readback unaligned words. Range 0x%08x - 0x%08x.\n",BaseAddr,TopAddr);
    c_print("Backup the targeted mem range\n");

    if ((TopAddr-BaseAddr) < MaxStep ) MaxStep = (TopAddr-BaseAddr)%MaxStep;

    // Generate random seed with SV $urand function:

#if (defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)) && !defined(NO_TBOX)
    TRICKBOX_IL->RND_MAX = 0xFFFF;
    SEED = TRICKBOX_IL->RND_VAL;
    srand(SEED);
#endif

    c_print("Write/readback unaligned words. Range 0x%08x - 0x%08x\n",BaseAddr,TopAddr);
    i = rand()%MaxStep;
    while((i+4)<(TopAddr-BaseAddr)) {
        if(Preserve) HW_REG_WORD(BackupAddr, 0) = HW_REG_WORD(BaseAddr, i);// Backup the targeted mem range
        Data = (rand()%0xFFFF)|(rand()%0xFFFF)<<16;
        //c_print("Wr 0x%08x to 0x%08x \n", Data, (BaseAddr+i));
        HW_REG_WORD(BaseAddr, i) = Data;
        //c_print("Rd 0x%08x from 0x%08x \n", Data, (BaseAddr+i));
        Actual = HW_REG_WORD(BaseAddr, i);
        if(Actual != Data) {
            c_print("ERROR: Addr 0x%08x, expected - 0x%08x actual - 0x%08x\n",(BaseAddr+i), Data, Actual);
            ErrCnt++;
        }
        #if defined(SYNC_CPU_CM3_0) || defined(SYNC_CPU_CM3_1)
        addr = BaseAddr + i;
        //read back address
        if(RbAddr != NULL)
        {
          addr_actual = *(RbAddr);
          if(addr_actual != addr){
            c_print("ERROR: Addr read back, expected - 0x%08x actual - 0x%08x\n", addr, addr_actual);
            ErrCnt++;
            }
        }
        #endif

        if(Preserve) HW_REG_WORD(BaseAddr, i) = HW_REG_WORD(BackupAddr, 0);// Restore the targeted mem range
        i += 4+(((rand()%0xFFFF)|(rand()%0xFFFF)<<16)%MaxStep)+MinOffset;
    }

    return(ErrCnt);
}

#endif //__SYSTEM_LEVEL_FUNCTIONS_C__




