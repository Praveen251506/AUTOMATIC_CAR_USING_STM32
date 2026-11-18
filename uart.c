#include "Header.h"
#include "FUNCTION.h"
#include "uart.h"


void USART2_GPIO_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= (1 << 0);

    /* PA2 = TX
       PA3 = RX
       Alternate Function mode */
    GPIOA->MODER &= ~((3 << 4) | (3 << 6));
    GPIOA->MODER |=  ((2 << 4) | (2 << 6));

    /* Push-pull */
    GPIOA->OTYPER &= ~((1 << 2) | (1 << 3));

    /* High speed */
    GPIOA->OSPEEDR |= ((3 << 4) | (3 << 6));

    /* AF7 = USART2 */
    GPIOA->AFRL &= ~((0xF << 8) | (0xF << 12));
    GPIOA->AFRL |=  ((7 << 8) | (7 << 12));
}


void USART2_Init(void)
{
    /* Enable USART2 clock */
    RCC->APB1ENR |= (1 << 17);

    /* Disable USART */
    UART->CR1 &= ~(1 << 13);

    /* 16 MHz clock
       9600 baud */
    UART->BRR = 0x683;

    /* Enable TX */
    UART->CR1 |= (1 << 3);

    /* Enable RX */
    UART->CR1 |= (1 << 2);

    /* Enable USART */
    UART->CR1 |= (1 << 13);
}


void USART2_SendChar(char ch)
{
    /* Wait until transmit data register is empty */
    while (!(UART->SR & (1 << 7)));

    UART->DR = (uint8_t)ch;
}


void USART2_SendString(char *str)
{
    while (*str)
    {
        USART2_SendChar(*str);
        str++;
    }
}


uint8_t USART2_ReceiveChar(char *ch)
{
    /* RXNE = bit 5 */
    if (UART->SR & (1 << 5))
    {
        *ch = (char)UART->DR;
        return 1;

    }
    delay(1000);
    return 0;
}
