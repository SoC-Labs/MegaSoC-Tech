/*------------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from Arm Limited or its affiliates.
 *
 *        (C) COPYRIGHT 2018-2021 Arm Limited or its affiliates.
 *            ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from Arm Limited or its affiliates.
 *
 *      Release Information : SSE710-r0p0-00rel0
 *
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdarg.h>
#include "system.h" 
#include "platform.h" 
#include "uart_stdout.h"
#include "sys_memory_map.h"
#include "intrinsics.h"


/** @file system.c
 *  @brief System Print, TEST_PASS/TEST_FAIL and placeholder for other helper functions 
 */

/* -----------------------------------------------------------------------------
 * Default exception handler function
 * -----------------------------------------------------------------------------
 */

/* Abort */
__attribute__((weak)) void abort_handler (void) {
  printf ("Unexpected abort exception.\n");
  TEST_FAIL();
}

/* -----------------------------------------------------------------------------
 * C library retarget
 * -----------------------------------------------------------------------------
 */ 

#ifndef SEMIHOST
__ASM(".global __use_no_semihosting");
//__ASM(".global __use_no_heap");

/*
* These must be defined to avoid linking in stdio.o from the
* C Library
*/

struct __FILE { int handle;   /* Add whatever you need here */};
FILE __stdout;
FILE __stdin;
FILE __stdout;

/*
* __backspace must return the last char read to the stream
* fgetc() needs to keep a record of whether __backspace was
* called directly before it
*/
int last_char_read;
int backspace_called;


/** @brief fuptc, print redirction function, characters written to UART or TBENCH component 
 *  @return int 
 */

int fputc(int ch, FILE* f) {
  unsigned char tempch = ch;
  UartPutc(ch);
  return ch;
}

#ifdef __ARMCC_VERSION__
/** @brief ferror, Empty function  
 *  @return int 
 */

int ferror(FILE* f) {
  return EOF;
}
#endif

/** @brief sys_exit, Overrides sys_exit to print ^D (EOT) to the Tube to end the simulation   
 *  @return int 
 */

void _sys_exit(int return_code) {
  __wfi();
}

/** @brief _sys_command_string, Required for RVCT6 
 *  @return char *
 */

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/** @brief _ttywrch, Required for RVCT6 
 *  @return void
 */

void _ttywrch(int ch)
{

}

/** @brief __backspace, Required for RVCT6 
 *  @return int
 */

int __backspace(FILE *f)
{
    backspace_called = 0X1;
    return 1;
}

#endif



/* -----------------------------------------------------------------------------
 * Fast printing functions
 * -----------------------------------------------------------------------------
 */ 

/** @brief c_print, This is fast print function where checks for % in input string, 
 *          if no % found, then calls c_print_str to save execution time of 
 *          full print function.
 *  @return int
 */

int c_print(const char * fmt, ...) {
  va_list args;
  int tmp, count = 0;
  char buffer[160];
  const char *parse_str = fmt;
  int flag = 0;

  while ((*parse_str != '\0') || (count++)) {
        if ((*parse_str == '%') || (count >= 160)) {
             if (count == 160) {
                   c_print_str("String too long for c_print function\n");
             }
             flag = 1;
             break;
         }
         parse_str++;
   }

   if (flag == 0) {
        c_print_str(fmt);
        return 0;
   }

  va_start(args,fmt);
  tmp = vsprintf(buffer, fmt, args);
  va_end(args);

  count = 0;
  do{
    UartPutc((unsigned int)buffer[count]);
    count++;
  } while(count<160 && buffer[count]!='\0');

  return tmp;
}  


/** @brief c_print_char, Fast print function to print char on UART
 *  @return void
 */

void c_print_char(const char ch)
{
    UartPutc(ch);
}


/** @brief c_print_str, Fast print function to print char on UART
 *  @return int
 */

int c_print_str(const char * fmt) {
  int i = 0;
  do{
    UartPutc(fmt[i]);
    i++;
  } while(i<79 && fmt[i]!='\0');
  
  return 1;
}


/* -----------------------------------------------------------------------------
 * Default test pass fail functions
 * -----------------------------------------------------------------------------
 */

/** @brief TEST_PASS, Terminates Test by printing test pass message
 *  @return void
 */

__attribute__((weak)) void TEST_PASS(void) {
  // Halt simulation
  printf("\n** TEST PASSED **\n");
  UartEndSimulation();
}

/** @brief TEST_FAIL, Terminates Test by printing test FAIL message
 *  @return void
 */

__attribute__((weak)) void TEST_FAIL(void) {
  // Halt simulation 
  printf("\n** TEST FAILED **\n");
  UartEndSimulation();

}

/** @brief access_addr, function to read/write num_locations from base address
 *  @return int
 */

int access_addr(unsigned long int base_address, unsigned int num_accesses)
{
  volatile unsigned int  read_data;
  volatile unsigned int  i;
  volatile unsigned int  offset;
  volatile unsigned int  write_data;
  volatile unsigned int  error_count = 0;

  offset = 0x0;
  write_data  = 0xA5A50000;

  for(i = 0; i < num_accesses; i++) {

    printf("Writing address \n");

    *(volatile unsigned long *)base_address = write_data;

    printf("Reading address \n");
    read_data =  *(volatile unsigned long *)base_address;
    if (read_data != write_data) {
      printf("Read value does not match Written value\n");
      error_count++;
    }
    else {
      printf("Read the right value\n");
    }

    offset += 0x4;
    write_data++;
  }

  return(error_count);
}

/** @brief access_addr_wdata, function to read/write num_locations from base addres with given data
 *  @return int
 */

int access_addr_wdata(unsigned long int base_address, unsigned int num_accesses, unsigned int write_data)
{
  volatile unsigned int  read_data;
  volatile unsigned int  i;
  volatile unsigned int  offset;
  volatile unsigned int  error_count = 0;

  offset = 0x0;

  for(i = 0; i < num_accesses; i++) {

    printf("WA \n");

    *(volatile unsigned long *)base_address = write_data;

    __dsb(0xf);
    __sev();

    printf("RA \n");
    read_data =  *(volatile unsigned long *)base_address;
    if (read_data != write_data) {
      printf("No MA \n");
      error_count++;
    }
    else {
      printf("MA \n");
    }

    offset += 0x4;
    write_data++;
  }

  return(error_count);
}


/** @brief access_addr_writes, function to read/write num_locations from base addres with given data
 *  @return int
 */

void access_addr_writes(unsigned long int base_address, unsigned int num_accesses, unsigned int write_data)
{
  volatile unsigned int  i;
  volatile unsigned int  offset;

  offset = 0x0;

  for(i = 0; i < num_accesses; i++) {

    printf("WA \n");

    *(volatile unsigned long *)base_address = write_data;

    offset += 0x4;
    write_data++;
  }
}   

