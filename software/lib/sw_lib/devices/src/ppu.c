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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include "ppu.h"

__attribute__((weak))  void call_wfi(){
} 


void ppu_set_PWRP(SCP_PPU_1_1_TypeDef *ppu, uint32_t state) {
  ppu->PPU_PWRP.W = state;
}

void ppu_set_power_policy(SCP_PPU_1_1_TypeDef *ppu, uint32_t state) {
  ppu->PPU_PWRP.B.PWR_POLICY = state;
}

void ppu_check_power_policy(SCP_PPU_1_1_TypeDef *ppu, uint32_t state) {
  while(ppu->PPU_PWSR.B.PWR_STATUS != state){
    continue;
  }
}

void ppu_clear_irq_mask(SCP_PPU_1_1_TypeDef *ppu){
  if(ppu->PPU_PWSR.B.PWR_DYN_STATUS==0x1){
    //Clear mask for dynamic mode
    ppu->PPU_AIMR.B.DYN_DENY_IRQ_MASK      = 0;
    ppu->PPU_AIMR.B.DYN_ACCEPT_IRQ_MASK    = 0;
  }else{
    //Clear mask for static mode
    ppu->PPU_IMR.B.STA_POLICY_TRN_IRQ_MASK = 0;
    ppu->PPU_IMR.B.STA_ACCEPT_IRQ_MASK = 0;
    ppu->PPU_IMR.B.STA_DENY_IRQ_MASK = 0;
  }
}

void ppu_set_irq_mask(SCP_PPU_1_1_TypeDef *ppu){
  if(ppu->PPU_PWSR.B.PWR_DYN_STATUS==0x1){
    //Clear mask for dynamic mode
    ppu->PPU_AIMR.B.DYN_DENY_IRQ_MASK      = 1;
    ppu->PPU_AIMR.B.DYN_ACCEPT_IRQ_MASK    = 1;
  }else{
    //Clear mask for static mode
    ppu->PPU_IMR.B.STA_POLICY_TRN_IRQ_MASK = 1;
    ppu->PPU_IMR.B.STA_ACCEPT_IRQ_MASK = 1;
    ppu->PPU_IMR.B.STA_DENY_IRQ_MASK = 1;
  }
}

void ppu_clear_intr(SCP_PPU_1_1_TypeDef *ppu) {

  /* Check for Static Policy Denial  event */
  if (ppu->PPU_ISR.B.STA_DENY_IRQ == 1) {
    ppu->PPU_ISR.W = 0x4;
  }

  /* Check for Static Policy Accept event */
  if (ppu->PPU_ISR.B.STA_ACCEPT_IRQ == 1) {
    ppu->PPU_ISR.W = 0x2;
  }

  /* Check for Policy Transition Completion event */
  if (ppu->PPU_ISR.B.STA_POLICY_TRN_IRQ == 1) {
    ppu->PPU_ISR.W = 0x1;
  }

  /* Check for Dynamic Policy Accept event */
  if (ppu->PPU_AISR.B.DYN_ACCEPT_IRQ == 1) {
    ppu->PPU_AISR.W = 0x2;
  }

  /* Check for Policy Transition Completion event */
  if (ppu->PPU_AISR.B.DYN_DENY_IRQ == 1) {
    ppu->PPU_AISR.W = 0x4;
  }

}

