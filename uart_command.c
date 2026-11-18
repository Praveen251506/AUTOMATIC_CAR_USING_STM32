#include "Header.h"
#include "FUNCTION.h"
#include "uart.h"
#include "motor.h"

extern volatile uint8_t system_on;


void uart_command_handler(char value)
{
    switch (value)
    {
        /* =========================
           SYSTEM ON
           ========================= */

        case 'S':
        case 's':

            system_on = 1;

            motor_stop();

            LCD_Command(0x01);
            delay(5000);

            LCD_SetCursor(0, 0);
            LCD_Print("System: ON");

            LCD_SetCursor(1, 0);
            LCD_Print("Dir: STOP");

            LCD_SetCursor(2, 0);
            LCD_Print("Speed: 0%");

            LCD_SetCursor(3, 0);
            LCD_Print("Accel: READY");

            USART2_SendString("SYSTEM ON\r\n");

            break;


        /* =========================
           SYSTEM OFF
           ========================= */

        case 'D':
        case 'd':

            system_on = 0;

            /*
             * motor_stop() requests
             * gradual deceleration.
             */
            motor_stop();

            /*
             * Force PWM immediately to zero
             * for system OFF.
             */
            TIM2->CCR1 = 0;
            TIM2->CCR2 = 0;

            /*
             * Force motor direction pins LOW.
             */
            GPIOB->ODR &= ~(1 << MOTOR_IN1);
            GPIOB->ODR &= ~(1 << MOTOR_IN2);
            GPIOB->ODR &= ~(1 << MOTOR_IN3);
            GPIOB->ODR &= ~(1 << MOTOR_IN4);

            LCD_Command(0x01);
            delay(5000);

            LCD_SetCursor(0, 0);
            LCD_Print("System: OFF");

            LCD_SetCursor(1, 0);
            LCD_Print("Dir: STOP");

            LCD_SetCursor(2, 0);
            LCD_Print("Speed: 0%");

            LCD_SetCursor(3, 0);
            LCD_Print("Vehicle Standby");

            USART2_SendString("SYSTEM OFF\r\n");

            break;


        /* =========================
           FORWARD
           ========================= */

        case '2':

            if (system_on)
            {
                motor_forward();

                LCD_Command(0x01);

                LCD_SetCursor(0, 0);
                LCD_Print("System: ON");

                LCD_SetCursor(1, 0);
                LCD_Print("Dir: FORWARD");

                USART2_SendString("FORWARD\r\n");
            }
            else
            {
                USART2_SendString("SYSTEM OFF\r\n");
            }

            break;


        /* =========================
           BACKWARD
           ========================= */

        case '4':

            if (system_on)
            {
                motor_backward();

                LCD_Command(0x01);

                LCD_SetCursor(0, 0);
                LCD_Print("System: ON");

                LCD_SetCursor(1, 0);
                LCD_Print("Dir: BACKWARD");

                USART2_SendString("BACKWARD\r\n");
            }
            else
            {
                USART2_SendString("SYSTEM OFF\r\n");
            }

            break;


        /* =========================
           RIGHT
           ========================= */

        case '1':

            if (system_on)
            {
                motor_right();

                LCD_Command(0x01);

                LCD_SetCursor(0, 0);
                LCD_Print("System: ON");

                LCD_SetCursor(1, 0);
                LCD_Print("Dir: RIGHT");

                USART2_SendString("RIGHT\r\n");
            }
            else
            {
                USART2_SendString("SYSTEM OFF\r\n");
            }

            break;


        /* =========================
           LEFT
           ========================= */

        case '3':

            if (system_on)
            {
                motor_left();

                LCD_Command(0x01);

                LCD_SetCursor(0, 0);
                LCD_Print("System: ON");

                LCD_SetCursor(1, 0);
                LCD_Print("Dir: LEFT");

                USART2_SendString("LEFT\r\n");
            }
            else
            {
                USART2_SendString("SYSTEM OFF\r\n");
            }

            break;


        /* =========================
           STOP
           ========================= */

        case '0':

            motor_stop();

            LCD_Command(0x01);

            LCD_SetCursor(0, 0);

            if (system_on)
                LCD_Print("System: ON");
            else
                LCD_Print("System: OFF");

            LCD_SetCursor(1, 0);
            LCD_Print("Dir: STOP");

            USART2_SendString("STOP\r\n");

            break;


        /* =========================
           ENTER / NEWLINE
           ========================= */

        case '\r':
        case '\n':

            break;


        /* =========================
           INVALID
           ========================= */

        default:

            USART2_SendString("INVALID COMMAND\r\n");

            break;
    }
}
