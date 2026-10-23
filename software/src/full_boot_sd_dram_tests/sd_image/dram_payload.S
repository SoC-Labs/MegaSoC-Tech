//------------------------------------------------------------------------------
// DRAM payload for full_boot_sd_dram_tests (vplan MEGASOC-BOOT-006)
//
// The "demo" stage of the full boot flow: this code is stored on the SD
// card, copied into DRAM by the loader running from QSPI flash, and then
// executed from DRAM.
//
// GNU assembler syntax (aarch64-linux-gnu-as), NOT part of the armclang
// build: gen_sd_boot_image.py assembles it into the SD card image
// (sd_boot_image.hex) that the bench preloads into the card model.
//
// Position independent: only PC-relative references (adr) and no data
// outside this one .text section, so it runs at whatever DRAM address the
// loader copies it to. It has no library of its own; the loader passes a
// print function in, so the hello-world text goes through the same printf
// path as the rest of the test.
//
// Calling convention (AAPCS64), called by the loader as:
//   uint32_t entry(int (*print)(const char *, ...), boot_result_t *result);
//     x0 = print function (the loader's printf, in flash)
//     x1 = result block (in SRAM), filled in by this code:
//            +0  magic     BOOT_PAYLOAD_MAGIC
//            +4  sum       1 + 2 + ... + 100 = 5050, computed here
//            +8  entry_pc  address this code actually ran at
//   returns w0 = BOOT_PAYLOAD_MAGIC
//------------------------------------------------------------------------------

        .equ    BOOT_PAYLOAD_MAGIC_LO, 0xB007
        .equ    BOOT_PAYLOAD_MAGIC_HI, 0xD7A3

        .text
        .global _start
_start:
payload_base:                           // local alias: adr to a global symbol needs a relocation
        stp     x29, x30, [sp, #-32]!
        mov     x29, sp
        stp     x19, x20, [sp, #16]
        mov     x19, x0                 // print function
        mov     x20, x1                 // result block

        adr     x2, payload_base        // where this code is running
        str     x2, [x20, #8]

        adr     x0, hello_msg
        blr     x19

        // A small loop, so the result proves real execution from DRAM
        mov     w3, #0
        mov     w4, #100
1:      add     w3, w3, w4
        subs    w4, w4, #1
        b.ne    1b
        str     w3, [x20, #4]

        movz    w5, #BOOT_PAYLOAD_MAGIC_LO
        movk    w5, #BOOT_PAYLOAD_MAGIC_HI, lsl #16
        str     w5, [x20, #0]

        mov     w0, w5
        ldp     x19, x20, [sp, #16]
        ldp     x29, x30, [sp], #32
        ret

        .balign 8
hello_msg:
        .asciz  "Hello world from DRAM (MegaSoC full boot test)\n"
        .balign 8
