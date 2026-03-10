#include "uart_stdout.h"
#include <stdio.h>
#include "system.h"


#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW16_REG(ADDRESS)  (*((volatile unsigned short *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))


int sram_test(unsigned long int base_address);

int main(void) {
    uint32_t errors = 0;
    UartStdOutInit();

    printf("Mem Tests - SoCLabs MegaSoC\n");

    printf(" - First 128KB\n");
    errors += sram_test(0x00800000);
    errors += sram_test(0x00802000);
    errors += sram_test(0x00804000);
    errors += sram_test(0x00808000);
    errors += sram_test(0x00810000);
    errors += sram_test(0x00812000);
    errors += sram_test(0x00814000);
    errors += sram_test(0x00818000);
    errors += sram_test(0x0081F000);

    if(errors!=0){
        TEST_FAIL();
    } 

    printf(" - Second 128KB\n");
    errors += sram_test(0x00820000);
    errors += sram_test(0x00822000);
    errors += sram_test(0x00824000);
    errors += sram_test(0x00828000);
    errors += sram_test(0x0082F000);
    errors += sram_test(0x00830000);
    errors += sram_test(0x00832000);
    errors += sram_test(0x00834000);
    errors += sram_test(0x00838000);
    errors += sram_test(0x0083F000);

    if(errors!=0){
        TEST_FAIL();
    } 

    printf(" - Third 128KB\n");
    errors += sram_test(0x00840000);
    errors += sram_test(0x00842000);
    errors += sram_test(0x00844000);
    errors += sram_test(0x00848000);
    errors += sram_test(0x0084F000);
    errors += sram_test(0x00850000);
    errors += sram_test(0x00852000);
    errors += sram_test(0x00854000);
    errors += sram_test(0x00858000);
    errors += sram_test(0x0085F000);

    if(errors!=0){
        TEST_FAIL();
    } 

    printf(" - Final 128KB\n");
    errors += sram_test(0x00860000);
    errors += sram_test(0x00862000);
    errors += sram_test(0x00864000);
    errors += sram_test(0x00868000);
    errors += sram_test(0x0086F000);
    errors += sram_test(0x00870000);
    errors += sram_test(0x00872000);
    errors += sram_test(0x00874000);
    errors += sram_test(0x00878000);
    errors += sram_test(0x0087F000);

    if(errors!=0){
        TEST_FAIL();
    } else {
        TEST_PASS();
    }
    UartEndSimulation();
}

/**************************************************/
/*          SRAM Test                             */
/**************************************************/

int sram_test(unsigned long int base_address){
    int result=0;

    // Test write 64 bits
    *(uint64_t *) base_address = 0x0123456789ABCDEF;
    __dsb(0xf);
    __sev();
    if(*(uint64_t *) base_address!=0x0123456789ABCDEF) { result++; }

    // Test write 32 bits
    *(uint32_t *) base_address = 0xA5A5A5A5;
    __dsb(0xf);
    __sev();

    // read 32 bits
    if(*(uint32_t *) base_address!=0xA5A5A5A5) { result++; }
    __dsb(0xf);
    __sev();
    // read 64 bits
    if(*(uint64_t *) base_address!=0x01234567A5A5A5A5) { result++; }

    // Test write 16 bits
    *(uint16_t *) base_address = 0x3210;
    __dsb(0xf);
    __sev();

    // read 16 bits
    if(*(uint16_t *) base_address!=0x3210) { result++; }
    __dsb(0xf);
    __sev();

    // read 32 bits
    if(*(uint32_t *) base_address!=0xA5A53210) { result++; }
    __dsb(0xf);
    __sev();
    // read 64 bits
    if(*(uint64_t *) base_address!=0x01234567A5A53210) { result++; }

    // Test write 8 bits
    *(uint8_t *) base_address = 0xFE;
    __dsb(0xf);
    __sev();

    // read 8 bits
    if(*(uint8_t *) base_address!=0xFE) { result++; }
    __dsb(0xf);
    __sev();


    // read 16 bits
    if(*(uint16_t *) base_address!=0x32FE) { result++; }
    __dsb(0xf);
    __sev();

    // read 32 bits
    if(*(uint32_t *) base_address!=0xA5A532FE) { result++; }
    __dsb(0xf);
    __sev();
    // read 64 bits
    if(*(uint64_t *) base_address!=0x01234567A5A532FE) { result++; }

    return result;
}

