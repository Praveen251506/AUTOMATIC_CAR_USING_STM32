#include "Header.h"
#include "FUNCTION.h"
#include "accel.h"

/* =========================================================
   SPI1 INITIALIZATION
   ========================================================= */

void spi1_init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= (1 << 0);

    /* Enable GPIOE clock */
    RCC->AHB1ENR |= (1 << 4);

    /* Enable SPI1 clock */
    RCC->APB2ENR |= (1 << 12);


    /* =====================================================
       PA5 -> SPI1_SCK
       ===================================================== */

    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |=  (2 << 10);


    /* =====================================================
       PA6 -> SPI1_MISO
       ===================================================== */

    GPIOA->MODER &= ~(3 << 12);
    GPIOA->MODER |=  (2 << 12);


    /* =====================================================
       PA7 -> SPI1_MOSI
       ===================================================== */

    GPIOA->MODER &= ~(3 << 14);
    GPIOA->MODER |=  (2 << 14);


    /* =====================================================
       Alternate Function 5
       ===================================================== */

    GPIOA->AFRL &= ~(0xF << 20);
    GPIOA->AFRL |=  (5 << 20);

    GPIOA->AFRL &= ~(0xF << 24);
    GPIOA->AFRL |=  (5 << 24);

    GPIOA->AFRL &= ~(0xF << 28);
    GPIOA->AFRL |=  (5 << 28);


    /* =====================================================
       PE3 -> Chip Select
       ===================================================== */

    GPIOE->MODER &= ~(3 << 6);
    GPIOE->MODER |=  (1 << 6);

    /* Push-pull */
    GPIOE->OTYPER &= ~(1 << ACC_CS);

    /* No pull-up / pull-down */
    GPIOE->PUPDR &= ~(3 << 6);

    /* CS inactive = HIGH */
    GPIOE->ODR |= (1 << ACC_CS);


    /* =====================================================
       SPI CONFIGURATION
       ===================================================== */

    SPI1->CR1 = 0;
    SPI1->CR2 = 0;


    /* Master mode */
    SPI1->CR1 |= (1 << 2);


    /* Software slave management */
    SPI1->CR1 |= (1 << 8);
    SPI1->CR1 |= (1 << 9);


    /* Baud rate = PCLK2 / 16 */
    SPI1->CR1 |= (3 << 3);


    /* =====================================================
       SPI MODE 3
       CPOL = 1
       CPHA = 1
       ===================================================== */

    SPI1->CR1 |= (1 << 1);
    SPI1->CR1 |= (1 << 0);


    /* Enable SPI */
    SPI1->CR1 |= (1 << 6);
}


/* =========================================================
   SPI TRANSFER
   ========================================================= */

uint8_t spi1_transfer(uint8_t data)
{
    uint8_t received;

    /* Wait until transmit buffer is empty */
    while (!(SPI1->SR & (1 << 1)))
    {
    }

    /* Send data */
    SPI1->DR = data;

    /* Wait until transfer is complete */
    while (!(SPI1->SR & (1 << 0)))
    {
    }

    /* Read received data */
    received = (uint8_t)SPI1->DR;

    return received;
}


/* =========================================================
   WRITE ACCELEROMETER REGISTER
   ========================================================= */

void accel_write_reg(uint8_t reg, uint8_t data)
{
    /* CS LOW -> select accelerometer */
    GPIOE->ODR &= ~(1 << ACC_CS);


    /* Write operation */
    spi1_transfer(reg & 0x7F);

    /* Send register data */
    spi1_transfer(data);


    /* CS HIGH -> deselect accelerometer */
    GPIOE->ODR |= (1 << ACC_CS);
}


/* =========================================================
   READ ACCELEROMETER REGISTER
   ========================================================= */

uint8_t accel_read_reg(uint8_t reg)
{
    uint8_t data;

    /* CS LOW */
    GPIOE->ODR &= ~(1 << ACC_CS);


    /* Read command */
    spi1_transfer(reg | 0x80);


    /* Dummy byte to generate clock */
    data = spi1_transfer(0x00);


    /* CS HIGH */
    GPIOE->ODR |= (1 << ACC_CS);


    return data;
}


