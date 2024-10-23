/*------------------------------------------------------------------------------
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
*/

MEMORY
{
  SSE710_SEC_ROM     (rx)  : ORIGIN = 0x00000000, LENGTH = 16K
  SSE710_SEC_SRAM    (rwx) : ORIGIN = 0x00004000, LENGTH = 4096K
}

GROUP(libgcc.a libc.a libm.a libnosys.a)

SECTIONS
{
  .SEC_ROM_BOOT_IMG :
  {
    *boot.s.v8-a.o(SECURE_ROM_BOOT)
    FILL(0xDEADDEAD);
    . = ORIGIN(SSE710_SEC_ROM) + LENGTH(SSE710_SEC_ROM);
  } > SSE710_SEC_ROM

  .LOAD_MAIN :
  {
    *(.text*)

    KEEP(*(.init))
    KEEP(*(.fini))

    /* .ctors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    /* .dtors */
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)

    *(.rodata*)

    KEEP(*(.eh_frame*))

    __data_start__ = .;
    *(.data*)

    . = ALIGN(4);
    /* preinit data */
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);


    . = ALIGN(4);
    /* finit data */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);

    KEEP(*(.jcr*))
    . = ALIGN(4);
    /* All data end */
    __data_end__ = .;

  } > SSE710_SEC_SRAM

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > SSE710_SEC_SRAM

  __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > SSE710_SEC_SRAM
  __exidx_end = .;

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > SSE710_SEC_SRAM

  .PAGE_TABLE_SECTIONS 0x02100000 :
  {
    *page_table.s.v8-a.o(*)
  } > SSE710_SEC_SRAM

  /* Notes section
   * This is not used so we discard it. Although not used it needs to be
   * explicitly mentioned in the linker script as some toolchains will place
   * the notes section at address 0 if it is not explicitly mentioned. */
  /DISCARD/ : { *(.note*) }

  .heap 0x02310000 (COPY):
  {
    __end__ = .;
    end = __end__;
    *(.heap*)
    __HeapLimit = .;
  } > SSE710_SEC_SRAM
 
  /* .stack_dummy section doesn't contains any symbols. It is only
   * used for linker to calculate size of stack sections, and assign
   * values to stack symbols later */
  .stack_dummy 0x023c0000 (COPY):
  {
    *(.stack*)
  } > SSE710_SEC_SRAM
 
  /* Set stack top to end of RAM, and stack limit move down by
   * size of stack_dummy section */
  __StackTop = ORIGIN(SSE710_SEC_SRAM) + LENGTH(SSE710_SEC_SRAM);
  __StackLimit = __StackTop - SIZEOF(.stack_dummy);
  PROVIDE(__stack = __StackTop);
 
  /* Check if data + heap + stack exceeds RAM limit */
  ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}

