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

#ifndef __PL011_UART_H__
#define __PL011_UART_H__

#ifdef __cplusplus
extern "C" {
  #endif

  /* -----------------------------------------------------------------------------
  * pl011 registers mask
  * -----------------------------------------------------------------------------
  */

  enum UARTFR_BITS {
    UARTFR_TXFIFOEMPTY    = 1<<7,
    UARTFR_RXFIFOFULL     = 1<<6,
    UARTFR_TXFIFOFULL     = 1<<5,
    UARTFR_RXFIFOEMPTY    = 1<<4,
    UARTFR_TXFIFONOTEMPTY = 1<<3
  };

  enum UARTLCR_H_BITS {
    UARTLCR_H_FIFOENABLE  = 1<<4 // Select 16-deep rather than 1-deep fifo.
  };

  enum UARTCR_BITS {
    UARTCR_ENABLE         = 1<<0,
    UARTCR_SIRENABLE      = 1<<1, // Not implemented in ISSM.
    UARTCR_LOOPBACK       = 1<<7, // Not implemented in ISSM.
    UARTCR_TXENABLE       = 1<<8,
    UARTCR_RXENABLE       = 1<<9
  };

  enum UARTRIS_BITS {
    UARTRIS_RTINTR        = 1<<6,   // Receive Timeout interrupt.
    UARTRIS_TXFIFOEMPTY   = 1<<5,
    UARTRIS_RXINTR        = 1<<4    // Fifo reaches tidemark.
  };

  enum UARTDMACR_BITS {
    UARTDMACR_TX          = 1<<1,
    UARTDMACR_RX          = 1<<0
  };


  #define PL011_UART_RES008_014_WIDHT 0x004 // (0x018 - 0x008) / 4
  #define PL011_UART_RES04c_07c_WIDHT 0x00D // (0x080 - 0x04C) / 4
  #define PL011_UART_RES080_08c_WIDHT 0x004 // (0x090 - 0x080) / 4
  #define PL011_UART_RES090_fcc_WIDHT 0x3D0 // (0xFD0 - 0x090) / 4
  #define PL011_UART_RESfd0_fdc_WIDHT 0x004 // (0xFE0 - 0xFD0) / 4


  /* -----------------------------------------------------------------------------
  * pl011 registers
  * -----------------------------------------------------------------------------
  */

  typedef struct {
    volatile unsigned int Data;
    volatile unsigned int RSR_ECR;
    volatile unsigned int zReserved008_014[PL011_UART_RES008_014_WIDHT];
    volatile unsigned int Flags;
    volatile unsigned int zReserved01c;
    volatile unsigned int IPLR;
    volatile unsigned int IBRD;
    volatile unsigned int FBRD;
    volatile unsigned int LCR_H;
    volatile unsigned int Ctrl;
    volatile unsigned int IFLS;
    volatile unsigned int MSC;
    volatile unsigned int RIS;
    volatile unsigned int MIS;
    volatile unsigned int IRQClear;
    volatile unsigned int DMACR; //0x048
    volatile unsigned int zReserved04c_07c[PL011_UART_RES04c_07c_WIDHT];
    //  volatile unsigned int zReservedForTest080_08c[PL011_UART_RES080_08c_WIDHT]; //0x080-0x08C
    // /*
    volatile unsigned int UARTTCR;//0x080
    volatile unsigned int UARTITIP;//0x084
    volatile unsigned int UARTITOP;//0x088
    volatile unsigned int UARTTDR;//0x08C
    //   */
    volatile unsigned int zReserved090_fcc[PL011_UART_RES090_fcc_WIDHT];
    volatile unsigned int zReservedForExpfd0_fdc[PL011_UART_RESfd0_fdc_WIDHT];
    volatile unsigned int PeriphID[4];
    volatile unsigned int CellID[4];
  } pl011_uart_t;

  /* -----------------------------------------------------------------------------
  * pl011 functions
  * -----------------------------------------------------------------------------
  */

  /* Function prototype - to initialize UART */
  extern void pl011_uart_init(void * );

  // UART baud rate setting
  extern void pl011_config_baud(void * pl011_uart_ptr);

  /* Function prototype - to get character from UART */
  extern char pl011_uart_getchar_polled(void *);

  /* Function prototype - to send character from UART */
  //   extern void pl011_uart_putc_polled(pl011_uart_t *, char);
  extern void pl011_uart_putc_polled(void *, char);

  /* Function prototype - to send strings from UART */
  //  extern void pl011_uart_print_string(pl011_uart_t *, unsigned char *);
  extern void pl011_uart_print_string(void *, unsigned char *);

  /* Function prototype - UART interrupt check test*/
  //   extern void pl011_uart_intr_chk(pl011_uart_t *, unsigned char *, int, int);
  extern void pl011_uart_intr_chk(void *, unsigned char *, int, int);
  extern void pl011_uart_intr_chk_baud(void *, unsigned char *, int, int,int);


  /* -----------------------------------------------------------------------------
  * End of pl011_uart.h
  * -----------------------------------------------------------------------------
  */

  #ifdef __cplusplus
}
#endif



#endif /* __PL011_UART_H__ */
