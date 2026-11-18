#include "Header.h"
#include "FUNCTION.h"
#include "adc.h"
#include "motor.h"
#include "uart.h"
#include "led.h"
#include "accel.h"


/* =========================
   LDR CONFIGURATION
   ========================= */

#define LDR_THRESHOLD 4050


/* =========================
   TILT CONFIGURATION
   ========================= */

/*
 * X-axis is assumed to be the
 * left/right tilt axis.
 *
 * Both +X and -X are treated
 * as tilt using absolute value.
 */

#define TILT_LEVEL       20
#define TILT_LOW         40
#define TILT_MEDIUM      60
#define TILT_DANGER      70


/* =========================
   PWM LIMITS
   ========================= */

#define PWM_NORMAL       300
#define PWM_HIGH_TILT    250
#define PWM_MEDIUM_TILT  200
#define PWM_LOW_TILT     100
#define PWM_DANGER       0


/* =========================
   SYSTEM STATUS
   ========================= */

volatile uint8_t system_on = 0;


/* =========================
   MAIN
   ========================= */

int main(void)
{
    char ch;

    uint16_t ldr_value;
    uint16_t accel_value;

    int8_t x;
    int8_t y;
    int8_t z;

    int16_t tilt_value;

    uint8_t speed;
    uint8_t direction;

    char lcd_text[21];

    /*
     * LCD update counter.
     * LCD should not be updated
     * on every loop.
     */
    uint16_t lcd_counter = 0;


    /* =========================
       INITIALIZATION
       ========================= */

    adc_init();

    led_init();

    motor_init();

    USART2_GPIO_Init();
    USART2_Init();

    i2c_init();
    LCD_Init();

    accel_init();


    /* =========================
       INITIAL SYSTEM STATE
       ========================= */

    system_on = 0;

    motor_stop();

    motor_set_safety_limit(PWM_NORMAL);


    /* =========================
       LCD INITIAL DISPLAY
       ========================= */

    LCD_Command(0x01);
    delay(5000);

    LCD_SetCursor(0, 0);
    LCD_Print("System: OFF        ");

    LCD_SetCursor(1, 0);
    LCD_Print("Tyre: LEVEL        ");

    LCD_SetCursor(2, 0);
    LCD_Print("Accel: 0           ");

    LCD_SetCursor(3, 0);
    LCD_Print("STOP Speed:  0%    ");


    /* =========================
       UART STARTUP MESSAGE
       ========================= */

    USART2_SendString("CAR READY\r\n");
    USART2_SendString("SYSTEM OFF\r\n");


    /* =========================
       MAIN LOOP
       ========================= */

    while (1)
    {
        /* =========================
           UART COMMAND
           ========================= */

        /*
         * Read ALL characters that
         * have arrived.
         *
         * This is important because
         * Bluetooth commands can arrive
         * while LCD / sensor processing
         * is happening.
         */

        while (USART2_ReceiveChar(&ch))
        {
            uart_command_handler(ch);
        }


        /* =========================
           READ LDR
           ========================= */

        ldr_value = adc_read();


        /* =========================
           READ ACCELEROMETER
           ========================= */

        accel_read_xyz(&x, &y, &z);


        /* =========================
           TOTAL ACCELERATION
           ========================= */

        /*
         * Accel =
         *
         * sqrt(X² + Y² + Z²)
         *
         * This value is displayed
         * on the LCD.
         */

        accel_value = accel_motion_magnitude(x, y, z);


        /* =========================
           CALCULATE TILT
           ========================= */

        /*
         * X is assumed to be the
         * left/right tilt axis.
         *
         * +X = one side
         * -X = opposite side
         *
         * Absolute value makes both
         * directions behave equally.
         */

        tilt_value = x;

        if (tilt_value < 0)
        {
            tilt_value = -tilt_value;
        }


        /* =========================
           TILT BASED SPEED CONTROL
           ========================= */

        if (tilt_value <= TILT_LEVEL)
        {
            /*
             * Vehicle is approximately
             * flat.
             */

            motor_set_safety_limit(PWM_NORMAL);
        }

        else if (tilt_value <= TILT_LOW)
        {
            /*
             * Small tilt.
             */

            motor_set_safety_limit(PWM_HIGH_TILT);
        }

        else if (tilt_value <= TILT_MEDIUM)
        {
            /*
             * Medium tilt.
             */

            motor_set_safety_limit(PWM_MEDIUM_TILT);
        }

        else if (tilt_value <= TILT_DANGER)
        {
            /*
             * High tilt.
             */

            motor_set_safety_limit(PWM_LOW_TILT);
        }

        else
        {
            /*
             * Dangerous tilt.
             *
             * Motor gradually slows
             * down to zero.
             */

            motor_set_safety_limit(PWM_DANGER);
        }


        /* =========================
           MOTOR UPDATE
           ========================= */

        motor_update_speed();


        /* =========================
           LED CONTROL
           ========================= */

        /*
         * LEFT TURN
         *
         * LDR is ignored.
         */

        if (motor_get_direction() == MOTOR_LEFT)
        {
            led_left();
        }

        /*
         * RIGHT TURN
         *
         * LDR is ignored.
         */

        else if (motor_get_direction() == MOTOR_RIGHT)
        {
            led_right();
        }

        /*
         * STOPPED / FORWARD / BACKWARD
         *
         * LDR controls the LEDs.
         */

        else
        {
            if (ldr_value >= LDR_THRESHOLD)
            {
                led_on();
            }
            else
            {
                led_off();
            }
        }


        /* =========================
           LCD UPDATE
           ========================= */

        lcd_counter++;

        /*
         * Do not update LCD every
         * iteration.
         *
         * This gives UART and motor
         * control priority.
         */

        if (lcd_counter >= 1000)
        {
            lcd_counter = 0;


            /* =========================
               GET MOTOR STATUS
               ========================= */

            speed = motor_get_speed_percent();

            direction = motor_get_direction();


            /* =========================
               LCD LINE 1
               ========================= */

            LCD_SetCursor(0, 0);

            if (system_on)
            {
                LCD_Print("System: ON         ");
            }
            else
            {
                LCD_Print("System: OFF        ");
            }


            /* =========================
               LCD LINE 2
               ========================= */

            LCD_SetCursor(1, 0);

            if (tilt_value <= TILT_LEVEL)
            {
                LCD_Print("Tyre: LEVEL        ");
            }

            else if (tilt_value <= TILT_MEDIUM)
            {
                LCD_Print("Tyre: TILTED       ");
            }

            else if (tilt_value <= TILT_DANGER)
            {
                LCD_Print("Tyre: HIGH TILT    ");
            }

            else
            {
                LCD_Print("Tyre: DANGER       ");
            }


            /* =========================
               LCD LINE 3
               ========================= */

            LCD_SetCursor(2, 0);

            sprintf(lcd_text,
                    "Accel: %3d         ",
                    accel_value);

            LCD_Print(lcd_text);


            /* =========================
               LCD LINE 4
               ========================= */

            LCD_SetCursor(3, 0);

            if (direction == MOTOR_FORWARD)
            {
                sprintf(lcd_text,
                        "FWD Speed:%3d%%",
                        speed);
            }

            else if (direction == MOTOR_BACKWARD)
            {
                sprintf(lcd_text,
                        "BWD Speed:%3d%%",
                        speed);
            }

            else if (direction == MOTOR_LEFT)
            {
                sprintf(lcd_text,
                        "LEFT Speed:%2d%%",
                        speed);
            }

            else if (direction == MOTOR_RIGHT)
            {
                sprintf(lcd_text,
                        "RIGHT Speed:%1d%%",
                        speed);
            }

            else
            {
                sprintf(lcd_text,
                        "STOP Speed:%2d%%",
                        speed);
            }

            LCD_Print(lcd_text);
        }
    }
}
