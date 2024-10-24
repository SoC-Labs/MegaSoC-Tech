#include "uart_stdout.h"
#include "system.h"
#include <stdio.h>

int main(void) {
  uint32_t errors = 0;
  UartStdOutInit();

  printf("Hello SoCLabs MegaSoC\n");
  TEST_PASS();
}