#include "uart_stdout.h"
#include <stdio.h>
#include "arm_neon.h"
#include "system.h"

int main(void) {
    uint32_t errors = 0;
    float32x4_t v1 = { 1.0, 2.0, 3.0, 4.0 }, v2 = { 4.0, 3.0, 2.0, 1.0 };

    float32x4_t sum;
    float32x4_t div;
    float32x4_t mul;

    UartStdOutInit();

    printf("Neon Tests - SoCLabs MegaSoC\n");

    printf("Neon Sum\n");
    sum = vaddq_f32(v1, v2);
    for (int i=0; i<4; i++){
        if(sum[i] != 5.0){errors++;}
    }
    if(errors!=0){TEST_FAIL();}

    printf("Neon Div\n");
    div = vdivq_f32(v1,v2);
    if(div[0]!=0.25){errors++;}
    if(div[2]!=1.5){errors++;}
    if(div[3]!=4.0){errors++;}
    if(errors!=0){
        printf("div[0] = %f\n", div[0]);
        printf("div[1] = %f\n", div[1]);
        printf("div[2] = %f\n", div[2]);
        printf("div[3] = %f\n", div[3]);
        TEST_FAIL();
    }

    printf("Neon Multiply\n");
    mul = vmulq_f32(v1,v2);
    if(mul[0]!=4.0){errors++;}
    if(mul[1]!=6.0){errors++;}
    if(mul[2]!=6.0){errors++;}
    if(mul[3]!=4.0){errors++;}

    if(errors!=0){TEST_FAIL();}
    else{TEST_PASS();}
}

