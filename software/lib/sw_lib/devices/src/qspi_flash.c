#include "qspi_flash.h"

void spi_reset(){
  SL_AHB_QSPI->QSPI_CMD = 0x066;
  SL_AHB_QSPI->QSPI_CMD = 0x166;
  while(SL_AHB_QSPI->QSPI_STATUS&1){
    ;
  }
  SL_AHB_QSPI->QSPI_CMD = 0x099;
  SL_AHB_QSPI->QSPI_CMD = 0x199;
  while(SL_AHB_QSPI->QSPI_STATUS&1){ ;}
}

void SET_QPI_MODE(){
  SL_AHB_QSPI->QSPI_CMD = 0x038;
  SL_AHB_QSPI->QSPI_CMD = 0x138;
  while(SL_AHB_QSPI->QSPI_STATUS&1){ ;}
  int32_t tmp_QSPI_CONTROL;
  tmp_QSPI_CONTROL = SL_AHB_QSPI->QSPI_CONTROL;
  tmp_QSPI_CONTROL = tmp_QSPI_CONTROL | 1;
  SL_AHB_QSPI->QSPI_CONTROL=tmp_QSPI_CONTROL;
  return;
}

void qspi_enable_cache(){
  // Don't enable prefetch, this causes problems for some reason
    CACHE_CTRL->CCR = 0x00;
    uint32_t QSPI_CTRL_tmp;
    QSPI_CTRL_tmp=SL_AHB_QSPI->QSPI_CONTROL;
    QSPI_CTRL_tmp = QSPI_CTRL_tmp | 1<<8;
    SL_AHB_QSPI->QSPI_CONTROL = QSPI_CTRL_tmp;

    CACHE_CTRL->CCR = 0x01;
    while((CACHE_CTRL->SR&0x3)!=2){;}

    return;
}

void qspi_xip_enable(){
    uint32_t QSPI_CTRL_tmp;
      // Setup continuous reading
    QSPI_CTRL_tmp=SL_AHB_QSPI->QSPI_CONTROL;
    QSPI_CTRL_tmp = QSPI_CTRL_tmp | 1<<8;
    SL_AHB_QSPI->QSPI_CONTROL = QSPI_CTRL_tmp;

    QSPI_CTRL_tmp=SL_AHB_QSPI->QSPI_CONTROL;
    QSPI_CTRL_tmp = QSPI_CTRL_tmp | ((1<<24) + (0x0A0<<16));
    SL_AHB_QSPI->QSPI_CONTROL = QSPI_CTRL_tmp;
    // 1 read with op code
    uint32_t rdata;
    rdata = QSPI_CACHE->DATA[0];
    // set no-opcode mode
    QSPI_CTRL_tmp=SL_AHB_QSPI->QSPI_CONTROL;
    QSPI_CTRL_tmp = QSPI_CTRL_tmp | (1<<25);
    SL_AHB_QSPI->QSPI_CONTROL = QSPI_CTRL_tmp;
    return;
}

int32_t SPI_READ_JEDIC(){
  SL_AHB_QSPI->QSPI_CMD = 0x0002009F;
  SL_AHB_QSPI->QSPI_CMD = 0x0002039F;
  while(SL_AHB_QSPI->QSPI_STATUS&1){ ;}
  return SL_AHB_QSPI->QSPI_RDATA0;
}

int32_t QPI_READ_JEDIC(){
  SL_AHB_QSPI->QSPI_CMD = 0x000210AF;
  SL_AHB_QSPI->QSPI_CMD = 0x000213AF;
  while(SL_AHB_QSPI->QSPI_STATUS&1){ ;}
  return SL_AHB_QSPI->QSPI_RDATA0;
}

