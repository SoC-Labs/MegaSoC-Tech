//#include "host_chassis_control.h"
//#include "system_level_functions.h"
//#include "intrinsics.h"

#include "system.h"
#include "qspi_flash.h"
#include "cpu_asm_codes.h"
#include "uart_stdout.h"
#include <stdio.h>
int main(void) {
  uint32_t errors = 0;
  UartStdOutInit();

  printf("\n***SoCLabs MegaSoC***\n");
  enable_caches();
  enable_caches_el1();

  SL_QSPI->CLK_DIV = 0x4;
  SPI_STARTUP(SL_QSPI);
  CACHE_STARTUP(CG092);
  printf("***Flash Enabled...Booting***\n\n\n");

  __dsb(0xf);
  __sev();

  void (*main_code)(void) = (void (*)())0x00400000;
  main_code();
}