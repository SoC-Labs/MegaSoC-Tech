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
#include "system.h"
#include "sys_memory_map.h"
#include "sys_intr_map.h"
#include "host_chassis_control.h"
#include "uart_stdout.h"
#include "cpu_asm_codes.h"
// ************* Include Intgeration Layer includes always as last *****************
#include "il_sys_includes.h"
// *********************************************************************************

//Integration layer init function
// __attribute__((weak)) void il_init(void){
//   HOST_CHASSIS_CTRL_TypeDef     *host_cpu_chassis_control = ((HOST_CHASSIS_CTRL_TypeDef     *) HOST_CPU_HOST_BASE_CONTROL_BASE);
//   printf("Extsys0 and Extsys1 CPUWAIT are de-asserted.\n");
//   host_cpu_chassis_control->EXT_SYS0_RST_CTRL.B.CPUWAIT = 0;
//   host_cpu_chassis_control->EXT_SYS1_RST_CTRL.B.CPUWAIT = 0;
//   }

// __attribute__((weak)) int main_ext(){
//   call_wfi();
//   return 0;
// }


/**-----------------------------------------------------------------------------
 Function name : cpu_test_hook
 Input Parameters  : void
 Return Type : void
 Descritpion : 1. Hook function to be called from the cpu_test function
               2. __weak definition as overriden by test
------------------------------------------------------------------------------*/
// __attribute__((weak)) void cpu_test_hook(void) {
// }


// @override
// Override weak Moonshine function in apps_v8_32/src/cpu_asm_codes.c,
// to not print unwanted characters in the log with slave CPUs
/**-----------------------------------------------------------------------------
 Function name : cpu_test
 Input Parameters  : void
 Return Type : void
 Descritpion : 1. Entry point for all slave CPU's
               2. __weak defination as overriden by test
               3. In the absence of Tets implementation, avoids data corruption and prints &
------------------------------------------------------------------------------*/
// void cpu_test(void)
// {
//   HOST_CHASSIS_CTRL_TypeDef     *host_cpu_chassis_control = ((HOST_CHASSIS_CTRL_TypeDef     *) HOST_CPU_HOST_BASE_CONTROL_BASE);

//   unsigned int cpu_num;
//   cpu_num = get_cpu_core_number();

//   cpu_test_hook();
//   while(1) {
//     call_wfe();
//   }
// }



// extern int main();

// void master_cpu_entry(void){
//   // Clear wakeup, so CORE0 can turn off if needed
//   HOST_CHASSIS_CTRL_TypeDef     *host_cpu_chassis_control = ((HOST_CHASSIS_CTRL_TypeDef     *) HOST_CPU_HOST_BASE_CONTROL_BASE);
//   host_cpu_chassis_control->HOST_CPU_WAKEUP.B.CORE0_WAKEUP = 0x0;

//   main();
//   call_wfi();
// }


/**-----------------------------------------------------------------------------
   Function name : TEST_PASS
   Input Parameters  : void
   Return Type :  void
   Descritpion :  Test pass function
   ------------------------------------------------------------------------------*/

void TEST_PASS(void) {
  printf("\n** TEST PASSED **\n");
  UartEndSimulation();
}

/**-----------------------------------------------------------------------------
   Function name : TEST_FAIL
   Input Parameters  : void
   Return Type :  void
   Descritpion :  Test fail function
   ------------------------------------------------------------------------------*/

void TEST_FAIL(void) {
  printf("\n** TEST FAILED **\n");
  UartEndSimulation();
}

