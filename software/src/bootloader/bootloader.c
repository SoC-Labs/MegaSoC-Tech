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
  spi_reset();
  int32_t rID = SPI_READ_JEDIC();
  SET_QPI_MODE();
  rID = QPI_READ_JEDIC();
  qspi_enable_cache();
  qspi_xip_enable();

  // uint32_t rdata[16];
  // for (int i=0;i<16;i++){
  //   rdata[i]= QSPI_CACHE->DATA[i];
  // }

  printf("***Flash Enabled...Booting***\n\n\n");

  void (*main_code)(void) = (void (*)())0x00400000;
  main_code();
}