/* =========================================================
   ACCELEROMETER INITIALIZATION
   ========================================================= */

void accel_init(void)
{
    /* Initialize SPI1 */
    spi1_init();

    delay(10000);


    /*
       CTRL_REG1 = 0x47

       0x40 -> 100 Hz
       0x07 -> X, Y, Z enabled

       Sensor active
    */

    accel_write_reg(LIS302DL_CTRL_REG1, 0x47);

    delay(10000);


    /* Reset software status */
    accel_reset_status();
}


/* =========================================================
   READ X Y Z
   ========================================================= */

void accel_read_xyz(int8_t *x,
                    int8_t *y,
                    int8_t *z)
{
    *x = (int8_t)accel_read_reg(LIS302DL_OUT_X);

    *y = (int8_t)accel_read_reg(LIS302DL_OUT_Y);

    *z = (int8_t)accel_read_reg(LIS302DL_OUT_Z);
}


/* =========================================================
   WHO AM I
   ========================================================= */

uint8_t accel_who_am_i(void)
{
    return accel_read_reg(LIS302DL_WHO_AM_I);
}


/* =========================================================
   ACCELERATION MAGNITUDE
   ========================================================= */

uint16_t accel_motion_magnitude(int8_t x,
                                int8_t y,
                                int8_t z)
{
    int16_t x2;
    int16_t y2;
    int16_t z2;

    uint32_t sum;

    uint16_t root;
    uint16_t i;


    /* Square X */
    x2 = (int16_t)x * x;

    /* Square Y */
    y2 = (int16_t)y * y;

    /* Square Z */
    z2 = (int16_t)z * z;


    /* X² + Y² + Z² */
    sum = (uint32_t)x2 +
          (uint32_t)y2 +
          (uint32_t)z2;


    /* Approximate square root */
    root = 0;

    for (i = 0; i <= 255; i++)
    {
        if (((uint32_t)i * i) > sum)
        {
            break;
        }

        root = i;
    }


    return root;
}


/* =========================================================
   READ TOTAL ACCELERATION
   ========================================================= */

uint16_t accel_read_total(void)
{
    int8_t x;
    int8_t y;
    int8_t z;


    accel_read_xyz(&x, &y, &z);


    return accel_motion_magnitude(x, y, z);
}


/* =========================================================
   MOTION STATUS
   ========================================================= */

uint8_t accel_motion_status(int8_t x,
                            int8_t y,
                            int8_t z)
{
    uint16_t magnitude;


    magnitude = accel_motion_magnitude(x, y, z);


    if (magnitude >= ACCEL_MOTION_WARNING)
    {
        return 1;
    }


    return 0;
}


/* =========================================================
   TILT STATUS
   ========================================================= */

uint8_t accel_tilt_status(int8_t x, int8_t y)
{
    int16_t abs_x;
    int16_t abs_y;

    /* Convert X to absolute value */

    abs_x = x;

    if (abs_x < 0)
    {
        abs_x = -abs_x;
    }


    /* Convert Y to absolute value */

    abs_y = y;

    if (abs_y < 0)
    {
        abs_y = -abs_y;
    }


    /*
     * Check dangerous tilt first.
     *
     * Either direction is dangerous.
     */

    if ((abs_x >= ACCEL_TILT_DANGER) ||
        (abs_y >= ACCEL_TILT_DANGER))
    {
        return 2;
    }


    /*
     * Check warning tilt.
     *
     * Either direction gives warning.
     */

    if ((abs_x >= ACCEL_TILT_WARNING) ||
        (abs_y >= ACCEL_TILT_WARNING))
    {
        return 1;
    }


    /*
     * Car is approximately level.
     */

    return 0;
}


/* =========================================================
   RESET STATUS
   ========================================================= */

void accel_reset_status(void)
{
    /*
       Currently no hardware reset is required.

       This function is kept so that the application
       can reset software safety status if required.
    */
}
