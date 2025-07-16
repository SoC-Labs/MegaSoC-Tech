#include "uart_stdout.h"
#include "system.h"
#include "sys_memory_map.h"
#include "system_level_functions.h"
#include <stdio.h>

int timer_id_test(uint32_t base_addr);
int dualtimer_id_test(uint32_t base_addr);
int uart_id_test(uint32_t base_addr);
int watchdog_id_test(uint32_t base_addr);
int rtc_id_test(uint32_t base_addr);
int spi_id_test(uint32_t base_addr);


int main(void) {
  uint32_t errors = 0;
  UartStdOutInit();

  printf("MegaSoC Peripheral Subsystem test\n");

  printf("Check ID of Timer 0\n");
  if(timer_id_test(TIMER0_BASE)!=0){errors+=1;}
  printf("Check ID of Timer 1\n");
  if(timer_id_test(TIMER1_BASE)!=0){errors+=2;}
  printf("Check ID of Dual Timer\n");
  if(dualtimer_id_test(DUALTIMER_BASE)!=0){errors+=4;}
  printf("Check ID of USRT 0\n");
  if(uart_id_test(SYS_USRT0_BASE)!=0){errors+=8;}
  printf("Check ID of USRT 1\n");
  if(uart_id_test(SYS_USRT1_BASE)!=0){errors+=16;}
  printf("Check ID of UART 0\n");
  if(uart_id_test(SYS_UART0_BASE)!=0){errors+=32;}
  printf("Check ID of UART 1\n");
  if(uart_id_test(SYS_UART1_BASE)!=0){errors+=64;}
  printf("Check ID of Watchdog\n");
  if(watchdog_id_test(SYS_WATCHDOG_BASE)!=0){errors+=128;}
  printf("Check ID of RTC\n");
  if(rtc_id_test(SYS_RTC_BASE)!=0){errors+=256;}
  printf("Check ID of SPI\n");
  if(spi_id_test(SYS_SPI_BASE)!=0){errors+=512;}


  printf("Error = 0x%x\n", errors);

  if(errors==0){TEST_PASS();}
  else{TEST_FAIL();}
}

int timer_id_test(uint32_t base_addr){
  uint8_t PID4 = 0x04;
  uint8_t PID5 = 0x00;
  uint8_t PID6 = 0x00;
  uint8_t PID7 = 0x00;
  uint8_t PID0 = 0x22;
  uint8_t PID1 = 0xB8;
  uint8_t PID2 = 0x1B;
  uint8_t PID3 = 0x00;

  uint8_t CID0 = 0x0D;
  uint8_t CID1 = 0xF0;
  uint8_t CID2 = 0x05;
  uint8_t CID3 = 0xB1;

  if(HW_REG_BYTE(base_addr,0xFD0) != PID4){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD4) != PID5){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD8) != PID6){return 1;}
  if(HW_REG_BYTE(base_addr,0xFDC) != PID7){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE0) != PID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE4) != PID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE8) != PID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFEC) != PID3){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF0) != CID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF4) != CID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF8) != CID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFFC) != CID3){return 1;}
  return 0;
}


int dualtimer_id_test(uint32_t base_addr){
  uint8_t PID4 = 0x04;
  uint8_t PID5 = 0x00;
  uint8_t PID6 = 0x00;
  uint8_t PID7 = 0x00;
  uint8_t PID0 = 0x23;
  uint8_t PID1 = 0xB8;
  uint8_t PID2 = 0x1B;
  uint8_t PID3 = 0x00;

  uint8_t CID0 = 0x0D;
  uint8_t CID1 = 0xF0;
  uint8_t CID2 = 0x05;
  uint8_t CID3 = 0xB1;

  if(HW_REG_BYTE(base_addr,0xFD0) != PID4){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD4) != PID5){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD8) != PID6){return 1;}
  if(HW_REG_BYTE(base_addr,0xFDC) != PID7){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE0) != PID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE4) != PID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE8) != PID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFEC) != PID3){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF0) != CID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF4) != CID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF8) != CID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFFC) != CID3){return 1;}
  return 0;
}
int uart_id_test(uint32_t base_addr){
  uint8_t PID4 = 0x04;
  uint8_t PID5 = 0x00;
  uint8_t PID6 = 0x00;
  uint8_t PID7 = 0x00;
  uint8_t PID0 = 0x21;
  uint8_t PID1 = 0xB8;
  uint8_t PID2 = 0x1B;
  uint8_t PID3 = 0x00;

  uint8_t CID0 = 0x0D;
  uint8_t CID1 = 0xF0;
  uint8_t CID2 = 0x05;
  uint8_t CID3 = 0xB1;

  if(HW_REG_BYTE(base_addr,0xFD0) != PID4){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD4) != PID5){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD8) != PID6){return 1;}
  if(HW_REG_BYTE(base_addr,0xFDC) != PID7){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE0) != PID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE4) != PID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE8) != PID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFEC) != PID3){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF0) != CID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF4) != CID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF8) != CID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFFC) != CID3){return 1;}
  return 0;

}
int watchdog_id_test(uint32_t base_addr){
  uint8_t PID4 = 0x04;
  uint8_t PID5 = 0x00;
  uint8_t PID6 = 0x00;
  uint8_t PID7 = 0x00;
  uint8_t PID0 = 0x24;
  uint8_t PID1 = 0xB8;
  uint8_t PID2 = 0x1B;
  uint8_t PID3 = 0x00;

  uint8_t CID0 = 0x0D;
  uint8_t CID1 = 0xF0;
  uint8_t CID2 = 0x05;
  uint8_t CID3 = 0xB1;

  if(HW_REG_BYTE(base_addr,0xFD0) != PID4){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD4) != PID5){return 1;}
  if(HW_REG_BYTE(base_addr,0xFD8) != PID6){return 1;}
  if(HW_REG_BYTE(base_addr,0xFDC) != PID7){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE0) != PID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE4) != PID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE8) != PID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFEC) != PID3){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF0) != CID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF4) != CID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF8) != CID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFFC) != CID3){return 1;}
  return 0;

}
int rtc_id_test(uint32_t base_addr){
  uint8_t PID0 = 0x31;
  uint8_t PID1 = 0x10;
  uint8_t PID2 = 0x04;
  uint8_t PID3 = 0x00;

  uint8_t CID0 = 0x0D;
  uint8_t CID1 = 0xF0;
  uint8_t CID2 = 0x05;
  uint8_t CID3 = 0xB1;

  if(HW_REG_BYTE(base_addr,0xFE0) != PID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE4) != PID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE8) != PID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFEC) != PID3){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF0) != CID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF4) != CID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF8) != CID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFFC) != CID3){return 1;}
  return 0;
}
int spi_id_test(uint32_t base_addr){
  uint8_t PID0 = 0x22;
  uint8_t PID1 = 0x10;
  uint8_t PID2 = 0x34;
  uint8_t PID3 = 0x00;

  uint8_t CID0 = 0x0D;
  uint8_t CID1 = 0xF0;
  uint8_t CID2 = 0x05;
  uint8_t CID3 = 0xB1;

  if(HW_REG_BYTE(base_addr,0xFE0) != PID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE4) != PID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFE8) != PID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFEC) != PID3){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF0) != CID0){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF4) != CID1){return 1;}
  if(HW_REG_BYTE(base_addr,0xFF8) != CID2){return 1;}
  if(HW_REG_BYTE(base_addr,0xFFC) != CID3){return 1;}
  return 0;
}
