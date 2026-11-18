#include "Header.h"
#include "FUNCTION.h"
#include "adc.h"

void adc_init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= (1 << 0);

    /* PA4 -> Analog mode */
    GPIOA->MODER &= ~(3 << 8);
    GPIOA->MODER |=  (3 << 8);

    /* No pull-up / pull-down */
    GPIOA->PUPDR &= ~(3 << 8);

    /* Enable ADC1 clock */
    RCC->APB2ENR |= (1 << 8);

    /* ADC reset configuration */
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    /* One conversion */
    ADC1->SQR1 = 0;

    /* Channel 4 */
    ADC1->SQR3 = 4;

    /* Maximum sampling time */
    ADC1->SMPR2 &= ~(7 << 12);
    ADC1->SMPR2 |=  (7 << 12);

    /* Enable ADC */
    ADC1->CR2 |= (1 << 0);

    delay(1000);
}


uint16_t adc_read(void)
{
    /* Start conversion */
    ADC1->CR2 |= (1 << 30);

    /* Wait for conversion */
    while (!(ADC1->SR & (1 << 1)));

    /* Return ADC value */
    return (uint16_t)ADC1->DR;
}
