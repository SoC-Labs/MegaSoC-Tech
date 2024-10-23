#include "uart_stdout.h"
#include <stdio.h>
#include "arm_neon.h"

int main(void) {
    uint32_t errors = 0;
    UartStdOutInit();

    printf("Neon Tests - SoCLabs MegaSoC\n");

    float f1 = 2.200002;
    float f2 = 2.200001;
    float ans = 1.0;
    printf("Starting SP Floating point ...\n");
    for (int i=0; i<10;i++){
        ans *= f1;
        ans *= f2;
    }
    printf("...Finshed\n");
    printf("ans = %f\n",ans);

    printf("Starting SP Vector Floating point ...\n");

    float32x4_t v1 = { 1.0, 2.0, 3.0, 4.0 }, v2 = { 1.0, 1.0, 1.0, 1.0 };
    float32x4_t sum = vaddq_f32(v1, v2);
    printf("sum = %f\n",sum[0]);

    printf("...Finshed\n");

    UartEndSimulation();
}

