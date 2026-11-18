#include "Header.h"
#include "led.h"

void led_init(void)
{
    RCC->AHB1ENR |= (1 << 3);

    /* PD13 -> LEFT LED */
    GPIOD->MODER &= ~(3 << 26);
    GPIOD->MODER |=  (1 << 26);

    /* PD15 -> RIGHT LED */
    GPIOD->MODER &= ~(3 << 30);
    GPIOD->MODER |=  (1 << 30);

    GPIOD->OTYPER &= ~(1 << 13);
    GPIOD->OTYPER &= ~(1 << 15);


}


void led_left(void)
{
    GPIOD->ODR |=  (1 << 13);
    GPIOD->ODR &= ~(1 << 15);
}


void led_on(void)
{
    GPIOD->ODR |= (1 << 13);
    GPIOD->ODR |=(1<<15);
}


void led_right(void)
{
    GPIOD->ODR &= ~(1 << 13);
    GPIOD->ODR |=  (1 << 15);
}


void led_off(void)
{
    GPIOD->ODR &= ~(1 << 13);
    GPIOD->ODR &= ~(1 << 15);
}
