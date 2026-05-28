#include <lpc214x.h>
#include "delay.h"

void Delay_us(unsigned int count) {
    // Basic software loop execution calibrated for standard LPC2148 CCLK configurations
    register unsigned int i;
    for (i = 0; i < (count * 12); i++) {
        __asm { nop }
    }
}

void Delay_ms(unsigned int count) {
    unsigned int i;
    for (i = 0; i < count; i++) {
        Delay_us(1000);
    }
}
