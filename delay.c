#include "delay.h"

void delay(volatile int i)
{
    while(i--)
    {
        __asm volatile("nop");
    }
}
