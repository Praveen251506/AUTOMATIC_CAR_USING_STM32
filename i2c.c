#include <FUNCTION.h>

#include <Header.h>
#include "i2c.h"
/* =========================
   I2C INITIALIZATION
   ========================= */

void i2c_init(void)
{
    /* GPIOB clock */
    RCC->AHB1ENR |= (1 << 1);

    /* I2C1 clock */
    RCC->APB1ENR |= (1 << 21);

    /* PB6 = SCL */
    GPIOB->MODER &= ~(3 << 12);
    GPIOB->MODER |=  (2 << 12);

    /* PB7 = SDA */
    GPIOB->MODER &= ~(3 << 14);
    GPIOB->MODER |=  (2 << 14);

    /* AF4 */

    GPIOB->AFRL &= ~(0xF << 24);
    GPIOB->AFRL |=  (4 << 24);

    GPIOB->AFRL &= ~(0xF << 28);
    GPIOB->AFRL |=  (4 << 28);

    /* Open drain */
    GPIOB->OTYPER |= (1 << 6);
    GPIOB->OTYPER |= (1 << 7);

    /* Pull-up */
    GPIOB->PUPDR &= ~(3 << 12);
    GPIOB->PUPDR |=  (1 << 12);

    GPIOB->PUPDR &= ~(3 << 14);
    GPIOB->PUPDR |=  (1 << 14);

    /* Reset I2C */
    I2C->CR1 |= (1 << 15);
    I2C->CR1 &= ~(1 << 15);

    /*
       16 MHz APB1

       Standard mode
       100 kHz
    */

    I2C->CR2 = 16;
    I2C->CCR = 80;
    I2C->TRISE = 17;

    /* Enable I2C */
    I2C->CR1 |= (1 << 0);
}

/* =========================
   START
   ========================= */

void I2C1_Start(void)
{
    I2C->CR1 |= (1 << 8);

    while (!(I2C->SR1 & (1 << 0)));
}

/* =========================
   ADDRESS
   ========================= */

void I2C1_WriteAddr(uint8_t address, uint8_t rw)
{
    volatile uint32_t temp;

    I2C->DR = (address << 1) | rw;

    while (!(I2C->SR1 & (1 << 1)));

    temp = I2C->SR1;
    temp = I2C->SR2;

    (void)temp;
}

/* =========================
   DATA
   ========================= */

void I2C1_WriteData(uint8_t data)
{
    while (!(I2C->SR1 & (1 << 7)));

    I2C->DR = data;

    while (!(I2C->SR1 & (1 << 2)));
}

/* =========================
   STOP
   ========================= */

void I2C1_Stop(void)
{
    I2C->CR1 |= (1 << 9);
}

/* =========================
   LCD PULSE
   ========================= */

void LCD_Pulse(uint8_t data)
{
    data |= LCD_ENABLE;

    I2C1_WriteData(data);

    delay(200);

    data &= ~LCD_ENABLE;

    I2C1_WriteData(data);

    delay(200);
}

/* =========================
   SEND NIBBLE
   ========================= */

void LCD_SendNibble(uint8_t nibble, uint8_t mode)
{
    uint8_t data;

    data = nibble & 0xF0;

    data |= LCD_BACKLIGHT;

    if (mode)
    {
        data |= LCD_RS;
    }
    else
    {
        data &= ~LCD_RS;
    }

    LCD_Pulse(data);
}

/* =========================
   SEND BYTE
   ========================= */

void LCD_SendByte(uint8_t value, uint8_t mode)
{
    LCD_SendNibble(value & 0xF0, mode);

    LCD_SendNibble((value << 4) & 0xF0, mode);
}

/* =========================
   COMMAND
   ========================= */

void LCD_Command(uint8_t cmd)
{
    I2C1_Start();

    I2C1_WriteAddr(LCD_ADDR, 0);

    LCD_SendByte(cmd, 0);

    I2C1_Stop();

    delay(500);
}

/* =========================
   DATA
   ========================= */

void LCD_Data(uint8_t data)
{
    I2C1_Start();

    I2C1_WriteAddr(LCD_ADDR, 0);

    LCD_SendByte(data, 1);

    I2C1_Stop();

    delay(100);
}

/* =========================
   LCD INITIALIZATION
   ========================= */

void LCD_Init(void)
{
    delay(20000);

    I2C1_Start();

    I2C1_WriteAddr(LCD_ADDR, 0);

    LCD_SendNibble(0x30, 0);
    delay(5000);

    LCD_SendNibble(0x30, 0);
    delay(5000);

    LCD_SendNibble(0x30, 0);
    delay(5000);

    LCD_SendNibble(0x20, 0);
    delay(5000);

    I2C1_Stop();

    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);

    delay(5000);
}

/* =========================
   CURSOR
   ========================= */

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t address;

    switch (row)
    {
        case 0:
            address = 0x80 + col;
            break;

        case 1:
            address = 0xC0 + col;
            break;

        case 2:
            address = 0x94 + col;
            break;

        case 3:
            address = 0xD4 + col;
            break;

        default:
            address = 0x80;
            break;
    }

    LCD_Command(address);
}

/* =========================
   PRINT
   ========================= */

void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_Data(*str);
        str++;
    }
}

/* =========================
   CLEAR LINE
   ========================= */

void LCD_ClearLine(uint8_t row)
{
    LCD_SetCursor(row, 0);

    LCD_Print("                    ");

    LCD_SetCursor(row, 0);
}
