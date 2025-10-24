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

#include "pl011_uart.h"

/*
 * UART Control Registers
 * in separate section so it can be placed correctly using scatter file
 */



/* UART Init Function */
void pl011_uart_init(void * pl011_uart_ptr) {
    pl011_uart_t * uart = (pl011_uart_t *) pl011_uart_ptr;

    /* Line control register */
    uart->LCR_H = 0x00000070;

    /* Enable UART for Transmit and Receive */
    uart->Ctrl  = UARTCR_ENABLE | UARTCR_TXENABLE | UARTCR_RXENABLE;
    //uart->LCR_H = UARTLCR_H_FIFOENABLE;

    /* Don't need to enable interrupts in this example
    UARTREGS[UARTIMSK] = UARTRIS_RTINTR | UARTRIS_RXINTR; */
}

// UART baud rate setting
void pl011_config_baud(void * pl011_uart_ptr) {

    pl011_uart_t * uart = (pl011_uart_t *) pl011_uart_ptr;

    /* Configure Baud Rate */
    uart->IBRD = 0x01;
    uart->FBRD = 0x0;
}

/* UART Read Function */
char pl011_uart_getchar_polled(void * pl011_uart_ptr) {
    pl011_uart_t * uart = (pl011_uart_t *) pl011_uart_ptr;
    
    while (uart->Flags & UARTFR_RXFIFOEMPTY) {
        /* waiting to receive data! */
    }
    
    return uart->Data;
}

/* UART Write Function */
void pl011_uart_putc_polled(void * pl011_uart_ptr, char c) {
    pl011_uart_t * uart = (pl011_uart_t *) pl011_uart_ptr;
    
    while (uart->Flags & (UARTFR_TXFIFOFULL)) {
      /*  waiting to send data! */
    }
    
    uart->Data = c;
}

void pl011_uart_print_string(void * pl011_uart_ptr, unsigned char *str){
  pl011_uart_t * uart = (pl011_uart_t *) pl011_uart_ptr;

  while(*str!=0){
    pl011_uart_putc_polled(uart, *str);
    str++;
  }
}

void pl011_uart_intr_chk(void * pl011_uart_ptr, unsigned char *str, int fifo_en, int tx){
  pl011_uart_t * uart = (pl011_uart_t *) pl011_uart_ptr;

  uart->IBRD  = 0x030; //9600 baud rate
  if(tx)
    uart->MSC   = 0x020; //TX intr enabled
  else
    uart->MSC   = 0x010; //RX intr enabled

  if(!fifo_en){
    uart->LCR_H = 0;     //Disable RX and TX FIFOS
  }
  uart->Ctrl  = 0x301; // Enable UART, TX and RX

  //Send str to TXFIFO. Loopback should be present (UART_TXD connected back to UART_RXD for RX intr to work)
  pl011_uart_print_string(uart, str);

  if(tx){
    //Wait till last char reaches the TXFIFO in UART
    while (uart->Flags & UARTFR_TXFIFOFULL) {
    }
  }
  //Disable RX and TX interrupt masks
  uart->MSC = 0;
}

void pl011_uart_intr_chk_baud(void * pl011_uart_ptr, unsigned char *str, int fifo_en, int tx, int brd){
  pl011_uart_t * uart = (pl011_uart_t *) pl011_uart_ptr;

  uart->IBRD  = brd; 
  if(tx)
    uart->MSC   = 0x020; //TX intr enabled
  else
    uart->MSC   = 0x010; //RX intr enabled

  if(!fifo_en){
    uart->LCR_H = 0;     //Disable RX and TX FIFOS
  }
  uart->Ctrl  = 0x301; // Enable UART, TX and RX

  //Send str to TXFIFO. Loopback should be present (UART_TXD connected back to UART_RXD for RX intr to work)
  pl011_uart_print_string(uart, str);

  if(tx){
    //Wait till last char reaches the TXFIFO in UART
    while (uart->Flags & UARTFR_TXFIFOFULL) {
    }
  }
  //Disable RX and TX interrupt masks
  uart->MSC = 0;
}
