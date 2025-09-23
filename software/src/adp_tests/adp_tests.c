#include "uart_stdout.h"
#include "system.h"
#include <stdio.h>

int main(void) {
  uint32_t errors = 0;
  unsigned char ch;
  UartStdOutInit();

  while  ((ch=UartGetc()) != 'X'){
    printf("'%c'\n", ch);
  }
  printf("ADP dummy test finished\n");

  TEST_PASS();
}